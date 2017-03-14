#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "req.h"
#include "hardware.h"

#include "list.h"

#include "twi.h"

#include "PointerCRC.h"

#include "SPI.h"

ComPort commem;
ComPort comtr;
//ComPort combf;
ComPort comrcv;

//ComPort::WriteBuffer wb;
//ComPort::ReadBuffer rb;


u32 fc = 0;

static u32 bfCRCOK = 0;
static u32 bfCRCER = 0;
static u32 bfURC = 0;
static u32 bfERC = 0;

//static bool waitSync = false;
static bool startFire = false;

static RequestQuery qrcv(&comrcv);
static RequestQuery qtrm(&comtr);
static RequestQuery qmem(&commem);

static R02 r02[2];

static Rsp02 manVec[6];

static byte curRcv[3] = {0};
static byte curVec[3] = {0};

static List<R02> freeR02;

//static RMEM rmem[4];
//static List<RMEM> lstRmem;
//static List<RMEM> freeRmem;

static byte fireType = 0;

static byte gain[8][3] = { { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 }, { 1, 3, 3 } };
static byte sampleTime[3] = { 5, 20, 20};
static u16 sampleLen[3] = { 64, 64, 64};
static u16 sampleDelay[3] = { 10, 10, 10};

u32 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

static u16 manReqWord = 0xAA00;
static u16 manReqMask = 0xFF00;

static u16 numDevice = 0;
static u16 verDevice = 1;

static u32 manCounter = 0;
static u32 fireCounter = 0;

static const u16 reqVoltage = 900;
static const byte reqFireCount = 1;

static u16 adcValue = 0;
static U32u filtrValue;
static u16 resistValue = 0;
static byte numStations = 0;
static u16 voltage = 0;
static i16 temperature = 0;

static byte mainModeState = 0;

static byte rcvStatus = 0;

//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;

//static u32 chnlCount[4] = {0};

static u32 crcErr02 = 0;
static u32 crcErr03 = 0;
static u32 crcErr04 = 0;

static byte savesCount = 0;

static TWI	twi;

static DSCTWI dsc;
static byte buf[100];

static u16 maxOff = 0;

static void SaveVars();

inline void SaveParams() { savesCount = 1; }


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//Response rsp;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static SPI spi;
static SPI::Buffer	bufAccel; 
static SPI::Buffer	bufGyro;

u8 txAccel[25] = { 0x87, 0, 0, 0xC7, 0, 0, 0x97, 0, 0, 0xD7, 0, 0, 0xA7, 0, 0, 0xE7, 0, 0, 0xB7, 0, 0, 0xF7, 0, 0, 0 };
u8 rxAccel[25];

u8 txGyro[25] = { 0x87, 0, 0, 0xC7, 0, 0, 0x97, 0, 0, 0xD7, 0, 0, 0xA7, 0, 0, 0xE7, 0, 0, 0xB7, 0, 0, 0xF7, 0, 0, 0 };
u8 rxGyro[25];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackAccel(SPI::Buffer *b)
{
	//DataPointer p(b->rxp);

	//union { float f; u16 w[2]; } u;

	//p.b += 1;

	//for (byte i = 0; i < 8; i++)
	//{
	//	u.f = (u16)(__rev(*p.d) >> 15) * 0.0003814697265625*1.00112267 - 0.00319055 - valueADC1[i];

	//	valueADC1[i] += u.f * (((u.w[1] & 0x7f80) < 0x3c00) ? 0.01 : 0.1);
	//	p.b += 3;
	//};

	//UpdatePWM();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackGyro(SPI::Buffer *b)
{
	//DataPointer p(b->rxp);

	//union { float f; u16 w[2]; } u;

	//p.b += 1;

	//for (byte i = 0; i < 8; i++)
	//{
	//	u.f = (u16)(__rev(*p.d) >> 15) * 0.0003814697265625*1.00112267 - 0.00319055 - valueADC2[i];

	//	valueADC2[i] += u.f * (((u.w[1] & 0x7f80) < 0x3c00) ? 0.01 : 0.1);
	//	p.b += 3;
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void InitADC16()
//{
//	using namespace HW;
//
//	//PMC->PCER0 = PID::SPI0_M;
//
//	//SPI0->CR = 1; 
//	//SPI0->MR = 0xFF010005;
//	//SPI0->CSR[0] = 0x00095482;
//
//	bufADC1.txp = &txADC1;
//	bufADC1.rxp = &rxADC1;
//	bufADC1.count = 25;
//	bufADC1.CSR = 0x00092302;
//	bufADC1.DLYBCS = 0x9;
//	bufADC1.PCS = 1;
//	bufADC1.pCallBack = CallBackADC1;
//
//	bufADC2.txp = &txADC2;
//	bufADC2.rxp = &rxADC2;
//	bufADC2.count = 25;
//	bufADC2.CSR = 0x00092302;
//	bufADC2.DLYBCS = 0x9;
//	bufADC2.PCS = 0;
//	bufADC2.pCallBack = CallBackADC2;
//
//	spi.AddRequest(&bufADC1);
//	spi.AddRequest(&bufADC2);
//
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateAccel()
{
	static byte i = 0;

	static TM32 tm;

	switch (i)
	{
		case 0:

			tm.Reset();
			i++;

			break;

		case 1:

			if (tm.Check(35))
			{

			};

			break;

	};

	if (bufAccel.ready)
	{
		bufAccel.txp = &txAccel;
		bufAccel.rxp = &rxAccel;
		bufAccel.count = 25;
		bufAccel.CSR = 0x00092302;
		bufAccel.DLYBCS = 0x9;
		bufAccel.PCS = 1;
		bufAccel.pCallBack = CallBackAccel;
		bufAccel.pio = HW::PIOA;
		bufAccel.mask = 1<<23;

		spi.AddRequest(&bufAccel);
	};

	//if (bufGyro.ready)
	//{
	//	spi.AddRequest(&bufGyro);
	//};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReqFire(REQ *q)
{
//	waitSync = true;;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReqFire(byte adr, byte n, u16 tryCount)
{
	static Req01 req;
	static ComPort::WriteBuffer wb;
	static REQ q;

	q.CallBack = CallBackRcvReqFire;
	q.rb = 0;
	q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	
	wb.data = &req;
	wb.len = sizeof(req);
	
	req.adr = adr;
	req.func = 1;
	req.n = n;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq02(REQ *q)
{
	Rsp02 &rsp = *((Rsp02*)q->rb->data);
	Req02 &req = *((Req02*)q->wb->data);
	
	bool crcOK;

	if (q->rb->recieved)
	{
		crcOK = GetCRC16(q->rb->data, q->rb->len) == 0;

		if (!crcOK) 
		{
			crcErr02++;
		};
	}
	else
	{
		crcOK = false;
	};

	if (crcOK)
	{
		rcvStatus |= 1 << (rsp.rw & 7);
	}
	else
	{
		rcvStatus &= ~(1 << ((req.adr-1) & 7)); 
	};

	if (!crcOK && q->tryCount > 0)
	{
		q->tryCount--;
		qrcv.Add(q);
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

R02* CreateRcvReq02(byte adr, byte n, byte chnl, u16 tryCount)
{
	adr = (adr-1)&7; 
	chnl &= 3; n %= 3;

	R02 *r02 = freeR02.Get();

	if (r02 == 0) return 0;

	R02 &r = *r02;

	Req02 &req = r.req;
	Rsp02 &rsp = r.rsp;
	
	ComPort::WriteBuffer &wb = r.wb;
	ComPort::ReadBuffer	 &rb = r.rb;
	
	REQ &q = r.q;

	q.CallBack = CallBackRcvReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	q.tryCount = tryCount;
	
	wb.data = &req;
	wb.len = sizeof(req);

	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);
	rb.recieved = false;
	
	req.adr = adr+1;
	req.func = 2;
	req.n = n;
	req.chnl = chnl;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return r02;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq03(REQ *q)
{
//	Rsp03 *rsp = (Rsp03*)q->rb->data;

	bool crcOK = GetCRC16(q->rb->data, q->rb->len) == 0;

	if (!crcOK) 
	{
		crcErr03++;

		if (q->tryCount > 0)
		{
			q->tryCount--;
			qrcv.Add(q);
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq03(byte adr, byte st[], u16 sl[], u16 sd[], u16 tryCount)
{
	static Req03 req;
	static Rsp03 rsp;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackRcvReq03;
	q.preTimeOut = US2RT(500);
	q.postTimeOut = US2RT(100);
	q.rb = &rb;
	q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	
	wb.data = &req;
	wb.len = sizeof(req);
	
	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);

	req.adr = adr;
	req.func = 3;

	req.st[0] = st[0];
	req.st[1] = st[1];
	req.st[2] = st[2];

	req.sl[0] = sl[0];
	req.sl[1] = sl[1];
	req.sl[2] = sl[2];

	req.sd[0] = sd[0];
	req.sd[1] = sd[1];
	req.sd[2] = sd[2];

	req.crc = GetCRC16(&req, sizeof(req)-2);
	
	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq04(REQ *q)
{
//	Rsp04 *rsp = (Rsp04*)q->rb->data;

	bool crcOK = GetCRC16(q->rb->data, q->rb->len) == 0;

	if (!crcOK) 
	{
		crcErr04++;

		if (q->tryCount > 0)
		{
			q->tryCount--;
			qrcv.Add(q);
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq04(byte adr, byte ka[], u16 tryCount)
{
	static Req04 req;
	static Rsp04 rsp;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackRcvReq04;
	q.preTimeOut = US2RT(500);
	q.postTimeOut = US2RT(100);
	q.rb = &rb;
	q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	
	wb.data = &req;
	wb.len = sizeof(req);
	
	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);

	req.adr = adr;
	req.func = 4;

	req.ka[0] = ka[0];
	req.ka[1] = ka[1];
	req.ka[2] = ka[2];

	req.crc = GetCRC16(&req, sizeof(req)-2);
	
	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CallBackTrmReqFire(REQ *q)
{
//	waitSync = true;;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static REQ* CreateTrmReqFire(byte n)
{
	static __packed struct { byte func; byte n; word crc; } req;

	static ComPort::WriteBuffer wb;
	static REQ q;

	const byte fn[3] = {3, 1, 2};

	q.CallBack = CallBackTrmReqFire;
	q.rb = 0;
	q.wb = &wb;
	
	wb.data = &req;
	wb.len = sizeof(req);
	
	req.func = 1;
	req.n = fn[n];
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool readyTrmreq02 = true;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CallBackTrmReq02(REQ *q)
{
	__packed struct Rsp { byte f; u16 hv; u16 crc; };

	Rsp *rsp = (Rsp*)q->rb->data;

	bool crcOK = GetCRC16(q->rb->data, q->rb->len) == 0;

	if (crcOK)
	{
		voltage = rsp->hv;
		readyTrmreq02 = true;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static REQ* CreateTrmReq02()
{
	static __packed struct { byte f; u16 crc; } req;
	static __packed struct { byte f; u16 hv; u16 crc; } rsp;

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackTrmReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	
	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);

	wb.data = &req;
	wb.len = sizeof(req);
	
	req.f = 2;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTrmReq02()
{
	static RTM32 rt;

	if (readyTrmreq02 || rt.Check(MS2RT(100)))
	{
		qtrm.Add(CreateTrmReq02());

		readyTrmreq02 = false;
		rt.Reset();
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CallBackTrmReq03(REQ *q)
{
	__packed struct Rsp { byte f; u16 crc; };

//	Rsp *rsp = (Rsp*)q->rb->data;

	bool crcOK = GetCRC16(q->rb->data, q->rb->len) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static REQ* CreateTrmReq03()
{
	static __packed struct { byte f; byte fireCount; u16 hv; u16 crc; } req;
	static __packed struct { byte f; u16 crc; } rsp;

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackTrmReq03;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	
	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);

	wb.data = &req;
	wb.len = sizeof(req);
	
	req.f = 3;
	req.fireCount = reqFireCount;
	req.hv = reqVoltage;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRmemList()
{
	for (u16 i = 0; i < ArraySize(r02); i++)
	{
		freeR02.Add(&r02[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void CallBackMemReq01(REQ *q)
//{
//	if (q != 0)
//	{
//		RMEM* rm = (RMEM*)q->ptr;
//	
//		if (rm != 0)
//		{
//			if (!HW::RamCheck(rm))
//			{
//				__breakpoint(0);
//			};
//
//			freeRmem.Add(rm); 
//		};
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//u32 countMemReq = 0;

//RMEM reqMem;

//REQ* CreateMemReq01()
//{
//	RMEM* rm = freeRmem.Get();
//
//	if (rm == 0) return 0;
//
//	ReqMem &req = rm->req;
////	RspMem &rsp = rm->rsp;
//	
//	ComPort::WriteBuffer &wb = rm->wb;
////	ComPort::ReadBuffer	 &rb = rm->rb;
//	
//	REQ &q = rm->q;
//
//
//	q.CallBack = CallBackMemReq01;
//	q.rb = 0;//&rb;
//	q.wb = &wb;
//	q.preTimeOut = MS2RT(1);
//	q.postTimeOut = 1;
//	q.ready = false;
//	q.ptr = rm;
//	
//	wb.data = &req;
//	wb.len = 4;//sizeof(req);
//
//	//rb.data = &rsp;
//	//rb.maxLen = sizeof(rsp);
//	//rb.recieved = false;
//
//	return &q;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CallBackMemReq02(REQ *q)
{
//	memBusy = false;

	if (q != 0)
	{
		R02* r = (R02*)q->ptr;
	
		if (r != 0)
		{
			freeR02.Add(r); 
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 countMemReq = 0;

//RMEM reqMem;

static void CreateMemReq02(R02 &r)
{
	//R02 &r = *r02;

	//ReqMem &req = rm->req;
	//RspMem &rsp = rm->rsp;

	ComPort::WriteBuffer &wb = r.wb;
//	ComPort::ReadBuffer	 &rb = rm->rb;
	
	REQ &q = r.q;

	q.CallBack = CallBackMemReq02;
	q.rb = 0;//&rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	q.ptr = &r;
	
	wb.data = &r.rsp;
	wb.len = r.rb.len;// l*4*2 + sizeof(req) - sizeof(req.data);

	//rb.data = &rsp;
	//rb.maxLen = sizeof(rsp);
	//rb.recieved = false;

	qmem.Add(&q);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_00(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0;
	manTrmData[1] = numDevice;
	manTrmData[2] = verDevice;

	mtb->data = manTrmData;
	mtb->len = 3;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(u16 *buf, u16 len, MTB* mtb)
{
	//__packed struct T { u16 g[8]; u16 st; u16 len; u16 delay; u16 voltage; };
	//__packed struct Rsp { u16 hdr; u16 rw; T t1, t2, t3; };
	
	if (buf == 0 || len == 0 || mtb == 0) return false;

	u16* p = manTrmData+1;

	for (byte i = 0; i < 3; i++)
	{
		*(p++) =  gain[0][i];
		*(p++) =  gain[1][i];
		*(p++) =  gain[2][i];
		*(p++) =  gain[3][i];
		*(p++) =  gain[4][i];
		*(p++) =  gain[5][i];
		*(p++) =  gain[6][i];
		*(p++) =  gain[7][i];
		*(p++) =  sampleTime[i];
		*(p++) =  sampleLen[i];
		*(p++) =  sampleDelay[i];
		*(p++) =  800;
	};

	manTrmData[0] = (manReqWord & manReqMask) | 0x10;

	mtb->data = manTrmData;
	mtb->len = 1+12*3;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(u16 *data, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = manReqWord|0x20;
	manTrmData[1] = GD(&manCounter, u16, 0);
	manTrmData[2] = GD(&manCounter, u16, 1);
	manTrmData[3] = voltage;
	manTrmData[4] = numStations|(((u16)rcvStatus)<<8);
	manTrmData[5] = resistValue;
	manTrmData[6] = temperature;
 
	mtb->data = manTrmData;
	mtb->len = 7;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_30(u16 *data, u16 len, MTB* mtb)
{
	__packed struct Req { u16 rw; u16 off; u16 len; };

//	__packed struct Hdr { u32 cnt; u16 gain; u16 st; u16 len; u16 delay; };

//	__packed struct St { Hdr h;  };

	struct Rsp { u16 rw; u16 data[128]; };
	
	static Rsp rsp; 

//	Hdr hdr;

//	static u16 max = 0;



	Req &req = *((Req*)data);

	//off 0...2005

//return false;
	if (buf == 0 || len == 3 || mtb == 0) return false;

	byte nf = ((req.rw>>4)-3)&3;
	byte nr = req.rw & 7;

	curRcv[nf] = nr;

	u16 c = 0;
	u16 off = 0;
	//u16 diglen = 500;
	//u16 hdrlen = (sizeof(hdr)/2);

	u16 sz = 6 + sampleLen[nf]*4;

	//manVec[nf].len = 29;
	//manVec[nf].gain = 0;
	//manVec[nf].st = 10;
	//manVec[nf].delay = 0;

	if (req.off == 0)
	{
		curVec[nf] = (curVec[nf] + 1) & 1;
	};

	if (len == 1)
	{
		off = 0;
		c = sz;
	}
	else
	{
		off = req.off;
		c = req.len;

		if (off > maxOff)
		{
			maxOff = off;
		};

		if (off >= sz)
		{
			c = 0; off = 0;
		}
		else if ((off+c) > sz)
		{
			c = sz-off;
		};
	};


	mtb->data = (u16*)&rsp;
	mtb->len = c+1;

	u16 *p = rsp.data;

	u16 *s = (u16*)&manVec[nf*2 + ((curVec[nf] + 1) & 1)];

	s += off + 1;

	while (c > 0)
	{
		*p++ = *s++; c--;
	};

	rsp.rw = req.rw;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 3 || mtb == 0) return false;

	switch (data[1])
	{
		case 1:

			numDevice = data[2];

			break;

		case 2:

			SetTrmBoudRate(data[2]-1);

			break;
	};

	manTrmData[0] = (manReqWord & manReqMask) | 0x80;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 3 || mtb == 0) return false;

	byte nf = ((data[1]>>4) & 3)-1;
	byte nr = data[1] & 0xF;
	byte st;
	u16 sl;

	if (nf > 2) return false;

	if (nr < 8)
	{
		gain[nr][nf] = data[2]&7;
		
		qrcv.Add(CreateRcvReq04(nr+1, gain[nr], 2));
	}
	else
	{
		switch(nr)
		{
			case 0x8:

				st = data[2] & 0xFF;
//				if (st > 0) st -= 1;
				sampleTime[nf] = st;

				break;

			case 0x9:

				sl = data[2];
				if (sl > 1024) sl = 1024;
				sampleLen[nf] = sl;

				break;

			case 0xA:

				sampleDelay[nf] = data[2];

				break;

			case 0xB:


				break;
		};

		qrcv.Add(CreateRcvReq03(0, sampleTime, sampleLen, sampleDelay, 2));
	};

	SaveParams();

	manTrmData[0] = (manReqWord & manReqMask) | 0x90;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || mtb == 0) return false;

	SaveParams();

	manTrmData[0] = (manReqWord & manReqMask) | 0xF0;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	bool r = false;

	byte i = (buf[0]>>4)&0xF;

	switch (i)
	{
		case 0: 	r = RequestMan_00(buf, len, mtb); break;
		case 1: 	r = RequestMan_10(buf, len, mtb); break;
		case 2: 	r = RequestMan_20(buf, len, mtb); break;
		case 3: 
		case 4: 
		case 5: 	r = RequestMan_30(buf, len, mtb); break;
		case 8: 	r = RequestMan_80(buf, len, mtb); break;
		case 9:		r = RequestMan_90(buf, len, mtb); break;
		case 0xF:	r = RequestMan_F0(buf, len, mtb); break;
		
		default:	bfURC++; 
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static bool RequestBF(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
//{
//	byte *p = (byte*)rb->data;
//	bool r = false;
//
//	switch(*p)
//	{
//		case 1:	// Запрос Манчестер
//
//			r = RequestMan(wb, rb);
//
//			break;
//
//	};
//
//	return r;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void UpdateBlackFin()
//{
//	static byte i = 0;
//	static ComPort::WriteBuffer wb;
//	static ComPort::ReadBuffer rb;
//	static byte buf[32];
//
//	switch(i)
//	{
//		case 0:
//
//			rb.data = buf;
//			rb.maxLen = sizeof(buf);
//			combf.Read(&rb, -1, 1);
//			i++;
//
//			break;
//
//		case 1:
//
//			if (!combf.Update())
//			{
//				if (rb.recieved && rb.len > 0 && GetCRC16(rb.data, rb.len) == 0)
//				{
//					if (RequestBF(&wb, &rb))
//					{
//						combf.Write(&wb);
//						i++;
//					}
//					else
//					{
//						i = 0;
//					};
//
//					bfCRCOK++;
//				}
//				else
//				{
//					bfCRCER++;
//					i = 0;
//				};
//			};
//
//			break;
//
//		case 2:
//
//			if (!combf.Update())
//			{
//				i = 0;
//			};
//
//			break;
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void FireHandler()
{
	using namespace HW;

//	USART0->CR = 0x40;
	USART0->THR = 0;

//	PIOA->SODR = 1<<7;

	TC0->C1.CCR = CLKDIS|SWTRG;
	TC0->C1.IDR = CPCS;

	u32 tmp = TC0->C1.SR;

//	PIOA->CODR = 1<<7;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static void StartTrmFire(u16 delay)
{
	using namespace HW;

	PMC->PCER0 = PID::TC1_M;

	VectorTableExt[HW::PID::TC1_I] = FireHandler;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;

	TC0->C1.CCR = CLKDIS|SWTRG;
	TC0->C1.IER = CPCS;
	TC0->C1.RC = MCK/2/1000000*delay;
	TC0->C1.CMR = 0;

//	UART1->CR = 0xA0;	// Disable transmit and receive

	PIOB->SODR = 1<<13;
	PIOA->SODR = 1<<4;

	UART1->CR = 0x40;
	USART0->CR = 0x40;


	if (delay != 0)
	{
		__enable_irq();
		UART1->THR = 0;
		TC0->C1.CCR = CLKEN|SWTRG;
	}
	else
	{
		UART1->THR = 0;
		USART0->THR = 0;
	};

//	while ((_SU->CSR & 0x200) == 0);

//	PIOB->CODR = 1<<13;

//	UART1->CR = 0xA0;	// Disable transmit and receive
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateRcvTrm()
{
	static byte i = 0, n = 0;
	static u16 sd = 0;
	static RTM32 rtm;

	static u32 fireTime = MS2RT(15);

//	static RTM32 rt2;

//	static Request req;

	REQ *reqr;
	REQ *reqt;
			
	switch(i)
	{
		case 0:
			
			qrcv.Update();

			i = (startFire) ? 2 : 1;

			break;

		case 1:

			qtrm.Update();
		
			i = (startFire) ? 2 : 0;

			break;

		case 2:

			qrcv.Stop();
			qtrm.Stop();
			n = fireType;
			sd = sampleDelay[n];

			i++;

			break;


		case 3:

			qrcv.Update();
			qtrm.Update();
 
			if (qrcv.Stoped() && qtrm.Stoped())
			{
				i++;
			};

			break;

		case 4:

			reqr = CreateRcvReqFire(0, n, 0);
			reqt = CreateTrmReqFire(n);

			comrcv.Write(reqr->wb);
			comtr.Write(reqt->wb);

			i++;

			break;


		case 5:

			if (!comrcv.Update() && !comtr.Update())
			{
				rtm.Reset();
				i++;
			};

			break;

		case 6:

			if (rtm.Check(3))
			{
				StartTrmFire(sd);

				//comrcv.TransmitByte(0);
				//comtr.TransmitByte(0);

				u32 t = (u32)sampleLen[n]*sampleTime[n]+sampleDelay[n] + 5000;

				fireTime = MS2RT(t/1000);

				i++;
			};

			break;

		case 7:

			if (rtm.Check(fireTime))
			{
				i++;
			};

			break;

		case 8:

			//for (byte j = 1; j < 9; j++)
			//{
			//	qrcv.Add(CreateRcvReq02(j, n, 0));
			//	qrcv.Add(CreateRcvReq02(j, n, 1));
			//};

			startFire = false;

			qrcv.Start();
			qtrm.Start();

			i = 0;

			break;
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool RequestTestCom01(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
//{
//	__packed struct Req { byte func; byte ft; u16 crc; };
//	__packed struct Rsp { byte func; u16 crc; };
//
//	Req *req = (Req*)rb->data;
//
//	static Rsp rsp;
//
//	if (rb->len < 2) return false;
//
//	if (req->ft <= 2)
//	{
//		fireType = req->ft;
//	};
//
//	rsp.func = 1;
//
//	rsp.crc = GetCRC16(&rsp, 1);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	startFire = true;
//
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool RequestTestCom02(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
//{
//	__packed struct Req { byte func; byte adr; byte n; byte ch; u16 crc; };
//
//	Req *req = (Req*)rb->data;
//
//	if (rb->len < 4) return false;
//	if (req->adr < 1 || req->adr > 8 || req->n > 2 || req->ch > 1) return false;
//
//	R02 &r = r02[req->adr-1][req->n][req->ch];
//
//	wb->data = &r.rsp;
//	wb->len = sizeof(r.rsp);
//
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool RequestTestCom03(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
//{
//	__packed struct Req { byte func; byte dt[3]; byte ka[3]; word crc; };
//	__packed struct Rsp { byte func; u16 crc; };
//
//	Req *req = (Req*)rb->data;
//
//	static Rsp rsp;
//
//	if (rb->len < 7) return false;
//
//	sampleTime[0] = req->dt[0];
//	sampleTime[1] = req->dt[1];
//	sampleTime[2] = req->dt[2];
//
//	gain[0] = req->ka[0];
//	gain[1] = req->ka[1];
//	gain[2] = req->ka[2];
//
//	qrcv.Add(CreateRcvReq03(0, sampleTime, gain));
//
//	rsp.func = 3;
//	rsp.crc = GetCRC16(&rsp, 1);
//
//	wb->data = &rsp;
//	wb->len = sizeof(rsp);
//
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool RequestTestCom(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
//{
//	byte *pb = (byte*)rb->data;
//
//	switch (pb[0])
//	{
//		case 1 : return RequestTestCom01(rb, wb);
//		case 2 : return RequestTestCom02(rb, wb);
//		case 3 : return RequestTestCom03(rb, wb);
//		default : return false;
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void UpdateCom1()
//{
//	static byte i = 0;
//
//	static ComPort::WriteBuffer wb;
//	static ComPort::ReadBuffer rb; // = r02[0][0][1].rb;
//	static byte buf[32];
//
//	static RTM32 rtm;
//
//
//	switch(i)
//	{
//		case 0:
//	
//			rb.data = buf;
//			rb.maxLen = sizeof(buf);
//			com1.Read(&rb, -1, 2);
//
//			i++;
//
//			break;
//
//		case 1:
//
//			if (!com1.Update())
//			{
//				i = (RequestTestCom(&rb, &wb)) ? i+1 : 0;
//			};
//
//			break;
//
//		case 2:
//
//			com1.Write(&wb);
//
//			i++;
//
//			break;
//
//		case 3: 
//
//			if (!com1.Update())
//			{
//				i = 0;
//			};
//
//			break;
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	if (HW::ADC->ISR & 8)
	{
		filtrValue.d += ((i32)(HW::ADC->CDR[3]) - (i32)filtrValue.w[1])*1024;
		adcValue = filtrValue.w[1] + (filtrValue.w[0]>>15);
		resistValue = ((u32)adcValue * (u16)(3.3 / 4096 / 0.000321 * 512))>>9;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitNumStations()
{
	u32 t = GetRTT();

	while ((GetRTT()-t) < MS2RT(100))
	{
		UpdateADC();
	};

	numStations = 8;//resistValue / 1000;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void MainMode()
{
	//fireType

	static byte rcv = 0;
	static byte chnl = 0;
	static REQ *req = 0;
	static R02 *r02 = 0;
	static RTM32 rt;
	static TM32 rt2;

//	REQ *rm = 0;

	switch (mainModeState)
	{
		case 0:

			req = CreateRcvReq03(0, sampleTime, sampleLen, sampleDelay, 0);

			if (req != 0)
			{
				qrcv.Add(req);

				mainModeState++;
			};

			break;

		case 1:

			if (req->ready)
			{
				startFire = true;
				
				mainModeState++;
			};

			break;

		case 2:

			if (!startFire)
			{
				rcv = 1; chnl = 0;

				mainModeState++;
			};

			break;

		case 3:

			r02 = CreateRcvReq02(rcv, fireType, chnl, 1);

			if (r02 != 0)
			{
				qrcv.Add(&r02->q);

				mainModeState++;
			};

			break;

		case 4:

			if (r02->q.ready)
			{
				if (curRcv[fireType] == (rcv-1))
				{
					u16 *s = (u16*)&r02->rsp;
					u16 *d = (u16*)&manVec[fireType*2 + curVec[fireType]];
					u16 c = r02->rb.len/2;

					while (c-- > 0)
					{
						*d++ = *s++;
					};

					manVec[fireType].cnt = fireCounter;
				};

				CreateMemReq02(*r02);

				if (rcv < numStations)
				{
					manCounter++;

					rcv += 1;
					chnl = 0;

					mainModeState++;
				}
				else
				{
					rcv = 1;
					mainModeState = 6;
				};

				rt.Reset();
			};

			break;

		case 5:

			if (rt.Check(US2RT(1)))
			{
				mainModeState = 3;
			};

			break;

		case 6:

			req = CreateRcvReq04(rcv, gain[rcv-1], 2);

			if (req != 0)
			{
				qrcv.Add(req);

				mainModeState++;
			};

			break;

		case 7:

			if (req->ready)
			{
				if (rcv < numStations)
				{
					rcv++;

					mainModeState -= 1;
				}
				else
				{
					mainModeState++;
				};
			};

			break;

		case 8:

			if (rt.Check(MS2RT(50)))
			{
				fireType = (fireType+1) % 3; 

				fireCounter += 1;

				mainModeState = (fireType == 0) ? 10 : 9;
			};

			break;

		case 9:

			if (voltage >= reqVoltage || rt.Check(MS2RT(300)))
			{
				mainModeState = 0;
			};

			break;

		case 10:

			if (rt2.Check(1000))
			{
				mainModeState = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	static MTB mtb;
	static MRB mrb;

	static byte i = 0;

	static RTM32 tm;

	bool parityErr;
	u32 *s;
	u16 *d;

	u16 c;

	switch (i)
	{
		case 0:

			mrb.data = manRcvData;
			mrb.maxLen = 5;
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			if (mrb.ready)
			{
//				HW::PIOE->CODR = 3;

				if (mrb.OK && mrb.len > 0)
				{
					parityErr = false;
					s = mrb.data;
					d = rcvBuf;
					c = mrb.len;

					while (c > 0)
					{
						parityErr |= (*s ^ CheckParity(*s >> 1))&1 != 0;

						*d++ = *s++ >> 1;
						c--;
					};

					if (!parityErr && (rcvBuf[0] & manReqMask) == manReqWord)
					{
						if (RequestMan(rcvBuf, mrb.len, &mtb))
						{
							tm.Reset();

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

				}
				else
				{
					i = 0;
				};
			};

			break;

		case 2:

			if (tm.Check(US2RT(500)))
			{
//				SetTrmBoudRate(3); /*mtb.data = tableCRC;*/ mtb.len = 5; SendMLT3(&mtb);
				SendManData(&mtb);

				i++;
			};

			break;

		case 3:

			if (mtb.ready)
			{
				i = 0;
			};

			break;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateADC()			);
		CALL( MainMode()			);
		CALL( SaveVars()			);
		CALL( UpdateTrmReq02()		);
		CALL( UpdateMan()			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateParams()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateRcvTrm()		);
		CALL( qmem.Update()			);
		CALL( UpdateMisc()			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRcv()
{
	REQ *req = 0;

	for (byte i = 1; i <= numStations; i++)
	{
		req = CreateRcvReq04(i, gain[i-1], 200);

		qrcv.Add(req);

		while(!req->ready)
		{
			qrcv.Update();
		};
	};

	req = CreateRcvReq03(0, sampleTime, sampleLen, sampleDelay, 0);
	
	qrcv.Add(req);

	while(!req->ready)
	{
		qrcv.Update();
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTemp()
{
	using namespace HW;

	PMC->PCER0 = PID::ADC_M;

	ADC->MR = 0x0001FF80;
	ADC->ACR = 0x10;
	ADC->CHER = 1<<15;
	ADC->CR = 2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	temperature = (((i32)HW::ADC->CDR[15] - 1787) * 11234) / 65536 + 27;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadVars()
{
	twi.Init(1);

	PointerCRC p(buf);

	dsc.MMR = 0x500200;
	dsc.IADR = 0;
	dsc.CWGR = 0x7575;
	dsc.data = buf;
	dsc.len = sizeof(buf);

	if (twi.Read(&dsc))
	{
		while (twi.Update());
	};

	bool c = false;

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		numDevice = p.ReadW();
		p.ReadArrayB(gain, sizeof(gain));
		p.ReadArrayB(sampleTime, sizeof(sampleTime));
		p.ReadArrayW(sampleLen, ArraySize(sampleLen));
		p.ReadArrayW(sampleDelay, ArraySize(sampleDelay));
		p.ReadW();

		if (p.CRC.w == 0) { c = true; break; };
	};

	if (!c)
	{
		numDevice = 0;

		for (byte i = 0; i < 8; i++)
		{
			gain[i][0] = 0;
			gain[i][1] = 7;
			gain[i][2] = 7;
		};

		sampleTime[0] = 10;
		sampleTime[1] = 20;
		sampleTime[2] = 20;

		sampleLen[0] = 512;
		sampleLen[1] = 512;
		sampleLen[2] = 512;

		sampleDelay[0] = 0;
		sampleDelay[1] = 0;
		sampleDelay[2] = 0;

		savesCount = 2;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	PointerCRC p(buf);

	static byte i = 0;
	static RTM32 tm;

	switch (i)
	{
		case 0:

			if (/*tm.Check(MS2RT(1000)) &&*/ savesCount > 0)
			{
				i++;
			};

			break;

		case 1:

			dsc.MMR = 0x500200;
			dsc.IADR = 0;
			dsc.CWGR = 0x07575; 
			dsc.data = buf;
			dsc.len = sizeof(buf);

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteW(numDevice);
				p.WriteArrayB(gain, sizeof(gain));
				p.WriteArrayB(sampleTime, sizeof(sampleTime));
				p.WriteArrayW(sampleLen, ArraySize(sampleLen));
				p.WriteArrayW(sampleDelay, ArraySize(sampleDelay));
				p.WriteW(p.CRC.w);
			};

			i = (twi.Write(&dsc)) ? (i+1) : 0;

			break;

		case 2:

			if (!twi.Update())
			{
				savesCount--;
				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
//	static byte i = 0;

	InitHardware();

	Init_time();

	LoadVars();

	InitNumStations();

	InitRmemList();

	InitTemp();

	commem.Connect(0, 6250000, 0);
	comtr.Connect(1, 1562500, 0);
//	combf.Connect(3, 6250000, 0);
	comrcv.Connect(2, 6250000, 0);

	InitRcv();


//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

//	__breakpoint(0);

	while(1)
	{
		HW::PIOA->SODR = 1UL<<8;

		UpdateParams();

		HW::PIOA->CODR = 1UL<<8;

		fps++;


		if (rtm.Check(MS2RT(1000)))
		{ 
			UpdateTemp();
			qtrm.Add(CreateTrmReq03());
			fc = fps; fps = 0; 
//			startFire = true;
//			com1.TransmitByte(0);
		};

	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
