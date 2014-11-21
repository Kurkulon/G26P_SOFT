#include "types.h"
#include "core.h"
#include "time.h"


u16 curHV = 0;
u16 reqHV = 800;

//#define eVal_IntervalTimer ((MCK / 5000) * 10)
//#define eVal_StartSample   ((MCK / 5000) *  6)
//
//enum {  eVal_MinValue = 10 } ;

bool	syncActive = false;
u32		syncTime = 0;


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

	CM0::SCB->VTOR = (u32)VectorTableInt;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/
extern "C" void SystemInit()
{
	u32 i;
	using namespace CM0;
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SWM_M | CLK::IOCON_M | CLK::GPIO_M | HW::CLK::MRT_M | HW::CLK::UART0_M | HW::CLK::CRC_M;

	GPIO->DIR0 |= (1<<11)|(1<<17)|(1<<0)|(1<<7);
	GPIO->CLR0 = 1<<11;


	IOCON->PIO0_1.B.MODE = 0;// &= ~(0x3 << 3);

	SWM->PINENABLE0.B.CLKIN = 0;// &= ~(0x1 << 7);

	for (i = 0; i < 200; i++) __nop();

	SYSCON->SYSPLLCLKSEL  = 3;					/* Select PLL Input         */
	SYSCON->SYSPLLCLKUEN  = 0;					/* Update Clock Source      */
	SYSCON->SYSPLLCLKUEN  = 1;					/* Update Clock Source      */
	while (!(SYSCON->SYSPLLCLKUEN & 1));		/* Wait Until Updated       */

	SYSCON->MAINCLKSEL    = 1;					/* Select PLL Clock Output  */
	SYSCON->MAINCLKUEN    = 0;					/* Update MCLK Clock Source */
	SYSCON->MAINCLKUEN    = 1;					/* Update MCLK Clock Source */
	while (!(SYSCON->MAINCLKUEN & 1));			/* Wait Until Updated       */

//	SYSCON->SYSAHBCLKDIV  = SYSAHBCLKDIV_Val;

	SYSCON->UARTCLKDIV = 1;
	SWM->U0_RXD = 14;
	SWM->U0_TXD = 6;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	using namespace HW;

	static u32 t = 0;

	if ((SPI0->STAT & 1) == 0) return;

	t += (i32)((SPI0->RXDAT & 0xFFF) << 6) - (i32)(t>>10);

	curHV = ((t>>16) * 18871) >> 16; // HV * 3.3 / 4095 / 56 * 20000

	SPI0->TXDATCTL = 0x0F100000; //SPI_TXDATCTL_FLEN(7) | SPI_TXDATCTL_EOT | SPI_TXDATCTL_SSEL_N(0xe);

	if (curHV < (reqHV-5))
	{
		GPIO->SET0 = 1<<11;
	}
	else if (curHV > (reqHV+5))
	{
		GPIO->CLR0 = 1<<11;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FireM()
{
	HW::SWM->CTOUT_0 = 7;
	HW::SWM->CTOUT_1 = -1;

	HW::SCT->CTRL_L &= ~(3<<1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FireXX()
{
	HW::SWM->CTOUT_0 = 17;
	HW::SWM->CTOUT_1 = 13;

	HW::SCT->CTRL_L &= ~(3<<1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FireYY()
{
	HW::SWM->CTOUT_0 = 12;
	HW::SWM->CTOUT_1 = 4;

	HW::SCT->CTRL_L &= ~(3<<1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void(*FireMode)() = FireXX;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void SyncFireHandler()
{
	FireMode();
	HW::PIN_INT->IST = 1;
	HW::PIN_INT->CIENF = 1;
	syncActive = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void WaitFireSync(byte t)
{
	HW::PIN_INT->CIENF = 1;

	switch(t)
	{
		case 0: FireMode = FireXX; break;
		case 1: FireMode = FireYY; break;
		case 2: FireMode = FireM;  break;
	};

	syncActive = true;
	syncTime = GetMilliseconds();

	HW::PIN_INT->SIENF = 1;
	HW::PIN_INT->FALL = 1;
	HW::PIN_INT->IST = 1;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFire()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SCT_M;
	GPIO->DIR0 |= (1<<13)|(1<<17)|(1<<12)|(1<<4);
	GPIO->CLR0 = (1<<17)|(1<<12);
	GPIO->SET0 = (1<<13)|(1<<4);
//	SWM->PINASSIGN.U0_TXD = 0;

	SCT->STATE_L = 0;
	SCT->REGMODE_L = 0;

	SCT->MATCH_L[0] = 0; 
	SCT->MATCH_L[1] = 25*10;
	SCT->MATCH_L[2] = 25*105;
	SCT->MATCH_L[3] = 25*115;
	SCT->MATCH_L[4] = 0;

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

	SCT->EVENT[4].STATE = 0;
	SCT->EVENT[4].CTRL = 0;

	SCT->EVENT[5].STATE = 0;
	SCT->EVENT[5].CTRL = 0;

	SCT->START_L = 0;
	SCT->STOP_L = 1<<3;
	SCT->HALT_L = 0;

	SCT->CONFIG = 1<<7; 

	//SWM->CTOUT_0 = 17;
	//SWM->CTOUT_1 = 13;

//	SCT->CTRL_L &= ~(3<<1);


	SYSCON->PINTSEL[0] = 14;
	PIN_INT->ISEL = 0;
	//PIN_INT->SIENF = 1;
	//PIN_INT->FALL = 1;
	//PIN_INT->IST = 1;

	VectorTableExt[PININT0_IRQ] = SyncFireHandler;
	CM0::NVIC->ISER[0] = 1<<PININT0_IRQ;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitADC()
{
	using namespace HW;

	//SWM->PINASSIGN[3] = (SWM->PINASSIGN[3] & 0x00FFFFFF) | 0x09000000;
	//SWM->PINASSIGN[4] = (SWM->PINASSIGN[4] & 0xFF000000) | 0x00100FFF;

	SWM->SPI0_SCK = 9;
	SWM->SPI0_MISO = 15;
	SWM->SPI0_SSEL = 16;



	SYSCON->SYSAHBCLKCTRL |= CLK::SPI0_M;

	SPI0->CFG = 0x05;	//SPI_CFG_MASTER | SPI_CFG_ENABLE;
	SPI0->DLY = 1;
	SPI0->DIV = 24;

	SPI0->TXDATCTL = 0x0F100000; //SPI_TXDATCTL_FLEN(7) | SPI_TXDATCTL_EOT | SPI_TXDATCTL_SSEL_N(0xe);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitVectorTable();
	Init_time();
	InitADC();
	InitFire();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC();
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
