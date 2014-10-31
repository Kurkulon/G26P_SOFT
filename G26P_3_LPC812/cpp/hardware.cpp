#include "types.h"
#include "core.h"



u16 HV = 0;






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
	GPIO->CLR0 |= 1<<11;


	IOCON->PIO0_1 &= ~(0x3 << 3);
	SWM->PINENABLE0 &= ~(0x1 << 7);
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

	i32 r = (SPI0->RXDAT & 0xFFF) << 6;
	i32 d = t>>10;
	t += r - d;

	HV = ((t>>16) * 18871) >> 16; // HV * 3.3 / 4095 / 56 * 20000

	SPI0->TXDATCTL = 0x0F100000; //SPI_TXDATCTL_FLEN(7) | SPI_TXDATCTL_EOT | SPI_TXDATCTL_SSEL_N(0xe);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitADC()
{
	using namespace HW;

	SWM->PINASSIGN[3] = (SWM->PINASSIGN[3] & 0x00FFFFFF) | 0x09000000;
	SWM->PINASSIGN[4] = (SWM->PINASSIGN[4] & 0xFF000000) | 0x00100FFF;
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
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
