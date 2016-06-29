#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"
#include "xtrap.h"
#include "flash.h"
#include "vector.h"
#include "list.h"

u32 fps = 0;

extern byte Heap_Mem[10];

u32 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

u32 readsFlash = 0;

static const u16 manReqWord = 0x3B00;
static const u16 manReqMask = 0xFF00;


static bool RequestMan(u16 *buf, u16 len, MTB* mtb);

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
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x20;
	manTrmData[1] = 0xEC00;
	manTrmData[2] = 3;
	manTrmData[3] = 4;
	manTrmData[5] = 6;
	manTrmData[7] = 8;
	manTrmData[9] = 10;
	manTrmData[11] = 12;
	manTrmData[14] = 15;
	manTrmData[15] = status;

	GetTime((RTC*)(&manTrmData[16]));

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

	status = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan32(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x32;

	mtb->data = manTrmData;
	mtb->len = 1;

	status = 0;

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

				fb->hdrLen = 0;
				fb->dataLen = 2048;

				for (u32 n = 0; n < fb->dataLen; n++)
				{
					fb->data[n] = cnt;
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

int main()
{
	InitHardware();

	InitEMAC();

	InitTraps();

	FLASH_Init();

	u32 f = 0;


	static RTM32 rtm;
	static RTM32 rtm2;



//	__breakpoint(0);

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
			CALL( UpdateTraps();	);
			CALL( NAND_Idle();		);
			CALL( UpdateMan();		);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

	
		HW::PIOB->CODR = 1<<13;
		
		f++;

		if (rtm.Check(MS2RT(1000)))
		{
			fps = f;
			f = 0;

		};

//		__asm { WFE };
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
