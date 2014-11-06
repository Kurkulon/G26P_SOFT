#include "types.h"
#include "core.h"



u16 curHV = 0;
u16 reqHV = 600;

#define eVal_IntervalTimer ((MCK / 5000) * 10)
#define eVal_StartSample   ((MCK / 5000) *  6)

enum {  eVal_MinValue = 10 } ;





extern u32 __Vectors;                         /* see startup_LPC43xx.s   */

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/
extern "C" void SystemInit()
{
	u32 i;
	using namespace CM3;
	using namespace HW;


	//HW::CREG->FLASHCFGA = (HW::CREG->FLASHCFGA & ~0xF000) | 0x6000;
	//HW::CREG->FLASHCFGB = (HW::CREG->FLASHCFGB & ~0xF000) | 0x6000;

	//HW::EMC->STATICCONFIG0 |= 0x80000;

	/* Disable SysTick timer                                                    */
	
//	SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

	/* Set vector table pointer */
//	SCB->VTOR = ((u32)(&__Vectors)) & 0xFFF00000UL;

	/* Configure PLL0 and PLL1, connect CPU clock to selected clock source */

	SYSCON->SYSAHBCLKCTRL |= CLK::SWM_M | CLK::IOCON_M | CLK::GPIO_M;

	GPIO->DIR0 |= (1<<11)|(1<<17);
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

	//if (curHV < (reqHV-5))
	//{
	//	GPIO->SET0 = 1<<11;
	//}
	//else if (curHV > (reqHV+5))
	//{
	//	GPIO->CLR0 = 1<<11;
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFire()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SCT_M;
	GPIO->DIR0 |= (1<<13)|(1<<17)|(1<<12)|(1<<4);
	GPIO->CLR0 = (1<<13)|(1<<17)|(1<<12)|(1<<4);
//	SWM->PINASSIGN.U0_TXD = 0;

	SCT->STATE_L = 0;
	SCT->REGMODE_L = 0;

	SCT->MATCH_L[0] = 1; 
	SCT->MATCH_L[1] = 25*330;
	SCT->MATCH_L[2] = 25*333;
	SCT->MATCH_L[3] = 25*666;
	SCT->MATCH_L[4] = 0;

	SCT->OUT[0].SET = 0x0001;
	SCT->OUT[0].CLR = 0x0002;

	SCT->OUT[1].SET = 0x0004;
	SCT->OUT[1].CLR = 0x0008;

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


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FireXX()
{
	HW::SWM->CTOUT_0 = 17;
	HW::SWM->CTOUT_1 = 13;

	HW::SCT->CTRL_L &= ~(3<<1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FireYY()
{
	HW::SWM->CTOUT_0 = 12;
	HW::SWM->CTOUT_1 = 4;

	HW::SCT->CTRL_L &= ~(3<<1);
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
	InitADC();
	InitFire();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
