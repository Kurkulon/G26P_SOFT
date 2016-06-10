#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"
#include "req.h"

static ComPort com1;
static ComPort::WriteBuffer wb;
static ComPort::ReadBuffer rb;

//static byte data[256*48];

static u32 rdata[100]; 
static u16 tdata[100];

static u16 reqManData[100];
static u16 rspManData[4096];

static bool ready1 = false, ready2 = false;

static MRB mrb;
static MTB mtb;

static u32 CRCOK = 0;
static u32 CRCER = 0;

static u16 manReqWord = 0xAA00;
static u16 manReqMask = 0xFF00;

static RequestQuery qcom(&com1);

static byte stateMan = 0;

static u32 pt = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CallBackReqMan(REQ *q)
{
	if (q->rb.len >= 4)
	{
		mtb.data = rspManData+1;
		mtb.len = (q->rb.len-2) >> 1;
		pt = GetRTT();
		stateMan = 3;
	}
	else
	{
		stateMan = 0;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

REQ* CreateReqMan(void *data, u16 len)
{
	static REQ q;
	
	q.CallBack = CallBackReqMan;
	q.preTimeOut = MS2CLK(10);
	q.postTimeOut = US2CLK(7.2);
	
	q.wb.data = data;
	q.wb.len = len;

	q.rb.data = rspManData;
	q.rb.maxLen = sizeof(rspManData);
	
	return &q;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestMan(MRB *mrb)
{


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	bool parityErr;
	u32 *s;
	u16 *d;

	u16 c;

	switch (stateMan)
	{
		case 0:

			mrb.data = rdata;
			mrb.maxLen = ArraySize(rdata);
			RcvManData(&mrb);

			stateMan++;

			break;

		case 1:

			if (mrb.ready)
			{
				parityErr = false;
				s = mrb.data;
				d = reqManData;
				c = mrb.len;

				*d++ = 0x5501;

				while (c > 0)
				{
					parityErr |= (*s ^ CheckParity(*s >> 1))&1 != 0;

					*d++ = *s++ >> 1;
					c--;
				};

				if (!parityErr && (reqManData[1] & manReqMask) == manReqWord)
				{
					if ((reqManData[1] & 0xFF) == 0x80 && reqManData[2] == 2 && reqManData[3] < 5)
					{
						SetTrmBoudRate(reqManData[3]);

						rspManData[1] = manReqWord|0x80;
						mtb.data = rspManData+1;
						mtb.len = 1;
						pt = GetRTT();
						stateMan = 3;
					}
					else
					{
						qcom.Add(CreateReqMan(reqManData, (mrb.len+1) << 1));
						stateMan = 2;
					};
				}
				else
				{
					stateMan = 0;
				};
			};

			break;

		case 2:

			break;

		case 3:

			if ((GetRTT() - pt) >= US2CLK(500))
			{
				stateMan = (SendManData(&mtb)) ? 4 : 0;
			};

			break;

		case 4:

			if (mtb.ready)
			{
				stateMan = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;

	static u32 pt = 0;
	static byte c = 0;

	TM32 tm;

	InitHardware();

	com1.Connect(6250000, 0);

	//for (u16 i = 0; i < sizeof(data); i++) { data[i] = i; };
	//wb.data = data;
	//wb.len = sizeof(data);

	//com1.Write(&wb);
//	ReadSPORT(spd1, spd2, sizeof(spd1), sizeof(spd2), 19, &ready1, &ready2);

	//*pPORTFIO_SET = 1<<5;
	//*pPORTFIO_CLEAR = 1<<7;


	while (1)
	{
//		*pPORTFIO_TOGGLE = 1<<2;

		//u32 t = GetRTT();

		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( qcom.Update();		);
			CALL( UpdateMan();			);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL


		

//		UpdateHardware();

		//if (!com1.Update())
		//{
		//	com1.Write(&wb);
		//};


//		*pPORTFIO_TOGGLE = 1<<2;

	};

	return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//bool Request(MRB *mrb, MTB *mtb)
//{
//	if (!mrb->OK || mrb->data == 0 || mrb->len == 0) return false;
//
//	bool result = false;
//
//	u32 *p = (u32*)mrb->data;
//
//	u16 func = (u16)(p[0]>>1);
//	u16 t;
//
//	tdata[0] = func;
//	mtb->data = tdata;
//	mtb->len = 1;
//
//	switch (func)
//	{
//		case 0xA900:
//
//			tdata[0] = 0xA900;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			mtb->data = tdata;
//			mtb->len = 3;
//			result = true;
//
//			break;
//
//		case 0xA910:
//
//			tdata[0] = 0xA910;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			tdata[6] = 1;
//			tdata[7] = 1;
//			tdata[8] = 1;
//			tdata[9] = 1;
//			tdata[10] = 1;
//			mtb->data = tdata;
//			mtb->len = 11;
//			result = true;
//
//			break;
//
//		case 0xA920:
//
//			tdata[0] = 0xA920;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			tdata[6] = 1;
//			tdata[7] = 1;
//			tdata[8] = 1;
//			tdata[9] = 1;
//			tdata[10] = 1;
//			mtb->data = tdata;
//			mtb->len = 11;
//			result = true;
//
//			break;
//
//		case 0xA930:
//		case 0xA931:
//		case 0xA932:
//		case 0xA933:
//		case 0xA934:
//		case 0xA935:
//		case 0xA936:
//		case 0xA937:
//		case 0xA938:
//		case 0xA939:
//
//			tdata[0] = func;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			mtb->data = tdata;
//			mtb->len = 6;
//
//			result = true;
//
//			if (mrb->len == 1)
//			{
//				mtb->len += 2048;
//			}
//			else if (mrb->len == 3)
//			{
//				mtb->len += (u16)(p[2]>>1);
//			};
//
//			break;
//
//		case 0xA990:
//
//			t = (u16)(p[1]>>1);
//
//			if (mrb->len >= 3 && (t < 3))
//			{
//				result = true;
//
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				if (t == 2)
//				{
//					t = (u16)(p[2]>>1);
//
//					if (t < 4)
//					{
//						SetTrmBoudRate(t);
//						result = true;
//					}
//					else
//					{
//						result = false;
//					};
//				};
//			};
//
//			break;
//
//		case 0xA9A0:
//
//			if (mrb->len >= 3)
//			{
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				result = true;
//			};
//
//			break;
//
//		case 0xA9F0:
//
//			if (mrb->len >= 1)
//			{
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				result = true;
//			};
//
//			break;
//
//		default:
//
//			result = false;
//
//			break;
//	};
//
//	return result;
//
//
//};
