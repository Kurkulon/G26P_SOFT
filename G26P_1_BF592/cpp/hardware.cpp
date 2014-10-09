#include "hardware.h"

#include <bfrom.h>
#include <sys\exception.h>
//#include <cdefBF592-A.h>
//#include <ccblkfn.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void LowLevelInit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRTT()
{
	*pTIMER0_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS;
	*pTIMER0_PERIOD = 0xFFFFFFFF;
	*pTIMER_ENABLE = TIMEN0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_PLL()
{
	u32 SIC_IWR1_reg;                /* backup SIC_IWR1 register */

	/* use Blackfin ROM SysControl() to change the PLL */
    ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };

	/* use the ROM function */
	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool defSport0_Ready = false, defSport1_Ready = false;
static bool *sport0_Ready = 0, *sport1_Ready = 0;

EX_INTERRUPT_HANDLER(SPORT_ISR)
{
	if (*pDMA1_IRQ_STATUS & 1)
	{
		*pDMA1_IRQ_STATUS = 1;
		*pSPORT0_RCR1 = 0;
		*pDMA1_CONFIG = 0;
		*sport0_Ready = true;
	};

	if (*pDMA3_IRQ_STATUS & 1)
	{
		*pDMA3_IRQ_STATUS = 1;
		*pSPORT1_RCR1 = 0;
		*pDMA3_CONFIG = 0;
		*sport1_Ready = true;
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(StartSPORT_ISR)
{
//	*pPORTFIO_SET = 1<<2;

	*pSPORT0_RCR1 = RCKFE|LARFS|LRFS|RFSR|IRFS|IRCLK|RSPEN;
	*pSPORT1_RCR1 = RCKFE|LARFS|LRFS|RFSR|IRFS|IRCLK|RSPEN;

	*pPORTFIO_MASKA = 0;
	*pPORTFIO_CLEAR = /*(1<<2)|*/(1<<12);


//	*pPORTFIO_CLEAR = 1<<2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitSPORT()
{
	*pDMA1_CONFIG = 0;
	*pDMA3_CONFIG = 0;
	*pSPORT0_RCR1 = 0;
	*pSPORT1_RCR1 = 0;

	*pEVT9 = (void*)SPORT_ISR;
	*pIMASK |= EVT_IVG9; 
	*pSIC_IMASK |= 1<<9;


	*pEVT11 = (void*)StartSPORT_ISR;
	*pSIC_IMASK |= IRQ_PFA_PORTF;
	*pIMASK |= EVT_IVG11; 

	*pPORTFIO_INEN = 1<<12;
	*pPORTFIO_EDGE = 1<<12;
	*pPORTFIO_BOTH = 0;
	*pPORTFIO_CLEAR = 1<<12;
	*pPORTFIO_MASKA = 0;
	*pPORTFIO_MASKB = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ReadSPORT(void *dst1, void *dst2, u16 len1, u16 len2, u16 clkdiv, bool *ready0, bool *ready1)
{
	sport0_Ready = (ready0 != 0) ? ready0 : &defSport0_Ready;
	sport1_Ready = (ready1 != 0) ? ready1 : &defSport1_Ready;

	*sport0_Ready = false;
	*sport1_Ready = false;

	*pDMA1_CONFIG = 0;
	*pDMA3_CONFIG = 0;

	*pSPORT0_RCR1 = 0;
	*pSPORT0_RCR2 = 15|RXSE;
	*pSPORT0_RCLKDIV = clkdiv;
	*pSPORT0_RFSDIV = 49;

	*pSPORT1_RCR1 = 0;
	*pSPORT1_RCR2 = 15|RXSE;
	*pSPORT1_RCLKDIV = clkdiv;
	*pSPORT1_RFSDIV = 49;

	*pDMA1_START_ADDR = dst1;
	*pDMA1_X_COUNT = len1/2;
	*pDMA1_X_MODIFY = 2;

	*pDMA3_START_ADDR = dst2;
	*pDMA3_X_COUNT = len2/2;
	*pDMA3_X_MODIFY = 2;

	*pDMA1_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;
	*pDMA3_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;

	*pSPORT0_RCR1 = RCKFE|LARFS|LRFS|RFSR|IRFS|IRCLK|RSPEN;
	*pSPORT1_RCR1 = RCKFE|LARFS|LRFS|RFSR|IRFS|IRCLK|RSPEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SyncReadSPORT(void *dst1, void *dst2, u16 len1, u16 len2, u16 clkdiv, bool *ready0, bool *ready1)
{
	sport0_Ready = (ready0 != 0) ? ready0 : &defSport0_Ready;
	sport1_Ready = (ready1 != 0) ? ready1 : &defSport1_Ready;

	*sport0_Ready = false;
	*sport1_Ready = false;

	*pDMA1_CONFIG = 0;
	*pDMA3_CONFIG = 0;

	*pSPORT0_RCR1 = 0;
	*pSPORT0_RCR2 = 15|RXSE;
	*pSPORT0_RCLKDIV = clkdiv;
	*pSPORT0_RFSDIV = 49;

	*pSPORT1_RCR1 = 0;
	*pSPORT1_RCR2 = 15|RXSE;
	*pSPORT1_RCLKDIV = clkdiv;
	*pSPORT1_RFSDIV = 49;

	*pDMA1_START_ADDR = dst1;
	*pDMA1_X_COUNT = len1/2;
	*pDMA1_X_MODIFY = 2;

	*pDMA3_START_ADDR = dst2;
	*pDMA3_X_COUNT = len2/2;
	*pDMA3_X_MODIFY = 2;

	*pDMA1_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;
	*pDMA3_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;


//	*pPORTF_FER &= ~(1<<12);
	*pPORTFIO_CLEAR = 1<<12;
	*pPORTFIO_MASKA = 1<<12;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();

								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x0300;		//  0000 0011 0000 0000
	*pPORTG_MUX = 0x0000;		//  0000 0000 0000 0000

	*pPORTF_FER = 0xFB0F;		//  1111 1011 0000 1111
	*pPORTG_FER = 0xE70F;		//  1110 0111 0000 1111

	*pPORTFIO_DIR = 0x04A0;		//  0000 0100 1010 0000
	*pPORTGIO_DIR = 0x1800;		//  0001 1000 0000 0000

	*pPORTFIO_INEN = 0x0003;	//  0000 0000 0000 0011
	*pPORTGIO_INEN = 0x0003;	//  0000 0000 0000 0000

	*pPORTGIO_SET = 3<<11;


//#ifndef WIN32
//
//	using namespace HW;
//
//	MATRIX->SCFG[0] |= 0x00060000;
//	MATRIX->SCFG[1] |= 0x00060000;
//
//	EFC0->FMR = 0x0400;
//	EFC1->FMR = 0x0400;
//
//	PMC->PCER0 = PID::PIOA_M|PID::PIOB_M|PID::PIOC_M|PID::PIOD_M|PID::UART_M|PID::SMC_M|PID::SPI0_M;
//	PMC->PCER1 = PID::ADC_M|PID::PWM_M|PID::DMAC_M;
//
//								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
//	PIOA->PDR =		0x3E043F41;	// 0011 1110 0000 0100 0011 1111 0100 0001
//	PIOA->ABSR =	0x20040041;	// 0010 0000 0000 0100 0000 0000 0100 0001		
//	PIOA->PER = 	0x00000082;	// 0000 0000 0000 0000 0000 0000 1000 0000		
//	PIOA->OER = 	0x00000082;	// 0000 0000 0000 0000 0000 0000 1000 0000		
//	PIOA->MDDR =	0x00000080;	// 0000 0000 0000 0000 0000 0000 1000 0000		
//	PIOA->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//
//								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
//	PIOB->PDR =		0x08300FFF;	// 0000 1000 0011 0000 0000 1111 1111 1111
//	PIOB->ABSR =	0x00300C00;	// 0000 0000 0011 0000 0000 1100 0000 0000		
//	PIOB->PER = 	0x07CFF000;	// 0000 0111 1100 1111 1111 0000 0000 0000		
//	PIOB->OER = 	0x000FF000;	// 0000 0000 0000 1111 1111 0000 0000 0000		
//	PIOB->MDDR =	0x000FF000;	// 0000 0000 0000 1111 1111 0000 0000 0000		
//	PIOB->OWER =	0x000FF000;	// 0000 0000 0000 1111 1111 0000 0000 0000		
//
//								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
//	PIOC->PDR =		0x7FE7FFFC;	// 0111 1111 1110 0111 1111 1111 1111 1100
//	PIOC->ABSR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//	PIOC->PER = 	0x00180002;	// 0000 0000 0001 1000 0000 0000 0000 0010		
//	PIOC->OER = 	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//	PIOC->MDDR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//	PIOC->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//
//								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
//	PIOD->PDR =		0x000004FF;	// 0000 0000 0000 0000 0000 0100 1111 1111
//	PIOD->ABSR =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//	PIOD->PER = 	0x00000300;	// 0000 0000 0000 0000 0000 0011 0000 0000		
//	PIOD->OER = 	0x00000300;	// 0000 0000 0000 0000 0000 0011 0000 0000		
//	PIOD->MDDR =	0x00000300;	// 0000 0000 0000 0000 0000 0011 0000 0000		
//	PIOD->OWER =	0x00000000;	// 0000 0000 0000 0000 0000 0000 0000 0000		
//
//
//	SMC->CSR[0].SETUP	= 0x00000001;
//	SMC->CSR[0].PULSE	= 0x04040403;
//	SMC->CSR[0].CYCLE	= 0x00040004;
//	SMC->CSR[0].TIMINGS = 0x00000000;
//	SMC->CSR[0].MODE	= 0x00001003;
//
//	//SMC->CSR[3].SETUP	= 0x00010001;
//	//SMC->CSR[3].PULSE	= 0x06030603;
//	//SMC->CSR[3].CYCLE	= 0x00060006;
//	//SMC->CSR[3].TIMINGS = 0x00000000;
//	//SMC->CSR[3].MODE	= 0x00000003;
//
////	WDT->MR = (((1000 * 32768 / 128 + 500) / 1000) & 0xFFF) | 0x0FFF6000;
//
//	if ((SUPC->SR & 0x80) == 0) { SUPC->CR = 0xA5000008; };
//
//	PMC->MOR	= 0x01370102; while ((PMC->SR & 1) == 0) {};
//
//	PMC->PLLAR	= 0x20140101; while ((PMC->SR & 2) == 0) {}; 
//
//	PMC->MCKR = (PMC->MCKR & 3) | 0x10; while ((PMC->SR & 8) == 0) {}; 
//
//	PMC->MCKR = 0x12; while ((PMC->SR & 8) == 0) {}; 
//
//	HW::WDT->MR = 0x8000;
//
//	//u32 *p = (u32*)0x60000000;
//
//	//u32 c = 0x55555555;
//	//u32 x;
//
//	//for (u32 j = 0; j < 1000; j++)
//	//{
//	//	x = c;
//
//	//	for (u32 i = 0; i < 0x80000; i++)
//	//	{
//	//		p[i] = c *= 999983;
//	//	};
//
//	//	c = x;
//
//	//	PIOA->SODR = 1;
//
//	//	for (u32 i = 0; i < 0x80000; i++)
//	//	{
//	//		if (p[i] != (c *= 999983))
//	//		{
//	//			while(1) {};
//	//		};
//	//	};
//
//	//	PIOA->CODR = 1;
//	//};
//
//#endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void WritePGA(u16 v)
{
	
	*pSPI1_BAUD = 7; // SCLK=7MHz
	*pSPI1_FLG = 0;//FLS5|FLS2;
	*pSPI1_CTL = SPE|MSTR|SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
	*pPORTGIO_CLEAR = 1<<11;
	*pSPI1_TDBR = v;

	while((*pSPI1_STAT&1) == 0) ;

	*pPORTGIO_SET = 1<<11;

	*pSPI1_CTL = 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 ReadADC()
{
	
	*pSPI1_BAUD = 3; // SCLK=16MHz
	*pSPI1_FLG = 0;//FLS5|FLS2;
	*pSPI1_STAT = 0x7F;
	*pSPI1_CTL = SPE|MSTR|CPOL|/*CPHA|*/SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
	*pPORTGIO_CLEAR = 1<<12;
	*pSPI1_TDBR = 0;

	while((*pSPI1_STAT&1) == 0) ;

	u16 v = *pSPI1_RDBR;

	*pPORTGIO_SET = 1<<12;

	*pSPI1_CTL = 0;

	return v;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void InitADC16()
{
	//using namespace HW;

	////PMC->PCER0 = PID::SPI0_M;

	////SPI0->CR = 1; 
	////SPI0->MR = 0xFF010005;
	////SPI0->CSR[0] = 0x00095482;

	//bufADC1.txp = &txADC1;
	//bufADC1.rxp = &rxADC1;
	//bufADC1.count = 25;
	//bufADC1.CSR = 0x00092302;
	//bufADC1.DLYBCS = 0x9;
	//bufADC1.PCS = 1;
	//bufADC1.pCallBack = CallBackADC1;

	//bufADC2.txp = &txADC2;
	//bufADC2.rxp = &rxADC2;
	//bufADC2.count = 25;
	//bufADC2.CSR = 0x00092302;
	//bufADC2.DLYBCS = 0x9;
	//bufADC2.PCS = 0;
	//bufADC2.pCallBack = CallBackADC2;

	//spi.AddRequest(&bufADC1);
	//spi.AddRequest(&bufADC2);

}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static void UpdateADC16()
{
	//using namespace HW;

	//if (bufADC1.ready)
	//{
	//	spi.AddRequest(&bufADC1);
	//};

	//if (bufADC2.ready)
	//{
	//	spi.AddRequest(&bufADC2);
	//};

}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
#ifndef WIN32

	LowLevelInit();

	InitRTT();

	InitSPORT();

	//InitDisplay();

	InitADC16();

	u32 t = GetRTT();

	while((GetRTT() - t) < 100000)
	{
		UpdateHardware();
	};

#else

	InitDisplay();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC16();

//	spi.Update();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
