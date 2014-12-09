#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"


static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd[3][2][512*2];

//static u16 spd2[512*2];
//
//static i16 ch1[512];
//static i16 ch2[512];
//static i16 ch3[512];
//static i16 ch4[512];

static bool ready1 = false, ready2 = false;

//static u32 CRCOK = 0;
//static u32 CRCER = 0;

static byte sampleTime[3] = { 19, 19, 9};
static byte gain[3] = { 7, 7, 7 };
static byte netAdr = 1;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Request
{
	byte adr;
	byte func;
	
	union
	{
		struct  { byte n; word crc; } f1;  // старт оцифровки
		struct  { byte n; byte chnl; word crc; } f2;  // чтение вектора
		struct  { byte dt[3]; byte ka[3]; word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Response
{
	byte adr;
	byte func;
	
	union
	{
		struct  { word crc; } f1;  // старт оцифровки
		struct  { byte n; byte chnl; u16 data1[500]; u16 data2[500]; word crc; } f2;  // чтение вектора
		struct  { word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc01(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	static Response rsp;

	if (req->adr != 0 && req->adr != netAdr) return false;

	byte n = req->f1.n;
	if (n > 2) n = 2;

	SetGain(gain[n]);
	SyncReadSPORT(spd[n][0], spd[n][1], 2000, 2000, sampleTime[n], &ready1, &ready2);

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc02(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;

	static Response rsp;

	if (req->adr == 0 || req->adr != netAdr) return false;

	byte n = req->f2.n;
	byte ch = (req->f2.chnl)&1;
//	byte cl = req->f2.chnl&1;

	rsp.adr = req->adr;
	rsp.func = req->func;
	rsp.f2.n = n;
	rsp.f2.chnl = req->f2.chnl;

	for (u16 i = 0; i < 500; i++)
	{
		rsp.f2.data1[i] = spd[n][ch][i*2+0];
		rsp.f2.data2[i] = spd[n][ch][i*2+1];
	};

	//rsp.f2.crc = GetCRC16(&rsp, sizeof(rsp.f2));

	wb->data = &rsp;
	wb->len = sizeof(rsp.f2)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc03(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	static Response rsp;

	if (req->adr != 0 && req->adr != netAdr) return false;

	sampleTime[0] = req->f3.dt[0];
	sampleTime[1] = req->f3.dt[1];
	sampleTime[2] = req->f3.dt[2];

	gain[0] = req->f3.ka[0];
	gain[1] = req->f3.ka[0];
	gain[2] = req->f3.ka[0];

	if (req->adr == 0) return  false;

	rsp.adr = req->adr;
	rsp.func = req->func;
	rsp.f3.crc = GetCRC16(&rsp, 2);

	wb->data = &rsp;
	wb->len = sizeof(rsp.f3)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	if (rb == 0 || rb->len < 2) return false;

	bool crcok = (GetCRC16(rb->data, rb->len) == 0);
	bool result = false;

	const Request *req = (Request*)rb->data;

	if (!crcok && req->func != 1) return false;

	switch(req->func)
	{
		case 1: result = RequestFunc01 (rb, wb, crcok); break;
		case 2: result = RequestFunc02 (rb, wb, crcok); break;
		case 3: result = RequestFunc03 (rb, wb, crcok); break;
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, (u32)-1, 720);
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0)
				{
					if (RequestFunc(&rb, &wb))
					{
						com.Write(&wb);
						i++;
					}
					else
					{
						i = 0;
					};
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

static void InitNetAdress()
{
	U32u f;
	
	f.w[1] = ReadADC();

	u32 t = GetRTT();

	while ((GetRTT()-t) < 1000000)
	{
		f.d += (i32)ReadADC() - (i32)f.w[1];
	};

	netAdr = (f.w[1] / 398) + 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;

	static u32 pt = 0;

	InitHardware();

	com.Connect(6250000, 0);

	InitNetAdress();

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<5;

		UpdateBlackFin();

		*pPORTFIO_TOGGLE = 1<<5;

	};

	return 0;
}
