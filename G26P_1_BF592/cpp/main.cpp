#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"


static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd[2][512*2];
static byte spTime[3];
static byte spGain[3];
static u16	spLen[3];


//static u16 spd2[512*2];
//
//static i16 ch1[512];
//static i16 ch2[512];
//static i16 ch3[512];
//static i16 ch4[512];

static bool ready1 = false, ready2 = false;

//static u32 CRCOK = 0;
//static u32 CRCER = 0;

static byte sampleTime[3] = { 9, 19, 19};
static byte gain[3] = { 3, 3, 3 };
static u16 sampleLen[3] = {512, 512, 512};
static u16 sampleDelay[3] = { 0, 0, 0};


static byte netAdr = 1;

static U32u fadc = 0;

static byte sportState = 0;
static byte fireN = 0;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#pragma pack(1)

struct Request
{
	byte adr;
	byte func;
	
	union
	{
		struct  { byte n; word crc; } f1;  // старт оцифровки
		struct  { byte n; byte chnl; word crc; } f2;  // чтение вектора
		struct  { u16 st[3]; u16 sl[3]; u16 sd[3]; word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
		struct  { byte ka[3]; word crc; } f4;  // старт оцифровки с установкой периода дискретизации вектора и коэффициента усиления
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
		struct  { byte n; byte chnl; byte count[4]; byte time; byte gain; byte delay; byte filtr; u16 len; u16 data[512]; word crc; } f2;  // чтение вектора
		struct  { word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
		struct  { word crc; } f4;  // старт оцифровки с установкой периода дискретизации вектора и коэффициента усиления
	};
};

#pragma pack()

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Response rsp02[3][4];

static byte rspBuf[10];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc01(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

	byte n = req->f1.n;
	if (n > 2) n = 2;

	spTime[n] = sampleTime[n];
	spGain[n] = gain[n];
	spLen[n] = sampleLen[n];

	SetGain(spGain[n]);
	SyncReadSPORT(spd[0], spd[1], spLen[n]*4, spLen[n]*4, spTime[n]-1, &ready1, &ready2);

	fireN = n;
	sportState = 0;

	if (req->adr == 0) return  false;

	rsp.adr = netAdr;
	rsp.func = 1;
	rsp.f1.crc = GetCRC16(&rsp, 2);

	wb->data = &rsp;
	wb->len = sizeof(rsp.f1)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc02(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;

	if (req->f2.n > 2)
	{
		return false;
	};

	if (req->adr == 0) return  false;

	byte n = req->f2.n;
	byte chnl = (req->f2.chnl)&3;

	byte ch = 0;
	byte cl = 0;
	u16 len = sampleLen[n];

	Response &rsp = rsp02[n][chnl];

	if (rsp.adr == 0 || rsp.func == 0)
	{
		rsp.adr = netAdr;
		rsp.func = 2;
		rsp.f2.n = n;
		rsp.f2.chnl = chnl;
		rsp.f2.time = sampleTime[n];
		rsp.f2.gain = gain[n];
		rsp.f2.delay = 0;
		rsp.f2.filtr = 0;
		rsp.f2.len = len;

		ch = chnl>>1;
		cl = chnl&1;

		for (u16 i = 0; i < len; i++)
		{
			rsp.f2.data[i] = spd[ch][i*2+cl] - 0x8000;
		};

		rsp.f2.data[len] = GetCRC16(&rsp, sizeof(rsp.f2) - sizeof(rsp.f2.data) + len*2);
	};

	rsp.adr = netAdr;
	rsp.func = 2;

	wb->data = &rsp;
	wb->len = sizeof(rsp.f2) - sizeof(rsp.f2.data) + rsp.f2.len*2 + 2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc03(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

	sampleTime[0] = req->f3.st[0];
	sampleTime[1] = req->f3.st[1];
	sampleTime[2] = req->f3.st[2];

	sampleLen[0] = req->f3.sl[0];
	sampleLen[1] = req->f3.sl[1];
	sampleLen[2] = req->f3.sl[2];

	sampleDelay[0] = req->f3.sd[0];
	sampleDelay[1] = req->f3.sd[1];
	sampleDelay[2] = req->f3.sd[2];

	if (req->adr == 0) return  false;

	rsp.adr = req->adr;
	rsp.func = req->func;
	rsp.f3.crc = GetCRC16(&rsp, 2);

	wb->data = &rsp;
	wb->len = sizeof(rsp.f3)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc04(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

	gain[0] = req->f4.ka[0];
	gain[1] = req->f4.ka[1];
	gain[2] = req->f4.ka[2];

	if (req->adr == 0) return  false;

	rsp.adr = req->adr;
	rsp.func = req->func;
	rsp.f3.crc = GetCRC16(&rsp, 2);

	wb->data = &rsp;
	wb->len = sizeof(rsp.f4)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFunc(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	if (rb == 0 || rb->len < 2 || rb->len > sizeof(Request)) return false;

	const Request *req = (Request*)rb->data;

	if (req->adr != 0 && req->adr != netAdr) return false;

	bool crcok = (GetCRC16(rb->data, rb->len) == 0);
	bool result = false;

	if (!crcok && req->func != 1)
	{
		return false;
	};

	switch(req->func)
	{
		case 1: result = RequestFunc01 (rb, wb, crcok); break;
		case 2: result = RequestFunc02 (rb, wb, crcok); break;
		case 3: result = RequestFunc03 (rb, wb, crcok); break;
		case 4: result = RequestFunc04 (rb, wb, crcok); break;
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[sizeof(Request)+10];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, (u32)-1, US2SCLK(50));
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

//#define NCoef 1

//i32 iir(i16 NewSample)
//{
//	const i16 A0 = 0.08658549734431006400 * 32768;
//	const i16 A1 = 0.08658549734431006400 * 32768;
//
////	float B0 = 1.00000000000000000000;
//	const i16 B1 = -0.82727194597247566000 * 32768;
//
//	static i32 y[2]; //output samples
//	static i16 x[2]; //input samples
//
//	//shift the old samples
//	x[1] = x[0];
//	y[1] = y[0];
//
//	//Calculate the new output
//	x[0] = NewSample;
//	y[0] = (A0 * x[0] + A1 * x[1] - B1 * y[1]) / 32768;
//
//	return y[0];
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateSport()
{
	const i16 A0 = 0.24523727540750304000 * 32768;
	const i16 A1 = 0.24523727540750304000 * 32768;

//	float B0 = 1.00000000000000000000;
	const i16 B1 = -0.50952544949442879000 * 32768;

	static i32 y[2]; //output samples
	static i16 x[2]; //input samples

	static byte n = 0;
	static byte chnl = 0;
	static u16 len = 0;
	static byte st = 0;
	static byte sg = 0;

	byte ch = 0;
	byte cl = 0;

	Response &rsp = rsp02[n][chnl];

	switch(sportState)
	{
		case 0:
			
			if (ready1 && ready2)
			{
				n = fireN;
				chnl = 0;
				len = spLen[n];
				st = spTime[n];
				sg = spGain[n];

				sportState++;
			};

			break;

		case 1:

			rsp.adr = netAdr;
			rsp.func = 2;
			rsp.f2.n = n;
			rsp.f2.chnl = chnl;
			rsp.f2.time = st;
			rsp.f2.gain = sg;
			rsp.f2.delay = 0;
			rsp.f2.filtr = 0;
			rsp.f2.len = len;

			ch = chnl>>1;
			cl = chnl&1;

			x[0] = 0;
			y[0] = 0;

			for (u16 i = 0; i < len; i++)
			{
				x[1] = x[0];
				y[1] = y[0];

				x[0] = spd[ch][i*2+cl] - 0x8000;
				y[0] = (A0 * x[0] + A1 * x[1] - B1 * y[1]) / 32768;

				rsp.f2.data[i] = y[0];
			};

			sportState++;

			break;

		case 2:

			*pPORTFIO_SET = 1<<8;

			rsp.f2.data[len] = GetCRC16(&rsp, sizeof(rsp.f2) - sizeof(rsp.f2.data) + len*2);

			*pPORTFIO_CLEAR = 1<<8;


			if (chnl < 3)
			{
				chnl += 1;
				sportState = 1;
			}
			else
			{
				sportState++;
			};

			break;

		case 3:

			break;

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateNetAdr()
{
	netAdr = (GetADC() / 398) + 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitNetAdr()
{
	u32 t = GetRTT();

	while ((GetRTT()-t) < 10000000)
	{
		UpdateHardware();
	};

	UpdateNetAdr();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void main( void )
{
//	static byte s = 0;

//	static u32 pt = 0;

	InitHardware();

	com.Connect(6250000, 0);

	InitNetAdr();

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<5;

		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateBlackFin()	);
			CALL( UpdateHardware()	);
			CALL( UpdateNetAdr()	);
			CALL( UpdateSport()		);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

		*pPORTFIO_TOGGLE = 1<<5;
	};

//	return 0;
}
