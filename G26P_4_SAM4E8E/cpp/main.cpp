#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"
#include "xtrap.h"
#include "flash.h"
#include "vector.h"
#include "list.h"
#include "main.h"

#pragma diag_suppress 546,550,177

#pragma O3
#pragma Otime


u32 fps = 0;

//extern byte Heap_Mem[10];

u16 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

u32 readsFlash = 0;

static const u16 manReqWord = 0x3B00;
static const u16 manReqMask = 0xFF00;



static bool RequestMan(u16 *buf, u16 len, MTB* mtb);

static i16 temperature = 0;


//static bool ReqMan00(u16 *buf, u16 len, MTB* mtb);
//static bool ReqMan01(u16 *buf, u16 len, MTB* mtb);
//static bool ReqMan02(u16 *buf, u16 len, MTB* mtb);
//static bool ReqMan03(u16 *buf, u16 len, MTB* mtb);
//static bool ReqMan04(u16 *buf, u16 len, MTB* mtb);
//
//typedef bool (*RMF)(u16 *buf, u16 len, MTB* mtb);
//
//static const RMF ReqManTbl[5] = {ReqMan00, ReqMan01, ReqMan02, ReqMan03, ReqMan04};

static byte status = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan00(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0;
	manTrmData[1] = GetNumDevice();
	manTrmData[2] = VERSION;

	mtb->data = manTrmData;
	mtb->len = 3;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan10(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x10;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan20(u16 *buf, u16 len, MTB* mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 temp; u16 status; RTC rtc; };

	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	Rsp &rsp = *((Rsp*)&manTrmData);

	rsp.rw = (manReqWord & manReqMask) | 0x20;
	rsp.device = 0xAA00;  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = temperature;
	rsp.status = FLASH_Status();

	GetTime(&rsp.rtc);

	mtb->data = manTrmData;
	mtb->len = 20;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan30(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 5 || len > 6 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x30;

	SetTime(*(RTC*)(&buf[1]));

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan31(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x31;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteEnable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan32(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x32;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteDisable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan33(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x33;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan80(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || len > 4 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x80;

	mtb->data = manTrmData;
	mtb->len = 1;

	switch (buf[1])
	{
		case 1:		SetNumDevice(buf[2]);		break;
		case 2:		SetTrmBoudRate(buf[2]-1);	break;
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan90(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || len > 4 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x90;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqManF0(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	SaveParams();

	manTrmData[0] = (manReqWord & manReqMask) | 0xF0;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	bool res = false;

	byte i = buf[0]&0xFF;

	switch (i)
	{
		case 0x00:	res = ReqMan00(buf, len, mtb); break;
		case 0x10:	res = ReqMan10(buf, len, mtb); break;
		case 0x20:	res = ReqMan20(buf, len, mtb); break;
		case 0x30:	res = ReqMan30(buf, len, mtb); break;
		case 0x31:	res = ReqMan31(buf, len, mtb); break;
		case 0x32:	res = ReqMan32(buf, len, mtb); break;
		case 0x33:	res = ReqMan33(buf, len, mtb); break;
		case 0x80:	res = ReqMan80(buf, len, mtb); break;
		case 0x90:	res = ReqMan90(buf, len, mtb); break;
		case 0xF0:	res = ReqManF0(buf, len, mtb); break;
	};

	return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	static MTB mtb;
	static MRB mrb;

	static byte i = 0;

	static RTM32 tm;


//	u16 c;

	switch (i)
	{
		case 0:

			mrb.data = manRcvData;
			mrb.maxLen = 5;
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			ManRcvUpdate();

			if (mrb.ready)
			{
				tm.Reset();

				if (mrb.OK && mrb.len > 0 && (manRcvData[0] & manReqMask) == manReqWord && RequestMan(manRcvData, mrb.len, &mtb))
				{
					i++;
				}
				else
				{
					i = 0;
				};
			}
			else if (mrb.len > 0)
			{
				//if ((manRcvData[0] & manReqMask) == manReqWord)
				//{
				//	byte i = manRcvData[0] & 0xFF;

				//	u16 l = 100;

				//	switch (i)
				//	{
				//		case 0x00:			
				//		case 0x10:			
				//		case 0x20:	l = 1;		break;

				//		case 0x30:	l = 5;		break;

				//		case 0x31:	
				//		case 0x32:	
				//		case 0x33:	l = 1;		break;

				//		case 0x80:	
				//		case 0x90:	l = 3;		break;

				//		case 0xF0:	l = 1;		break;
				//	};

				//	if (mrb.len >= l)
				//	{
				//		ManRcvStop();
				//	};
				//};
			};

			break;

		case 2:

			if (tm.Check(US2RT(100)))
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

//static void UpdateMan()
//{
//	static MTB mtb;
//	static MRB mrb;
//
//	static byte i = 0;
//
//	static RTM32 tm;
//
//	bool parityErr;
//	u32 *s;
//	u16 *d;
//
//	u16 c;
//
//	switch (i)
//	{
//		case 0:
//
//			if (tm.Check(MS2RT(100)))
//			{
//				mtb.data = tableCRC;
//				mtb.len = 70;
//
//				SetTrmBoudRate(2); SendMLT3(&mtb);
////				SetTrmBoudRate(0); SendManData(&mtb);
//
//				i++;
//			};
//
//			break;
//
//		case 1:
//
//			if (mtb.ready)
//			{
//				i = 0;
//			};
//
//			break;
//
//	};
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTemp()
{
	using namespace HW;

	PMC->PCER0 = PID::AFEC0_M;

	AFE0->MR = 0x0031FF80;
	AFE0->CHER = (1<<15)|15;
	AFE0->CR = 2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	HW::AFE0->CSELR = 15;
	temperature = (((i32)HW::AFE0->CDR - 1787*2) * 11234/2) / 65536 + 27;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateEMAC();	);
		CALL( UpdateTraps();	);
		CALL( UpdateMan();		);
		CALL( FLASH_Update();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( NAND_Idle();	);
		CALL( UpdateMisc();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

//	__breakpoint(0);

	InitEMAC();

	InitTraps();

	FLASH_Init();


	InitTemp();

	u32 f = 0;


	static TM32 tm;
//	static RTM32 rtm2;

	HW::PIOB->PER = 1<<13;
	HW::PIOB->OER = 1<<13;

	while(1)
	{
//		HW::PIOB->SODR = 1<<13;

		Update();		

		//static byte i = 0;

		//#define CALL(p) case (__LINE__-S): p; break;

		//enum C { S = (__LINE__+3) };
		//switch(i++)
		//{
		//	CALL( UpdateEMAC();		);
		//	CALL( UpdateMisc();		);
		//};

		//i = (i > (__LINE__-S-3)) ? 0 : i;

		//#undef CALL

	
//		HW::PIOB->CODR = 1<<13;

		RTC rtc;

		GetTime(&rtc);

		if (rtc.msec > 999)
		{
			__breakpoint(0);
		};
		
		f++;

		if (tm.Check(1000))
		{
			UpdateTemp();
			fps = f;
			f = 0;
		};

		HW::ResetWDT();

//		__asm { WFE };
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
