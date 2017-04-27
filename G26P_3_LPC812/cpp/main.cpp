#include "hardware.h"
#include "time.h"
#include "ComPort.h"
#include "crc16.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 fps = 0;

static ComPort com;


typedef bool (*REQF)(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb);



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request01(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	// Запуск импульса излучателя
	
	byte t = ((byte*)rb->data)[1] - 1;
	
	if (t < 3) 
	{
		WaitFireSync(t);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request02(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	// Чтение текущего значения высокого напряжения

	__packed struct Rsp { byte f; u16 hv; u16 crc; };

	static Rsp rsp;
	
	if (GetCRC(rb->data, rb->len) != 0) return false;

	rsp.f = 2;
	rsp.hv = GetCurHV();
	rsp.crc = GetCRC(&rsp, 3);
	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request03(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	// Установка значения требуемого высокого напряжения

	__packed struct Req { byte f; byte fireCount; u16 hv; u16 crc; };
	__packed struct Rsp { byte f; u16 crc; };

	static Rsp rsp;
	
	Req *req = (Req*)rb->data;
	
	if (GetCRC(rb->data, rb->len) != 0) return false;

//	SetReqFireCount(req->fireCount);
	SetReqHV(req->hv);

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
	byte f = *(byte*)rb->data;

	f -= 1;

	if (f > 2) return false;

	return listReq[f](wb, rb);
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
				com.Read(&rb, -1, 800);
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

	com.Connect(0, 1562500, 0);
	
	while (1)
	{
		//if (j >= 1000)
		//{
		//	HW::GPIO->SET0 = 1<<7;
		//	j = 0;
		//}
		//else if (j == 499)
		//{
		//	HW::GPIO->CLR0 = 1<<7;
		//};

		//if (tm.Check(200))
		//{
		//	WaitFireSync(i++);

		//	HW::SCT->CTRL_L = (HW::SCT->CTRL_L & ~(3<<1)) | (1<<3);

		//	if (i > 2) { i = 0; };
		//};

		UpdateHardware();
		UpdateCom();

		j++;
	};
}
