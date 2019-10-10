#include "emac.h"
#include "core.h"
#include "xtrap.h"
#include "flash.h"
#include "time.h"
#include "ComPort.h"
#include "twi.h"
#include "hardware.h"
#include "main.h"

#pragma diag_suppress 546,550,177

//#pragma O3
//#pragma Otime

byte buf[5000] = {0x55,0,0,0,0,0,0,0,0,0x55};

static ComPort com1;
//static TWI twi;

u32 fps = 0;
u32 f = 0;

ComPort::WriteBuffer wb = { .transmited = false, .len = 0, .data = buf };
ComPort::ReadBuffer rb = { .recieved = false, .len = 0, .maxLen = sizeof(buf), .data = buf };

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len);
static void Init_UART_DMA();
static void Send_UART_DMA();

static byte len = 1;

static u16 temp = 0;

u16 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

u32 readsFlash = 0;

static const u16 manReqWord = 0x3B00;
static const u16 manReqMask = 0xFF00;


static bool RequestMan(u16 *buf, u16 len, MTB* mtb);

static i16 temperature = 0;

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
	rsp.device = 0xEC00;  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = temp;
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

	SetClock(*(RTC*)(&buf[1]));

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

	static RTM tm;


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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	static byte i = 0;

	static DSCTWI dsc, dsc2;
	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];
	//static byte *romData = 0;
	//static u16 romAdr = 0;
	//static u16 revRomAdr = 0;
	//static u16 romWrLen = 0;
	//static u16 romRdLen = 0;
	//static u16 pageLen = 0;

	static TM32 tm;

//	HW::GPIO->SET0 = 1<<12;

	switch (i)
	{
		case 0:

			if (tm.Check(100))
			{
				buf[0] = 0x11;

				dsc.adr = 0x68;
				dsc.wdata = buf;
				dsc.wlen = 1;
				dsc.rdata = &rbuf;
				dsc.rlen = 2;
				dsc.wdata2 = 0;
				dsc.wlen2 = 0;

				//buf[0] = 0x0E;
				//buf[1] = 0x20;
				//buf[2] = 0xC8;

				//dsc.adr = 0x68;
				//dsc.wdata = buf;
				//dsc.wlen = 3;
				//dsc.rdata = 0;
				//dsc.rlen = 0;
				//dsc.wdata2 = 0;
				//dsc.wlen2 = 0;

				if (AddRequest_TWI(&dsc))
				{
					i++;
				};
			};

			break;

		case 1:

			if (dsc.ready)
			{
				temp = ((i16)ReverseWord(rbuf) + 128) / 256;

				i++;
			};

			break;

		case 2:

//			if (tm.Check(100))
			{
				buf[0] = 0x0E;
				buf[1] = 0x20;
				buf[2] = 0xC8;

				dsc2.adr = 0x68;
				dsc2.wdata = buf;
				dsc2.wlen = 3;
				dsc2.rdata = 0;
				dsc2.rlen = 0;
				dsc2.wdata2 = 0;
				dsc2.wlen2 = 0;

				if (AddRequest_TWI(&dsc2))
				{
					i++;
				};
			};

			break;

		case 3:

			if (dsc2.ready)
			{
				i = 0;
			};

			break;
	};

//	HW::GPIO->CLR0 = 1<<12;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateMan();		);
		CALL( UpdateTemp();		);
		CALL( FLASH_Update();	);
		CALL( UpdateEMAC();	);
		CALL( UpdateTraps();	);
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
	static TM32 rtm;
	//static MTB mtb;
	//static u16 manbuf[10];
	//static DSCTWI dsctwi;
	//static DSCTWI dsc;

	//__breakpoint(0);

	InitHardware();

	InitEMAC();

	InitTraps();

	FLASH_Init();


	//buf[4999] = 0x55;

	//wb.len = 5000;

	while(1)
	{
		HW::P2->BSET(6);
		f++;

		Update();

		HW::P2->BCLR(6);

		if (rtm.Check(1000))
		{	
			fps = f; f = 0; 

			//mtb.data = manbuf;
			//mtb.len = 3;

			//manbuf[0] = 0xAA00;

			//SendManData(&mtb);
			//SendMLT3(&mtb);
		};
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

