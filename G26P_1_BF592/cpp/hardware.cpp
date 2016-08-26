#include "hardware.h"

#include <bfrom.h>
#include <sys\exception.h>
//#include <cdefBF592-A.h>
//#include <ccblkfn.h>

//#pragma optimize_for_speed

const u32 coreCLK = CCLK;
const u32 sysCLK = SCLK;

U32u adcValue;

u16 pgaValue = 0x2A01;
bool pgaSet = true;

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
void BootHook(ADI_BOOT_DATA *) 
{

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

	*TIMER0_CONFIG
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();

								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x0300;		//  0000 0011 0000 0000
	*pPORTG_MUX = 0x0000;		//  0000 0000 0000 0000

	*pPORTF_FER = 0x180F;		//  0001 1000 0000 1111
	*pPORTG_FER = 0x070F;		//  0000 0111 0000 1111

	*pPORTFIO_DIR = 0x05A0;		//  0000 0101 1010 0000
	*pPORTGIO_DIR = 0x1800;		//  0001 1000 0000 0000

	*pPORTFIO_INEN = 0x0003;	//  0000 0000 0000 0011
	*pPORTGIO_INEN = 0x0003;	//  0000 0000 0000 0000

	*pPORTGIO_SET = 3<<11;

	*pWDOG_CNT = MS2SCLK(10);
	*pWDOG_CTL = WDEV_RESET|WDEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void WritePGA(u16 v)
//{
//	
//	*pSPI1_BAUD = 7; // SCLK=7MHz
//	*pSPI1_FLG = 0;//FLS5|FLS2;
//	*pSPI1_CTL = SPE|MSTR|SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
//	*pPORTGIO_CLEAR = 1<<11;
//	*pSPI1_TDBR = v;
//
//	while((*pSPI1_STAT&1) == 0) ;
//
//	*pPORTGIO_SET = 1<<11;
//
//	*pSPI1_CTL = 0;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	static byte i = 0;

	static TM32 tm;

	u16 t;

	switch (i)
	{
		case 0:

			if (tm.Check(MS2SCLK(1)))
			{
				*pSPI1_BAUD = 50; // SCLK=16MHz
				*pSPI1_FLG = 0;//FLS5|FLS2;
				*pSPI1_STAT = 0x7F;
				*pSPI1_CTL = SPE|MSTR|CPOL|/*CPHA|*/SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
				*pPORTGIO_CLEAR = 1<<12;
				
				t = *pSPI1_RDBR;
				
				*pSPI1_TDBR = 0;

				i++;
			}
			else
			{
				i = 2;
			};

			break;

		case 1:

			if ((*pSPI1_STAT&1) != 0)
			{
				adcValue.d += ((i32)(*pSPI1_RDBR) - (i32)adcValue.w[1])*8192;

//				adcValue.w[1] = *pSPI1_RDBR;

				*pPORTGIO_SET = 1<<12;

				*pSPI1_CTL = 0;

				i++;
			};

			break;

		case 2:

			if (pgaSet)
			{
				*pSPI1_BAUD = 7; // SCLK=7MHz
				*pSPI1_FLG = 0;//FLS5|FLS2;
				*pSPI1_CTL = SPE|MSTR|SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
				*pPORTGIO_CLEAR = 1<<11;
				*pSPI1_TDBR = pgaValue;

				pgaSet = false;

				i++;
			}
			else
			{
				i = 0;
			};

			break;

		case 3:

			if ((*pSPI1_STAT&1) != 0)
			{
				*pPORTGIO_SET = 1<<11;

				*pSPI1_CTL = 0;

				i = 0;
			};

			break;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	LowLevelInit();

	InitRTT();

	InitSPORT();

	u32 t = GetRTT();

	while((GetRTT() - t) < 100000)
	{
		UpdateHardware();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateADC();

	*pWDOG_STAT = 0; //Reset WDT
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
