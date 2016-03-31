#include "core.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" void SystemInit (void)
{
	using namespace HW;

	//MATRIX->SCFG[0] |= 0x00060000;
	//MATRIX->SCFG[1] |= 0x00060000;

	EFC0->FMR = 0x0500;
	EFC1->FMR = 0x0500;

	PMC->PCER0 = PID::PIOA_M|PID::PIOB_M|PID::UART0_M|PID::UART1_M|PID::USART0_M|PID::USART1_M;
	//PMC->PCER1 = PID::ADC_M|PID::PWM_M|PID::DMAC_M;

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOA->PDR =		0x00620660;	// 0000 0000 0110 0010 0000 0110 0110 0000
	PIOA->ABCDSR1 =	0x00020000;	// 0000 0000 0000 0010 0000 0000 0000 0000		
	PIOA->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOA->PER = 	0x80100010;	// 1000 0000 0001 0000 0000 0000 0001 0000		
	PIOA->OER = 	0x80000010;	// 1000 0000 0000 0000 0000 0000 0001 0000		
	//PIOA->MDDR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	//PIOA->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOB->PDR =		0x0000000C;	// 0000 0000 0000 0000 0000 0000 0000 1100
	PIOB->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->PER = 	0x00002000;	// 0000 0000 0000 0000 0010 0000 0000 0000		
	PIOB->OER = 	0x00002000;	// 0000 0000 0000 0000 0010 0000 0000 0000		
	//PIOB->MDDR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	//PIOB->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		



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

	CMCC->CTRL = 1; // cache enable
	CMCC->MAINT0 = 1; // invalidate all cache entries

	__asm { DSB };
	__asm { ISB };

}

