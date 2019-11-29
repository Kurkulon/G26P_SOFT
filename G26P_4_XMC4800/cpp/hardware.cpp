#include "core.h"
#include "time.h"
#include "hardware.h"
#include "twi.h"

//#pragma O3
//#pragma Otime


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define D_FIRE		(1<<16)
#define SYNC		(1<<20)
#define SPI			HW::SPI5
#define L1			(1<<0)
#define H1			(1<<1)
#define L2			(1<<9)
#define H2			(1<<10)
#define RXD			(1<<5) 
#define ERR			(1<<4) 
#define ENVCORE		(1<<6) 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void EnableVCORE() { HW::P2->CLR(ENVCORE); }
inline void DisableVCORE() { HW::P2->SET(ENVCORE); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ HW::P0->CLR(L1|L2); HW::P0->SET(H1|H2);} // 0110  
inline void ManOne()		{ HW::P0->CLR(L2); HW::P0->SET(L1|H1); HW::P0->CLR(H2);} // 1100
inline void ManZero()		{ HW::P0->CLR(L1); HW::P0->SET(L2|H2); HW::P0->CLR(H1);} // 0011

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static byte manInv = 0;

#define BOUD2CLK(x) ((u32)((MCK/8.0)/x+0.5))
#define ManTT		HW::CCU41_CC40
#define ManRT		HW::CCU41_CC42
#define ManTmr		HW::CCU41_CC41
#define ManCCU		HW::CCU41
#define ManRxd()	(HW::PIOA->IN & 1)
#define MT(v)		((u16)((MCK_MHz*(v)+64)/128))

// RXD CCU41.IN2C

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

u16 mltArr[4] = { 0, L2|H2, 0, L1|H1 };
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

//	HW::P5->BSET(7);

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

			ManTT->TCCLR = CC4_TRBC;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

//	HW::P5->BCLR(7);
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

static void InitManTransmit()
{
	using namespace HW;

	HW::CCU_Enable(PID_CCU41);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_S0I|CCU4_PRB;

	ManTT->PRS = trmHalfPeriod-1;
	ManTT->PSC = 3; //0.08us

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
//	ManRT->INTENCLR = TCC_OVF;

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
//
//void ManRcvStop()
//{
//	ManRcvEnd(true);
//}
//
////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	using namespace HW;

	HW::CCU_Enable(PID_CCU41);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_CS1I|CCU4_CS2I|CCU4_SPRB;

	ManRT->PRS = MT(12)-1;
	ManRT->PSC = 7; //1.28us

	ManTmr->PRS = 0xFFFF;
	ManTmr->PSC = 7; //1.28us

	ManCCU->GCSS = CCU4_S1SE|CCU4_S2SE;  

	ManRT->INS = CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->INTE = 0;//CC4_PME;
	ManRT->SRS = CC4_POSR(2);

	ManTmr->INS = 0;
	ManTmr->CMC = 0;
	ManTmr->TC = CC4_TSSM;

	ManTmr->INTE = 0;//CC4_PME;


	//ManRT->TCSET = 1;

	//VectorTableExt[CCU41_2_IRQn] = ManRcvIRQ2;
	//CM4::NVIC->CLR_PR(CCU41_2_IRQn);
	//CM4::NVIC->SET_ER(CCU41_2_IRQn);	
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
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
	ManRT->PSC = 7; //1.28us

	ManCCU->GCSS = CCU4_S2SE;  

	ManRT->INS = CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->SRS = CC4_POSR(2);

	ManTmr->TC = CC4_TSSM;
	ManTmr->TCSET = CC4_TRB;

	VectorTableExt[CCU41_2_IRQn] = ManRcvIRQ2;
	CM4::NVIC->CLR_PR(CCU41_2_IRQn);
	CM4::NVIC->SET_ER(CCU41_2_IRQn);	

	ManRT->INTE = CC4_PME;

	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Init_ERU()
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetClock(const RTC &t)
{
	static DSCTWI dsc;

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
		AddRequest_TWI(&dsc);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitClock()
{
	DSCTWI dsc;

	byte reg = 0;
	byte buf[10];
	
	RTC t;

	dsc.adr = 0x68;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	AddRequest_TWI(&dsc);

	while (!dsc.ready);

	t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
	t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
	t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
	t.day	= (buf[4]&0xF) + ((buf[4]>>4)*10);
	t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
	t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

	SetTime(t);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	Init_time();
	RTT_Init();
	Init_TWI();
	InitClock();

	InitManTransmit();
	InitManRecieve();
	
	EnableVCORE();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
