#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"

#include "at25df021.h"

static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd[2][1024*2];
static byte spTime[3];
static byte spGain[3];
static u16	spLen[3];
static u16	spDelay[3];


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

static u16 flashCRC = 0;
static u32 flashLen = 0;

static u16 vectorCount = 0;


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

//		struct  { byte subfunc; ka[3]; word crc; } fAD;  // Перепрошивка флэшки
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
//		struct  { byte n; byte chnl; byte count[4]; byte time; byte gain; byte delay; byte filtr; u16 len; u16 data[512]; word crc; } f2;  // чтение вектора
//		struct  { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[1024*4]; word crc; } f2;  // чтение вектора
		struct  { word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
		struct  { word crc; } f4;  // старт оцифровки с установкой периода дискретизации вектора и коэффициента усиления
	};
};

#pragma pack()

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pack(1)

struct Response02 { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[1024*4]; word crc; }; // чтение вектора

#pragma pack()

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Response02 rsp02;

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
	spDelay[n] = sampleDelay[n];

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
//	byte chnl = (req->f2.chnl)&3;

	byte ch = 0;
	byte cl = 0;
	u16 len = sampleLen[n];

	Response02 &rsp = rsp02;

	if (rsp.rw == 0)
	{
		rsp.rw = 0xAA30 + (n<<4) + req->adr-1;
		rsp.cnt = vectorCount++;
		rsp.gain = gain[n]; 
		rsp.st = sampleTime[n]; 
		rsp.len = len; 
		rsp.delay = sampleDelay[n];

		u16 *p1 = rsp.data+len*0;
		u16 *p2 = rsp.data+len*1;
		u16 *p3 = rsp.data+len*2;
		u16 *p4 = rsp.data+len*3;

		for (u16 i = 0; i < len; i++)
		{
			*p1++ = spd[0][i*2+0] - 0x8000;
			*p2++ = spd[0][i*2+1] - 0x8000;
			*p3++ = spd[1][i*2+0] - 0x8000;
			*p4++ = spd[1][i*2+1] - 0x8000;
		};

		rsp.data[len*4] = GetCRC16(&rsp, sizeof(rsp) - sizeof(rsp.data) - sizeof(rsp.crc) + len*8);
	};

	//rsp.adr = netAdr;
	//rsp.func = 2;

	wb->data = &rsp;
	wb->len = sizeof(rsp) - sizeof(rsp.data) + rsp.len*8;

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

	if (sampleLen[0] > 1024) { sampleLen[0] = 1024; };
	if (sampleLen[1] > 1024) { sampleLen[1] = 1024; };
	if (sampleLen[2] > 1024) { sampleLen[2] = 1024; };

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

bool RequestFuncAD(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb, bool crcok)
{
	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

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

		case 0xAD: result = RequestFuncAD (rb, wb, crcok); break;
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

static const i16 RC[100] = {
	0.024819*32768,
	0.049023*32768,
	0.072625*32768,
	0.095642*32768,
	0.118088*32768,
	0.139977*32768,
	0.161322*32768,
	0.182138*32768,
	0.202437*32768,
	0.222232*32768,
	0.241536*32768,
	0.260360*32768,
	0.278718*32768,
	0.296620*32768,
	0.314077*32768,
	0.331102*32768,
	0.347703*32768,
	0.363893*32768,
	0.379681*32768,
	0.395077*32768,
	0.410091*32768,
	0.424732*32768,
	0.439010*32768,
	0.452933*32768,
	0.466511*32768,
	0.479752*32768,
	0.492665*32768,
	0.505256*32768,
	0.517536*32768,
	0.529510*32768,
	0.541188*32768,
	0.552575*32768,
	0.563680*32768,
	0.574509*32768,
	0.585070*32768,
	0.595368*32768,
	0.605411*32768,
	0.615204*32768,
	0.624755*32768,
	0.634068*32768,
	0.643150*32768,
	0.652007*32768,
	0.660644*32768,
	0.669067*32768,
	0.677281*32768,
	0.685290*32768,
	0.693101*32768,
	0.700718*32768,
	0.708146*32768,
	0.715390*32768,
	0.722454*32768,
	0.729342*32768,
	0.736060*32768,
	0.742611*32768,
	0.748999*32768,
	0.755229*32768,
	0.761304*32768,
	0.767228*32768,
	0.773006*32768,
	0.778639*32768,
	0.784133*32768,
	0.789491*32768,
	0.794716*32768,
	0.799811*32768,
	0.804780*32768,
	0.809625*32768,
	0.814350*32768,
	0.818958*32768,
	0.823451*32768,
	0.827833*32768,
	0.832106*32768,
	0.836273*32768,
	0.840337*32768,
	0.844299*32768,
	0.848164*32768,
	0.851932*32768,
	0.855607*32768,
	0.859191*32768,
	0.862686*32768,
	0.866094*32768,
	0.869417*32768,
	0.872658*32768,
	0.875819*32768,
	0.878901*32768,
	0.881907*32768,
	0.884838*32768,
	0.887696*32768,
	0.890483*32768,
	0.893201*32768,
	0.895852*32768,
	0.898437*32768,
	0.900958*32768,
	0.903416*32768,
	0.905813*32768,
	0.908151*32768,
	0.910430*32768,
	0.912653*32768,
	0.914821*32768,
	0.916935*32768,
	0.918997*32768
};



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateSport()
{
	const i16 A0 = 0.96952119255082503000 * 32768;
	const i16 A1 = -0.9695211925508250300 * 32768;

//	float B0 = 1.00000000000000000000;
	const i16 B1 = -0.93906250582030759000 * 32768;

	const u16 freq0 = 4000;

	const u16 freq[3] = {40000, 10000, 10000}; // М, ДХ, ДУ


	static i32 y[2]; //output samples
	static i16 x[2]; //input samples

	static i16 C0 = 0.118089 * 32768;
	static i32 F0 = 0;

	static byte n = 0;
	static byte chnl = 0;
	static u16 len = 0;
	static byte st = 0;
	static byte sg = 0;
	static u16 sd = 0;

	byte ch = 0;
	byte cl = 0;
	byte t = 0;

	Response02 &rsp = rsp02;

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
				sd = spDelay[n];

				t = freq[n] * st / freq0;

				if (t > 0) t -= 1;
				if (t >= ArraySize(RC)) t = ArraySize(RC) - 1;

				C0 = RC[t];

				sportState++;
			};

			break;

		case 1:

			rsp.rw = 0xAA30 + (n<<4) + netAdr-1;
			rsp.cnt = vectorCount++;
			rsp.gain = sg; 
			rsp.st = st; 
			rsp.len = len; 
			rsp.delay = spDelay[n];

			//x[0] = 0;
			//y[0] = 0;

			//F0 = 0;

			*pPORTFIO_SET = 1<<8;

			{
				spd[0][0] = spd[0][2];
				spd[0][1] =	spd[0][3];
				spd[1][0] =	spd[1][2];
				spd[1][1] =	spd[1][3];

				u16 *p1 = rsp.data+len*0;
				u16 *p2 = rsp.data+len*1;
				u16 *p3 = rsp.data+len*2;
				u16 *p4 = rsp.data+len*3;

				for (u16 i = 0; i < len; i++)
				{
					*p1++ = spd[0][i*2+0] - 0x8000;
					*p2++ = spd[0][i*2+1] - 0x8000;
					*p3++ = spd[1][i*2+0] - 0x8000;
					*p4++ = spd[1][i*2+1] - 0x8000;
				};
			};

			//for (u16 i = 0; i < len; i++)
			//{
			//	//x[1] = x[0];
			//	//y[1] = y[0];

			//	x[0] = spd[ch][i*2+cl] - 0x8000;
			//	//y[0] = (A0 * x[0] + A1 * x[1] - B1 * y[1]) / 32768;

			//	//rsp.f2.data[i] = y[0];

			//	//F0 += ((spd[ch][i*2+cl] - 0x8000) - F0/32768) * C0;
			//	//rsp.f2.data[i] = F0/32768;

			//	rsp.f2.data[i] = x[0];
			//};

			*pPORTFIO_CLEAR = 1<<8;

			sportState++;

			break;

		case 2:

			rsp.data[len*4] = GetCRC16(&rsp, sizeof(rsp) - sizeof(rsp.data) - sizeof(rsp.crc) + len*8);

			sportState++;

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

//static word GetFlashCRC16(u32 count)
//{
//	DataCRC CRC = { 0xFFFF };
//
//	byte buf[256];
//
//	u32 adr = 0;
//
//	while (count > 0)
//	{
//		u16 len = (count > sizeof(buf)) ? sizeof(buf) : count;
//
//		at25df021_Read(buf, adr, len);
//
//		adr += len;
//
//		byte *s = buf;
//
//		for ( ; len > 0; len--)
//		{
//			CRC.w = tableCRC[CRC.b[0] ^ *(s++)] ^ CRC.b[1];
//		};
//	};
//
//	return CRC.w;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CheckFlash()
{
	static BOOT_HEADER bh;

	byte *p = (byte*)&bh;

	u32 count = 0;
	u32 adr = 0;

	bool ready = false;

	while (1)
	{
		at25df021_Read(p, adr, sizeof(bh));

//		while(!ready) {};

		adr += sizeof(bh);

		if ((bh.blockCode & BFLAG_FILL) == 0)
		{
			adr += bh.byteCount;	
		};

		if (bh.blockCode & BFLAG_FINAL)
		{
			break;
		};
	};

	flashLen = adr;

	flashCRC = at25df021_GetCRC16(0, adr);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void main( void )
{
//	static byte s = 0;

//	static u32 pt = 0;


	InitHardware();

	com.Connect(6250000, 0);

	InitNetAdr();

//	CheckFlash();

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
