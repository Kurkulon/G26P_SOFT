#include "hardware.h"
#include "time.h"
#include "ComPort.h"
#include "crc16.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 fps = 0;

static ComPort com(USART_USIC_NUM, ~0, PIN_UTX, PIN_URX, PIN_RTS);

union Req
{
	struct  { u16 len; u16 version; u16 func; u16 n;																									u16 crc; } F1;  // старт излучателя
	struct  { u16 len; u16 version; u16 func;																											u16 crc; } F2;  // чтение параметров
	struct  { u16 len; u16 version; u16 func; u16 fireCountM; u16 fireCountXY; u16 hv; u16 fireFreqM; u16 fireFreqXY; u16 fireDutyM; u16 fireDutyXY;	u16 crc; } F3;  // установка шага и длины оцифровки вектора
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union Rsp
{
	struct  { u16 len; u16 version; u16 func; u16 numDev; u16 hv; i16 temp;																				u16 crc; } F2;  // чтение вектора
	struct  { u16 len; u16 version; u16 func; u16 fireCountM; u16 fireCountXY; u16 hv; u16 fireFreqM; u16 fireFreqXY; u16 fireDutyM; u16 fireDutyXY;	u16 crc; } F3;  // установка шага и длины оцифровки вектора
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef bool (*REQF)(Req *req, ComPort::WriteBuffer *wb);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request01(Req *req, ComPort::WriteBuffer *wb)
{
	// Запуск импульса излучателя
	
	byte t = req->F1.n - 1;
	
	if (t < 3) 
	{
		WaitFireSync(t);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request02(Req *req, ComPort::WriteBuffer *wb)
{
	// Чтение текущего значения высокого напряжения

	__packed struct Rsp { byte f; u16 hv; u16 crc; };

	static Rsp rsp;
	
	rsp.f = 2;
	rsp.hv = GetCurHV();
	rsp.crc = GetCRC(&rsp, 3);
	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request03(Req *req, ComPort::WriteBuffer *wb)
{
	// Установка значения требуемого высокого напряжения

	__packed struct Rsp { byte f; u16 crc; };

	static Rsp rsp;
	
	SetReqFireCountM(req->F3.fireCountM);
	SetReqFireCountXY(req->F3.fireCountXY);
	SetReqHV(req->F3.hv);
	SetReqFireFreqM(req->F3.fireFreqM, req->F3.fireDutyM);
	SetReqFireFreqXY(req->F3.fireFreqXY, req->F3.fireDutyXY);

	rsp.f = 3;
	rsp.crc = GetCRC(&rsp, 1);
	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static REQF listReq[3] = { Request01, Request02, Request03 };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool UpdateRequest(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	static Req nulReq;

	static const byte fl[3] = { sizeof(nulReq.F1)+1, sizeof(nulReq.F2)+1, sizeof(nulReq.F3)+1 };

	if (rb == 0 || rb->len < 4) return false;

	bool result = false;

	u16 rlen = rb->len;

	byte *p = (byte*)rb->data;

	while(rlen > 3)
	{
		byte len = p[0];
		byte func = p[1]-1;

		if (func < 4 && len == fl[func] && len < rlen && GetCRC16(p+1, len) == 0)
		{
			Req *req = (Req*)p;

			result = listReq[func](req, wb);

			break;
		}
		else
		{
			p += 1;
			rlen -= 1;
		};
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static byte buf[256];

	static byte i = 0;

	switch(i)
	{ 
		case 0:

			if (!IsSyncActive())
			{
				rb.data = buf;
				rb.maxLen = sizeof(buf);
				com.Read(&rb, ~0, US2COM(32));
				i++;
			};

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && UpdateRequest(&wb, &rb))
				{
					com.Write(&wb);
					i++;
				}
				else
				{
					i = 0;
				};
			};

			break;

		case 2:

			if (!com.Update())
			{
				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	byte i = 0;
	u32 j = 0;

	TM32	tm;

	InitHardware();

	com.Connect(ComPort::ASYNC, 1562500, 0);
	
	while (1)
	{
		UpdateHardware();
		UpdateCom();

		j++;
	};
}
