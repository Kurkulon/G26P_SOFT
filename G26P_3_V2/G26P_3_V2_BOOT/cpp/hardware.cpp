#include "hardware.h"
#include "time.h"

//#pragma O3
//#pragma Ospace

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	using namespace HW;

	Init_time(MCK);

	SYSCON->SYSAHBCLKCTRL |= HW::CLK::WWDT_M;
	SYSCON->PDRUNCFG &= ~(1<<6); // WDTOSC_PD = 0
	SYSCON->WDTOSCCTRL = (1<<5)|59; // 600kHz/60 = 10kHz = 0.1ms

#ifndef _DEBUG

	//WDT->TC = 2500; // * 0.4ms
	//WDT->MOD = 0x3;
	//ResetWDT();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
