#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "req.h"

ComPort com1;
ComPort combf;
ComPort comrcv;

//ComPort::WriteBuffer wb;
//ComPort::ReadBuffer rb;


u32 fc = 0;

static u32 bfCRCOK = 0;
static u32 bfCRCER = 0;

//static u32 rcvCRCOK = 0;
//static u32 rcvCRCER = 0;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


Response rsp;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



static RequestQuery reqQuery(&comrcv);

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
	static byte i = 0, j = 0;
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
			
//			if (rtm.Check(MS2RT(20)))
			{ 
				req.adr = 0;
				req.func = 1;
				req.f1.n = 0;

				req.f1.crc = GetCRC16(&req, sizeof(req.f1));

				wb.data = &req;
				wb.len = sizeof(req.f1) + 2;

				comrcv.Write(&wb);
				i++;
			};

			break;

		case 1:

			if (!comrcv.Update())
			{
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
				j = 0;
				i++;
			};

			break;

		case 4:

			req.adr = 1;
			req.func = 2;
			req.f2.n = 0;
			req.f2.chnl = j&3;

			req.f2.crc = GetCRC16(&req, sizeof(req.f2));

			wb.data = &req;
			wb.len = sizeof(req.f2) + 2;

			comrcv.Write(&wb);
			i++;

			break;

		case 5:

			if (!comrcv.Update())
			{
				rb.data = &rsp;
				rb.maxLen = sizeof(rsp);
				comrcv.Read(&rb, MS2RT(10), 1);
				i++;
			};

			break;

		case 6:

			if (!comrcv.Update())
			{
				j++;

				i = (j < 32) ? 4 : 0;
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
