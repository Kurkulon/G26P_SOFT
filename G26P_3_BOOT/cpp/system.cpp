#include "types.h"
#include "core.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define PIN_FX1			17 
#define PIN_FX2			13 
#define PIN_FY1			12 
#define PIN_FY2			4 
#define PIN_EN			11
#define PIN_CHARGE		8
#define PIN_TR1			7
#define PIN_RTS			0
#define PIN_UTX			6
#define PIN_URX			14
#define PIN_SCK			9
#define PIN_MISO		15
#define PIN_ADCS		16

#define FX1				(1<<PIN_FX1) 
#define FX2				(1<<PIN_FX2) 
#define FY1				(1<<PIN_FY1) 
#define FY2				(1<<PIN_FY2) 
#define EN				(1<<PIN_EN) 
#define CHARGE			(1<<PIN_CHARGE) 
#define TR1				(1<<PIN_TR1) 
#define RTS				(1<<PIN_RTS) 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" void SystemInit()
{
	u32 i;
	using namespace CM0;
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SWM_M | CLK::IOCON_M | CLK::GPIO_M | HW::CLK::MRT_M | HW::CLK::UART0_M | HW::CLK::CRC_M;

	GPIO->CLR0 = FX1|FY2|CHARGE|TR1;	//(1<<17)|(1<<4);
	GPIO->SET0 = FX2|FY1;				//(1<<13)|(1<<12);

	GPIO->DIR0 |= EN|RTS|TR1|CHARGE|FX1|FX2|FY1|FY2;	//(1<<11)|(1<<17)|(1<<0)|(1<<7)|(1<<8)|(1<<13)|(1<<12)|(1<<4);
	GPIO->CLR0 = EN|FX1|FY2|CHARGE|TR1;					//(1<<11)|(1<<17)|(1<<4);
	GPIO->SET0 = FX2|FY1;								//(1<<8)|(1<<13)|(1<<12);


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
	SWM->U0_RXD = PIN_URX;
	SWM->U0_TXD = PIN_UTX;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

