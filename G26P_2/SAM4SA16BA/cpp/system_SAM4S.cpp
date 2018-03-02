#include "core.h"
#include "hardware.h"
#include "time.h"

#pragma O3
#pragma Otime

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

	__enable_irq();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" void SystemInit (void)
{
	using namespace HW;

	//MATRIX->SCFG[0] |= 0x00060000;
	//MATRIX->SCFG[1] |= 0x00060000;

	InitVectorTable();

	EFC0->FMR = 0x0500;
	EFC1->FMR = 0x0500;

	PMC->PCER0 = PID::PIOA_M|PID::PIOB_M|PID::UART0_M|PID::UART1_M|PID::USART0_M|PID::USART1_M|PID::TWI1_M;
	//PMC->PCER1 = PID::ADC_M|PID::PWM_M|PID::DMAC_M;

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOA->PDR =		0x01007660;	// 0000 0001 0000 0000 0111 0110 0110 0000
	PIOA->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOA->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOA->PER = 	0xFEFF899F;	// 1111 1110 1111 1111 1000 1001 1001 1111		
	PIOA->OER = 	0xFEFE099F;	// 1111 1110 1110 1110 0000 1001 1001 1111		
	PIOA->ODR = 	0x00018000;	// 0000 0000 0000 0001 1000 0000 0000 0000		
	//PIOA->MDDR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOA->OWER =	0x0000000F;	// 0000 0000 0000 0000 0000 0000 0000 1111		

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOB->PDR =		0x0000003C;	// 0000 0000 0000 0000 0000 0000 0011 1100
	PIOB->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->PER = 	0x00006D03;	// 0000 0000 0000 0000 0110 1101 0000 0011		
	PIOB->OER = 	0x00006D00;	// 0000 0000 0000 0000 0110 1101 0000 0000		
	PIOB->ODR = 	0x00000003;	// 0000 0000 0000 0000 0000 0000 0000 0011		
	//PIOB->MDDR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	//PIOB->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		


	MATRIX->SYSIO = (1<<4)|(1<<5)|(1<<10)|(1<<11); // PB4 PB5 PB10 PB11

//	WDT->MR = (((1000 * 32768 / 128 + 500) / 1000) & 0xFFF) | 0x0FFF6000;
	HW::WDT->MR = 0x8000;	// Disable Watchdog

	PMC->MOR	= 0x01370102; while ((PMC->SR & 1) == 0) {};

	PMC->PLLAR	= 0x20030101; while ((PMC->SR & 2) == 0) {}; 

//	PMC->MCKR = (PMC->MCKR & 3) | 0x10; while ((PMC->SR & 8) == 0) {}; 

	PMC->MCKR = 0x02; while ((PMC->SR & 8) == 0) {};

	PMC->PCK[1] = 0x64;
	PMC->SCER = 0x200;



// Init ADC

	PMC->PCER0 = PID::ADC_M;

	ADC->MR = 0x2F3FFF80;
	ADC->ACR = 0x00;
	ADC->CHER = 0x0008;
	ADC->CR = 10;


	PIOA->PUDR = 1<<20;
	PIOA->PPDDR = 1<<20;

	__asm { DSB };
	__asm { ISB };

	CMCC->CTRL = 1;		// cache enable
	CMCC->MAINT0 = 1;	// invalidate all cache entries

	__asm { DSB };
	__asm { ISB };

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ HW::PIOA->ODSR = 0x05;} // 0101  
inline void ManOne()		{ HW::PIOA->ODSR = 0x05; __nop(); __nop(); __nop(); HW::PIOA->ODSR = 0x03;} // 1100
inline void ManZero()		{ HW::PIOA->ODSR = 0x05; __nop(); __nop(); __nop(); HW::PIOA->ODSR = 0x0C;} // 0011

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static byte manInv = 0;

#define BOUD2CLK(x) ((u32)((MCK/2.0)/x+0.5))
#define ManTmr HW::TC0->C2
#define ManRxd (((HW::PIOB->PDSR)^manInv)&1)

static const u16 manboud[4] = { BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz


u16 trmHalfPeriod = BOUD2CLK(20833)/2;
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&3]/2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;

const u16 rcvPeriod = BOUD2CLK(20833);
//const u16 rcvHalfPeriod = rcvPeriod/2;
//const u16 rcvHalfPeriodMin = rcvHalfPeriod*0.75;
//const u16 rcvHalfPeriodMax = rcvHalfPeriod*1.25;
//const u16 rcvQuartPeriod = rcvPeriod/4;
//const u16 rcvSyncPulse = rcvPeriod * 1.5;
//const u16 rcvSyncPulseMin = rcvSyncPulse * 0.8;
//const u16 rcvSyncPulseMax = rcvSyncPulse * 1.2;
//const u16 rcvSyncHalf = rcvSyncPulseMax + rcvHalfPeriod;
//const u16 rcvPeriodMin = rcvPeriod * 0.8;
//const u16 rcvPeriodMax = rcvPeriod * 1.2;

//static byte rcvSyncState = 0;
//static byte rcvDataState = 0;

static u16* rcvManPtr = 0;
static u16 rcvManCount = 0;


static u16 rcvManLen = 0;
static u32 rcvManPrevTime = 0;


static MRB *manRB = 0;


static __irq void WaitManCmdSync();
//static __irq void WaitManDataSync();
//static __irq void ManRcvSync();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool trmStartCmd = false;
//bool trmStartData = false;

//u32 icount = 0;
//static byte ib;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// 11110, 01001, 10100, 10101, 01010, 01011, 01110, 01111, 10010, 10011, 10110, 10111, 11010, 11011, 11100, 11101

byte tbl_4B5B[16] = { 0x1E, 0x09, 0x14, 0x15, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13, 0x16, 0x17, 0x1A, 0x1B, 0x1C, 0x1D };

inline u32 Encode_4B5B(u16 v) { return tbl_4B5B[v&15]|(tbl_4B5B[(v>>4)&15]<<5)|(tbl_4B5B[(v>>8)&15]<<10)|(tbl_4B5B[(v>>12)&15]<<15); }


// 0 - L1
// 1 - H1
// 2 - H2
// 3 - L2

// L2 H2 H1 L1
// Z - 1111, P - 1100, Z - 1111, N - 0011

byte mltArr[4] = { 0x03, 0x0C, 0x03, 0x0C };
byte mltSeq = 0;

#define MltTmr HW::TC0->C1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void MltOff()	{ HW::PIOA->ODSR = 0x05;} // 0110  
inline void MltZ()		{ HW::PIOA->ODSR = 0x05; mltSeq = 0; HW::PIOA->ODSR = 0x0F;} // 1111
inline void MltNext()	{ HW::PIOA->ODSR = 0x05; mltSeq = (mltSeq+1)&3; HW::PIOA->ODSR = mltArr[mltSeq]; }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte stateMLT3 = 0;
static MTB *mltTB = 0;
static bool mltBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void MLT3_TrmIRQ()
{
	static u32 tw = 0;
//	static u32 pw = 0;
	static byte nrz = 0;
//	static byte pnrz = 0;
	static u16 count = 0;
//	static byte i = 0;
	static const u16 *data = 0;
	static u16 len = 0;


	switch (stateMLT3)
	{
		case 0:	// Idle; 

			MltZ();
		
			data = mltTB->data;
			len = mltTB->len;

//			pw = 0;
			tw = Encode_4B5B(*data);

			data++;
			len--;

			nrz = 0;

			count = 20;

			stateMLT3 = 1;

			break;

		case 1: // Start data

			if (tw & 0x80000)
			{
				nrz ^= 1;
//				MltNext();
			};

			if (nrz != 0)
			{
				MltNext();
			};

			count--;
//			pw = tw;
			tw <<= 1;

			if (count == 0)
			{
				if (len > 0)
				{
					tw = Encode_4B5B(*data);

					data++;
					len--;
					count = 20;
				}
				else
				{
					stateMLT3++;
				};
			};

			break;

		case 2:	

			MltOff();
			stateMLT3 = 0;

			MltTmr.IDR = CPCS;
			MltTmr.CCR = CLKDIS; 

			//*pTIMER_DISABLE = TIMDIS1;
			//*pSIC_IMASK &= ~IRQ_TIMER1;

			mltTB->ready = true;
			mltBusy = false;

			break;


	}; // 	switch (stateManTrans)

	u32 tmp = MltTmr.SR;

//	HW::PIOE->CODR = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendMLT3(MTB *mtb)
{
	if (mltBusy || mtb == 0 || mtb->data == 0 || mtb->len == 0)
	{
		return false;
	};

	mtb->ready = false;

	mltTB = mtb;

	stateMLT3 = 0;

	HW::PIOB->IDR = 1;

	VectorTableExt[HW::PID::TC1_I] = MLT3_TrmIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	CM4::NVIC->ISER[0] = HW::PID::TC1_M;	
	MltTmr.IER = CPCS;

	u32 tmp = MltTmr.SR;

	MltTmr.RC = trmHalfPeriod;

	MltTmr.CMR = CPCTRG;
	
	MltTmr.CCR = CLKEN|SWTRG;

	return mltBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static const u16 *data = 0;
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

	HW::PIOB->IDR = 1;

	HW::PMC->PCER0 = HW::PID::TC2_M;

	VectorTableExt[HW::PID::TC2_I] = ManTrmIRQ;
	CM4::NVIC->ICPR[0] = HW::PID::TC2_M;
	CM4::NVIC->ISER[0] = HW::PID::TC2_M;	
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

	PMC->PCER0 = PID::TC2_M;

	PIOA->OWER = 0xF;

	//ManTmr.RC = trmHalfPeriod;
	//ManTmr.CMR = CPCTRG;
	ManTmr.CCR = CLKDIS;

	ManDisable();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
	HW::PIOB->SODR = 1<<8;

	HW::PIOB->IDR = 1;
	ManTmr.IDR = CPCS;

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = rcvManLen;
	ManTmr.CCR = CLKDIS;
	
	rcvBusy = false;

	HW::PIOB->CODR = 1<<8;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
static __irq void ManRcvIRQ()
{
	static u32 rw;
	static u16 l;

	HW::PIOB->SODR = 1<<14;

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

					VectorTableExt[HW::PID::PIOB_I] = WaitManDataSync;

					ManTmr.CCR = CLKEN|SWTRG;
					ManTmr.RC = rcvSyncHalf;
					rcvDataState = 0;
				};
			}
			else
			{
				ManTmr.IDR = CPCS;
			};

			break;

		case 2:

			ManRcvEnd(true);

			break;

	};


	u32 tmp = ManTmr.SR;

	HW::PIOB->CODR = 1<<14;
}
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
static __irq void WaitManCmdSync()
{
	u32 t = ManTmr.CV;

	bool c = false;


	switch (rcvSyncState)
	{
		case 0:

			if (ManRxd)
			{
				ManTmr.CCR = CLKEN|SWTRG;
				rcvSyncState++;
			};

			break;

		case 1:


			if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
	HW::PIOB->SODR = 1<<8;
				rcvSyncState = 0;
				ManTmr.CCR = CLKDIS|SWTRG;
			}
			else if (t < rcvSyncPulseMax)
			{
	HW::PIOB->SODR = 1<<10;
				rcvSyncState++;
				ManTmr.CCR = CLKEN|SWTRG;
			}
			else
			{
	HW::PIOB->SODR = 1<<11;
				c = true; 
				rcvSyncState = 0;
			};

			break;

		case 2:

			if (t < rcvHalfPeriodMin)
			{
	HW::PIOA->SODR = 1<<7;
				rcvSyncState = 0;
				ManTmr.CCR = CLKDIS|SWTRG;
			}
			else if (t < rcvHalfPeriodMax)
			{
	HW::PIOA->SODR = 1<<21;
				c = true; 
				rcvSyncState = 0;
			}
			else if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
	HW::PIOA->SODR = 1<<27;
				rcvSyncState = 0;
				ManTmr.CCR = CLKDIS|SWTRG;
			}
			else if (t > rcvSyncPulseMax)
			{
	HW::PIOA->SODR = 1<<29;
				manInv ^= 1;

				c = true; 
				rcvSyncState = 0;
			};

			break;
	};

	if (c)
	{
		VectorTableExt[HW::PID::PIOB_I] = ManRcvSync;

		t = ManTmr.SR;

		ManTmr.CMR = 0;
		ManTmr.CCR = CLKEN|SWTRG;
		ManTmr.IER = CPCS;
		ManTmr.RC = rcvQuartPeriod;
	};

	t = HW::PIOB->ISR;

	HW::PIOA->CODR = (1<<7)|(1<<21)|(1<<27)|(1<<29);
	HW::PIOB->CODR = (1<<8)|(1<<10)|(1<<11);
}
*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*static __irq void WaitManDataSync()
{
//	HW::PIOE->SODR = 1;

	u32 t = ManTmr.CV;


	switch (rcvDataState)
	{
		case 0:

			if (ManRxd)
			{
				if (t < rcvHalfPeriod)
				{
					ManTmr.CCR = CLKEN|SWTRG;
					rcvDataState++;
				}
				else
				{
//	HW::PIOA->SODR = 1<<7;

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
//	HW::PIOA->SODR = 1<<21;
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
//	HW::PIOA->SODR = 1<<27;
				ManRcvEnd(true);
			};

			break;

		case 2:

			if (t < rcvSyncPulseMin || t > rcvSyncHalf)
			{
//	HW::PIOA->SODR = 1<<29;
				ManRcvEnd(true);
			}
			else if (t > rcvSyncPulseMax)
			{
				VectorTableExt[HW::PID::PIOB_I] = ManRcvSync;

				t = ManTmr.SR;

				stateManRcvr = 0;

				ManTmr.IER = CPCS;
				ManTmr.RC = rcvQuartPeriod;
				ManTmr.CMR = 0;
				ManTmr.CCR = CLKEN|SWTRG;

				rcvDataState = 0;
			};

			break;
	};

	t = HW::PIOB->ISR;

//	HW::PIOA->CODR = (1<<7)|(1<<21)|(1<<27)|(1<<29);
}*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
static __irq void ManRcvSync()
{
	u32 t = ManTmr.CV;

	if (t > rcvPeriodMax)
	{
		ManRcvEnd(false);
	}
	else if (t > rcvPeriodMin)
	{

		t = ManTmr.SR;

		ManTmr.IER = CPCS;
		ManTmr.RC = rcvQuartPeriod;
		ManTmr.CMR = 0;
		ManTmr.CCR = CLKEN|SWTRG;
	}
	else
	{
//		if ((ManTmr.SR & CLKSTA) == 0)
		{
//	HW::PIOB->SODR = 1<<11;
//			ManTmr.CCR = CLKEN|SWTRG;
//	HW::PIOB->CODR = 1<<11;
		};
	};

	t = HW::PIOB->ISR;

//	HW::PIOA->CODR = (1<<7)|(1<<21)|(1<<27)|(1<<29);
}
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Receiver
{
	private:
		u32 _number;
		u32 _length;
		u32 _data_temp;
		u32 _data;
		bool _command_temp;
		bool _command;
		bool _parity_temp;
		bool _parity;
		bool _sync;
		bool _state;

    public:

		enum status_type
		{
			STATUS_WAIT = 0,
			STATUS_SYNC,
			STATUS_HANDLE,
			STATUS_CHECK,
			STATUS_READY,
			STATUS_ERROR_LENGTH,
			STATUS_ERROR_STRUCT,
		};

		Receiver(bool parity);
		status_type Parse(u16 len);	
		u16 GetData() { return _data; }
		bool GetType() { return _command; }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Receiver::Receiver(bool parity)
{
	_state = false;
	_number = 0;
	_length = 0;
	_data_temp = 0;
	_data = 0;
	_command_temp = false;
	_command = false;
	_parity = parity;
	_parity_temp = false;
	_sync = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Receiver::status_type Receiver::Parse(u16 len)
{	
	_state = !_state;

	if((len <= 25) || (len > 225))
	{
//		HW::PIOB->SODR = 1<<10;

		_number = 0;
		_length = 0;
		_sync = false;

//		HW::PIOB->CODR = 1<<10;

		return STATUS_ERROR_LENGTH;
	}
	else if(len <= 75)                
	{	
		_length++;
	}
	else if(len <= 125)
	{	
		_length += 2;
	}
	else
	{
		_sync = true;
		_data_temp = 0;
		_parity_temp = _parity;
		_number = 0;
		_length = (len <= 175) ? 1 : 2;
		_command_temp = !_state; 
	};

	if(_length >= 3)
	{
		_number = 0;
		_length = 0;
		_sync = false;


		return STATUS_ERROR_STRUCT;
	};

	if(_sync)
	{
		if(_length == 2)
		{
			if(_number < 16)
			{
				_data_temp <<= 1;
				_data_temp |= _state;
				_parity_temp ^= _state;
				_number ++;
				_length = 0;
			}
		 	else
			{
				_data = _data_temp;
				_data_temp = 0;
				_command = _command_temp;
				_command_temp = false;
				_number = 0;
				_length = 0;
				_sync = false;

				if(_state != _parity_temp)
				{
					_state = !_state;
					_data = (~_data);
					_command = !_command;
				};

				return STATUS_READY;
			};
		};

		return (_number < 16) ? STATUS_HANDLE : STATUS_CHECK;
	};

	return STATUS_WAIT;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Receiver manRcv(true);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void WaitManCmdSync()
{
	u32 t = ManTmr.CV;

//	HW::PIOB->SODR = 1<<10;

	ManTmr.CCR = CLKEN|SWTRG;

	if(manRcv.Parse(t * 100 / rcvPeriod) == manRcv.STATUS_READY)
	{
		rcvManPrevTime = GetRTT();

		if (rcvManLen == 0)
		{
			if(manRcv.GetType())
			{
//				HW::PIOB->SODR = 1<<11;

				*rcvManPtr++ = manRcv.GetData();
				rcvManLen = 1;

//				HW::PIOB->CODR = 1<<11;
			}
		}
		else 
		{
			if(rcvManLen < rcvManCount)
			{
				*rcvManPtr++ = manRcv.GetData();
			};

			rcvManLen += 1;	
		};
	};

	t = HW::PIOB->ISR;

//	HW::PIOB->CODR = 1<<10;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		bool c = ManTmr.SR & CPCS;

		if (rcvManLen > 0 && c)//(GetRTT() - rcvManPrevTime) > US2RT(3000)))
		{
//			HW::PIOB->SODR = 1<<10;

			ManRcvEnd(true);

//			HW::PIOB->CODR = 1<<10;
		}
//		else if (rcvManLen >= rcvManCount)
//		{
////			HW::PIOB->SODR = 1<<10;
//
//			ManRcvEnd(true);
//
////			HW::PIOB->CODR = 1<<10;
//		}
		else
		{
			manRB->len = rcvManLen;
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvStop()
{
	ManRcvEnd(true);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	using namespace HW;

	PMC->PCER0 = PID::TC2_M|PID::PIOB_M;

	VectorTableExt[HW::PID::PIOB_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOB_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOB_M;	

	HW::PIOB->IER = 1;
	HW::PIOB->IFER = 1;

//	HW::PIOA->OER = 15;

	//VectorTableExt[HW::PID::TC2_I] = ManRcvIRQ;
	//CM4::NVIC->ICPR[0] = HW::PID::TC2_M;
	//CM4::NVIC->ISER[0] = HW::PID::TC2_M;

	HW::PMC->PCER0 = HW::PID::TC2_M;

	ManTmr.CMR = 0x8040;
	ManTmr.RC = 15000;
	ManTmr.CCR = CLKEN|SWTRG;
	ManTmr.IDR = -1;//CPCS;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
//	HW::PIOE->SODR = 1;

	if (rcvBusy || trmBusy || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

//	HW::PIOE->SODR = 2;

	ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	//stateManRcvr = 0;
	//rcvSyncState = 0;

	rcvManLen = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;


	VectorTableExt[HW::PID::PIOB_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOB_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOB_M;	

	HW::PIOB->IER = 1;
	HW::PIOB->IFER = 1;

//	tmp = HW::PIOB->ISR;

	HW::PMC->PCER0 = HW::PID::TC2_M;

	ManTmr.CCR = CLKDIS|SWTRG;
	ManTmr.IDR = -1;
	ManTmr.RC = 15000;
	ManTmr.CMR = 0x8040;

	u32 tmp = ManTmr.SR;

	//HW::PMC->PCER0 = HW::PID::TC3_M;

	//ManTimeout.CCR = CLKDIS|SWTRG;
	//ManTimeout.IER = CPCS;
	//ManTimeout.RC = US2RT(1440);
	//ManTimeout.CMR = WAVE|CPCDIS|CPCSTOP|TIMER_CLOCK5;

	//VectorTableExt[HW::PID::TC3_I] = ManRcvIRQ;
	//CM4::NVIC->ICPR[0] = HW::PID::TC3_M;
	//CM4::NVIC->ISER[0] = HW::PID::TC3_M;	


	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitManTransmit();
	InitManRecieve();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
