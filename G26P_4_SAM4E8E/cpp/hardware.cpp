#include "core.h"
#include "time.h"
#include "hardware.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void IntDummyHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void HardFaultHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void MemFaultHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void BusFaultHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void UsageFaultHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ExtDummyHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitVectorTable()
{
	for (u32 i = 0; i < ArraySize(VectorTableInt); i++)
	{
		VectorTableInt[i] = IntDummyHandler;
	};

	for (u32 i = 0; i < ArraySize(VectorTableExt); i++)
	{
		VectorTableExt[i] = ExtDummyHandler;
	};

	VectorTableInt[3] = HardFaultHandler;
	VectorTableInt[4] = MemFaultHandler;
	VectorTableInt[5] = BusFaultHandler;
	VectorTableInt[6] = UsageFaultHandler;

	CM4::SCB->VTOR = (u32)VectorTableInt;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ HW::PIOB->ODSR = 0x06;} // 0110  
inline void ManOne()		{ HW::PIOB->ODSR = 0x08;} // 1000
inline void ManZero()		{ HW::PIOB->ODSR = 0x0D;} // 1101

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define BOUD2CLK(x) ((u32)((MCK/2.0)/x+0.5))
#define ManTmr HW::TC0->C1

static const u16 manboud[4] = { BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz


u16 trmHalfPeriod = BOUD2CLK(41666);
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&3];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;
const u16 rcvQuartPeriod = BOUD2CLK(20833)/2;

static MRB *manRB = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool trmStartCmd = false;
bool trmStartData = false;

u32 icount = 0;
static byte ib;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ()
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

			ManTmr.CCR = CLKDIS; 

			//*pTIMER_DISABLE = TIMDIS1;
			//*pSIC_IMASK &= ~IRQ_TIMER1;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

	u32 tmp = ManTmr.SR;

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

	VectorTableExt[HW::PID::TC1_I] = ManTrmIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;	
	ManTmr.IER = CPCS;

	ManTmr.RC = trmHalfPeriod;

	ManTmr.CMR = CPCTRG;
	
	ManTmr.CCR = CLKEN|SWTRG;

	return trmBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit()
{
	using namespace HW;

	PMC->PCER0 = PID::TC0_M;

	PIOB->OWER = 0xF;


	ManTmr.RC = trmHalfPeriod;
	ManTmr.CMR = CPCTRG;
	ManTmr.CCR = CLKDIS;

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
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//static void InitManRecieve()
//{
//	*pTSCALE = 0;
//	*pTPERIOD = rcvQuartPeriod;
//	*pEVT6 = (void*)MANRCVR_ISR;
//	*pTCNTL = TMPWR;;
//
//	*pIMASK |= EVT_IVTMR; 
//}
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool RcvManData(MRB *mrb)
//{
//	if (rcvBusy || trmBusy || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
//	{
//		return false;
//	};
//
//	mrb->ready = mrb->OK = false;
//	mrb->len = 0;
//
//	manRB = mrb;
//	
//	stateManRcvr = 0;
//	rcvCount = 0;
//
//	*pEVT11 = (void*)WAITMANRCVR_ISR;
//	*pSIC_IMASK |= IRQ_PFA_PORTF;
//	*pIMASK |= EVT_IVG11; 
//
//	*pPORTFIO_EDGE = 1<<10;
//	*pPORTFIO_BOTH = 1<<10;
//	*pPORTFIO_CLEAR = 1<<10;
//	*pPORTFIO_MASKA = 1<<10;
//	*pPORTFIO_MASKB = 0;
//
//	return rcvBusy = true;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitVectorTable();

	InitTimer();
	RTT_Init();

	InitManTransmit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
