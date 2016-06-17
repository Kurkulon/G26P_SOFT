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
inline void ManOne()		{ HW::PIOB->ODSR = 0x06; __nop(); __nop(); __nop(); HW::PIOB->ODSR = 0x03;} // 1100
inline void ManZero()		{ HW::PIOB->ODSR = 0x06; __nop(); __nop(); __nop(); HW::PIOB->ODSR = 0x0C;} // 0011

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define BOUD2CLK(x) ((u32)((MCK/2.0)/x+0.5))
#define ManTmr HW::TC0->C1
#define ManRxd ((HW::PIOE->PDSR>>3)&1)

static const u16 manboud[4] = { BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz


u16 trmHalfPeriod = BOUD2CLK(20833)/2;
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&3];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;

const u16 rcvPeriod = BOUD2CLK(20833);
const u16 rcvHalfPeriod = rcvPeriod/2;
const u16 rcvQuartPeriod = rcvPeriod/4;
const u16 rcvSyncPulseMin = rcvPeriod * 1.4;
const u16 rcvSyncPulseMax = rcvPeriod * 1.6;
const u16 rcvSyncHalf = rcvSyncPulseMax + rcvHalfPeriod;
const u16 rcvPeriodMin = rcvPeriod * 0.9;
const u16 rcvPeriodMax = rcvPeriod * 1.2;

static byte rcvSyncState = 0;
static byte rcvDataState = 0;

static u32* rcvManPtr = 0;
static u16 rcvManCount = 0;

static MRB *manRB = 0;


static __irq void WaitManCmdSync();
static __irq void WaitManDataSync();
static __irq void ManRcvSync();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool trmStartCmd = false;
//bool trmStartData = false;

//u32 icount = 0;
//static byte ib;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static u16 *data = 0;
	static u16 len = 0;


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

//			HW::PIOE->SODR = 1;

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

//			ManOne();
			stateManTrans++;

			break;

		case 7:

			ManDisable();
			stateManTrans = 0;

			ManTmr.IDR = CPCS;
			ManTmr.CCR = CLKDIS; 

			//*pTIMER_DISABLE = TIMDIS1;
			//*pSIC_IMASK &= ~IRQ_TIMER1;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

	u32 tmp = ManTmr.SR;

//	HW::PIOE->CODR = 1;
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

	HW::PIOE->IDR = 1<<3;

	VectorTableExt[HW::PID::TC1_I] = ManTrmIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;	
	ManTmr.IER = CPCS;

	u32 tmp = ManTmr.SR;

	ManTmr.RC = trmHalfPeriod;

	ManTmr.CMR = CPCTRG;
	
	ManTmr.CCR = CLKEN|SWTRG;

	return trmBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit()
{
	using namespace HW;

	PMC->PCER0 = PID::TC1_M;

	PIOB->OWER = 0xF;

	//ManTmr.RC = trmHalfPeriod;
	//ManTmr.CMR = CPCTRG;
	ManTmr.CCR = CLKDIS;

	ManDisable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
	HW::PIOE->IDR = 1<<3;
	ManTmr.IDR = CPCS;

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = manRB->maxLen - rcvManCount;
	ManTmr.CCR = CLKDIS;
	
	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManRcvIRQ()
{
	static u32 rw;
	static u16 l;

	HW::PIOE->SODR = 4;

	switch (stateManRcvr)
	{
		case 0:

			l = 16;

			rw = 0;
			rw |= ManRxd; // синхро бит 

			stateManRcvr++;

			ManTmr.IDR = CPCS;

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

					VectorTableExt[HW::PID::PIOE_I] = WaitManDataSync;

					ManTmr.CCR = CLKEN|SWTRG;
					ManTmr.RC = rcvSyncHalf;
					rcvDataState = 0;
				};
			};

			break;

		case 2:

			ManRcvEnd(true);

			break;

	};


	u32 tmp = ManTmr.SR;

	HW::PIOE->CODR = 4;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void WaitManCmdSync()
{
	u32 t = ManTmr.CV;

	switch (rcvSyncState)
	{
		case 0:

			if (HW::PIOE->PDSR & 8)
			{
				ManTmr.CCR = CLKEN|SWTRG;
				rcvSyncState++;
			};

			break;

		case 1:

			if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
				rcvSyncState = 0;
				ManTmr.CCR = CLKDIS|SWTRG;
			}
			else if (t > rcvSyncPulseMax)
			{
				HW::PIOE->SODR = 1;

				VectorTableExt[HW::PID::PIOE_I] = ManRcvSync;

				t = ManTmr.SR;

				ManTmr.CCR = CLKEN|SWTRG;
				ManTmr.IER = CPCS;
				rcvSyncState = 0;
			};

			break;
	};

	t = HW::PIOE->ISR;

	HW::PIOE->CODR = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void WaitManDataSync()
{
	HW::PIOE->SODR = 1;

	u32 t = ManTmr.CV;

	switch (rcvDataState)
	{
		case 0:

			if (HW::PIOE->PDSR & 8)
			{
				if (t < rcvHalfPeriod)
				{
					ManTmr.CCR = CLKEN|SWTRG;
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
					ManTmr.CCR = CLKEN|SWTRG;
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
				ManTmr.CCR = CLKEN|SWTRG;
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
				VectorTableExt[HW::PID::PIOE_I] = ManRcvSync;

				t = ManTmr.SR;

				stateManRcvr = 0;

				ManTmr.CCR = CLKEN|SWTRG;
				ManTmr.IER = CPCS;
				ManTmr.RC = rcvQuartPeriod;

				rcvDataState = 0;
			};

			break;
	};

	t = HW::PIOE->ISR;

	HW::PIOE->CODR = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManRcvSync()
{

	u32 t = ManTmr.CV;

	if (t > rcvPeriodMax)
	{
		ManRcvEnd(false);
	}
	else if (t > rcvPeriodMin)
	{
		HW::PIOE->SODR = 2;

		t = ManTmr.SR;

		ManTmr.CCR = CLKEN|SWTRG;
		ManTmr.IER = CPCS;
	};

	t = HW::PIOE->ISR;

	HW::PIOE->CODR = 2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	using namespace HW;

	PMC->PCER0 = PID::TC1_M|PID::PIOE_M;

	VectorTableExt[HW::PID::PIOE_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOE_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOE_M;	

	//HW::PIOE->IER = 1<<3;
	//HW::PIOE->IFER = 1<<3;

	HW::PIOE->OER = 7;

	VectorTableExt[HW::PID::TC1_I] = ManRcvIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;	
	ManTmr.IDR = CPCS;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
	if (rcvBusy || trmBusy || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	stateManRcvr = 0;
	rcvSyncState = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;


	VectorTableExt[HW::PID::PIOE_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOE_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOE_M;	

	HW::PIOE->IER = 1<<3;
	HW::PIOE->IFER = 1<<3;

	ManTmr.CCR = CLKDIS|SWTRG;
	ManTmr.IDR = CPCS;
	ManTmr.RC = rcvQuartPeriod;
	ManTmr.CMR = 0;

	u32 tmp = ManTmr.SR;

	VectorTableExt[HW::PID::TC1_I] = ManRcvIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;	


	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitVectorTable();

	InitTimer();
	RTT_Init();

	InitManTransmit();
	InitManRecieve();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
