#include "hardware.h"

#include <bfrom.h>
#include <sys\exception.h>
//#include <cdefBF592-A.h>
//#include <ccblkfn.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void ManDisable() { *pPORTFIO = 0x50; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; }
inline void ManOne() { /**pPORTFIO = 0x50;*/ *pPORTFIO = 0xC0; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0xC0; *pPORTFIO_SET = 0x30; }
inline void ManZero() { /**pPORTFIO = 0x50;*/ *pPORTFIO = 0x30; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0x30; *pPORTFIO_SET = 0xC0; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define BOUD2CLK(x) ((u32)((SCLK/2.0)/x+0.5))

static const u16 manboud[4] = { BOUD2CLK(20833), BOUD2CLK(41666), BOUD2CLK(62500), BOUD2CLK(83333) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz


u16 trmHalfPeriod = BOUD2CLK(41666);
byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetTrmBoudRate(byte i)
{
	trmHalfPeriod = manboud[i&3];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;
const u16 rcvQuartPeriod = BOUD2CLK(20833)/2;

static MRB *manRB = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool trmStartCmd = false;
bool trmStartData = false;

u32 icount = 0;
static byte ib;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 CheckParity(u16 x)
{
	u16 y = x ^ (x >> 1);

	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	
	return (y ^ (y >> 8))^1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRTT()
{
	*pTIMER0_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS;
	*pTIMER0_PERIOD = 0;
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

static void InitSPORT()
{
	*pDMA1_CONFIG = 0;
	*pDMA3_CONFIG = 0;
	*pSPORT0_RCR1 = 0;
	*pSPORT1_RCR1 = 0;
	//*pSPORT0_RCR2 = 15; //|RXSE;
	//*pSPORT0_RCLKDIV = 0;
	//*pSPORT0_RFSDIV = 34;
	//*pSPORT0_RCR1 = RCKFE|LARFS|LRFS|RFSR|IRFS|IRCLK|RSPEN;

	*pEVT9 = (void*)SPORT_ISR;
	*pIMASK |= EVT_IVG9; 
	*pSIC_IMASK |= 1<<9;

	//register_handler(ik_ivg9, SPORT_ISR);


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

static void LowLevelInit()
{
	Init_PLL();

								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x0000;		//  0000 0000 0000 0000
//	*pPORTG_MUX = 0x0000;		//  0000 0000 0000 0000

	*pPORTF_FER = 0x1800;		//  0001 1000 0000 0000
//	*pPORTG_FER = 0xE70F;		//  0000 0000 0000 0000

	*pPORTFIO_DIR = 0x00F4;		//  0000 0000 1111 0100
//	*pPORTGIO_DIR = 0x1800;		//  0001 1000 0000 0000

	*pPORTFIO_INEN = 0x0601;	//  0000 0110 0000 0001
//	*pPORTGIO_INEN = 0x0003;	//  0000 0000 0000 0000

	*pPORTF_PADCTL = 0x0011;	//  0000 0000 0001 0000 Schmitt trigger

	ManDisable();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void SendManCmd(void *data, u16 len)
//{
//	stateManTrans = 0;
//	trmStartCmd = true;
//	trmData = (u16*)data;
//	trmDataLen = len;
//	*pTIMER1_PERIOD = trmHalfPeriod;
//	*pTIMER_ENABLE = TIMEN1;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(TIMER_ISR)
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static u16 *data = 0;
	static u16 len = 0;

	*pPORTFIO_SET = 1<<2;

	switch (stateManTrans)
	{
		case 0:	// Idle; 

			ManDisable();
		
			data = (u16*)manTB->data;
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

			ManDisable();
			stateManTrans = 0;
			*pTIMER_DISABLE = TIMDIS1;
			*pSIC_IMASK &= ~IRQ_TIMER1;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)

	*pTIMER_STATUS = 2;

	*pPORTFIO_CLEAR = 1<<2;
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

	*pEVT11 = (void*)TIMER_ISR;
	*pIMASK |= EVT_IVG11; 
	*pSIC_IMASK |= IRQ_TIMER1;

	*pTIMER1_PERIOD = trmHalfPeriod;
	*pTIMER1_WIDTH = 1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS|IRQ_ENA;
	*pTIMER_ENABLE = TIMEN1;

	return trmBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManTransmit()
{
	//*pEVT11 = (void*)TIMER_ISR;
	//*pIMASK |= EVT_IVG11; 
	//*pSIC_IMASK |= IRQ_TIMER1;

	*pTIMER1_PERIOD = trmHalfPeriod;
	*pTIMER1_WIDTH = 1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS|IRQ_ENA;
	*pTIMER_DISABLE = TIMDIS1;

	//register_handler(ik_ivg9, SPORT_ISR);


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(MANRCVR_ISR)
{
	u16 t;
	static byte ib;
	static u32 rw;
	static u16 l, lp;
//	static byte lastbit, bit;
//	static byte sc;
	static u32 *p;
	static u16 count;
	static bool c;

	enum {FSP = 0, SSP = 1, FH = 3, SH = 4, END = 49, ERR = 50};

	*pPORTFIO_SET = 1<<2;

	if (((rcvCount++) & 1) == 0) switch (stateManRcvr)
	{
		case 0:

			l = 17;
			lp = 7;
			p = (u32*)manRB->data;
			count = manRB->maxLen;
			c = false;

			rw = 0;
			rw |= *pPORTFIO & 1; // синхро бит 
			lp--;

			stateManRcvr++;

			break;

		case 1:

			rw <<= 1;
			rw |= *pPORTFIO & 1; // синхро бит 
			lp--;

			if (rw >= 14 || lp == 0)
			{
				stateManRcvr++;
			};

			break;

		case 2:

			rw <<= 1;
			rw |= *pPORTFIO & 1; // бит данных
			l--;

			if (l > 0)
			{
				stateManRcvr++;
			}
			else
			{
				*p++ = rw;
				count--;

				rw = 0;

				lp = 6;
				stateManRcvr = (count == 0) ? END : 4;
			};

			break;

		case 3:

			stateManRcvr--;

			break;

		case 4:

			rw <<= 1;
			rw |= *pPORTFIO & 1; // синхробит
			lp--;

			if (lp == 0)
			{
				l = 17;

				if (rw == 0 || rw >=63)
				{
					stateManRcvr = END;
				}
				else 
				{
					c = c || !((rw == 0x38) || rw == 7);
					stateManRcvr = 3;
				};
			};

			break;

		case END:
			
			manRB->OK = !c;
			manRB->ready = true;
			manRB->len = manRB->maxLen - count;
			*pTCNTL = 0;
			*pPORTFIO_MASKA = 0;
			*pPORTFIO_CLEAR = 1<<10;
			*pSIC_IMASK &= ~IRQ_PFA_PORTF;

			rcvBusy = false;

			break;
	};

	*pPORTFIO_CLEAR = (1<<2);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(WAITMANRCVR_ISR)
{
	*pPORTFIO_SET = 1<<2;

	*pTCNTL = TMPWR;

	*pPORTFIO_CLEAR = (1<<2)|(1<<10);

	*pTPERIOD = rcvQuartPeriod;

	*pPORTFIO_SET = 1<<2;

	*pTCNTL = TAUTORLD|TMREN|TMPWR;

	if ((*pILAT & EVT_IVTMR) == 0) { rcvCount = 0; };

	*pPORTFIO_CLEAR = 1<<2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitManRecieve()
{
	*pTSCALE = 0;
	*pTPERIOD = rcvQuartPeriod;
	*pEVT6 = (void*)MANRCVR_ISR;
	*pTCNTL = TMPWR;;

	*pIMASK |= EVT_IVTMR; 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
	if (rcvBusy || trmBusy || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	stateManRcvr = 0;
	rcvCount = 0;

	*pEVT11 = (void*)WAITMANRCVR_ISR;
	*pSIC_IMASK |= IRQ_PFA_PORTF;
	*pIMASK |= EVT_IVG11; 

	*pPORTFIO_EDGE = 1<<10;
	*pPORTFIO_BOTH = 1<<10;
	*pPORTFIO_CLEAR = 1<<10;
	*pPORTFIO_MASKA = 1<<10;
	*pPORTFIO_MASKB = 0;

	return rcvBusy = true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{

//	spi.Update();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	LowLevelInit();

	InitRTT();

	InitManTransmit();

	InitManRecieve();

//	InitSPORT();

	//InitDisplay();

//	InitADC16();

	//u32 t = GetRTT();

	//while((GetRTT() - t) < 100000)
	//{
	//	UpdateHardware();
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


