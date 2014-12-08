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

static bool waitSync = false;
static bool startFire = false;

static RequestQuery qrcv(&comrcv);
static RequestQuery qtrm(&comtr);

static R02 r02[8][3][2];

static byte fireType = 0;


//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Response rsp;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackRcvReqFire(REQ *q)
{
	waitSync = true;;
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
	Rsp03 *rsp = (Rsp03*)q->rb->data;


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
	waitSync = true;;
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
	req.n = n;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	//static byte i = 0;
	//static ComPort::WriteBuffer wb;
	//static ComPort::ReadBuffer rb;
	//static byte buf[1024];

	//switch(i)
	//{
	//	case 0:

	//		rb.data = buf;
	//		rb.maxLen = sizeof(buf);
	//		combf.Read(&rb, -1, 1);
	//		i++;

	//		break;

	//	case 1:

	//		if (!combf.Update())
	//		{
	//			if (rb.recieved && rb.len > 0)
	//			{
	//				if (GetCRC16(rb.data, rb.len) == 0)	bfCRCOK++;	else	bfCRCER++;


	//				wb.data = rb.data;
	//				wb.len = rb.len;

	//				DataPointer p(wb.data);
	//				p.b += wb.len;
	//				*p.w = GetCRC16(wb.data, wb.len);
	//				wb.len += 2;

	//				combf.Write(&wb);
	//				i++;
	//			}
	//			else
	//			{
	//				i = 0;
	//			};
	//		};

	//		break;

	//	case 2:

	//		if (!combf.Update())
	//		{
	//			i = 0;
	//		};

	//		break;
	//};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateRcvTrm()
{
	static byte i = 0, n = 0;
	static 	RTM32 rtm;
//	static RTM32 rt2;

	static Request req;

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

bool RequestTestCom(ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	byte *pb = (byte*)rb->data;

	switch (pb[0])
	{
		case 1 : return RequestTestCom01(rb, wb);
		case 2 : return RequestTestCom02(rb, wb);
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
