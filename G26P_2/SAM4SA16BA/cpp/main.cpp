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

static RequestQuery qrcv(&comrcv);

static R02 r02[8][3][2];



//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Response rsp;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReq01(REQ *q)
{
	waitSync = true;;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq01(byte adr, byte n)
{
	static Req01 req;
	static ComPort::WriteBuffer wb;
	static REQ q;

	q.CallBack = CallBackReq01;
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

void CallBackReq02(REQ *q)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq02(byte adr, byte n, byte chnl)
{
	adr = (adr-1)&7; chnl &= 1; n = (n+1)&3 - 1;

	Req02 &req = r02[adr][n][chnl].req;
	Rsp02 &rsp = r02[adr][n][chnl].rsp;
	
	ComPort::WriteBuffer &wb = r02[adr][n][chnl].wb;
	ComPort::ReadBuffer	 &rb = r02[adr][n][chnl].rb;
	
	REQ &q = r02[adr][n][chnl].q;


	q.CallBack = CallBackReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = MS2RT(1);
	q.postTimeOut = 1;
	
	wb.data = &req;
	wb.len = sizeof(req)-sizeof(req.crc);

	rb.data = &rsp;
	rb.maxLen = sizeof(rsp);
	
	req.adr = 1;//adr;
	req.func = 2;
	req.n = n;
	req.chnl = chnl;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReq03(REQ *q)
{
	Rsp03 *rsp = (Rsp03*)q->rb->data;


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq03(byte adr, byte dt[], byte ka[])
{
	static Req03 req;
	static Rsp03 rsp;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static REQ q;

	q.CallBack = CallBackReq03;
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
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


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
				if (rb.recieved && rb.len > 0)
				{
					if (GetCRC16(rb.data, rb.len) == 0)	bfCRCOK++;	else	bfCRCER++;


					wb.data = rb.data;
					wb.len = rb.len;

					DataPointer p(wb.data);
					p.b += wb.len;
					*p.w = GetCRC16(wb.data, wb.len);
					wb.len += 2;

					combf.Write(&wb);
					i++;
				}
				else
				{
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

static void UpdateRecievers()
{
	static byte i = 0, n = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];
	static 	RTM32 rtm;
//	static RTM32 rt2;

	static Request req;
			
	DataPointer p(buf);

	switch(i)
	{
		case 0:
			
			waitSync = false;
			
			qrcv.Add(CreateReq01(0, n));

			i++;

			break;

		case 1:

			qrcv.Update();
		
			if (waitSync)
			{
				waitSync = false;
				rtm.pt = GetRTT();
				i++;
			};

			break;

		case 2:

			if (rtm.Check(3))
			{
				comrcv.TransmitByte(0);
				i++;
			};

			break;

		case 3:

			if (rtm.Check(MS2RT(15)))
			{
				i++;
			};

			break;

		case 4:

			for (byte j = 1; j < 9; j++)
			{
				qrcv.Add(CreateReq02(j, n, 0));
				qrcv.Add(CreateReq02(j, n, 1));
			};

			i++;

			break;

		case 5:

			qrcv.Update();

			if (qrcv.Ready())
			{
				i++;
			}

			break;

		case 6:

			if (rtm.Check(MS2RT(75)))
			{
				i = 0;
			};

			break;
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTransmiter()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	static RTM32 rtm;
	
	DataPointer p(buf);

	switch(i)
	{
		case 0:
			//wb.data = buf;
			//wb.len = 10;
			//p.b += wb.len;

			//*p.w = GetCRC16(wb.data, wb.len);
			//wb.len += 2;
			//comtr.Write(&wb);

			comtr.TransmitByte(0);
			
			i++;

			break;

		case 1:

			//if (!comtr.Update())
			{
				i++;
			};

			break;

		case 2:

			if (rtm.Check(MS2RT(1000)))
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
		CALL( UpdateRecievers()		);
		CALL( UpdateTransmiter()	);
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


	com1.Connect(0, 6250000, 0);
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

		if (rtm.Check(32768))
		{ 
			fc = fps; fps = 0; 
//			com1.TransmitByte(0);
		};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
