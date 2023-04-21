#include "hardware.h"
#include "time.h"




u16 curHV = 0;
//u16 reqHV = 900;
u16 p_M = 100;
u16 p_XY = 100;
u16 t_M = 100;
u16 t_XY = 100;

static u16 dstHV = 900;

//byte reqFireCount = 1;
byte reqFireCountM = 1;
byte reqFireCountXY = 1;
u16 reqFireFreqM = 16000;
u16 reqFireFreqXY = 2000;
u16 reqFireDutyM = 5000; //0.01%
u16 reqFireDutyXY = 5000; //0.01%

static byte fireCount = 0;
static u32 fireMaskClr = 0;
static u32 fireEndTime = 0;

//#define eVal_IntervalTimer ((MCK / 5000) * 10)
//#define eVal_StartSample   ((MCK / 5000) *  6)
//
//enum {  eVal_MinValue = 10 } ;

bool	syncActive = false;
u32		syncTime = 0;

bool	charge = false;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	using namespace HW;

	static u32 t = 0;

	if ((SPI0->STAT & 1) == 0) return;

	t += (i32)((SPI0->RXDAT & 0xFFF) << 6) - (i32)(t>>10);

	curHV = ((t>>16) * 19600) >> 16; // HV * 3.3 / 4095 / 56 * 20000

	SPI0->TXDATCTL = 0x0F100000; //SPI_TXDATCTL_FLEN(7) | SPI_TXDATCTL_EOT | SPI_TXDATCTL_SSEL_N(0xe);

	if (curHV < (dstHV-5))
	{
		GPIO->SET0 = EN;
	}
	else if (curHV > (dstHV+5))
	{
		GPIO->CLR0 = EN;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void UpdateCharge()
//{
//	using namespace HW;
//
//	static byte i = 0;
//
//	static TM32 tm;
//
//	//GPIO->DIR0 |= (1<<8);
//	//GPIO->CLR0 = 1<<8;
//
//	if (charge)
//	{
//		switch (i)
//		{
//			case 0:
//
//				if (tm.Check(1))
//				{
//					GPIO->CLR0 = CHARGE;
//
//					i++;
//				};
//
//				break;
//
//			case 1:
//
//				if (tm.Check(100))
//				{
//					GPIO->SET0 = CHARGE;
//				
//					charge = false;
//				};
//
//				break;
//		};
//	}
//	else
//	{
//		GPIO->SET0 = CHARGE;
//		GPIO->DIR0 |= CHARGE;
//		i = 0;
//
//		tm.Reset();
//	};
//}
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetReqHV(u16 v)
{
	if (v > 950) { v = 950; };

	dstHV = v;

	//u16 t = (reqHV > 0) ? (14400/reqHV) : 50;

	//if (t > 100) { t = 100; } else if (t < 16) { t = 16; };

	//mt = t;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetReqFireFreqM(u16 freq, u16 duty)
{
	if (freq < 10000) { freq = 10000; } else if (freq > 30000) { freq = 30000; };
	
	if (duty > 6000) { duty = 6000; };

	reqFireFreqM = freq;
	reqFireDutyM = duty;

	p_M = (MCK+freq/2) / freq;

	u32 t = ((u32)duty * 429497 + 32768) >> 16;

	t_M = (p_M * t + 32768) >> 16; //(u32)p_M * duty / 10000;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetReqFireFreqXY(u16 freq, u16 duty)
{
	if (freq < 1000) { freq = 1000; } else if (freq > 10000) { freq = 10000; };
	
	if (duty > 6000) { duty = 6000; };

	reqFireFreqXY = freq;
	reqFireDutyXY = duty;

	p_XY = (MCK+freq/2) / freq;

	u32 t = ((u32)duty * 429497 + 32768) >> 16;

	t_XY = (p_XY * t + 32768) >> 16; //(u32)p_XY * duty / 10000;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFireM()
{
	using namespace HW;

	//u16 t = (((u32)reqHV * 2548) >> 16) + 15;

	//u16 t = mt;

	//if (t > 100) { t = 100; } else if (t < 16) { t = 16; };

	SCT->CTRL_L = (1<<2); // HALT

	SCT->MATCH_L[0] = 0; 
	SCT->MATCH_L[1] = 25*2;
	SCT->MATCH_L[2] = t_M+25*2; //335
	SCT->MATCH_L[3] = t_M+25*4; //345
	SCT->MATCH_L[4] = p_M;

	SCT->OUTPUT = 2;
	HW::SWM->CTOUT_0 = PIN_TR1;
	HW::SWM->CTOUT_1 = PIN_CHARGE;
	
	GPIO->CLR0 = TR1|CHARGE;	

	fireEndTime = GetMilliseconds();

	SCT->LIMIT_L = 1<<4;
	SCT->HALT_L = 0;//1<<4;
	SCT->EVFLAG = 1<<3;
	SCT->EVEN = 1<<3;

	fireCount = reqFireCountM;
	fireMaskClr = CHARGE;

//	charge = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFireXX()
{
	using namespace HW;

	SCT->CTRL_L = (1<<2); // HALT

	SCT->MATCH_L[0] = 0; 
	SCT->MATCH_L[1] = 25*20;
	SCT->MATCH_L[2] = t_XY+25*20; 
	SCT->MATCH_L[3] = t_XY+25*40; 
	SCT->MATCH_L[4] = p_XY;

	SCT->OUTPUT = 0;
	HW::SWM->CTOUT_0 = PIN_FX1; //17;
	HW::SWM->CTOUT_1 = PIN_FX2; //13;

	fireEndTime = GetMilliseconds();

	SCT->LIMIT_L = 1<<4;
	SCT->HALT_L = 0;//1<<4;
	SCT->EVFLAG = 1<<3;
	SCT->EVEN = 1<<3;

	fireCount = reqFireCountXY;
	fireMaskClr = 0;

//	HW::SCT->CTRL_L = (HW::SCT->CTRL_L & ~(3<<1)) | (1<<3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFireYY()
{
	using namespace HW;

	SCT->CTRL_L = (1<<2); // HALT

	SCT->MATCH_L[0] = 0; 
	SCT->MATCH_L[1] = 25*20;
	SCT->MATCH_L[2] = t_XY+25*20; 
	SCT->MATCH_L[3] = t_XY+25*40; 
	SCT->MATCH_L[4] = p_XY;

	SCT->OUTPUT = 0;
	HW::SWM->CTOUT_0 = PIN_FY2; //4;
	HW::SWM->CTOUT_1 = PIN_FY1; //12;

	fireEndTime = GetMilliseconds();

	SCT->LIMIT_L = 1<<4;
	SCT->HALT_L = 0;//1<<4;
	SCT->EVFLAG = 1<<3;
	SCT->EVEN = 1<<3;

	fireCount = reqFireCountXY;
	fireMaskClr = 0;

	//W::SCT->CTRL_L = (HW::SCT->CTRL_L & ~(3<<1)) | (1<<3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void(*FireMode)() = FireXX;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void SCT_Handler()
{
	if (HW::SCT->EVFLAG & (1<<3))
	{
		fireCount--;

		HW::SCT->EVFLAG = 1<<3;

		if (fireCount == 0)
		{
			HW::SCT->LIMIT_L = 0;
			HW::SCT->HALT_L = 1<<4;
			HW::SCT->EVFLAG = 1<<4;
			HW::SCT->EVEN = 1<<4;
		};
	}
	else if (HW::SCT->EVFLAG & (1<<4))
	{
		HW::SCT->CTRL_L = 1<<2;
		HW::SCT->EVFLAG = 1<<4;
		HW::SCT->EVEN = 0;

//		HW::GPIO->CLR0 = fireMaskClr;
		HW::SWM->CTOUT_0 = ~0;
		HW::SWM->CTOUT_1 = ~0;
		fireEndTime = GetMilliseconds();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void SyncFireHandler()
{
//	FireMode();

	HW::SCT->CTRL_L = (1<<3);//(HW::SCT->CTRL_L & ~(3<<1)) | (1<<3);

	HW::PIN_INT->IENR = 0;
	HW::PIN_INT->IENF = 0;
	HW::PIN_INT->IST = 1;
	syncActive = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void WaitFireSync(byte t)
{
	HW::PIN_INT->ISEL = 0;
	HW::PIN_INT->IENR = 0;
	HW::PIN_INT->IENF = 1;

	switch(t)
	{
		case 0: InitFireXX(); break;
		case 1: InitFireYY(); break;
		case 2: InitFireM();  break;
	};

	syncActive = true;
	syncTime = GetMilliseconds();

	HW::PIN_INT->IST = 1;
//	HW::PIN_INT->FALL = 1;
	HW::PIN_INT->IENF = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFire()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SCT_M;
	GPIO->DIR0 |= FX1|FX2|FY1|FY2|TR1|CHARGE;	//(1<<13)|(1<<17)|(1<<12)|(1<<4);
	GPIO->CLR0 = FX1|FX2|FY1|FY2|TR1|CHARGE;					//(1<<17)|(1<<4);
	//GPIO->SET0 = FX2|FY1|CHARGE;				//(1<<13)|(1<<12);
//	SWM->PINASSIGN.U0_TXD = 0;

	SCT->STATE_L = 0;
	SCT->REGMODE_L = 0;

	SCT->MATCH_L[0] = 0; 
	SCT->MATCH_L[1] = 25*10;
	SCT->MATCH_L[2] = 25*258; //335
	SCT->MATCH_L[3] = 25*288; //345
	SCT->MATCH_L[4] = 25*566;

	SCT->OUT[0].SET = 0x0002;
	SCT->OUT[0].CLR = 0x0005;

	SCT->OUT[1].SET = 0x0008;
	SCT->OUT[1].CLR = 0x0001;

//	SCT->OUTPUT |= 1;



	SCT->EVENT[0].STATE = 1;
	SCT->EVENT[0].CTRL = (1<<5)|(0<<6)|(1<<12)|0;

	SCT->EVENT[1].STATE = 1;
	SCT->EVENT[1].CTRL = (1<<5)|(0<<6)|(1<<12)|1;

	SCT->EVENT[2].STATE = 1;
	SCT->EVENT[2].CTRL = (1<<5)|(1<<6)|(1<<12)|2;

	SCT->EVENT[3].STATE = 1;
	SCT->EVENT[3].CTRL = (1<<5)|(1<<6)|(1<<12)|3;

	SCT->EVENT[4].STATE = 1;
	SCT->EVENT[4].CTRL = (1<<5)|(1<<6)|(1<<12)|4;

	SCT->EVENT[5].STATE = 0;
	SCT->EVENT[5].CTRL = 0;

	SCT->LIMIT_L = 0;
	SCT->START_L = 0;
	SCT->STOP_L = 0;
	SCT->HALT_L = 0;

	SCT->CONFIG = 1<<7; 

	//SWM->CTOUT_0 = 17;
	//SWM->CTOUT_1 = 13;

//	SCT->CTRL_L &= ~(3<<1);


	SYSCON->PINTSEL[0] = PIN_URX; //14;
	PIN_INT->ISEL = 0;
	//PIN_INT->SIENF = 1;
	//PIN_INT->FALL = 1;
	//PIN_INT->IST = 1;

//	SCT->EVEN = 1<<4;

	VectorTableExt[SCT_IRQ] = SCT_Handler;
	CM0::NVIC->ISER[0] = 1<<SCT_IRQ;

	VectorTableExt[PIN_INT0_IRQ] = SyncFireHandler;
	CM0::NVIC->ISER[0] = 1<<PIN_INT0_IRQ;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitADC()
{
	using namespace HW;

	//SWM->SPI0_SCK	= PIN_SCK;	//9;
	//SWM->SPI0_MISO	= PIN_MISO;	//15;
	//SWM->SPI0_SSEL	= PIN_ADCS;	//16;

	//SYSCON->SYSAHBCLKCTRL |= CLK::SPI0_M;

	//SPI0->CFG = 0x05;	//SPI_CFG_MASTER | SPI_CFG_ENABLE;
	//SPI0->DLY = 1;
	//SPI0->DIV = 24;

	//SPI0->TXDATCTL = 0x0F100000; //SPI_TXDATCTL_FLEN(7) | SPI_TXDATCTL_EOT | SPI_TXDATCTL_SSEL_N(0xe);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	using namespace HW;

	Init_time(MCK);
	InitADC();
	InitFire();

	SYSCON->SYSAHBCLKCTRL |= HW::CLK::WWDT_M;
	SYSCON->PDRUNCFG &= ~(1<<6); // WDTOSC_PD = 0
	SYSCON->WDTOSCCTRL = (1<<5)|1; // 150 kHz 6.66us

	SetReqHV(900);

#ifndef _DEBUG

	WDT->TC = 10 * 150000 / 1000; //0x1FF; 10ms
	WDT->MOD = 0x3;
	ResetWDT();

#else

	ResetWDT();
	WDT->MOD = 0;
	ResetWDT();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC();
	//UpdateCharge();

	if ((GetMilliseconds() - fireEndTime) >= 10)
	{
		HW::GPIO->DIR0 |= FX1|FX2|FY1|FY2|TR1|CHARGE;	
		HW::GPIO->CLR0 = FX1|FY2|FX2|FY1|TR1;					
		HW::GPIO->SET0 = CHARGE;				
	};

	HW::ResetWDT();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 GetCRC(const void *data, u32 len)
{
	union { u32 *pd; u16 *pw; u8 *pb; };

	pb = (byte*)data;

//	byte * const p = (byte*)HW::CRC->B;

	HW::CRC->MODE = 0x15;
	HW::CRC->SEED = 0xFFFF;

	u32 dl = len>>2;
	u32 wl = (len&3)>>1;
	u32 bl = (len&1);

	for ( ; dl > 0; dl--) 
	{
		HW::CRC->D = *(pd++);
	};

	for ( ; wl > 0; wl--) 
	{
		HW::CRC->W = *(pw++);
	};

	for ( ; bl > 0; bl--) 
	{
		HW::CRC->B = *(pb++);
	};

	//for ( ; len > 0; len--) 
	//{
	//	HW::CRC->B = *(pb++);
	//};

	return HW::CRC->SUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <system_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
