#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#include "hardware.h"
#include "time.h"
#include "comport.h"
#include "req.h"

bool run = true;

static ComPort com;
static RequestQuery qcom(&com);

static Req01 req01;
static Rsp01 rsp01;

static Req02 req02;
static Rsp02 rsp02;

static byte comState = 0;
static byte fireType = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReqFire(REQ *q)
{
	comState = 2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReqFire(byte ft)
{
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	
	static REQ q;
	
	q.CallBack = CallBackReqFire;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = 100;
	q.postTimeOut = 1;
	
	wb.data = &req01;
	wb.len = sizeof(req01)-sizeof(req01.crc);

	rb.data = &rsp01;
	rb.maxLen = sizeof(rsp01);
	
	req01.func = 1;
	req01.n = ft;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReq02(REQ *q)
{
//	Rsp02 *rsp = (Rsp02*)q->rb->data;
	byte *buf = (byte*)q->rb->data;

	if (q->rb->len == q->rb->maxLen)
	{
		DrawWave(0, buf+4);
		DrawWave(1, buf+4+1000);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq02(byte adr, byte n, byte chnl)
{
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	
	static REQ q;

	q.CallBack = CallBackReq02;
	q.rb = &rb;
	q.wb = &wb;
	q.preTimeOut = 100;
	q.postTimeOut = 1;
	
	wb.data = &req02;
	wb.len = sizeof(req02)-sizeof(req02.crc);

	rb.data = &rsp02;
	rb.maxLen = sizeof(rsp02);
	
	req02.adr = adr;
	req02.func = 2;
	req02.n = n;
	req02.chnl = chnl;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void UpdateCom()
{
	static TM32 tm;

	switch(comState)
	{
		case 0:

			qcom.Add(CreateReqFire(fireType));
			comState++;

			break;

		case 1:

			qcom.Update();

			//if (qcom.Idle())
			//{
			//	comState++;
			//};

			break;

		case 2:
	
			tm.Reset();

			comState++;

			break;


		case 3:

			if (tm.Check(30))
			{
				qcom.Add(CreateReq02(1, fireType, 0));
				qcom.Add(CreateReq02(1, fireType, 1));

				comState++;
			};

			break;

		case 4:

			qcom.Update();

			if (qcom.Idle())
			{
				tm.Reset();
				comState++;
			};

			break;

		case 5:

			if (tm.Check(1))
			{
				comState = 0;
			};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	com.Connect("COM2", 921600, 0);
	
	while(1)
	{
		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateHardware();		);
			CALL( UpdateCom();			);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL
	};
}
