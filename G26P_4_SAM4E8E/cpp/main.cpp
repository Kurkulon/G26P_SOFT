#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"
#include "xtrap.h"
#include "flash.h"
#include "vector.h"
#include "list.h"
#include "fram.h"
#include "twi.h"
#include "PointerCRC.h"

#pragma diag_suppress 546,550,177

u32 fps = 0;

//extern byte Heap_Mem[10];

u32 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

u32 readsFlash = 0;

static const u16 manReqWord = 0x3B00;
static const u16 manReqMask = 0xFF00;


static bool RequestMan(u16 *buf, u16 len, MTB* mtb);

static i16 temperature = 0;

static NVV nvv;

static byte buf[sizeof(nvv)*2+2];

static byte savesCount = 0;

static TWI	twi;

static void SaveVars();

inline void SaveParams() { savesCount = 2; }

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
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0;
	manTrmData[1] = 0xEC00;
	manTrmData[2] = 0xEC00;

	mtb->data = manTrmData;
	mtb->len = 3;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan10(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x10;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan20(u16 *buf, u16 len, MTB* mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 temp; u16 status; RTC rtc; };

	if (buf == 0 || len == 0 || mtb == 0) return false;

	Rsp &rsp = *((Rsp*)&manTrmData);

	rsp.rw = (manReqWord & manReqMask) | 0x20;
	rsp.device = 0xEC00;  
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
	if (buf == 0 || len < 5 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x30;

	SetTime(*(RTC*)(&buf[1]));

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan31(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x31;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteEnable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan32(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x32;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteDisable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan33(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x33;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan80(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x80;

	mtb->data = manTrmData;
	mtb->len = 1;

	if (buf[1] == 2)
	{
		SetTrmBoudRate(buf[2]-1);
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan90(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x90;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqManF0(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

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

	bool parityErr;
	u32 *s;
	u16 *d;

	u16 c;

	switch (i)
	{
		case 0:

			mrb.data = manRcvData;
			mrb.maxLen = 5;
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			if (mrb.ready)
			{
				HW::PIOE->CODR = 3;

				if (mrb.OK && mrb.len > 0)
				{
					parityErr = false;
					s = mrb.data;
					d = rcvBuf;
					c = mrb.len;

					while (c > 0)
					{
						parityErr |= (*s ^ CheckParity(*s >> 1))&1 != 0;

						*d++ = *s++ >> 1;
						c--;
					};

					if (!parityErr && (rcvBuf[0] & manReqMask) == manReqWord)
					{
						if (RequestMan(rcvBuf, mrb.len, &mtb))
						{
							tm.Reset();

							i++;
						}
						else
						{
							i = 0;
						};
					}
					else
					{
						i = 0;
					};

				}
				else
				{
					i = 0;
				};
			};

			break;

		case 2:

			if (tm.Check(US2RT(500)))
			{
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
static void UpdateWriteFlash()
{
	static FLWB *fb = 0;
	static byte i = 0;
	static byte cnt = 1;
	static u32  w = 0;
	static bool ready;

	switch (i)
	{
		case 0:

			if ((fb = AllocFlashWriteBuffer()) != 0)
			{
				fb->ready = &ready;

//				fb->hdrLen = 0;
				fb->dataLen = 2048;

				for (u32 n = 0; n < fb->dataLen; n++)
				{
	//				fb->data[n] = cnt;
				};

				cnt++;
				w++;

				RequestFlashWrite(fb);

				i++;
			};

			break;

		case 1:

			if (ready)
			{
				if (w >= 50000)
				{
					w = 0;
					NAND_NextSession();
				};

				i = 0;
			};

			break;
	};
}

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

static void LoadVars()
{
	twi.Init(1);

	PointerCRC p(buf);

	static DSCTWI dsc;

	dsc.MMR = 0x500200;
	dsc.IADR = 0;
	dsc.CWGR = 0x7575;
	dsc.data = buf;
	dsc.len = sizeof(buf);

	if (twi.Read(&dsc))
	{
		while (twi.Update());
	};

	bool c = false;

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&nvv, sizeof(nvv));

		if (p.CRC.w == 0) { c = true; break; };
	};

	if (!c)
	{
		nvv.numDevice = 0;
		nvv.index = 0;

		nvv.si.session = 0;
		nvv.si.size = 0;
		nvv.si.last_adress = 0;
		GetTime(&nvv.si.start_rtc);
		GetTime(&nvv.si.stop_rtc);
		nvv.si.flags = 0;

		savesCount = 2;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	PointerCRC p(buf);

	static DSCTWI dsc;

	static byte i = 0;
	static RTM32 tm;

	switch (i)
	{
		case 0:

			if (/*tm.Check(MS2RT(1000)) ||*/ savesCount > 0)
			{
				i++;
			};

			break;

		case 1:

			dsc.MMR = 0x500200;
			dsc.IADR = 0;
			dsc.CWGR = 0x07575; 
			dsc.data = buf;
			dsc.len = sizeof(buf);

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&nvv, sizeof(nvv)-sizeof(nvv.crc));
				p.WriteW(p.CRC.w);
			};

			i = (twi.Write(&dsc)) ? (i+1) : 0;

			break;

		case 2:

			if (!twi.Update())
			{
				savesCount--;
				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateTraps();	);
		CALL( NAND_Idle();		);
		CALL( UpdateMan();		);
		CALL( SaveVars();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

//	__breakpoint(0);

	LoadVars();

	InitEMAC();

	InitTraps();

	FLASH_Init();


//	InitTemp();

	u32 f = 0;


	static RTM32 rtm;
	static RTM32 rtm2;




	HW::PIOB->PER = 1<<13;
	HW::PIOB->OER = 1<<13;

	while(1)
	{
		HW::PIOB->SODR = 1<<13;

		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateEMAC();		);
			CALL( UpdateMisc();		);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

	
		HW::PIOB->CODR = 1<<13;
		
		f++;

		if (rtm.Check(MS2RT(1000)))
		{
//			UpdateTemp();
			fps = f;
			f = 0;

			savesCount++;
		};

		HW::ResetWDT();

//		__asm { WFE };
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
