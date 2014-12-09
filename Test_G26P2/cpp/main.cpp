#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#include "hardware.h"
#include "time.h"
#include "comport.h"
#include "req.h"

bool run = true;

static ComPort com;
static RequestQueue qcom(&com);

static Req01 req01;
static Rsp01 rsp01;

//static Queue<Req02> qreq02;
//static Queue<Rsp02> qrsp02;

static Req02 req02[32];
static Rsp02 rsp02[32];

static Req03 req03;
static Rsp03 rsp03;

//static QItem<Req02> ireq02[32];
//static QItem<Rsp02> irsp02[32];

static REQ	 q02[32];
static RequestQueue qfreq(0);


static byte comState = 0;
static byte fireType = 0;

static byte sampleTime[3] = { 19, 19, 9};
static byte gain[3] = { 7, 7, 7 };


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReqFire(REQ *q)
{
	comState = 2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReqFire(byte ft)
{
	static REQ q;
	
	q.CallBack = CallBackReqFire;
	q.preTimeOut = 100;
	q.postTimeOut = 1;
	
	q.wb.data = &req01;
	q.wb.len = sizeof(req01)-sizeof(req01.crc);

	q.rb.data = &rsp01;
	q.rb.maxLen = sizeof(rsp01);
	
	req01.func = 1;
	req01.n = ft;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReq02(REQ *q)
{
//	Rsp02 *rsp = (Rsp02*)q->rb->data;
	byte *buf = (byte*)q->rb.data;
	byte n = buf[2]&3;
	byte ch = buf[3]&1;

	if (q->rb.len == q->rb.maxLen)
	{
		DrawWave(ch*2+0, buf+4);
		DrawWave(ch*2+1, buf+4+1000);
	};

	qfreq.Add(q);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq02(byte adr, byte n, byte chnl)
{
	REQ* q = qfreq.Get(); 

	if (q == 0) return 0;

	q->CallBack = CallBackReq02;
	q->preTimeOut = 100;
	q->postTimeOut = 1;
	
	q->wb.len = sizeof(Req02)-2;
	q->rb.maxLen = sizeof(Rsp02);
	
	Req02 &req = *(Req02*)q->wb.data;

	req.adr = adr;
	req.func = 2;
	req.n = n;
	req.chnl = chnl;

	return q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReq03(REQ *q)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReq03(byte dt[], byte ka[])
{
	static REQ q;
	
	q.CallBack = CallBackReq03;
	q.preTimeOut = 100;
	q.postTimeOut = 1;
	
	q.wb.data = &req03;
	q.wb.len = sizeof(req03)-sizeof(req03.crc);

	q.rb.data = &rsp03;
	q.rb.maxLen = sizeof(rsp03);
	
	req03.func = 3;
	req03.dt[0] = dt[0];
	req03.dt[1] = dt[1];
	req03.dt[2] = dt[2];

	req03.ka[0] = ka[0];
	req03.ka[1] = ka[1];
	req03.ka[2] = ka[2];

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

			if (tm.Check(300))
			{
				comState = 0;
			};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init()
{
	for (u32 i = 0; i < ArraySize(req02); i++)
	{
		q02[i].wb.data = &req02[i];
		q02[i].rb.data = &rsp02[i];
		qfreq.Add(&q02[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateKeys(char k)
{
	if (k == '-' || k == '=')
	{
		if (gain[0] > 0 && k == '-') gain[0] -= 1;
		if (gain[0] < 7 && k == '=') gain[0] += 1;
		gain[1] = gain[2] = gain[0];
		qcom.Add(CreateReq03(sampleTime, gain));
	};

	if (k == '[' || k == ']')
	{
		if (sampleTime[0] > 1 && k == '[') sampleTime[0] -= 1;
		if (sampleTime[0] < 19 && k == ']') sampleTime[0] += 1;
		sampleTime[1] = sampleTime[0];
		qcom.Add(CreateReq03(sampleTime, gain));
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	Init();

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

		if (_kbhit())
		{
			UpdateKeys(_getch());
		};
	};
}
