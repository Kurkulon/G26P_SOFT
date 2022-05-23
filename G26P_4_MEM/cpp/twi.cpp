#include "twi.h"
#include "COM_DEF.h"
#include "time.h"
#include "hw_conf.h"

#ifdef WIN32
#include <windows.h>

static byte fram_I2c_Mem[0x10000];

#endif

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

#define __DX0CR (DSEL(1) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX1CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(1) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

#define __PCR (STIM)

#define __FDR ((1024 - (((MCK + 400000/2) / 400000 + 8) / 16)) | DM(1))

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

//byte state = 0;

u32 twiErr = 0;

static volatile byte intCount = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static __irq void Handler_TWI()
{
	using namespace HW;

	u32 a = TWI->PSR_IICMode;

	if(a & ACK)
	{
		intCount++;

		if (wrCount > 0)
		{
//			state = 1;

			TWI->TBUF[0] = TDF_MASTER_SEND | *wrPtr++;

			wrCount--;

			dsc->ack = true;

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
//				state = 2;

				TWI->TBUF[0] = TDF_MASTER_RECEIVE_ACK; 
			}
			else
			{
//				state = 3;

				TWI->TBUF[0] = TDF_MASTER_RESTART | (adr << 1) | 1;
			};
		}
		else
		{
//			state = 4;

			TWI->TBUF[0] = TDF_MASTER_STOP;
		};
	}
	else if (a & (RIF|AIF))
	{
		intCount++;

//		state = 5;

		byte t = TWI->RBUF;

		if (rdCount > 0)
		{
			*rdPtr++ = t; // receive data
			rdCount--;
		};
			
		TWI->TBUF[0] = (rdCount > 0) ? TDF_MASTER_RECEIVE_ACK : TDF_MASTER_RECEIVE_NACK; 
	}
	else if ((a & PCR) == 0)
	{
//		state = 6;

		TWI->TBUF[0] = TDF_MASTER_STOP; 
	}
	else
	{
		dsc->ready = true;
		dsc->readedLen = dsc->rlen - rdCount;

		DSCTWI *ndsc = dsc->next;

		if (ndsc != 0)
		{
//			state = 7;

			dsc->next = 0;
			dsc = ndsc;

			dsc->ready = false;
			dsc->ack = false;
			dsc->readedLen = 0;

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

			TWI->PSCR = ~0;

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
}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Write_TWI(DSCTWI *d)
{
#ifndef WIN32

	using namespace HW;

	if (dsc != 0 || d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	lastDsc = dsc = d;

	VectorTableExt[USIC2_0_IRQn] = Handler_TWI;
	CM4::NVIC->CLR_PR(USIC2_0_IRQn);
	CM4::NVIC->SET_ER(USIC2_0_IRQn);

	d->ready = false;
	d->ack = false;
	d->readedLen = 0;

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

	//state = (wrCount == 0) ? 1 : 0;

	TWI->TBUF[0] = TDF_MASTER_START | (dsc->adr << 1) | ((wrCount == 0) ? 1 : 0);

	TWI->CCR |= RIEN|AIEN;
	TWI->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

	__enable_irq();

#endif

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool AddRequest_TWI(DSCTWI *d)
{
#ifndef WIN32

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

#else

	if (d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

	u16 adr;

	switch (d->adr)
	{
		case 0x49: //Temp

			if (d->rlen >= 2)
			{
				byte *p = (byte*)d->rdata;

				p[0] = 0;
				p[1] = 0;
			};
				
			d->readedLen = d->rlen;
			d->ack = true;
			d->ready = true;

			break;

		case 0x50: // FRAM

			d->readedLen = 0;

			if (d->wlen == 2)
			{
				adr = ReverseWord(*((u16*)d->wdata));

				adr %= sizeof(fram_I2c_Mem);

				if (d->wdata2 != 0 && d->wlen2 != 0)
				{
					u16 count = d->wlen2;
					byte *s = (byte*)d->wdata2;
					byte *d = fram_I2c_Mem + adr;

					while (count-- != 0) { *(d++) = *(s++); adr++; if (adr >= sizeof(fram_I2c_Mem)) { adr = 0; d = fram_I2c_Mem; }; };
				}
				else if (d->rdata != 0 && d->rlen != 0)
				{
					d->readedLen = d->rlen;
					u16 count = d->rlen;

					byte *p = (byte*)(d->rdata);
					byte *s = fram_I2c_Mem + adr;

					while (count-- != 0) { *(p++) = *(s++); adr++; if (adr >= sizeof(fram_I2c_Mem)) { adr = 0; s = fram_I2c_Mem; }; };
				};
			};

			d->ack = true;
			d->ready = true;

			break;
	};

#endif

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Init_TWI()
{
#ifndef WIN32

	using namespace HW;

	//num &= 1;

	HW::Peripheral_Disable(PID_USIC2);

	P5->ModePin2(G_OD);
	P5->ModePin0(G_OD);
	P5->BSET(0);

	for (byte i = 0; i < 32; i++)
	{
		P5->BTGL(2);
		delay(MCK_MHz);
	};

	P5->BCLR(0);

	for (byte i = 0; i < 32; i++)
	{
		P5->BTGL(2);
		delay(MCK_MHz);
	};

	P5->BSET(2);
	P5->BSET(0);

	HW::Peripheral_Enable(PID_USIC2);

 	P5->ModePin0(A1OD);
	P5->ModePin2(A1PP);

	TWI->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

	TWI->SCTR = __SCTR;

	TWI->FDR = __FDR;
	TWI->BRG = __BRG;
    
	TWI->TCSR = __TCSR;

	TWI->PSCR = ~0;

	TWI->CCR = 0;

	TWI->DX0CR = __DX0CR;
	TWI->DX1CR = __DX1CR;

	TWI->CCR = __CCR;


	TWI->PCR_IICMode = __PCR;

	delay(1000);

#else

	HANDLE h;

	h = CreateFile("FRAM_I2C_STORE.BIN", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	};

	dword bytes;

	ReadFile(h, fram_I2c_Mem, sizeof(fram_I2c_Mem), &bytes, 0);
	CloseHandle(h);

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Update_TWI()
{
#ifndef WIN32

	using namespace HW;

	static TM32 tm;

//	HW::P5->BSET(7);

	__disable_irq();

	if (dsc != 0)
	{
		if (intCount != 0)
		{
			intCount = 0;

			tm.Reset();
		}
		else if (tm.Check(10))
		{
			HW::Peripheral_Disable(PID_USIC2);

			twiErr++;

 			P5->ModePin0(A1OD);
			P5->ModePin2(A1PP);

			HW::Peripheral_Enable(PID_USIC2);

			TWI->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

			TWI->SCTR = __SCTR;

			TWI->FDR = __FDR;
			TWI->BRG = __BRG;
		    
			TWI->TCSR = __TCSR;

			TWI->PSCR = ~0;

			TWI->CCR = 0;

			TWI->DX0CR = __DX0CR;
			TWI->DX1CR = __DX1CR;

			TWI->CCR = __CCR;

			TWI->PCR_IICMode = __PCR;

			dsc->ready = true;
			dsc->readedLen = dsc->rlen - rdCount;

			DSCTWI *ndsc = dsc->next;

			if (ndsc != 0)
			{
				dsc->next = 0;
				dsc = ndsc;

				dsc->ready = false;
				dsc->ack = false;
				dsc->readedLen = 0;

				wrPtr = (byte*)dsc->wdata;	
				rdPtr = (byte*)dsc->rdata;	
				wrPtr2 = (byte*)dsc->wdata2;	
				wrCount = dsc->wlen;
				wrCount2 = dsc->wlen2;
				rdCount = dsc->rlen;
				adr = dsc->adr;

				if (wrPtr2 == 0) wrCount2 = 0;

				TWI->PSCR = ~0;//RIF|AIF|TBIF|ACK|NACK|PCR;

				TWI->CCR |= RIEN|AIEN;
				TWI->PCR_IICMode |= PCRIEN|NACKIEN|ARLIEN|SRRIEN|ERRIEN|ACKIEN;

				TWI->TBUF[0] = TDF_MASTER_START | (dsc->adr << 1) | ((wrCount == 0) ? 1 : 0);
			}
			else
			{
				TWI->CCR = __CCR;
				TWI->PCR_IICMode = __PCR;

				lastDsc = dsc = 0;
			};
		};
	}
	else
	{
		intCount = 0;

		tm.Reset();
	};
	
	__enable_irq();

//	HW::P5->BCLR(7);

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

void Destroy_TWI()
{
	HANDLE h;

	h = CreateFile("FRAM_I2C_STORE.BIN", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	};

	dword bytes;

	if (!WriteFile(h, fram_I2c_Mem, sizeof(fram_I2c_Mem), &bytes, 0))
	{
		dword le = GetLastError();
	};

	CloseHandle(h);
}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
