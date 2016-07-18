#include "hardware.h"

#include <bfrom.h>
#include <sys\exception.h>
//#include <cdefBF592-A.h>
//#include <ccblkfn.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(WaitManCmdSync);
EX_INTERRUPT_HANDLER(WaitManDataSync);
EX_INTERRUPT_HANDLER(ManRcvSyncIrq);
EX_INTERRUPT_HANDLER(ManRcvTimerIRQ);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma always_inline
inline void ManDisable() { *pPORTFIO = 0x50; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; }
#pragma always_inline
inline void ManOne() { *pPORTFIO = 0x50; *pPORTFIO = 0xC0; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0xC0; *pPORTFIO_SET = 0x30; }
#pragma always_inline
inline void ManZero() { *pPORTFIO = 0x50; *pPORTFIO = 0x30; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0x30; *pPORTFIO_SET = 0xC0; }

#pragma always_inline
inline void ManRcvTmrEn(u32 t) { *pTCOUNT = t; *pTCNTL = TMREN|TMPWR; }

#pragma always_inline
inline void ManRcvTmrDis() { *pTCNTL = 0; }
			
#pragma always_inline
inline void ManRcvTmrReset(u32 t) { *pTCNTL = 0; *pTCOUNT = t; *pTCNTL = TMREN|TMPWR; }

#pragma always_inline
inline void ManRcvTmrIrqEn() { *pIMASK |= EVT_IVTMR; } 

#pragma always_inline
inline void ManRcvTmrIrqDis() { *pIMASK &= ~EVT_IVTMR; } 


static u32 manPulseTmr = 0;

#pragma always_inline
inline void ManPulseTmrReset() { manPulseTmr = *pTIMER0_COUNTER; } 

#pragma always_inline
inline u32 ManPulseTmrGet() { return *pTIMER0_COUNTER - manPulseTmr; } 


#define	ManRcvTmrIrqPtr (*pEVT6)
#define	ManRcvPinIrq (*pEVT11)
#define PINIRQ (1<<10)

//	*pTPERIOD = rcvQuartPeriod;
//
////	*pPORTFIO_SET = 1<<2;
//
//	*pTCNTL = TAUTORLD|TMREN|TMPWR;

	//*pEVT11 = (void*)WAITMANRCVR_ISR;
	//*pSIC_IMASK |= IRQ_PFA_PORTF;
	//*pIMASK |= EVT_IVG11; 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define BOUD2CLK(x) ((u32)((SCLK/1.0)/x+0.5))

static const u16 manboud[8] = { BOUD2CLK(20833), BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333), BOUD2CLK(83333), BOUD2CLK(83333), BOUD2CLK(83333) };


u16 trmHalfPeriod = BOUD2CLK(20833)/2;
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&7]/2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ManRxd (*pPORTFIO & 1)

const u16 rcvPeriod = BOUD2CLK(20833);
const u16 rcvHalfPeriod = rcvPeriod/2;
const u16 rcvQuartPeriod = rcvPeriod/4;
const u16 rcvSyncPulse = rcvPeriod * 1.5;
const u16 rcvSyncPulseMin = rcvSyncPulse - rcvQuartPeriod;
const u16 rcvSyncPulseMax = rcvSyncPulse + rcvQuartPeriod;
const u16 rcvSyncHalf = rcvSyncPulseMax + rcvHalfPeriod;
const u16 rcvPeriodMin = rcvPeriod - rcvQuartPeriod;
const u16 rcvPeriodMax = rcvPeriod + rcvQuartPeriod;

static u32* rcvManPtr = 0;
static u16 rcvManCount = 0;

static byte rcvSyncState = 0;
static byte rcvDataState = 0;

static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;
//const u16 rcvQuartPeriod = BOUD2CLK(20833)/2;

static MRB *manRB = 0;

//static void(*ManRcvPinIrq)() = WaitManCmdSync;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool trmStartCmd = false;
bool trmStartData = false;

u32 icount = 0;
static byte ib;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRTT()
{
	*pTIMER0_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS;
	*pTIMER0_PERIOD = 0;
	*pTIMER_ENABLE = TIMEN0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_PLL()
{
	u32 SIC_IWR1_reg;                /* backup SIC_IWR1 register */

	/* use Blackfin ROM SysControl() to change the PLL */
    ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };

	/* use the ROM function */
	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();

								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x0000;		//  0000 0000 0000 0000
//	*pPORTG_MUX = 0x0000;		//  0000 0000 0000 0000

	*pPORTF_FER = 0x1800;		//  0001 1000 0000 0000
//	*pPORTG_FER = 0xE70F;		//  0000 0000 0000 0000

	*pPORTFIO_DIR = 0x21F4;		//  0010 0001 1111 0100
//	*pPORTGIO_DIR = 0x1800;		//  0001 1000 0000 0000

	*pPORTFIO_INEN = 0x0601;	//  0000 0110 0000 0001
//	*pPORTGIO_INEN = 0x0003;	//  0000 0000 0000 0000

	*pPORTF_PADCTL = 0x0011;	//  0000 0000 0001 0000 Schmitt trigger

	ManDisable();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void SendManCmd(void *data, u16 len)
//{
//	stateManTrans = 0;
//	trmStartCmd = true;
//	trmData = (u16*)data;
//	trmDataLen = len;
//	*pTIMER1_PERIOD = trmHalfPeriod;
//	*pTIMER_ENABLE = TIMEN1;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(TIMER_ISR)
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static u16 *data = 0;
	static u16 len = 0;

//	*pPORTFIO_SET = 1<<2;

	switch (stateManTrans)
	{
		case 0:	// Idle; 

			ManDisable();
		
			data = manTB->data;
			len = manTB->len;
			stateManTrans = 1;

			break;

		case 1: // Start data

			i = 3;
			tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

			data++;
			len--;

			ManOne();

			stateManTrans++;

			break;

		case 2:	// Wait data 1-st sync imp

			i--;

			if (i == 0)
			{
				stateManTrans++;
				ManZero();
				i = 3;
			};

			break;

		case 3: // Wait 2-nd sync imp

			i--;

			if (i == 0)
			{
				stateManTrans++;
				count = 17;

				if (tw & 0x10000) ManZero(); else ManOne();
			};

			break;

		case 4: // 1-st half bit wait

			if (tw & 0x10000) ManOne(); else ManZero();

			count--;

			if (count == 0)
			{
				stateManTrans = (len > 0) ? 1 : 6;
			}
			else
			{
				stateManTrans++;
			};

			break;

		case 5: // 2-nd half bit wait

			tw <<= 1;
			stateManTrans = 4;
			if (tw & 0x10000) ManZero(); else ManOne();

			break;

		case 6:

			ManDisable();
			stateManTrans = 0;
			*pTIMER_DISABLE = TIMDIS1;
			*pSIC_IMASK &= ~IRQ_TIMER1;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

	*pTIMER_STATUS = 2;

//	*pPORTFIO_CLEAR = 1<<2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData(MTB *mtb)
{
	if (trmBusy || rcvBusy || mtb == 0 || mtb->data == 0 || mtb->len == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	stateManTrans = 0;

	*pEVT11 = (void*)TIMER_ISR;
	*pIMASK |= EVT_IVG11; 
	*pSIC_IMASK |= IRQ_TIMER1;

	*pTIMER1_PERIOD = trmHalfPeriod;
	*pTIMER1_WIDTH = 1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS|IRQ_ENA;
	*pTIMER_ENABLE = TIMEN1;

	return trmBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit()
{
	//*pEVT11 = (void*)TIMER_ISR;
	//*pIMASK |= EVT_IVG11; 
	//*pSIC_IMASK |= IRQ_TIMER1;

	*pTIMER1_PERIOD = trmHalfPeriod;
	*pTIMER1_WIDTH = 1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS|IRQ_ENA;
	*pTIMER_DISABLE = TIMDIS1;

	//register_handler(ik_ivg9, SPORT_ISR);


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//EX_INTERRUPT_HANDLER(MANRCVR_ISR)
//{
//	u16 t;
//	static byte ib;
//	static u32 rw;
//	static u16 l, lp;
////	static byte lastbit, bit;
////	static byte sc;
//	static u32 *p;
//	static u16 count;
//	static bool c;
//
//	enum {FSP = 0, SSP = 1, FH = 3, SH = 4, END = 49, ERR = 50};
//
////	*pPORTFIO_SET = 1<<2;
//
//	if (((rcvCount++) & 1) == 0) switch (stateManRcvr)
//	{
//		case 0:
//
//			l = 16;
//			lp = 8;
//			p = manRB->data;
//			count = manRB->maxLen;
//			c = false;
//
//			rw = 0;
//			rw |= *pPORTFIO & 1; // синхро бит 
//			lp--;
//
//			stateManRcvr++;
//
//			break;
//
//		case 1:
//
//			rw <<= 1;
//			rw |= *pPORTFIO & 1; // синхро бит 
//			lp--;
//
//			if (rw >= 0x1C)
//			{
//				if (((rw+1)&3) > 1)
//				{
//					stateManRcvr = 3;
//				}
//				else
//				{
//					*pTCNTL = 0;
//					stateManRcvr = 0;
//				};
//			}
//			else if (lp == 0)
//			{
////				*pPORTFIO_POLAR ^= (1<<10)|0xF1;
//				stateManRcvr++;
//			};
//
//			break;
//
//		case 2:
//
//			rw <<= 1;
//			rw |= *pPORTFIO & 1; // бит данных
//			l--;
//
//			if (l > 0)
//			{
//				stateManRcvr++;
//			}
//			else
//			{
//				*p++ = rw;
//				count--;
//
//				rw = 0;
//
//				lp = 6;
//				stateManRcvr = (count == 0) ? END : 4;
//			};
//
//			break;
//
//		case 3:
//
//			stateManRcvr--;
//
//			break;
//
//		case 4:
//
//			rw <<= 1;
//			rw |= *pPORTFIO & 1; // синхробит
//			lp--;
//
//			if (lp == 0)
//			{
//				l = 17;
//
//				if (rw == 0 || rw >=63)
//				{
//					stateManRcvr = END;
//				}
//				else 
//				{
//					c = c || !((rw == 0x38) || rw == 7);
//					stateManRcvr = 3;
//				};
//			};
//
//			break;
//
//		case END:
//			
//			manRB->OK = !c;
//			manRB->ready = true;
//			manRB->len = manRB->maxLen - count;
//			*pTCNTL = 0;
//			*pPORTFIO_MASKA = 0;
//			*pPORTFIO_CLEAR = 1<<10;
//			*pSIC_IMASK &= ~IRQ_PFA_PORTF;
//
//			rcvBusy = false;
//
//			break;
//	};
//
////	*pPORTFIO_CLEAR = (1<<2);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//EX_INTERRUPT_HANDLER(WAITMANRCVR_ISR)
//{
////	*pPORTFIO_SET = 1<<2;
//
//	*pTCNTL = TMPWR;
//
//	*pPORTFIO_CLEAR = /*(1<<2)|*/(1<<10);
//
//	*pTPERIOD = rcvQuartPeriod;
//
////	*pPORTFIO_SET = 1<<2;
//
//	*pTCNTL = TAUTORLD|TMREN|TMPWR;
//
//	if ((*pILAT & EVT_IVTMR) == 0) { rcvCount = 0; };
//
////	*pPORTFIO_CLEAR = 1<<2;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
	*pTCNTL = 0;
	*pPORTFIO_MASKA = 0;
	*pPORTFIO_CLEAR = PINIRQ;
	*pSIC_IMASK &= ~IRQ_PFA_PORTF;

	ManRcvTmrDis();
	ManRcvTmrIrqDis();

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = manRB->maxLen - rcvManCount;

	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(ManRcvTimerIRQ)
{
	static u32 rw;
	static u16 l;

	*pPORTFIO_SET = 1<<13;

//	ManRcvTmrIrqDis(); //ManTmr.IDR = CPCS;

	switch (stateManRcvr)
	{
		case 0:

			l = 16;

			rw = 0;
			rw |= ManRxd; // синхро бит 

			stateManRcvr++;

			break;

		case 1:

			rw <<= 1;
			rw |= ManRxd; // бит данных
			l--;

			if (l == 0)
			{
				*rcvManPtr++ = rw;
				rcvManCount--;

				if (rcvManCount == 0)
				{
					ManRcvEnd(true);
				}
				else
				{
					stateManRcvr++;

					ManRcvPinIrq = (void*)WaitManDataSync;

					ManRcvTmrEn(rcvSyncHalf);
					rcvDataState = 0;
				};
			};

			break;

		case 2:

			ManRcvEnd(true);

			break;

	};

	*pPORTFIO_CLEAR = 1<<13;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(WaitManCmdSync)
{
	*pPORTFIO_CLEAR = PINIRQ;
	
//	*pPORTFIO_SET = 1<<8;

	u32 t = ManPulseTmrGet();

	switch (rcvSyncState)
	{
		case 0:

			if (ManRxd)
			{
				ManPulseTmrReset();
				rcvSyncState++;
			};

			break;

		case 1:

			if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
				rcvSyncState = 0;
//				ManPulseTmrDis();
			}
			else if (t > rcvSyncPulseMax)
			{
				ManPulseTmrReset();

				ManRcvPinIrq = (void*)ManRcvSyncIrq;

				ManRcvTmrEn(rcvQuartPeriod);
				ManRcvTmrIrqEn();

				rcvSyncState = 0;
			};

			break;
	};

//	t = HW::PIOE->ISR;

//	*pPORTFIO_CLEAR = 1<<8;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(WaitManDataSync)
{
	*pPORTFIO_CLEAR = PINIRQ;
	
	u32 t = ManPulseTmrGet();

	ManRcvTmrReset(rcvSyncHalf);

	switch (rcvDataState)
	{
		case 0:

			if (ManRxd)
			{
				if (t < rcvHalfPeriod)
				{
					ManPulseTmrReset();
					rcvDataState++;
				}
				else
				{
					ManRcvEnd(true);
				};
			}
			else
			{
				if (t > rcvSyncPulseMin && t < rcvSyncHalf)
				{
					ManPulseTmrReset();
					rcvDataState += 2;
				}
				else
				{
					ManRcvEnd(true);
				};
			};

			break;

		case 1:

			if (t > rcvSyncPulseMin && t < rcvSyncPulseMax)
			{
				ManPulseTmrReset();
				rcvDataState++;
			}
			else
			{
				ManRcvEnd(true);
			};

			break;

		case 2:

			if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
				ManRcvEnd(true);
			}
			else if (t > rcvSyncPulseMax)
			{
				ManPulseTmrReset();

				ManRcvPinIrq = (void*)ManRcvSyncIrq;

				stateManRcvr = 0;

				ManRcvTmrEn(rcvQuartPeriod);
				ManRcvTmrIrqEn();

				rcvDataState = 0;
			};

			break;
	};

//	t = HW::PIOE->ISR;

//	HW::PIOE->CODR = 3;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(ManRcvSyncIrq)
{
	*pPORTFIO_CLEAR = PINIRQ;

//	*pPORTFIO_SET = 1<<13;

	u32 t = ManPulseTmrGet();

	if (t > rcvPeriodMax)
	{
		ManRcvEnd(false);
	}
	else if (t > rcvPeriodMin)
	{
		ManPulseTmrReset();

		ManRcvTmrEn(rcvQuartPeriod);
		ManRcvTmrIrqEn();
	};

//	*pPORTFIO_CLEAR = 1<<13;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//EX_INTERRUPT_HANDLER(ManRcvSync)
//{
//	if (*pSIC_ISR & IRQ_PFA_PORTF)
//	{
//		ManRcvPinIrq();
//	};
//
//	if (*pSIC_ISR & IRQ_TIMER1)
//	{
//		ManRcvTimerIRQ();
//	};
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	*pTSCALE = 0;
	*pTPERIOD = rcvQuartPeriod;
	*pEVT6 = (void*)ManRcvTimerIRQ;
	*pTCNTL = TMPWR;;

	*pIMASK |= EVT_IVTMR; 

	ManPulseTmrReset();

	//using namespace HW;

	//PMC->PCER0 = PID::TC1_M|PID::PIOE_M;

	//VectorTableExt[HW::PID::PIOE_I] = WaitManCmdSync;
	//CM4::NVIC->ICPR[0] = HW::PID::PIOE_M;
	//CM4::NVIC->ISER[0] = HW::PID::PIOE_M;	

	////HW::PIOE->IER = 1<<3;
	////HW::PIOE->IFER = 1<<3;

	//HW::PIOE->OER = 7;

	//VectorTableExt[HW::PID::TC1_I] = ManRcvIRQ;
	//CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	//CM4::NVIC->ISER[0] = HW::PID::TC1_M;	
	//ManTmr.IDR = CPCS;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
	if (rcvBusy || trmBusy || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	stateManRcvr = 0;
	rcvSyncState = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;

	rcvCount = 0;

	ManPulseTmrReset();

	ManRcvTmrDis();
	ManRcvTmrIrqDis();
	ManRcvTmrIrqPtr = (void*)ManRcvTimerIRQ;

	ManRcvPinIrq = (void*)WaitManCmdSync;
	*pSIC_IMASK |= IRQ_PFA_PORTF;
	*pIMASK |= EVT_IVG11; 

	*pPORTFIO_EDGE = PINIRQ;
	*pPORTFIO_BOTH = PINIRQ;
	*pPORTFIO_CLEAR = PINIRQ;
	*pPORTFIO_MASKA = PINIRQ;
	*pPORTFIO_MASKB = 0;

	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{

//	spi.Update();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	LowLevelInit();

	InitRTT();

	InitManTransmit();

	InitManRecieve();

//	InitSPORT();

	//InitDisplay();

//	InitADC16();

	//u32 t = GetRTT();

	//while((GetRTT() - t) < 100000)
	//{
	//	UpdateHardware();
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


