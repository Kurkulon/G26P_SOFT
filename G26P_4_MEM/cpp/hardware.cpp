#include "core.h"
#include "time.h"
#include "hardware.h"
#include "hw_conf.h"
#include "hw_rtm.h"
#include "SEGGER_RTT.h"
#include "CRC16.h"

#ifndef WIN32

//#include "system_XMC4800.h"

#else

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>

#endif 

//#pragma O3
//#pragma Otime


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define D_FIRE		(1<<16)
//#define SYNC		(1<<20)
//#define SPI			HW::SPI5
//#define L1			(1<<0)
//#define H1			(1<<1)
//#define L2			(1<<9)
//#define H2			(1<<10)
//#define RXD			(1<<5) 
//#define ERR			(1<<4) 
//#define ENVCORE		(1<<6) 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void EnableVCORE() { PIO_ENVCORE->CLR(ENVCORE); }
inline void DisableVCORE() { PIO_ENVCORE->SET(ENVCORE); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init) { return GetCRC16(data, len, init, 0); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ PIO_MANCH->CLR(L1|L2); PIO_MANCH->SET(H1|H2);} // 0110  
inline void ManZero()		{ PIO_MANCH->CLR(L2); PIO_MANCH->SET(L1|H1); PIO_MANCH->CLR(H2);} // 1100
inline void ManOne()		{ PIO_MANCH->CLR(L1); PIO_MANCH->SET(L2|H2); PIO_MANCH->CLR(H1);} // 0011

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static byte manInv = 0;

//#define BOUD2CLK(x) ((u32)((MCK/8.0)/x+0.5))
//#define ManTT		HW::CCU41_CC40
//#define ManRT		HW::CCU41_CC42
//#define ManTmr		HW::CCU41_CC41
//#define ManCCU		HW::CCU41
//#define ManRxd()	(HW::PIOA->IN & 1)
//#define MT(v)		((u16)((MCK_MHz*(v)+64)/128))

// RXD CCU41.IN2C

static const u16 manbaud[5] = { BAUD2CLK(20833), BAUD2CLK(41666), BAUD2CLK(62500), BAUD2CLK(83333), BAUD2CLK(104166) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz

static u16 trmHalfPeriod = (manbaud[0]+1)/2;
static u16 trmHalfPeriod2 = manbaud[0];
static u16 trmHalfPeriod3 = (manbaud[0]*3+1)/2;
static u16 trmHalfPeriod4 = manbaud[0] * 2;
static u16 trmHalfPeriod6 = manbaud[0] * 3;
static u16 trmHalfPeriod7 = (manbaud[0] * 7 + 1) / 2;


//u16 trmHalfPeriod = BAUD2CLK(20833)/2;
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static u16 GetTrmBaudRate(byte i)
{
	if (i >= ArraySize(manbaud)) { i = ArraySize(manbaud) - 1; };

	return (manbaud[i]+1)/2;
}
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = GetTrmBaudRate(i);
	trmHalfPeriod2 = trmHalfPeriod * 2 - 1;
	trmHalfPeriod3 = trmHalfPeriod * 3 - 1;
	trmHalfPeriod4 = trmHalfPeriod * 4 - 1;
	trmHalfPeriod6 = trmHalfPeriod * 6 - 1;
	trmHalfPeriod7 = trmHalfPeriod * 7 - 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;

const u16 rcvPeriod = BAUD2CLK(20833);

static u16* rcvManPtr = 0;
static u16 rcvManCount = 0;

static u16 rcvManLen = 0;
static u16 rcvManPrevTime = 0;

static MRB *manRB = 0;


//static __irq void WaitManCmdSync();
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

u32 mltArr[4] = { 0, L2|H2, 0, L1|H1 };
byte mltSeq = 0;

#define MltTmr HW::CCU40_CC40
#define MLT3_JK 0x311	//11000 10001
#define MLT3_TR	0x1A7	//01101 00111

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void MltOff()	{ HW::P0->CLR(L1|L2); HW::P0->SET(H1|H2);} // 0110  
inline void MltZ()		{ HW::P0->SET(H1|H2); mltSeq = 0; HW::P0->SET(L1|L2);} // 1111
inline void MltNext()	{ HW::P0->SET(H1|H2); mltSeq = (mltSeq+1)&3; HW::P0->SET(L1|L2); HW::P0->CLR(mltArr[mltSeq]); }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte stateMLT3 = 0;
static MTB *mltTB = 0;
static bool mltBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

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
		
			data = mltTB->data1;
			len = mltTB->len1;

//			pw = 0;
			tw = MLT3_JK<<10; //11000 10001 //Encode_4B5B(*data);

			//data++;
			//len--;

			nrz = 0;

			count = 10;

			stateMLT3 = 1;

			break;

		case 1: // Start data

			if (tw & 0x80000)
			{
				HW::P5->BSET(1);

//				nrz ^= 1;
				MltNext();
			}
			else
			{
				HW::P5->BCLR(1);
			};

			//if (nrz != 0)
			//{
			//	MltNext();
			//};

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
				else if (data != 0)
				{
					tw = MLT3_TR<<10;
					count = 10;
					data = 0;
				}
				else
				{
					stateMLT3++;
				};
			};

			break;

		case 2:

			MltZ();
			stateMLT3++;

			break;

		case 3:

			MltOff();
			stateMLT3 = 0;

			ManTT->TCCLR = CC4_TRBC;

			mltTB->ready = true;
			mltBusy = false;

			break;


	}; // 	switch (stateManTrans)

//	u32 tmp = MltTmr.SR;

}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendMLT3(MTB *mtb)
{
#ifndef WIN32
	if (mltBusy || mtb == 0 || mtb->data1 == 0 || mtb->len1 == 0)
	{
		return false;
	};

	mtb->ready = false;

	mltTB = mtb;

	stateMLT3 = 0;

	ManTT->PRS = trmHalfPeriod-1;
	ManTT->PSC = 3; //0.08us

	ManCCU->GCSS = CCU4_S0SE;  

	VectorTableExt[CCU40_0_IRQn] = MLT3_TrmIRQ;
	CM4::NVIC->CLR_PR(CCU40_0_IRQn);
	CM4::NVIC->SET_ER(CCU40_0_IRQn);	

	ManTT->SRS = 0;

	ManTT->SWR = ~0;
	ManTT->INTE = CC4_PME;

	ManTT->TCSET = CC4_TRBS;

	return mltBusy = true;

#else

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef MAN_TRANSMIT_V1

#ifndef WIN32

static __irq void ManTrmIRQ()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static const u16 *data = 0;
	static u16 len = 0;

//	HW::P5->BSET(7);

	switch (stateManTrans)
	{
		case 0:	// Idle; 

			ManDisable();
		
			data = manTB->data1;
			len = manTB->len1;
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

			ManTT->TCCLR = CC4_TRBC;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

//	HW::P5->BCLR(7);
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData(MTB *mtb)
{
#ifndef WIN32

	if (trmBusy || rcvBusy || mtb == 0 || mtb->data1 == 0 || mtb->len1 == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	stateManTrans = 0;

	ManTT->PRS = trmHalfPeriod-1;
	ManTT->PSC = 3; //0.08us

	ManCCU->GCSS = CCU4_S0SE;  

	VectorTableExt[CCU40_0_IRQn] = ManTrmIRQ;
	CM4::NVIC->CLR_PR(CCU40_0_IRQn);
	CM4::NVIC->SET_ER(CCU40_0_IRQn);	

	ManTT->SRS = 0;

	ManTT->SWR = ~0;
	ManTT->INTE = CC4_PME;

	ManTT->TCSET = CC4_TRBS;

	return trmBusy = true;

#else

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static __irq void ManTrmIRQ2()
//{
//	HW::P5->BSET(7);
//
////	ManTT->SWR = CC4_RPM;
//
//	HW::P5->BCLR(7);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void InitManTransmit()
{
	using namespace HW;

	HW::CCU_Enable(ManCCU_PID);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_S0I|CCU4_PRB;

	ManTT->PRS = trmHalfPeriod-1;
	ManTT->PSC = ManRT_PSC; //0.08us

	ManCCU->GCSS = CCU4_S0SE;  

	//ManTT->INS = 0;
	//ManTT->CMC = 0;
	//ManTT->TC = 0;

	VectorTableExt[CCU41_0_IRQn] = ManTrmIRQ;
	CM4::NVIC->CLR_PR(CCU41_0_IRQn);
	CM4::NVIC->SET_ER(CCU41_0_IRQn);	

	ManTT->SRS = 0;

	ManTT->SWR = ~0;
	ManTT->INTE = CC4_PME;

//	ManTT->TCSET = 1;

	ManDisable();
}

#endif 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#else // #ifdef MAN_TRANSMIT_V1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static __irq void ManTrmIRQ2()
{
	static u32 tw = 0;
	static u16 count = 0;
	//static byte i = 0;
	static const u16* data = 0;
	static u16 len = 0;
	static bool cmd = false;

	Pin_ManTrmIRQ_Set();

	switch (stateManTrans)
	{
		case 0:	// 1-st sync imp 

			data = manTB->data1;
			len = manTB->len1;
			cmd = false;
			stateManTrans++;

		case 1:

			if (len == 0)
			{
				data = manTB->data2;
				len = manTB->len2;
				manTB->len2 = 0;
			};

			if (len != 0)
			{
				tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

				data++;
				len--;

				count = 17;

				u32 tadd = (cmd) ? trmHalfPeriod : 0;

				ManT_SET_CR(trmHalfPeriod3+tadd);

				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod7+tadd); //US2MT(96);
					stateManTrans += 2;
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod6+tadd); //US2MT(72);
					stateManTrans++;
				};

				ManT_SHADOW_SYNC();

				tw <<= 1;
				count--;
			}
			else
			{
				stateManTrans = 4;
			};

			break;

		case 2:	

			ManT_SET_CR(trmHalfPeriod);

			if (count == 0)
			{
				ManT_SET_PR(trmHalfPeriod2);
				cmd = false;
				stateManTrans = 1;
			}
			else
			{
				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod3);

					if (count == 1)
					{
						cmd = true;
						stateManTrans = 1;
					}
					else
					{
						stateManTrans++;
					};
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod2);
				};

				tw <<= 1;
				count--;
			};

			ManT_SHADOW_SYNC();

			break;

		case 3: 

			if (tw & 0x10000)
			{
				ManT_SET_CR(trmHalfPeriod);
				ManT_SET_PR(trmHalfPeriod2);

				tw <<= 1;
				count--;

				if (count == 0)
				{
					cmd = true;
					stateManTrans = 1;
				};
			}
			else
			{
				tw <<= 1;
				count--;

				ManT_SET_CR(trmHalfPeriod2);

				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod4);
					
					if (count == 1)
					{
						cmd = true;
						stateManTrans = 1;
					};
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod3);

					if (count == 0)
					{
						cmd = false;
						stateManTrans = 1;
					}
					else
					{
						stateManTrans--;
					}
				};

//				if (count != 0)
				{
					tw <<= 1;
					count--;
				};
			};

			ManT_SHADOW_SYNC();

			break;

		case 4:

			stateManTrans++;
			break;

		case 5:

			stateManTrans = 0;

			HW::SCU_GENERAL->CCUCON &= ~ManT_CCUCON;


			ManT1->TCCLR = CC8_TRBC;
			ManT1->INTE = 0;
			ManT2->TCCLR = CC8_TRBC;
			ManT3->TCCLR = CC8_TRBC;

			ManT_CCU8->GCSS = ManT_OUT_GCSS;
			ManT_CCU8->GCSC = ManT_OUT_GCSC;
			//ManT_CCU8->GIDLS = ManT_CCU8_GIDLS;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)


	Pin_ManTrmIRQ_Clr();
}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData(MTB* mtb)
{
#ifndef WIN32
	if (trmBusy || rcvBusy || mtb == 0 || mtb->data1 == 0 || mtb->len1 == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	//trmHalfPeriod = GetTrmBaudRate(mtb->baud);
	//trmHalfPeriod2 = trmHalfPeriod * 2 - 1;
	//trmHalfPeriod3 = trmHalfPeriod * 3 - 1;
	//trmHalfPeriod4 = trmHalfPeriod * 4 - 1;
	//trmHalfPeriod6 = trmHalfPeriod * 6 - 1;
	//trmHalfPeriod7 = trmHalfPeriod * 7 - 1;

	stateManTrans = 0;

#ifdef CPU_SAME53	


	ManTT->CTRLA = TC_MODE_COUNT8;
	ManTT->WAVE = TC_WAVEGEN_NPWM;
	ManTT->PER8 = GetTrmBaudRate(mtb->baud) - 1; //trmHalfPeriod-1;

	ManTT->INTENCLR = ~TC_OVF;
	ManTT->INTENSET = TC_OVF;

	ManTT->INTFLAG = ~0;

	ManTT->CTRLA = TC_MODE_COUNT8 | TC_ENABLE;
	ManTT->CTRLBSET = TC_CMD_RETRIGGER;

	//ManTT->CTRLA = 0;

	//ManTT->PER = trmHalfPeriod-1;

	//ManTT->INTENCLR = ~TCC_OVF;
	//ManTT->INTENSET = TCC_OVF;

	//ManTT->INTFLAG = ~0;

	//ManTT->CTRLA = TCC_ENABLE;

#elif defined(CPU_XMC48)

	ManT_SET_PR(US2MT(50)-1); //trmHalfPeriod - 1;
	ManT1->CR2S = (~0); ManT2->CR1S = (0); ManT2->CR2S = (0); ManT3->CR1S = (~0);

	ManT1->PSC = ManRT_PSC; //0.08us
	ManT2->PSC = ManRT_PSC; //0.08us
	ManT3->PSC = ManRT_PSC; //0.08us

	//ManT1->PSL = ManT1_PSL;
	//ManT2->PSL = ManT2_PSL;
	//ManT3->PSL = ManT3_PSL;

	ManT1->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	ManT2->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	ManT3->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;

	ManT1->CMC = CC4_STRTS_EVENT0;
	ManT2->CMC = CC4_STRTS_EVENT0;
	ManT3->CMC = CC4_STRTS_EVENT0;
	
	ManT1->TC = CC8_STRM;
	ManT2->TC = CC8_STRM;
	ManT3->TC = CC8_STRM;

	ManT1->SWR = ~0;
	ManT2->SWR = ~0;
	ManT3->SWR = ~0;

	ManT1->CHC = ManT1_CHC;
	ManT2->CHC = ManT2_CHC;
	ManT3->CHC = ManT3_CHC;

	ManT1->INTE = CC8_PME;
	ManT2->INTE = 0;
	ManT3->INTE = 0;

	ManT_CCU8->GCSS = ManT_CCU8_GCSS;
	ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;

	HW::SCU_GENERAL->CCUCON |= ManT_CCUCON;

#endif

	return trmBusy = true;
#else

	mtb->ready = true;

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

void InitManTransmit()
{
	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "Manchester transmit2 Init ... ");

	VectorTableExt[MANT_CCU8_IRQ] = ManTrmIRQ2;
	CM4::NVIC->CLR_PR(MANT_CCU8_IRQ);
	CM4::NVIC->SET_ER(MANT_CCU8_IRQ);

	HW::CCU_Enable(ManT_CCU8_PID);

	ManT_CCU8->GCTRL = 0;
	ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;
	ManT_CCU8->GCSS = ManT_OUT_GCSS;
	ManT_CCU8->GCSC = ManT_OUT_GCSC;

	PIO_MANCH->ModePin(PIN_L1, A3PP);
	PIO_MANCH->ModePin(PIN_H1, A3PP);
	PIO_MANCH->ModePin(PIN_L2, A3PP);
	PIO_MANCH->ModePin(PIN_H2, A3PP);

	//ManT_CCU8->GIDLS = ManT_CCU8_GIDLS;

	//ManT1->PRS = US2MT(100) - 1;
	//ManT1->CR1S = US2MT(50) - 1;
	//ManT1->CR2S = US2MT(50) - 1;

	//ManT1->PSC = ManT_PSC; 
	//ManT2->PSC = ManT_PSC;
	//ManT3->PSC = ManT_PSC; 

	////ManT->CHC = CC8_;

	////ManT->TCSET = CC8_TRBS;

	////ManT1->PSL = ManT1_PSL;
	////ManT2->PSL = ManT2_PSL;
	////ManT3->PSL = ManT3_PSL;

	//ManT1->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	//ManT2->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;

	//ManT1->CMC = CC4_STRTS_EVENT0;
	//ManT2->CMC = CC4_STRTS_EVENT0;
	//ManT3->CMC = CC4_STRTS_EVENT0;

	ManT1->CHC = ManT1_CHC;
	ManT2->CHC = ManT2_CHC;
	ManT3->CHC = ManT3_CHC;

	//ManT_CCU8->GCSS = ManT_CCU8_GCSS;

	//ManT1->SRS = 0;

	//ManT1->SWR = ~0;
	//ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;
	//ManT->INTE = CC8_PME;

	SEGGER_RTT_WriteString(0, "OK\n");
}

#endif	// #ifndef WIN32

#endif  // #ifdef MAN_TRANSMIT_V1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef MAN_RECIEVE_V1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
#ifdef CPU_SAME53	
	ManRT->INTENCLR = ~0;
#elif defined(CPU_XMC48)
	ManRT->INTE = 0;
#endif

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = rcvManLen;
	
	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static Receiver manRcv(true);

u32 lastCaptureValue = 0;
byte manRcvState = 0;

u16 manRcvTime1 = 0;
//u32 manRcvTime2 = 0;
//u32 manRcvTime3 = 0;
//u32 manRcvTime4 = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static __irq void ManRcvIRQ2()
{
	using namespace HW;

	static u32 _number = 0;
	static u32 _length = 0;
	static u32 _data = 0;
	static bool _command = false;
	static bool _parity_temp = false;
	const bool _parity = true;
	static bool _sync = false;
	static bool _state = false;

	u16 len = ManTmr->TIMER-1;

	HW::P5->BSET(1);
 
//	u16 t = GetRTT();
	
	ManTmr->TCCLR = CC4_TCC;
	ManTmr->TCSET = CC4_TRB;
	
//	manRcvTime1 = t;

	_state = !_state;

	if (len <= MT(60))
	{
		_length += (len <= MT(36)) ? 1 : 2;

		if(_length >= 3)
		{
			_sync = false;
		};
	}
	else
	{
		if(len > MT(108))
		{
			_sync = false;
		}
		else
		{
			manRcvTime1 = GetRTT();

			_sync = true;
			_data = 0;
			_parity_temp = _parity;
			_number = 0;
			_length = (len <= MT(84)) ? 1 : 2;
			_command = !_state; 
		};
	};

	if(_sync && _length == 2)
	{
		manRcvTime1 = GetRTT();

		if(_number < 16)
		{
			_data <<= 1;
			_data |= _state;
			_parity_temp ^= _state;
			_number++;
			_length = 0;
		}
	 	else
		{
			_sync = false;

			if(_state != _parity_temp)
			{
				_state = !_state;
				_data = (~_data);
				_command = !_command;
			};

			//rcvManPrevTime = t;

			if (rcvManLen == 0)
			{
				if(_command)
				{
					*rcvManPtr++ = _data;
					rcvManLen = 1;
				};
			}
			else 
			{
				if(rcvManLen < rcvManCount)
				{
					*rcvManPtr++ = _data;
				};

				rcvManLen += 1;	
			};
		};
	};

	HW::P5->BCLR(1);
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		//bool c = false;//ManTT.SR & CPCS;

		if (rcvManLen > 0 && (GetRTT() - manRcvTime1) > US2RT(200))
		{
//			__disable_irq();

//			HW::PIOA->BSET(3);
			ManRcvEnd(true);
//			HW::PIOA->BCLR(3);

//			__enable_irq();
		}
		else
		{
			manRB->len = rcvManLen;
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void InitManRecieve()
{
	using namespace HW;

	HW::CCU_Enable(ManCCU_PID);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = ManCCU_GIDLC;//CCU4_CS1I|CCU4_CS2I|CCU4_SPRB;

	ManRT->PRS = MT(12)-1;
	ManRT->PSC = ManRT_PSC; //1.28us

	ManTmr->PRS = MT(250);
	ManTmr->PSC = ManRT_PSC; //1.28us

	ManCCU->GCSS = ManCCU_GCSS;//CCU4_S1SE|CCU4_S2SE;  

	ManRT->INS = ManRT_INS;//CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->INTE = 0;//CC4_PME;
	ManRT->SRS = CC4_POSR(2);

	ManTmr->INS = 0;
	ManTmr->CMC = 0;
	ManTmr->TC = CC4_TSSM;

	ManTmr->INTE = 0;//CC4_PME;
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
#ifndef WIN32

	if (rcvBusy /*|| trmBusy*/ || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	//ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	rcvManLen = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;

	ManRT->PRS = MT(12)-1;
	ManRT->PSC = ManRT_PSC; //1.28us

	ManCCU->GCSS = CCU4_S2SE;  

	ManRT->INS = ManRT_INS;//CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->SRS = CC4_POSR(2);

	ManTmr->TC = CC4_TSSM;
	ManTmr->TCSET = CC4_TRB;

	VectorTableExt[MANR_IRQ] = ManRcvIRQ2;
	CM4::NVIC->CLR_PR(MANR_IRQ);
	CM4::NVIC->SET_ER(MANR_IRQ);	

	ManRT->INTE = CC4_PME;

	return rcvBusy = true;

#else

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#else // #ifdef MAN_RECIEVE_V1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 rcvManLen12 = 0;
static u32 rcvManSum12 = 0;
static u16 rcvManCount12 = 0;
u16 rcvManQuality = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
#ifdef CPU_SAME53	
	ManRT->INTENCLR = ~0;
#elif defined(CPU_XMC48)
	ManTmr->INTE = 0;
#endif

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = rcvManLen;

	rcvManLen12 = (rcvManCount12 != 0) ? (rcvManSum12 / rcvManCount12) : 0;

	rcvManQuality = (rcvManLen12 > US2MT(12)) ? 0 : (((US2MT(12) - rcvManLen12) * 100 + US2MT(6))/US2MT(12));

	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static RTM manRcvTime;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static __irq void ManRcvIRQ2()
{
	using namespace HW;

	static u32 _number = 0;
	static u32 _length = 0;
	static u32 _data = 0;
	static bool _command = false;
	static bool _parity_temp = false;
	const bool _parity = true;
	static bool _sync = false;
	static bool _state = false;

	Pin_ManRcvIRQ_Set();

	#ifdef CPU_SAME53	

		u32 len = ManRT->CC[0];

		ManRT->CTRLBSET = TCC_CMD_RETRIGGER;

	#elif defined(CPU_XMC48)

		u16 len = ManTmr->CV[1];

		//ManTmr->TCCLR = CC4_TCC;
		//ManTmr->TCSET = CC4_TRB;

	#endif

	_state = !_state;

	if (len <= US2MT(60))
	{
		i16 dl;

		if (len <= US2MT(36))
		{
			_length += 1; dl = len - US2MT(24); 
		}
		else
		{
			_length += 2; dl = len - US2MT(48);
		};

		if (dl < 0) dl = -dl; rcvManSum12 += dl; rcvManCount12++;

		if(_length >= 3)
		{
			_sync = false;
		};
	}
	else
	{
		if(len > US2MT(108))
		{
			_sync = false;
		}
		else
		{
			manRcvTime.Reset();

			_sync = true;
			_data = 0;
			_parity_temp = _parity;
			_number = 0;
			_command = !_state; 

			i16 dl;

			if (len <= US2MT(84))
			{
				_length = 1; dl = len - US2MT(72); 
			}
			else
			{
				_length = 2; dl = len - US2MT(96); 
			};

			if (dl < 0) dl = -dl; rcvManSum12 += dl; rcvManCount12++;
		};
	};

	if(_sync && _length == 2)
	{
		manRcvTime.Reset();

		if(_number < 16)
		{
			_data <<= 1;
			_data |= _state;
			_parity_temp ^= _state;
			_number++;
			_length = 0;
		}
	 	else
		{
			Pin_ManRcvSync_Set();

			_sync = false;

			if(_state != _parity_temp)
			{
				_state = !_state;
				_data = (~_data);
				_command = !_command;
			};

			if (rcvManLen == 0)
			{
				if(_command)
				{
					*rcvManPtr++ = _data;
					rcvManLen = 1;
				};
			}
			else 
			{
				if(rcvManLen < rcvManCount)
				{
					*rcvManPtr++ = _data;
				};

				rcvManLen += 1;	
			};

			Pin_ManRcvSync_Clr();
		};
	};

	#ifdef CPU_SAME53	
		ManRT->INTFLAG = ~0;
	#elif defined(CPU_XMC48)
	#endif

	Pin_ManRcvIRQ_Clr();
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		u32 irq = Push_IRQ();

		if (rcvManLen > 0 && manRcvTime.Timeout(US2RT(200)))
		{
			ManRcvEnd(true);
		}
		else
		{
			manRB->len = rcvManLen;
		};

		Pop_IRQ(irq);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

void InitManRecieve()
{
	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "Manchester Reciever Init ... ");

	VectorTableExt[MANR_IRQ] = ManRcvIRQ2;
	CM4::NVIC->CLR_PR(MANR_IRQ);
	CM4::NVIC->SET_ER(MANR_IRQ);	

#ifdef CPU_SAME53	

	HW::GCLK->PCHCTRL[GCLK_TCC2_TCC3]	= GCLK_GEN(GEN_1M)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[GCLK_TC0_TC1]		= GCLK_GEN(GEN_1M)|GCLK_CHEN;

	HW::MCLK->APBCMASK |= APBC_TCC2;
	HW::MCLK->APBAMASK |= APBA_TC1;

	HW::GCLK->PCHCTRL[EVENT_MANR_1+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[EVENT_MANR_2+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	EIC->CTRLA = 0;
	while(EIC->SYNCBUSY);

	EIC->EVCTRL |= EIC_EXTINT0<<MANR_EXTINT;
	EIC->SetConfig(MANR_EXTINT, 1, EIC_SENSE_BOTH);
	EIC->INTENCLR = EIC_EXTINT0<<MANR_EXTINT;
	EIC->CTRLA = EIC_ENABLE;

	EVSYS->CH[EVENT_MANR_1].CHANNEL = (EVGEN_EIC_EXTINT_0+MANR_EXTINT)|EVSYS_PATH_ASYNCHRONOUS;
	EVSYS->USER[EVSYS_USER_TC1_EVU] = EVENT_MANR_1+1;

	EVSYS->CH[EVENT_MANR_2].CHANNEL = EVGEN_TC1_OVF|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;;
	EVSYS->USER[EVSYS_USER_TCC2_MC_0] = EVENT_MANR_2+1;

	PIO_MANCHRX->DIRCLR = MANCHRX;
	PIO_MANCHRX->CTRL |= MANCHRX;

	PIO_RXD->DIRCLR = RXD;
	PIO_RXD->CTRL |= RXD;

	PIO_MANCHRX->SetWRCONFIG(	MANCHRX,	PORT_PMUX(0)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);
	PIO_RXD->SetWRCONFIG(		RXD,		PORT_PMUX(0)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);

	//PIO_MANCHRX->PINCFG[PIN_MANCHRX] = PINGFG_INEN|PINGFG_PMUXEN;
	//PIO_RXD->PINCFG[PIN_RXD] = PINGFG_INEN|PINGFG_PMUXEN;

	ManRT->CTRLA = TCC_SWRST;
	while(ManRT->SYNCBUSY);

	ManRT->CTRLA = TCC_CPTEN0;
	ManRT->EVCTRL = TCC_MCEI0;

	ManRT->PER = ~0;
	ManRT->CC[1] = 250;

	ManRT->INTENCLR = ~0;
	//ManRT->INTENSET = TCC_MC0;

	ManRT->CTRLA = TCC_CPTEN0|TCC_ENABLE;


	ManIT->CTRLA = TCC_SWRST;

	while(ManIT->SYNCBUSY);

	ManIT->CTRLA = TC_MODE_COUNT8;
	ManIT->WAVE = TC_WAVEGEN_NPWM;

	ManIT->EVCTRL = TC_TCEI|TC_EVACT_RETRIGGER|TC_OVFEO;

	ManIT->PER8 = 11;
	ManIT->CC8[0] = ~0;
	ManIT->CC8[1] = ~0;

	ManIT->INTENCLR = ~0;

	ManIT->INTFLAG = ~0;

	ManIT->CTRLA = TC_MODE_COUNT8|TC_ENABLE;

	ManIT->CTRLBSET = TC_ONESHOT;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManCCU_PID);

	P1->ModePin10(I2DPU);
	//P1->ModePin11(I2DPU);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = ManCCU_GIDLC;//CCU4_CS1I|CCU4_CS2I|CCU4_SPRB;

	ManRT->PRS = US2MT(12) - 1;
	ManRT->CRS = US2MT(12) - 1;
	ManRT->PSC = ManRT_PSC; //1.28us

	ManTmr->PRS = US2MT(250);
	ManTmr->PSC = ManRT_PSC; //1.28us

	ManCCU->GCSS = ManCCU_GCSS;//CCU4_S1SE|CCU4_S2SE;  

	ManRT->INS = ManRT_INS;//CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->INTE = 0;//CC4_PME;
	ManRT->SRS = 0;//ManRT_SRS;//CC4_POSR(2);

	ManTmr->INS = ManTmr_INS;
	ManTmr->CMC = CC4_CAP0S_EVENT0|CC4_STRTS_EVENT0;
	ManTmr->SRS = CC4_E0SR(2);
	ManTmr->TC = CC4_TSSM|CC4_CAPC_ALWAYS;
	ManTmr->TCSET = CC4_TRB;

	ManTmr->INTE = 0;//CC4_PME;

#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
#ifndef WIN32

	if (rcvBusy /*|| trmBusy*/ || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	//ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	rcvManLen = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;

	rcvManSum12 = 0; rcvManCount12 = 0;

	#ifdef CPU_SAME53	

		ManRT->INTFLAG = ~0;
		ManRT->INTENSET = TCC_MC0;

	#elif defined(CPU_XMC48)

		ManTmr->SWR = CC4_RE0A;
		ManTmr->INTE = CC4_E0AE;

	#endif

	return rcvBusy = true;

#else

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // #else // #ifdef MAN_RECIEVE_V1

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void Init_ERU()
{
	using namespace HW;

	HW::CCU_Enable(PID_ERU1);

	// Event Request Select (ERS)
	
	ERU1->EXISEL = 0;
	
	// Event Trigger Logic (ETL)

	ERU1->EXICON[0] = 0;
	ERU1->EXICON[1] = 0;
	ERU1->EXICON[2] = 0;
	ERU1->EXICON[3] = 0;

	// Cross Connect Matrix

	// Output Gating Unit (OGU)

	ERU1->EXOCON[0] = 0;
	ERU1->EXOCON[1] = 0;
	ERU1->EXOCON[2] = 0;
	ERU1->EXOCON[3] = 0;
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetClock(const RTC &t)
{
#ifndef WIN32

	static DSCI2C dsc;

	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];

	buf[0] = 0;
	buf[1] = ((t.sec/10) << 4)|(t.sec%10);
	buf[2] = ((t.min/10) << 4)|(t.min%10);
	buf[3] = ((t.hour/10) << 4)|(t.hour%10);
	buf[4] = 1;
	buf[5] = ((t.day/10) << 4)|(t.day%10);
	buf[6] = ((t.mon/10) << 4)|(t.mon%10);

	byte y = t.year % 100;

	buf[7] = ((y/10) << 4)|(y%10);

	dsc.adr = 0x68;
	dsc.wdata = buf;
	dsc.wlen = 8;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	if (SetTime(t))
	{
		I2C_AddRequest(&dsc);
	};
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void InitClock()
{
	DSCI2C dsc;

	byte reg = 0;
	byte buf[10];
	
	RTC t;

	buf[0] = 0x0F;
	buf[1] = 0x88;
	dsc.adr = 0x68;
	dsc.wdata = buf;
	dsc.wlen = 2;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	dsc.adr = 0x68;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
	t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
	t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
	t.day	= (buf[4]&0xF) + ((buf[4]>>4)*10);
	t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
	t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

	SetTime(t);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WDT_Init()
{
	HW::WDT_Enable();

	HW::WDT->WLB = OFI_FREQUENCY/2;
	HW::WDT->WUB = (3 * OFI_FREQUENCY)/2;
	HW::SCU_CLK->WDTCLKCR = 0|SCU_CLK_WDTCLKCR_WDTSEL_OFI;

	#ifndef _DEBUG
	HW::WDT->CTR = WDT_CTR_ENB_Msk|WDT_CTR_DSP_Msk;
	#else
	HW::WDT->CTR = WDT_CTR_ENB_Msk;
	#endif

	//HW::ResetWDT();
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

//extern dword DI;
//extern dword DO;
char pressedKey;
//extern  dword maskLED[16];

//byte _array1024[0x60000]; 

HWND  hWnd;

HCURSOR arrowCursor;
HCURSOR handCursor;

HBRUSH	redBrush;
HBRUSH	yelBrush;
HBRUSH	grnBrush;
HBRUSH	gryBrush;

RECT rectLed[10] = { {20, 41, 33, 54}, {20, 66, 33, 79}, {20, 91, 33, 104}, {21, 117, 22, 118}, {20, 141, 33, 154}, {218, 145, 219, 146}, {217, 116, 230, 129}, {217, 91, 230, 104}, {217, 66, 230, 79}, {217, 41, 230, 54}  }; 
HBRUSH* brushLed[10] = { &yelBrush, &yelBrush, &yelBrush, &gryBrush, &grnBrush, &gryBrush, &redBrush, &redBrush, &redBrush, &redBrush };

int x,y,Depth;

HFONT font1;
HFONT font2;

HDC memdc;
HBITMAP membm;

//HANDLE facebitmap;

static const u32 secBufferWidth = 80;
static const u32 secBufferHeight = 12;
static const u32 fontWidth = 12;
static const u32 fontHeight = 16;


static char secBuffer[secBufferWidth*secBufferHeight*2];

static u32 pallete[16] = {	0x000000,	0x800000,	0x008000,	0x808000,	0x000080,	0x800080,	0x008080,	0xC0C0C0,
							0x808080,	0xFF0000,	0x00FF00,	0xFFFF00,	0x0000FF,	0xFF00FF,	0x00FFFF,	0xFFFFFF };

const char lpAPPNAME[] = "8юд73_лдл_соп";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



static __int64		tickCounter = 0;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

static void InitDisplay()
{
	WNDCLASS		    wcl;

	wcl.hInstance		= NULL;
	wcl.lpszClassName	= lpAPPNAME;
	wcl.lpfnWndProc		= WindowProc;
	wcl.style	    	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	wcl.hIcon	    	= NULL;
	wcl.hCursor	    	= NULL;
	wcl.lpszMenuName	= NULL;

	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hbrBackground	= NULL;

	RegisterClass (&wcl);

	int sx = screenWidth = GetSystemMetrics (SM_CXSCREEN);
	int sy = screenHeight = GetSystemMetrics (SM_CYSCREEN);

	hWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME,	WS_DLGFRAME|WS_POPUP, 0, 0,	640, 480, NULL,	NULL, NULL, NULL);

	if(!hWnd) 
	{
		cputs("Error creating window\r\n");
		exit(0);
	};

	RECT rect;

	if (GetClientRect(hWnd, &rect))
	{
		MoveWindow(hWnd, 0, 0, 641 + 768 - rect.right - 2, 481 - rect.bottom - 1 + 287, true);
	};

	ShowWindow (hWnd, SW_SHOWNORMAL);

	font1 = CreateFont(30, 14, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");
	font2 = CreateFont(fontHeight, fontWidth-2, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");

	if (font1 == 0 || font2 == 0)
	{
		cputs("Error creating font\r\n");
		exit(0);
	};

	GetClientRect(hWnd, &rect);
	HDC hdc = GetDC(hWnd);
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, rect.right - rect.left + 1, rect.bottom - rect.top + 1 + secBufferHeight*fontHeight);
    SelectObject(memdc, membm);
	ReleaseDC(hWnd, hdc);


	arrowCursor = LoadCursor(0, IDC_ARROW);
	handCursor = LoadCursor(0, IDC_HAND);

	if (arrowCursor == 0 || handCursor == 0)
	{
		cputs("Error loading cursors\r\n");
		exit(0);
	};

	LOGBRUSH lb;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = RGB(0xFF, 0, 0);

	redBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0xFF, 0xFF, 0);

	yelBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0x7F, 0x7F, 0x7F);

	gryBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0, 0xFF, 0);

	grnBrush = CreateBrushIndirect(&lb);

	for (u32 i = 0; i < sizeof(secBuffer); i+=2)
	{
		secBuffer[i] = 0x20;
		secBuffer[i+1] = 0xF0;
	};
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

void UpdateDisplay()
{
	static byte curChar = 0;
	static byte c = 0, i = 0;
	//static byte flashMask = 0;
	static const byte a[4] = { 0x80, 0x80+0x40, 0x80+20, 0x80+0x40+20 };

	MSG msg;

	static dword pt = 0;

	if(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		GetMessage (&msg, NULL, 0, 0);

		TranslateMessage (&msg);

		DispatchMessage (&msg);
	};

	static char buf[80];
	static char buf1[sizeof(secBuffer)];

	u32 t = GetTickCount();

	if ((t-pt) > 10)
	{
		pt = t;

		bool rd = true;

		//flashMask += 1;

		//for (u32 i = 0; i < sizeof(buf1); i++)
		//{
		//	char t = secBuffer[i];

		//	if (buf1[i] != t) 
		//	{ 
		//		buf1[i] = t; rd = true; 
		//	};
		//};

		if (rd)
		{
			//SelectObject(memdc, font1);
			//SetBkColor(memdc, 0x074C00);
			//SetTextColor(memdc, 0x00FF00);
			//TextOut(memdc, 443, 53, buf, 20);
			//TextOut(memdc, 443, 30*1+53, buf+20, 20);
			//TextOut(memdc, 443, 30*2+53, buf+40, 20);
			//TextOut(memdc, 443, 30*3+53, buf+60, 20);

			SelectObject(memdc, font2);

			for (u32 j = 0; j < secBufferHeight; j++)
			{
				for (u32 i = 0; i < secBufferWidth; i++)
				{
					u32 n = (j*secBufferWidth+i)*2;

					u8 t = secBuffer[n+1];

					SetBkColor(memdc, pallete[(t>>4)&0xF]);
					SetTextColor(memdc, pallete[t&0xF]);
					TextOut(memdc, i*fontWidth, j*fontHeight+287, secBuffer+n, 1);
				};
			};

			RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);

		}; // if (rd)

		//if (!rd) { RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE); };


	}; // if ((t-pt) > 10)

}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	bool c;
	static int x, y;
	int x0, y0, r0;
	static char key = 0;
	static char pkey = 0;

	RECT rect;

	static bool move = false;
	static int movex, movey = 0;

//	char *buf = (char*)screenBuffer;

    switch (message)
	{
        case WM_CREATE:

            break;

	    case WM_DESTROY:

		    break;

        case WM_PAINT:

			if (GetUpdateRect(hWnd, &rect, false)) 
			{
				hdc = BeginPaint(hWnd, &ps);

				//printf("Update RECT: %li, %li, %li, %li\r\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

				c = BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left + 1, ps.rcPaint.bottom - ps.rcPaint.top + 1, memdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	 
				EndPaint(hWnd, &ps);
			};

            break;

        case WM_CHAR:

			pressedKey = wParam;

			if (pressedKey == '`')
			{
				GetWindowRect(hWnd, &rect);

				if ((rect.bottom-rect.top) > 340)
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-fontHeight*secBufferHeight, true); 
				}
				else
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top+fontHeight*secBufferHeight, true); 
				};
			};

            break;

		case WM_MOUSEMOVE:

			x = LOWORD(lParam); y = HIWORD(lParam);

			if (move)
			{
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, HWND_TOP, rect.left+x-movex, rect.top+y-movey, 0, 0, SWP_NOSIZE); 
			};

			return 0;

			break;

		case WM_MOVING:

			return TRUE;

			break;

		case WM_SYSCOMMAND:

			return 0;

			break;

		case WM_LBUTTONDOWN:

			move = true;
			movex = x; movey = y;

			SetCapture(hWnd);

			return 0;

			break;

		case WM_LBUTTONUP:

			move = false;

			ReleaseCapture();

			return 0;

			break;

		case WM_MBUTTONDOWN:

			ShowWindow(hWnd, SW_MINIMIZE);

			return 0;

			break;

		case WM_ACTIVATE:

			if (HIWORD(wParam) != 0 && LOWORD(wParam) != 0)
			{
				ShowWindow(hWnd, SW_NORMAL);
			};

			break;

		case WM_CLOSE:

			//run = false;

			break;
	};
    
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif		
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

int PutString(u32 x, u32 y, byte c, const char *str)
{
	char *dst = secBuffer+(y*secBufferWidth+x)*2;
	dword i = secBufferWidth-x;

	while (*str != 0 && i > 0)
	{
		*(dst++) = *(str++);
		*(dst++) = c;
		i -= 1;
	};

	return secBufferWidth-x-i;
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef	WIN32

int Printf(u32 x, u32 y, byte c, const char *format, ... )
{
	char buf[1024];

	va_list arglist;

    va_start(arglist, format);
    vsprintf(buf, format, arglist);
    va_end(arglist);

	return PutString(x, y, c, buf);
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	Init_time(MCK);
	RTT_Init();
	I2C_Init();
	//SPI_Init();

#ifndef WIN32

	InitClock();

	InitManTransmit();
	InitManRecieve();
	
	EnableVCORE();

	WDT_Init();

#else

	InitDisplay();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
#ifndef WIN32

	static byte i = 0;

	static Deb db(false, 20);

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( I2C_Update();		);
		CALL( SPI_Update();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

