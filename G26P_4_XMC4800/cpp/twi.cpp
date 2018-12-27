#include "twi.h"
#include "COM_DEF.h"

//#pragma O3
//#pragma Otime

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

#define __DX0CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX1CR (DSEL(1) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

#define __PCR (STIM | SMD(1) | SP(9) | RSTEN(1) | TSTEN(1))

#define __FDR (STEP(0x3FF) | DM(1))

#define __TCSR (TDEN(1)|TDSSM(1))

#define TWI1 (HW::USIC1_CH1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte *twi_wrPtr = 0;
static byte *twi_rdPtr = 0;
static u16 twi_wrCount = 0;
static u16 twi_rdCount = 0;
static byte *twi_wrPtr2 = 0;
static u16 twi_wrCount2 = 0;
static byte twi_adr = 0;
static DSCTWI* twi_dsc = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI::Init(byte num)
{
	using namespace HW;

	//num &= 1;

	hw = TWI1;


	SCU_CLK->CGATCLR1 = SCU_CLK_CGATSTAT1_USIC1_Msk;
	SCU_RESET->PRCLR1 = SCU_RESET_PRCLR1_USIC1RS_Msk;

	hw->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

	hw->SCTR = __SCTR;

	hw->FDR = (1024 - (((MCK + 400000/2) / 400000 + 8) / 16)) | DM(1);
	hw->BRG = __BRG;
    
	hw->TCSR = __TCSR;

	hw->PSCR = ~0;

	hw->CCR = 0;

	hw->DX0CR = __DX0CR;
	hw->DX1CR = __DX1CR;

	hw->CCR = __CCR;

 	P0->ModePin13(A2OD);
	P3->ModePin15(A2OD);

//	hw->PCR_IICMode = __PCR;

  //XMC_I2C_CH_MasterStart(XMC_I2C1_CH0, IO_EXPANDER_ADDRESS, XMC_I2C_CH_CMD_WRITE);

	while(hw->TCSR & USIC_CH_TCSR_TDV_Msk);

	hw->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	hw->TBUF[0] = 0xA0 | TDF_MASTER_START | 1;


	while((hw->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	hw->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;

  //XMC_I2C_CH_MasterTransmit(XMC_I2C1_CH0, IO_DIR);

	while(hw->TCSR & USIC_CH_TCSR_TDV_Msk);

	hw->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	hw->TBUF[0] = 0 | TDF_MASTER_RECEIVE_NACK;


	while((hw->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	hw->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;


 // XMC_I2C_CH_MasterTransmit(XMC_I2C1_CH0, 0xffU);


	while(hw->TCSR & USIC_CH_TCSR_TDV_Msk);

	hw->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	hw->TBUF[0] = 0xFF | TDF_MASTER_RECEIVE_NACK;


	while((hw->PSR_IICMode & USIC_CH_PSR_IICMode_ACK_Msk) == 0U);
  
	hw->PSCR = USIC_CH_PSR_IICMode_ACK_Msk;







	//VectorTableExt[USIC1_1_IRQn] = Handler0;
	//CM4::NVIC->CLR_PR(USIC1_1_IRQn);
	//CM4::NVIC->SET_ER(USIC1_1_IRQn);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI::Write(DSCTWI *d)
{
	using namespace HW;

	if (twi_dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

//	smask = 1<<13;
	dsc = d;

	VectorTableExt[USIC1_1_IRQn] = Handler0;
	CM4::NVIC->CLR_PR(USIC1_1_IRQn);
	CM4::NVIC->SET_ER(USIC1_1_IRQn);

	dsc->ready = false;

	twi_wrPtr = (byte*)twi_dsc->wdata;	
	twi_rdPtr = (byte*)twi_dsc->rdata;	
	twi_wrPtr2 = (byte*)twi_dsc->wdata2;	
	twi_wrCount = twi_dsc->wlen;
	twi_wrCount2 = twi_dsc->wlen2;
	twi_rdCount = twi_dsc->rlen;
	twi_adr = twi_dsc->adr;

	if (twi_wrPtr2 == 0) twi_wrCount2 = 0;

	__disable_irq();

	while(hw->TCSR & USIC_CH_TCSR_TDV_Msk);

	hw->PSCR = USIC_CH_PSR_IICMode_TBIF_Msk;

	hw->TBUF[0] = TDF_MASTER_START | (twi_dsc->adr << 1) | ((twi_wrCount == 0) ? 1 : 0);

	hw->PCR_IICMode |= ACKIEN;

	__enable_irq();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI::Read(DSCTWI *d)
{
	//if (dsc != 0 || d == 0) { return false; };
	//if (d->data == 0 || d->len == 0) { return false; }

	//dsc = d;

	//__enable_irq();

	//dsc->ready = false;
	//hw->CR = 0x80;
	//hw->MMR = dsc->MMR|0x1000;
	//hw->IADR = dsc->IADR;
	//hw->CWGR = dsc->CWGR;
	//hw->CR = 0x24;

	//hw->PDC.RPR = dsc->data;
	//
	//if (dsc->len > 1)
	//{
	//	hw->PDC.RCR = dsc->len-1;
	//	hw->IER = 1<<12;
	//	hw->PDC.PTCR = 0x201;
	//	hw->CR = 0x1;
	//}
	//else
	//{
	//	hw->IER = 2;
	//	hw->CR = 0x3;
	//};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TWI::Update()
{
	return false;

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

__irq void TWI::Handler0()
{
	using namespace HW;

//	__breakpoint(0);

	u32 a = TWI1->PSR_IICMode;

	if(a & NACK) // NACK
	{
		//TWI1->PDC.PTCR = 0x202;;
		//TWI1->IDR = 3<<12;
	}
	else if(a & ACK) // ENDTX
	{
		if (twi_wrCount > 0)
		{
			TWI1->TBUF[0] = TDF_MASTER_SEND | *twi_wrPtr++;

			twi_wrCount--;

			if(twi_wrCount == 0 && twi_wrCount2 != 0)
			{
				twi_wrPtr = twi_wrPtr2;
				twi_wrCount = twi_wrCount2;
				twi_wrCount2 = 0;
			};
		}
		else if (twi_rdCount > 0)
		{
			TWI1->TBUF[0] = TDF_MASTER_RESTART | (twi_adr << 1) | 1;
		}
		else
		{
			TWI1->TBUF[0] = TDF_MASTER_STOP;
		};

		TWI1->PSCR = ACK;
	}
	else if(a & (1<<12)) // ENDRX
	{
		//TWI1->PDC.PTCR = 2;
		//TWI1->IDR = 1<<12;
		//TWI1->CR = 2;
		//TWI1->IER = 2;
	}
	else if(a & 2) // RXRDY
	{
		//TWI1->IDR = 2;
		//*(T_HW::AT91_REG*)TWI1->PDC.RPR = TWI1->RHR;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
