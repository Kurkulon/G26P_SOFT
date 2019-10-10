#include "twi.h"
#include "COM_DEF.h"

#pragma O3
#pragma Otime

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* -----------------------------  USIC_CH_PCR_IICMode  ---------------------------- */
#define SLAD(v)		((v)&0xffff)                /*!< USIC_CH PCR_IICMode: SLAD (Bitfield-Mask: 0xffff)           */
#define ACK00     	(1<<16UL)                    /*!< USIC_CH PCR_IICMode: ACK00 (Bit 16)                         */
#define STIM      	(1<<17UL)                    /*!< USIC_CH PCR_IICMode: STIM (Bit 17)                          */
#define SCRIEN    	(1<<18UL)                    /*!< USIC_CH PCR_IICMode: SCRIEN (Bit 18)                        */
#define RSCRIEN   	(1<<19UL)                    /*!< USIC_CH PCR_IICMode: RSCRIEN (Bit 19)                       */
#define PCRIEN    	(1<<20UL)                    /*!< USIC_CH PCR_IICMode: PCRIEN (Bit 20)                        */
#define NACKIEN   	(1<<21UL)                    /*!< USIC_CH PCR_IICMode: NACKIEN (Bit 21)                       */
#define ARLIEN    	(1<<22UL)                    /*!< USIC_CH PCR_IICMode: ARLIEN (Bit 22)                        */
#define SRRIEN    	(1<<23UL)                    /*!< USIC_CH PCR_IICMode: SRRIEN (Bit 23)                        */
#define ERRIEN    	(1<<24UL)                    /*!< USIC_CH PCR_IICMode: ERRIEN (Bit 24)                        */
#define SACKDIS   	(1<<25UL)                    /*!< USIC_CH PCR_IICMode: SACKDIS (Bit 25)                       */
#define HDEL(v)		(((v)&0xF)<<26UL)                    /*!< USIC_CH PCR_IICMode: HDEL (Bit 26)                          */
#define ACKIEN    	(1<<30UL)                    /*!< USIC_CH PCR_IICMode: ACKIEN (Bit 30)                        */
//#define MCLK      	(1<<31UL)                    /*!< USIC_CH PCR_IICMode: MCLK (Bit 31)                          */

#define SLSEL         (0x1UL)                   /*!< USIC_CH PSR_IICMode: SLSEL (Bitfield-Mask: 0x01)            */
#define WTDF          (0x2UL)                   /*!< USIC_CH PSR_IICMode: WTDF (Bitfield-Mask: 0x01)             */
#define SCR           (0x4UL)                   /*!< USIC_CH PSR_IICMode: SCR (Bitfield-Mask: 0x01)              */
#define RSCR          (0x8UL)                   /*!< USIC_CH PSR_IICMode: RSCR (Bitfield-Mask: 0x01)             */
#define PCR           (0x10UL)                  /*!< USIC_CH PSR_IICMode: PCR (Bitfield-Mask: 0x01)              */
#define NACK          (0x20UL)                  /*!< USIC_CH PSR_IICMode: NACK (Bitfield-Mask: 0x01)             */
#define ARL           (0x40UL)                  /*!< USIC_CH PSR_IICMode: ARL (Bitfield-Mask: 0x01)              */
#define SRR           (0x80UL)                  /*!< USIC_CH PSR_IICMode: SRR (Bitfield-Mask: 0x01)              */
#define ERR           (0x100UL)                 /*!< USIC_CH PSR_IICMode: ERR (Bitfield-Mask: 0x01)              */
#define ACK           (0x200UL)                 /*!< USIC_CH PSR_IICMode: ACK (Bitfield-Mask: 0x01)              */
#define RSIF          (0x400UL)                 /*!< USIC_CH PSR_IICMode: RSIF (Bitfield-Mask: 0x01)             */
#define DLIF          (0x800UL)                 /*!< USIC_CH PSR_IICMode: DLIF (Bitfield-Mask: 0x01)             */
#define TSIF          (0x1000UL)                /*!< USIC_CH PSR_IICMode: TSIF (Bitfield-Mask: 0x01)             */
#define TBIF          (0x2000UL)                /*!< USIC_CH PSR_IICMode: TBIF (Bitfield-Mask: 0x01)             */
#define RIF           (0x4000UL)                /*!< USIC_CH PSR_IICMode: RIF (Bitfield-Mask: 0x01)              */
#define AIF           (0x8000UL)                /*!< USIC_CH PSR_IICMode: AIF (Bitfield-Mask: 0x01)              */
#define BRGIF         (0x10000UL)               /*!< USIC_CH PSR_IICMode: BRGIF (Bitfield-Mask: 0x01)            */

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define TDF_MASTER_SEND				(0U << 8U)
#define TDF_SLAVE_SEND				(1U << 8U)
#define TDF_MASTER_RECEIVE_ACK   	(2U << 8U)
#define TDF_MASTER_RECEIVE_NACK  	(3U << 8U)
#define TDF_MASTER_START         	(4U << 8U)
#define TDF_MASTER_RESTART      	(5U << 8U)
#define TDF_MASTER_STOP         	(6U << 8U)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define __SCTR (SDIR(1) | TRM(3) | FLE(0x3F) | WLE(7))

#define __CCR (MODE(4))

#define __BRG (DCTQ(24))

#define __DX0CR (DSEL(1) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX1CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

#define __PCR (STIM)

#define __FDR (STEP(0x3FF) | DM(1))

#define __TCSR (TDEN(1)|TDSSM(1))

#define TWI (HW::USIC2_CH0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte *wrPtr = 0;
static byte *rdPtr = 0;
static u16 wrCount = 0;
static u16 rdCount = 0;
static byte *wrPtr2 = 0;
static u16 wrCount2 = 0;
static byte adr = 0;
static DSCTWI* dsc = 0;
static DSCTWI* lastDsc = 0;
static byte state = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Update_TWI()
{
	if (dsc == 0)
	{ 
		return false; 
	}
	//else if ((hw->SR & 1) != 0)
	//{
	//	hw->PDC.PTCR = 0x202;
	//	dsc->ready = true;
	//	dsc = 0;

	//	return false;
	//}
	else
	{
		return true;
	};

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void Handler_TWI()
{
	using namespace HW;

	HW::P5->BSET(7);

	u32 a = TWI->PSR_IICMode;

	if(a & ACK)
	{
		if (wrCount > 0)
		{
			TWI->TBUF[0] = TDF_MASTER_SEND | *wrPtr++;

			wrCount--;

			if(wrCount == 0 && wrCount2 != 0)
			{
				wrPtr = wrPtr2;
				wrCount = wrCount2;
				wrCount2 = 0;
			};
		}
		else if (rdCount > 0)
		{
			if(a & (SCR|RSCR))
			{
				TWI->TBUF[0] = TDF_MASTER_RECEIVE_ACK; 
			}
			else
			{
				TWI->TBUF[0] = TDF_MASTER_RESTART | (adr << 1) | 1;
			};
		}
		else
		{
			TWI->TBUF[0] = TDF_MASTER_STOP;
		};
	}
	else if (a & (RIF|AIF))
	{
		byte t = TWI->RBUF;

		if (rdCount > 0)
		{
			*rdPtr++ = t; // receive data
			rdCount--;
		};
			
		TWI->TBUF[0] = (rdCount > 0) ? TDF_MASTER_RECEIVE_ACK : TDF_MASTER_RECEIVE_NACK; 
	}
	//else if (wrCount != 0 || rdCount != 0)
	//{
	//	TWI->PSCR = a;

	//	TWI->TBUF[0] = TDF_MASTER_STOP; 

	//	wrCount = 0;
	//	rdCount = 0;
	//}
	else
	{
		dsc->ready = true;

		state = 0;
		
		DSCTWI *ndsc = dsc->next;

		if (ndsc != 0)
		{
			dsc->next = 0;
			dsc = ndsc;

			dsc->ready = false;

			wrPtr = (byte*)dsc->wdata;	
			rdPtr = (byte*)dsc->rdata;	
			wrPtr2 = (byte*)dsc->wdata2;	
			wrCount = dsc->wlen;
			wrCount2 = dsc->wlen2;
			rdCount = dsc->rlen;
			adr = dsc->adr;

			if (wrPtr2 == 0) wrCount2 = 0;

			//TWI->CCR |= RIEN|AIEN;
			//TWI->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

			TWI->TBUF[0] = TDF_MASTER_START | (dsc->adr << 1) | ((wrCount == 0) ? 1 : 0);
		}
		else
		{
			TWI->CCR = __CCR;
			TWI->PCR_IICMode = __PCR;

			lastDsc = dsc = 0;
		};

//		TWI->PSCR = PCR|NACK;
	};

	TWI->PSCR = a;

	HW::P5->BCLR(7);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Write_TWI(DSCTWI *d)
{
	using namespace HW;

	if (dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	lastDsc = dsc = d;

	VectorTableExt[USIC2_0_IRQn] = Handler_TWI;
	CM4::NVIC->CLR_PR(USIC2_0_IRQn);
	CM4::NVIC->SET_ER(USIC2_0_IRQn);

	d->ready = false;

	wrPtr = (byte*)dsc->wdata;	
	rdPtr = (byte*)dsc->rdata;	
	wrPtr2 = (byte*)dsc->wdata2;	
	wrCount = dsc->wlen;
	wrCount2 = dsc->wlen2;
	rdCount = dsc->rlen;
	adr = dsc->adr;

	if (wrPtr2 == 0) wrCount2 = 0;

	__disable_irq();

	//while(TWI->TCSR & USIC_CH_TCSR_TDV_Msk);

	TWI->PSCR = ~0;//RIF|AIF|TBIF|ACK|NACK|PCR;

	state = (wrCount == 0) ? 1 : 0;

	TWI->TBUF[0] = TDF_MASTER_START | (dsc->adr << 1) | state;

	TWI->CCR |= RIEN|AIEN;
	TWI->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

	__enable_irq();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool AddRequest_TWI(DSCTWI *d)
{
	d->next = 0;
	d->ready = false;

	__disable_irq();

	if (lastDsc == 0)
	{
		lastDsc = d;

		__enable_irq();

		return Write_TWI(d);
	}
	else
	{

		lastDsc->next = d;
		lastDsc = d;

		__enable_irq();
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Init_TWI()
{
	using namespace HW;

	//num &= 1;

	HW::Peripheral_Enable(PID_USIC2);

 	P5->ModePin0(A1OD);
	P5->ModePin2(A1OD);

	TWI->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

	TWI->SCTR = __SCTR;

	TWI->FDR = (1024 - (((MCK + 400000/2) / 400000 + 8) / 16)) | DM(1);
	TWI->BRG = __BRG;
    
	TWI->TCSR = __TCSR;

	TWI->PSCR = ~0;

	TWI->CCR = 0;

	TWI->DX0CR = __DX0CR;
	TWI->DX1CR = __DX1CR;

	TWI->CCR = __CCR;


	TWI->PCR_IICMode = __PCR;

  //XMC_I2C_CH_MasterStart(XMC_I2C1_CH0, IO_EXPANDER_ADDRESS, XMC_I2C_CH_CMD_WRITE);

/*	while(TWI->TCSR & USIC_CH_TCSR_TDV_Msk);

	TWI->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	TWI->TBUF[0] = 0xA0 | TDF_MASTER_START | 1;


	while((TWI->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	TWI->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;

  //XMC_I2C_CH_MasterTransmit(XMC_I2C1_CH0, IO_DIR);

	while(TWI->TCSR & USIC_CH_TCSR_TDV_Msk);

	TWI->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	TWI->TBUF[0] = 0 | TDF_MASTER_RECEIVE_NACK;


	while((TWI->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	TWI->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;


 // XMC_I2C_CH_MasterTransmit(XMC_I2C1_CH0, 0xffU);


	while(TWI->TCSR & USIC_CH_TCSR_TDV_Msk);

	TWI->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	TWI->TBUF[0] = 0xFF | TDF_MASTER_RECEIVE_NACK;


	while((TWI->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	TWI->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;
*/






	//VectorTableExt[USIC1_1_IRQn] = Handler0;
	//CM4::NVIC->CLR_PR(USIC1_1_IRQn);
	//CM4::NVIC->SET_ER(USIC1_1_IRQn);

//	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
