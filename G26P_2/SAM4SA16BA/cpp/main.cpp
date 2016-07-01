#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "req.h"

#include "list.h"

ComPort commem;
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
static RequestQuery qmem(&commem);

static R02 r02[8][3][4];
static RMEM rmem[96];
static List<RMEM> lstRmem;
static List<RMEM> freeRmem;

static byte fireType = 0;

static byte gain[8][3];// = { { 7, 7, 7, 7, 7, 7, 7, 7 }, { 7, 7, 7, 7, 7, 7, 7, 7 }, { 7, 7, 7, 7, 7, 7, 7, 7 } };
static byte sampleTime[3] = { 9, 19, 19};
static u16 sampleLen[3] = { 512, 512, 512};
static u16 sampleDelay[3] = { 0, 0, 0};


static u16 manReqWord = 0xAA00;
static u16 manReqMask = 0xFF00;

static u16 numDevice = 0;
static u16 verDevice = 1;

static u32 manCounter = 0;

static u16 adcValue = 0;
static U32u filtrValue;
static u16 resistValue = 0;
static byte numStations = 0;

static byte mainModeState = 0;

//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;

static u32 chnlCount[4] = {0};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//Response rsp;
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
	q.ready = false;
	
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

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq02(byte adr, byte n, byte chnl)
{
	adr = (adr-1)&7; 
	chnl &= 3; n %= 3;

	R02 &r = r02[adr][n][chnl];

	if (r.memNeedSend)
	{
		return 0;
	};

	Req02 &req = r.req;
	Rsp02 &rsp = r.rsp;
	
	ComPort::WriteBuffer &wb = r02[adr][n][chnl].wb;
	ComPort::ReadBuffer	 &rb = r02[adr][n][chnl].rb;
	
	REQ &q = r02[adr][n][chnl].q;


	q.CallBack = CallBackRcvReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	
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

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReq03(REQ *q)
{
//	Rsp03 *rsp = (Rsp03*)q->rb->data;


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq03(byte adr, byte st[], u16 sl[], u16 sd[])
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
	q.ready = false;
	
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
//	Rsp03 *rsp = (Rsp03*)q->rb->data;


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateRcvReq04(byte adr, byte ka[])
{
	static Req04 req;
	static Rsp04 rsp;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackRcvReq04;
	q.preTimeOut = MS2RT(10);
	q.postTimeOut = 1;
	q.rb = &rb;
	q.wb = &wb;
	q.ready = false;
	
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
	wb.len = sizeof(req);
	
	req.func = 1;
	req.n = n+1;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRmemList()
{
	for (u16 i = 0; i < ArraySize(rmem); i++)
	{
		freeRmem.Add(&rmem[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackMemReq01(REQ *q)
{
	if (q != 0)
	{
		RMEM* rm = (RMEM*)q->ptr;
	
		if (rm != 0)
		{
			if (!HW::RamCheck(rm))
			{
				__breakpoint(0);
			};

			freeRmem.Add(rm); 
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//u32 countMemReq = 0;

//RMEM reqMem;

REQ* CreateMemReq01()
{
	RMEM* rm = freeRmem.Get();

	if (rm == 0) return 0;

	ReqMem &req = rm->req;
//	RspMem &rsp = rm->rsp;
	
	ComPort::WriteBuffer &wb = rm->wb;
//	ComPort::ReadBuffer	 &rb = rm->rb;
	
	REQ &q = rm->q;


	q.CallBack = CallBackMemReq01;
	q.rb = 0;//&rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	q.ptr = rm;
	
	wb.data = &req;
	wb.len = 4;//sizeof(req);

	//rb.data = &rsp;
	//rb.maxLen = sizeof(rsp);
	//rb.recieved = false;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackMemReq02(REQ *q)
{
//	memBusy = false;

	if (q != 0)
	{
		RMEM* rm = (RMEM*)q->ptr;
	
		if (rm != 0)
		{
			if (rm->r02 != 0)
			{
				rm->r02->memNeedSend = false;
			};

			if (!HW::RamCheck(rm))
			{
				__breakpoint(0);
			};

			freeRmem.Add(rm); 
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//u32 countMemReq = 0;

//RMEM reqMem;

REQ* CreateMemReq02(byte adr, byte n, byte chnl)
{
	adr = (adr-1)&7; 
	chnl &= 3; n %= 3;

	RMEM* rm = freeRmem.Get();

	if (rm == 0) return 0;

	R02 &r = r02[adr][n][chnl];

	rm->r02 = &r;

	r.memNeedSend = true;

//	ReqMem &req = rm->req;
	RspMem &rsp = rm->rsp;
	
	ComPort::WriteBuffer &wb = rm->wb;
	ComPort::ReadBuffer	 &rb = rm->rb;
	
	REQ &q = rm->q;


	q.CallBack = CallBackMemReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	q.ready = false;
	q.ptr = rm;
	
	wb.data = &r.rsp;
	wb.len = sizeof(r.rsp);

	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);
	rb.recieved = false;

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
		rsp.t1.g[i] = gain[0][i];
		rsp.t2.g[i] = gain[1][i];
		rsp.t3.g[i] = gain[2][i];
	};

	rsp.t1.st = sampleTime[0];
	rsp.t1.len = sampleLen[0];
	rsp.t1.delay = 0;
	rsp.t1.voltage = 800;

	rsp.t2.st = sampleTime[1];
	rsp.t2.len = sampleLen[1];
	rsp.t2.delay = 0;
	rsp.t2.voltage = 800;

	rsp.t3.st = sampleTime[2];
	rsp.t3.len = sampleLen[2];
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
	__packed struct Req { u16 rw; u16 off; u16 len; };

	__packed struct Hdr { u32 cnt; u16 gain; u16 st; u16 len; u16 delay; };

	__packed struct St { Hdr h; u16 data[2000]; };

	__packed struct Rsp { u16 hdr; u16 rw; St st; };
	
	static Rsp rsp; 

	Hdr hdr;


	const u16 sz = sizeof(rsp.st)/2;

	Req &req = *((Req*)data);

	//off 0...2005


	if (wb == 0 || !(len == 1 || len == 3)) return false;

	byte nf = ((req.rw>>4)-3)&3;
	byte nr = req.rw & 7;

	u16 *p = (u16*)&rsp.st;

	u16 c = 0;
	u16 off = 0;
	u16 diglen = 500;
	u16 hdrlen = (sizeof(hdr)/2);

	if (len == 1)
	{
		off = 0;
		c = sz;
	}
	else
	{
		off = req.off;
		c = req.len;

		if (off >= sz)
		{
			c = 0; off = 0;
		}
		else if ((off+c) > sz)
		{
			c = sz-off;
		};
	};

	wb->data = &rsp;
	wb->len = (c+2)<<1;

	if (off < hdrlen)
	{
		hdr.cnt = manCounter;
		hdr.gain = gain[nf][nr];
		hdr.st = sampleTime[nf];
		hdr.len = diglen;
		hdr.delay = 0;

		u16 *s = ((u16*)&hdr) + off;

		u16 l = hdrlen - off;

		c -= l;
		off = 0;

		while (l-- > 0) { *p++ = *s++;};
	}
	else
	{
		off -= hdrlen;
	};

	byte chnl = off / diglen;

	u16 i = 0;

	u16 j = off % diglen;

	while (c > 0)
	{
		chnlCount[chnl]++;

		u16 k = diglen - j;

		if (c < k)
		{
			k = c;
			c = 0;
		}
		else
		{
			c -= k;
		};

		while (k-- > 0)
		{
			*p++ = r02[nr][nf][chnl].rsp.data[j++] + chnl*15000; 
		};

		j = 0;
		chnl++;
	};

	rsp.hdr = 0x5501;
	rsp.rw = req.rw;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[2];

	if (wb == 0) return false;

	rsp[0] = 0x5501;
	rsp[1] = manReqWord|0x80;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[2];

	if (wb == 0 || len < 3) return false;


	byte nf = ((data[1]>>4) & 3)-1;
	byte nr = data[1] & 0xF;
	byte st;
	u16 sl;

	if (nf > 2) return false;

	if (nr < 8)
	{
		gain[nr][nf] = data[2]&7;
		
		qrcv.Add(CreateRcvReq04(nr+1, gain[nr]));
	}
	else
	{
		switch(nr)
		{
			case 0x8:

				st = data[2] & 0xFF;
				if (st > 0) st -= 1;
				sampleTime[nf] = st;

				break;

			case 0x9:

				sl = data[2];
				if (sl > 512) sl = 512;
				sampleLen[nf] = sl;

				break;

			case 0xA:


				break;

			case 0xB:


				break;
		};

		qrcv.Add(CreateRcvReq03(0, sampleTime, sampleLen, sampleDelay));
	};


	rsp[0] = 0x5501;
	rsp[1] = manReqWord|0x90;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[2];

	if (wb == 0) return false;

	rsp[0] = 0x5501;
	rsp[1] = manReqWord|0xF0;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
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

	numStations = resistValue / 1000;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void MainMode()
{
	//fireType

	static byte rcv = 0;
	static byte chnl = 0;
	static REQ *req = 0;
	static RTM32 rt;

	REQ *rm = 0;

	switch (mainModeState)
	{
		case 0:

			startFire = true;
			
			mainModeState++;

			break;

		case 1:

			if (!startFire)
			{
				rcv = 1; chnl = 0;

				mainModeState++;
			};

			break;

		case 2:

			req = CreateRcvReq02(rcv, fireType, chnl);

			if (req != 0)
			{
				qrcv.Add(req);

				mainModeState++;
			};

			break;

		case 3:

			if (req->ready)
			{

				rm = CreateMemReq02(rcv, fireType, chnl);

				if (rm == 0) break;

				qmem.Add(rm);

				if (chnl < 3)
				{
					chnl += 1;

					mainModeState = 2;
				}
				else if (rcv < numStations)
				{
					manCounter++;

					rcv += 1;
					chnl = 0;

					mainModeState++;
				}
				else
				{
					mainModeState = 5;
				};

				rt.Reset();
			};

			break;

		case 4:

			if (rt.Check(US2RT(1)))
			{
				mainModeState = 2;
			};

			break;

		case 5:

			if (rt.Check(MS2RT(15)))
			{
				fireType = (fireType+1) % 3; 

				mainModeState = 0;
			};

			break;

		case 6:

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
		CALL( qmem.Update()			);
		CALL( UpdateADC()			);
		CALL( MainMode()			);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL

	//REQ* req = CreateMemReq02(1,1,1);

	//if (req != 0)
	//{
	//	qmem.Add(req);
	//};

	//ComPort::WriteBuffer wb;

	//wb.data = &r02[0][0][0];
	//wb.len = 1000;

	////if (!commem.Update())
	////{
	////	commem.Write(&wb);
	////};

	//HW::PIOA->SODR = 1UL<<31;
	//HW::UART0->CR = 0x40;

	//if (HW::UART0->SR & 2)
	//{
	//	HW::UART0->THR = 0x55;
	//};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
//	static byte i = 0;

	RTT_Init();

	InitNumStations();

	InitRmemList();

	commem.Connect(0, 6250000, 0);
	comtr.Connect(1, 1562500, 0);
	combf.Connect(3, 6250000, 0);
	comrcv.Connect(2, 6250000, 0);

//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

//	__breakpoint(0);

	while(1)
	{
//		HW::PIOA->SODR = 1UL<<31;

		UpdateMisc();

//		HW::PIOA->CODR = 1UL<<31;

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
