#include "core.h"
#include "time.h"
#include "hardware.h"

//#pragma O3
//#pragma Otime

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

#define L1 1
#define H1 2
#define L2 8
#define H2 4

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable()	{ HW::PIOB->ODSR = H1|H2;} // 0110  
inline void ManOne()		{ HW::PIOB->SODR = H1; HW::PIOB->CODR = L2; __nop(); __nop(); __nop(); HW::PIOB->ODSR = L1|H1; } // 1100
inline void ManZero()		{ HW::PIOB->SODR = H2; HW::PIOB->CODR = L1; __nop(); __nop(); __nop(); HW::PIOB->ODSR = L2|H2; } // 0011

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static byte manInv = 0;

#define BOUD2CLK(x) ((u32)((MCK/2.0)/x+0.5))
#define ManTmr HW::TC0->C1
//#define ManRxd (((HW::PIOE->PDSR>>3)^manInv)&1)

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

inline void MltOff()	{ HW::PIOB->ODSR = 0x06;} // 0110  
inline void MltZ()		{ HW::PIOB->ODSR = 0x06; mltSeq = 0; HW::PIOB->ODSR = 0x0F;} // 1111
inline void MltNext()	{ HW::PIOB->ODSR = 0x06; mltSeq = (mltSeq+1)&3; HW::PIOB->ODSR = mltArr[mltSeq]; }


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

	HW::PIOE->IDR = 1<<3;

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
	manRB->len = rcvManLen;
	ManTmr.CCR = CLKDIS;
	
	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
		_number = 0;
		_length = 0;
		_sync = false;

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

//	HW::PIOB->SODR = 1<<13;

	ManTmr.CCR = CLKEN|SWTRG;

	if(manRcv.Parse(t * 100 / rcvPeriod) == manRcv.STATUS_READY)
	{
		rcvManPrevTime = GetRTT();

		if (rcvManLen == 0)
		{
			if(manRcv.GetType())
			{
				//HW::PIOB->SODR = 1<<11;

				*rcvManPtr++ = manRcv.GetData();
				rcvManLen = 1;

				//HW::PIOB->CODR = 1<<11;
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

	t = HW::PIOE->ISR;

//	HW::PIOB->CODR = 1<<13;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		bool c = ManTmr.SR & CPCS;

		if (rcvManLen > 0 && c /*(GetRTT() - rcvManPrevTime) > US2RT(1440)) || rcvManLen >= rcvManCount*/)
		{
			ManRcvEnd(true);
		}
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

	PMC->PCER0 = PID::TC1_M|PID::PIOE_M;

	VectorTableExt[HW::PID::PIOE_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOE_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOE_M;	

	//HW::PIOE->IER = 1<<3;
	//HW::PIOE->IFER = 1<<3;

//	HW::PIOE->OER = 7;

	//VectorTableExt[HW::PID::TC1_I] = ManRcvIRQ;
	//CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	//CM4::NVIC->ISER[0] = HW::PID::TC1_M;	

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


	VectorTableExt[HW::PID::PIOE_I] = WaitManCmdSync;
	CM4::NVIC->ICPR[0] = HW::PID::PIOE_M;
	CM4::NVIC->ISER[0] = HW::PID::PIOE_M;	

	HW::PIOE->IER = 1<<3;
	HW::PIOE->IFER = 1<<3;

//	tmp = HW::PIOE->ISR;

	HW::PMC->PCER0 = HW::PID::TC1_M;

	ManTmr.CCR = CLKDIS|SWTRG;
	ManTmr.IDR = -1;
	ManTmr.RC = 15000;
	ManTmr.CMR = 0x8040;


	u32 tmp = ManTmr.SR;

	//VectorTableExt[HW::PID::TC1_I] = ManRcvIRQ;
	//CM4::NVIC->ICPR[0] = HW::PID::TC1_M;
	//CM4::NVIC->ISER[0] = HW::PID::TC1_M;	


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
