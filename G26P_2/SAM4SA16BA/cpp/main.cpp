#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "req.h"

ComPort com1;
ComPort comtr;
ComPort combf;
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

static R02 r02[8][3][2];

static byte fireType = 0;

static byte sampleTime[3] = { 19, 19, 9};
static byte gain[3] = { 7, 7, 7 };

static u16 manReqWord = 0xAA00;
static u16 manReqMask = 0xFF00;

static u16 numDevice = 0;
static u16 verDevice = 1;

static u32 manCounter = 0;

//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Response rsp;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReqFire(REQ *q)
{
//	waitSync = true;;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReqFire(byte adr, byte n)
{
	static Req01 req;
	static ComPort::WriteBuffer wb;
	static REQ q;

	q.CallBack = CallBackRcvReqFire;
	q.rb = 0;
	q.wb = &wb;
	
	wb.data = &req;
	wb.len = sizeof(req)-sizeof(req.crc);
	
	req.adr = adr;
	req.func = 1;
	req.n = n;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq02(REQ *q)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq02(byte adr, byte n, byte chnl)
{
	adr = (adr-1)&7; //chnl &= 1; n = (n+1)&3 - 1;

	Req02 &req = r02[adr][n][chnl].req;
	Rsp02 &rsp = r02[adr][n][chnl].rsp;
	
	ComPort::WriteBuffer &wb = r02[adr][n][chnl].wb;
	ComPort::ReadBuffer	 &rb = r02[adr][n][chnl].rb;
	
	REQ &q = r02[adr][n][chnl].q;


	q.CallBack = CallBackRcvReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	
	wb.data = &req;
	wb.len = sizeof(req)-sizeof(req.crc);

	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);
	rb.recieved = false;
	
	req.adr = 1;//adr;
	req.func = 2;
	req.n = n;
	req.chnl = chnl;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq03(REQ *q)
{
//	Rsp03 *rsp = (Rsp03*)q->rb->data;


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq03(byte adr, byte dt[], byte ka[])
{
	static Req03 req;
	static Rsp03 rsp;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackRcvReq03;
	q.preTimeOut = MS2RT(10);
	q.postTimeOut = 1;
	q.rb = &rb;
	q.wb = &wb;
	
	wb.data = &req;
	wb.len = sizeof(req)-sizeof(req.crc);
	
	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);

	req.adr = adr;
	req.func = 3;
	req.dt[0] = dt[0];
	req.dt[1] = dt[1];
	req.dt[2] = dt[2];
	req.ka[0] = ka[0];
	req.ka[1] = ka[1];
	req.ka[2] = ka[2];
	
	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackTrmReqFire(REQ *q)
{
//	waitSync = true;;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateTrmReqFire(byte n)
{
	static __packed struct { byte func; byte n; word crc; } req;

	static ComPort::WriteBuffer wb;
	static REQ q;

	q.CallBack = CallBackTrmReqFire;
	q.rb = 0;
	q.wb = &wb;
	
	wb.data = &req;
	wb.len = sizeof(req)-sizeof(req.crc);
	
	req.func = 1;
	req.n = n+1;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_00(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[4];

	if (wb == 0) return false;

	rsp[0] = 0x5501;
	rsp[1] = manReqWord;
	rsp[2] = numDevice;
	rsp[3] = verDevice;

	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	__packed struct T { u16 g[8]; u16 st; u16 len; u16 delay; u16 voltage; };
	__packed struct Rsp { u16 hdr; u16 rw; T t1, t2, t3; };
	
	static Rsp rsp;

	if (wb == 0) return false;

	rsp.hdr = 0x5501;
	rsp.rw = manReqWord|0x10;

	for (byte i = 0; i < 8; i++)
	{
		rsp.t1.g[i] = gain[0];
		rsp.t2.g[i] = gain[1];
		rsp.t3.g[i] = gain[2];
	};

	rsp.t1.st = sampleTime[0];
	rsp.t1.len = 500;
	rsp.t1.delay = 0;
	rsp.t1.voltage = 800;

	rsp.t2.st = sampleTime[1];
	rsp.t2.len = 500;
	rsp.t2.delay = 0;
	rsp.t2.voltage = 800;

	rsp.t3.st = sampleTime[2];
	rsp.t3.len = 500;
	rsp.t3.delay = 0;
	rsp.t3.voltage = 800;

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[4];

	if (wb == 0) return false;

	rsp[0] = 0x5501;
	rsp[1] = manReqWord|0x20;
	rsp[2] = GD(&manCounter, u16, 0);
	rsp[3] = GD(&manCounter, u16, 1);
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_30(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	__packed struct Rsp { u16 hdr; u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[2048]; };
	
	static Rsp rsp; 

	if (wb == 0 || !(len == 1 || len == 3)) return false;

	u16 c = (len == 1) ? 2048 : data[2];

	if (c > 2048) return false;

	rsp.hdr = 0x5501;
	rsp.rw = *data;

	rsp.cnt = manCounter;
	rsp.gain = gain[((rsp.rw>>4)-3)&3];
	rsp.st = sampleTime[((rsp.rw>>4)-3)&3];
	rsp.len = 500;
	rsp.delay = 0;

	wb->data = &rsp;
	wb->len = 18 + (c<<1);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	u16 *p = (u16*)rb->data;
	bool r = false;

	u16 t = p[1];

	if ((t & manReqMask) != manReqWord || rb->len < 6)
	{
		bfERC++; 
		return false;
	};

	u16 len = (rb->len - 4)>>1;

	t = (t>>4) & 0xF;

	switch (t)
	{
		case 0: 	r = RequestMan_00(p+1, len, wb); break;
		case 1: 	r = RequestMan_10(p+1, len, wb); break;
		case 2: 	r = RequestMan_20(p+1, len, wb); break;
		case 3: 
		case 4: 
		case 5: 	r = RequestMan_30(p+1, len, wb); break;
		case 8: 	r = RequestMan_80(p+1, len, wb); break;
		case 9:		r = RequestMan_90(p+1, len, wb); break;
		case 0xF:	r = RequestMan_F0(p+1, len, wb); break;
		
		default:	bfURC++; 
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestBF(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	byte *p = (byte*)rb->data;
	bool r = false;

	switch(*p)
	{
		case 1:	// Запрос Манчестер

			r = RequestMan(wb, rb);

			break;

	};

	return r;
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
			combf.Read(&rb, -1, 1);
			i++;

			break;

		case 1:

			if (!combf.Update())
			{
				if (rb.recieved && rb.len > 0 && GetCRC16(rb.data, rb.len) == 0)
				{
					if (RequestBF(&wb, &rb))
					{
						combf.Write(&wb);
						i++;
					}
					else
					{
						i = 0;
					};

					bfCRCOK++;
				}
				else
				{
					bfCRCER++;
					i = 0;
				};
			};

			break;

		case 2:

			if (!combf.Update())
			{
				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateRcvTrm()
{
	static byte i = 0, n = 0;
	static 	RTM32 rtm;
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

			reqr = CreateRcvReqFire(0, n);
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
				comrcv.TransmitByte(0);
				comtr.TransmitByte(0);
				i++;
			};

			break;

		case 7:

			if (rtm.Check(MS2RT(15)))
			{
				i++;
			};

			break;

		case 8:

			for (byte j = 1; j < 9; j++)
			{
				qrcv.Add(CreateRcvReq02(j, n, 0));
				qrcv.Add(CreateRcvReq02(j, n, 1));
			};

			startFire = false;

			qrcv.Start();
			qtrm.Start();

			i = 0;

			break;
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestTestCom01(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	__packed struct Req { byte func; byte ft; u16 crc; };
	__packed struct Rsp { byte func; u16 crc; };

	Req *req = (Req*)rb->data;

	static Rsp rsp;

	if (rb->len < 2) return false;

	if (req->ft <= 2)
	{
		fireType = req->ft;
	};

	rsp.func = 1;

	rsp.crc = GetCRC16(&rsp, 1);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	startFire = true;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestTestCom02(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	__packed struct Req { byte func; byte adr; byte n; byte ch; u16 crc; };

	Req *req = (Req*)rb->data;

	if (rb->len < 4) return false;
	if (req->adr < 1 || req->adr > 8 || req->n > 2 || req->ch > 1) return false;

	R02 &r = r02[req->adr-1][req->n][req->ch];

	wb->data = &r.rsp;
	wb->len = sizeof(r.rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestTestCom03(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	__packed struct Req { byte func; byte dt[3]; byte ka[3]; word crc; };
	__packed struct Rsp { byte func; u16 crc; };

	Req *req = (Req*)rb->data;

	static Rsp rsp;

	if (rb->len < 7) return false;

	sampleTime[0] = req->dt[0];
	sampleTime[1] = req->dt[1];
	sampleTime[2] = req->dt[2];

	gain[0] = req->ka[0];
	gain[1] = req->ka[1];
	gain[2] = req->ka[2];

	qrcv.Add(CreateRcvReq03(0, sampleTime, gain));

	rsp.func = 3;
	rsp.crc = GetCRC16(&rsp, 1);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestTestCom(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	byte *pb = (byte*)rb->data;

	switch (pb[0])
	{
		case 1 : return RequestTestCom01(rb, wb);
		case 2 : return RequestTestCom02(rb, wb);
		case 3 : return RequestTestCom03(rb, wb);
		default : return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateCom1()
{
	static byte i = 0;

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb; // = r02[0][0][1].rb;
	static byte buf[32];

	static RTM32 rtm;


	switch(i)
	{
		case 0:
	
			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com1.Read(&rb, -1, 2);

			i++;

			break;

		case 1:

			if (!com1.Update())
			{
				i = (RequestTestCom(&rb, &wb)) ? i+1 : 0;
			};

			break;

		case 2:

			com1.Write(&wb);

			i++;

			break;

		case 3: 

			if (!com1.Update())
			{
				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateBlackFin()		);
		CALL( UpdateRcvTrm()		);
		CALL( UpdateCom1()			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


int main()
{
//	static byte i = 0;

	RTT_Init();


	com1.Connect(0, 921600, 0);
	comtr.Connect(1, 1562500, 0);
	combf.Connect(3, 6250000, 0);
	comrcv.Connect(2, 6250000, 0);

//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

	while(1)
	{
		UpdateMisc();

		fps++;

		if (rtm.Check(MS2RT(500)))
		{ 
			fc = fps; fps = 0; 
//			startFire = true;
//			com1.TransmitByte(0);
		};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
