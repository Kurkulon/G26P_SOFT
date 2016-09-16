#include "core.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" void SystemInit (void)
{
	using namespace HW;

	//MATRIX->SCFG[0] |= 0x00060000;
	//MATRIX->SCFG[1] |= 0x00060000;

	EFC->FMR = 0x0500;

	PMC->PCER0 = PID::PIOA_M|PID::PIOB_M|PID::PIOC_M|PID::PIOD_M|PID::PIOE_M|PID::UART0_M|PID::DMAC_M|PID::TWI1_M;
	PMC->PCER1 = PID::UART1_M;

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOA->PDR =		0x00000660;	// 0000 0000 0000 0000 0000 0110 0110 0000
	PIOA->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOA->ABCDSR2 =	0x00000060;	// 0000 0000 0000 0000 0000 0000 0110 0000		
	PIOA->PER = 	0x0F81E110;	// 0000 1111 1000 0001 1110 0001 0001 0000		
	PIOA->OER = 	0x0F81E010;	// 0000 1111 1000 0001 1110 0000 0001 0000	
	
	PIOA->SODR = (0xF<<13)|(0xF<<23);

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOB->PDR =		0x00000030;	// 0000 0000 0000 0000 0000 0000 0011 0000
	PIOB->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOB->PER = 	0x0000000F;	// 0000 0000 0000 0000 0000 0000 0000 1111		
	PIOB->OER = 	0x0000000F;	// 0000 0000 0000 0000 0000 0000 0000 1111		
	PIOB->SODR = 6;
	PIOB->CODR = 9;

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOC->PDR =		0x000306FF;	// 0000 0000 0000 0011 0000 0110 1111 1111
	PIOC->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOC->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOC->PER = 	0x80000000;	// 1000 0000 0000 0000 0000 0000 0000 0000		
	PIOC->OER = 	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOD->PDR =		0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000
	PIOD->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOD->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOD->PER = 	0x00040000;	// 0000 0000 0000 0100 0000 0000 0000 0000		
	PIOD->OER = 	0x00040000;	// 0000 0000 0000 0100 0000 0000 0000 0000	

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOE->PDR =		0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000
	PIOE->ABCDSR1 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOE->ABCDSR2 =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
	PIOE->PER = 	0x00000008;	// 0000 0000 0000 0000 0000 0000 0000 1000		
	PIOE->OER = 	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		


	MATRIX->SYSIO = (1<<4)|(1<<5); // PB4 PB5


//	WDT->MR = (((1000 * 32768 / 128 + 500) / 1000) & 0xFFF) | 0x0FFF6000;
	HW::WDT->MR = 0x8000;	// Disable Watchdog

	PMC->MOR	= 0x01370102; while ((PMC->SR & 1) == 0) {};

	PMC->PLLAR	= 0x20030101; while ((PMC->SR & 2) == 0) {}; 

//	PMC->MCKR = (PMC->MCKR & 3) | 0x10; while ((PMC->SR & 8) == 0) {}; 

	PMC->MCKR = 0x02; while ((PMC->SR & 8) == 0) {};

	//PMC->PCK[1] = 0x64;
	//PMC->SCER = 0x200;


// Init FPU

	CM4::SCB->CPACR |= 0xF << 20;

	__asm { DSB };
	__asm { ISB };


// Init NAND Flash

	MATRIX->SMCNFCS = 1;

	// NCS_RD_SETUP = 0, NRD_SETUP = 0, NCS_WR_SETUP = 0, NWE_SETUP >= 8 ns
	// NCS_RD_PULSE >= 25 ns, NRD_PULSE >= 12 ns, NCS_WR_PULSE >= 25 ns, NWE_PULSE >= 12 ns
	// NRD_CYCLE >= 25 ns, NWE_CYCLE >= 25 ns 


//AC Timing Characteristics for Command / Address / Data Input
//tCLS	12 
//tCLH	5
//tCS	20 
//tCH	5
//tWP	12 
//tALS	12 
//tALH	5
//tDS	12 
//tDH	5
//tWC	25 
//tWH	10 
//tADL	70 

//AC Characteristics for Operation
//Symbol	Min Max
//tR		-	20
//tAR		10	- 
//tCLR		10	- 
//tRR		20	- 
//tRP		12	- 
//tWB		-	100
//tRC		25	- 
//tREA 		-	20
//tCEA 		-	25
//tRHZ 		-	100 
//tCHZ 		-	30
//tRHOH 	15	- 
//tRLOH 	5	-
//tCOH 		15	- 
//tREH 		10	- 
//tIR		0	-
//tRHW		100 - 
//tWHR		60	- 
//tRST		-	5/10/500


	PMC->PCER0 = PID::SMC_M;

	SMC->CSR[0].SETUP = 1;
	SMC->CSR[0].PULSE = 0x09090909;
	SMC->CSR[0].CYCLE = 0x00130013;
	SMC->CSR[0].MODE = 0x00000003;


	__asm { DSB };
	__asm { ISB };

	CM4::SCB->SHCSR |= 1<<16; // Memory Management Fault Enable

	CM4::MPU->RNR = 0;
	CM4::MPU->RBAR = 0x60000000;
	CM4::MPU->RASR = (1<<28)|(3<<24)|(23<<1)|1; // Non-cacheable, Strongly-ordered, XN=1(Instruction fetches disabled), AP=3(RW/RW), SIZE=23(16MB), ENABLE=1
	CM4::MPU->CTRL = 5; // Enable MPU

	__asm { DSB };
	__asm { ISB };

	HW::CMCC->CTRL = 1; // cache enable
	HW::CMCC->MAINT0 = 1; // invalidate all cache entries

	__asm { DSB };
	__asm { ISB };

// Init ADC

	//PMC->PCER0 = PID::ADC_M;

	//ADC->MR = 0x2F3FFF80;
	//ADC->ACR = 0x00;
	//ADC->CHER = 0x0008;
	//ADC->CR = 2;



}

