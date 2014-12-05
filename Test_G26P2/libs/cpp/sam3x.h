#ifndef SAM3X_H__24_12_2012__15_40
#define SAM3X_H__24_12_2012__15_40

#ifndef CORETYPE_SAM3X
#error  Must #include "core.h"
#endif 

#include "types.h"
#include "cm3.h"

#define MCK 84000000


#ifndef WIN32
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(p))
#else
extern byte core_sys_array[0x100000]; 
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(core_sys_array-0x40000000+p))
#endif


#define MKPID(n,i) n##_M=(1UL<<(i&31)), n##_I=i

namespace T_HW
{
	typedef volatile unsigned int AT91_REG;// Hardware register definition
	typedef volatile void * AT91_PTR;// Hardware register definition

	typedef void(*AT91_IHP)() __irq;	// Interrupt handler pointer

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SMC
	{
		AT91_REG	NFC_CFG;	
		AT91_REG	NFC_CTRL;	
		AT91_REG	NFC_SR;	
		AT91_REG	NFC_IER;	
		AT91_REG	NFC_IDR;	
		AT91_REG	NFC_IMR;	
		AT91_REG	NFC_ADDR;	
		AT91_REG	NFC_BANK;	
		AT91_REG	ECC_CTRL;	
		AT91_REG	ECC_MD;	
		AT91_REG	ECC_SR1;	
		AT91_REG	ECC_PR0;	
		AT91_REG	ECC_PR1;	
		AT91_REG	ECC_SR2;	
		AT91_REG	ECC_PR2;	
		AT91_REG	ECC_PR3;	
		AT91_REG	ECC_PR4;	
		AT91_REG	ECC_PR5;	
		AT91_REG	ECC_PR6;	
		AT91_REG	ECC_PR7;	
		AT91_REG	ECC_PR8;	
		AT91_REG	ECC_PR9;	
		AT91_REG	ECC_PR10;	
		AT91_REG	ECC_PR11;	
		AT91_REG	ECC_PR12;	
		AT91_REG	ECC_PR13;	
		AT91_REG	ECC_PR14;	
		AT91_REG	ECC_PR15;	

		struct S_CSR { AT91_REG	SETUP, PULSE, CYCLE, TIMINGS, MODE;	} CSR[8];

		AT91_REG	OCMS;	
		AT91_REG	KEY1;	
		AT91_REG	KEY2;	
		AT91_REG	WPCR;	
		AT91_REG	WPSR;	
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PMC
	{
		AT91_REG	SCER;
		AT91_REG	SCDR;
		AT91_REG	SCSR;
		AT91_REG	z__reserved1;
		AT91_REG	PCER0;
		AT91_REG	PCDR0;
		AT91_REG	PCSR0;
		AT91_REG	UTMICR;
		AT91_REG	MOR;
		AT91_REG	MCFR;
		AT91_REG	PLLAR;
		AT91_REG	z__reserved3;
		AT91_REG	MCKR;
		AT91_REG	z__reserved4;
		AT91_REG	USBCR;
		AT91_REG	z__reserved5;
		AT91_REG	PCK[3];
		AT91_REG	z__reserved6[5];
		AT91_REG	IER;
		AT91_REG	IDR;
		AT91_REG	SR;
		AT91_REG	IMR;
		AT91_REG	FSMR;
		AT91_REG	FSPR;
		AT91_REG	FOCR;
		AT91_REG	z__reserved7[26];
		AT91_REG	WPMR;
		AT91_REG	WPSR;
		AT91_REG	z__reserved8[5];
		AT91_REG	PCER1;
		AT91_REG	PCDR1;
		AT91_REG	PCSR1;
		AT91_REG	PCR;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//struct S_EBI
	//{
	//	AT91_REG	CSA;
	//	AT91_REG	z__reserved1[3];
	//	S_SMC		SMC;
	//};

	//S_EBI * const EBI = MK_PTR(S_EBI,0xFFFFFF80);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PIO
	{
		AT91_REG PER;
		AT91_REG PDR;
		AT91_REG PSR;
		AT91_REG z__reserved1;
		AT91_REG OER;
		AT91_REG ODR;
		AT91_REG OSR;
		AT91_REG z__reserved2;
		AT91_REG IFER;
		AT91_REG IFDR;
		AT91_REG IFSR;
		AT91_REG z__reserved3;
		AT91_REG SODR;
		AT91_REG CODR;
		AT91_REG ODSR;
		AT91_REG PDSR;
		AT91_REG IER;
		AT91_REG IDR;
		AT91_REG IMR;
		AT91_REG ISR;
		AT91_REG MDER;
		AT91_REG MDDR;
		AT91_REG MDSR;
		AT91_REG z__reserved4;
		AT91_REG PUDR;
		AT91_REG PUER;
		AT91_REG PUSR;
		AT91_REG z__reserved5;
		AT91_REG ABSR;
		AT91_REG z__reserved6[3];
		AT91_REG SCIFSR;
		AT91_REG DIFSR;
		AT91_REG IFDGSR;
		AT91_REG SCDR;
		AT91_REG z__reserved7[4];
		AT91_REG OWER;
		AT91_REG OWDR;
		AT91_REG OWSR;
		AT91_REG z__reserved8;
		AT91_REG AIMER;
		AT91_REG AIMDR;
		AT91_REG AIMMR;
		AT91_REG z__reserved9;
		AT91_REG ESR;
		AT91_REG LSR;
		AT91_REG ELSR;
		AT91_REG z__reserved10;
		AT91_REG FELLSR;
		AT91_REG REHLSR;
		AT91_REG FRLHSTR;
		AT91_REG z__reserved11;
		AT91_REG LOCKSR;
		AT91_REG WPMR;
		AT91_REG WPSR;
	};
	
	typedef S_PIO S_PIOA, S_PIOB, S_PIOC, S_PIOD, S_PIOE;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_MC
	{
		AT91_REG RCR;	
		AT91_REG ASR;	
		AT91_REG AASR;	
		AT91_REG _RESERVED;	
		AT91_REG PUIA0;	
		AT91_REG PUIA1;	
		AT91_REG PUIA2;	
		AT91_REG PUIA3;	
		AT91_REG PUIA4;	
		AT91_REG PUIA5;	
		AT91_REG PUIA6;	
		AT91_REG PUIA7;	
		AT91_REG PUIA8;	
		AT91_REG PUIA9;	
		AT91_REG PUIA10;	
		AT91_REG PUIA11;	
		AT91_REG PUIA12;	
		AT91_REG PUIA13;	
		AT91_REG PUIA14;	
		AT91_REG PUIA15;	
		AT91_REG PUP;	
		AT91_REG PUER;	
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_TC
	{
		struct 
		{
			AT91_REG CCR;
			AT91_REG CMR;
			AT91_REG SMMR;
			AT91_REG z__reserved1;
			AT91_REG CV;
			AT91_REG RA;
			AT91_REG RB;
			AT91_REG RC;
			AT91_REG SR;
			AT91_REG IER;
			AT91_REG IDR;
			AT91_REG IMR;
			AT91_REG z__reserved2[4];
		} C0, C1, C2;

		AT91_REG BCR;
		AT91_REG BMR;

		AT91_REG QIER;
		AT91_REG QIDR;
		AT91_REG QIMR;
		AT91_REG QISR;
		AT91_REG FMR;
		AT91_REG WPMR;
	};

	typedef S_TC S_TC0, S_TC1, S_TC2;


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PDC
	{
		AT91_PTR RPR;	// Receive Pointer Register	
		AT91_REG RCR;	// Receive Counter Register
		AT91_PTR TPR;	// Transmit Pointer Register
		AT91_REG TCR;	// Transmit Counter Register
		AT91_PTR RNPR;	// Receive Next Pointer Register	
		AT91_REG RNCR;	// Receive Next Pointer Register
		AT91_PTR TNPR;	// Transmit Next Pointer Register
		AT91_REG TNCR;	// Transmit Next Counter Register
		AT91_REG PTCR;	// PDC Transfer Control Register	
		AT91_REG PTSR;	// PDC Transfer Status Register
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_UDP	// 0xFFFB0000
	{
		AT91_REG FRM_NUM;
		AT91_REG GLB_STAT;
		AT91_REG FADDR;
		AT91_REG z__reserved1;
		AT91_REG IER;
		AT91_REG IDR;
		AT91_REG IMR;
		AT91_REG ISR;
		AT91_REG ICR;
		AT91_REG z__reserved2;
		AT91_REG RST_EP;
		AT91_REG z__reserved3;
		AT91_REG CSR[8];
		AT91_REG FDR[8];
		AT91_REG z__reserved4;
		AT91_REG TXVC;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_USART
	{
		AT91_REG CR;				// Control Register 	
		AT91_REG MR;				// Mode Register
		AT91_REG IER;				// Interrupt Enable Register
		AT91_REG IDR;				// Interrupt Disable Register
		AT91_REG IMR;				// Interrupt Mask Register
		AT91_REG CSR;				// Channel Status Register
		AT91_REG RHR;				// Receiver Holding Register
		AT91_REG THR;				// Transmitter Holding Register	
		AT91_REG BRGR;				// Baud Rate Generator Register
		AT91_REG RTOR;				// Receiver Time-out Register
		AT91_REG TTGR;				// Transmitter Timeguard Register
		AT91_REG z__reserved1[5];	
		AT91_REG FIDI;				// FI DI Ratio Register
		AT91_REG NER;				// Number of Errors Register
		AT91_REG z__reserved2;	
		AT91_REG IF;				// IrDA Filter Register
		AT91_REG MAN;				// Manchester Encode Decode Register
		AT91_REG z__reserved3[43];	

		S_PDC PDC;
	};

	typedef S_USART S_USART0, S_USART1, S_USART2, S_USART3;


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_UART
	{
		AT91_REG CR;	
		AT91_REG MR;	
		AT91_REG IER;	
		AT91_REG IDR;	
		AT91_REG IMR;	
		AT91_REG SR;	
		AT91_REG RHR;	
		AT91_REG THR;	
		AT91_REG BRGR;	
		AT91_REG z__reserved[55];	

		S_PDC PDC;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PIT
	{
		AT91_REG		MR;
		const AT91_REG	SR;
		const AT91_REG	PIVR;
		const AT91_REG	PIIR;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_RTT
	{
		AT91_REG		MR;
		AT91_REG		AR;
		const AT91_REG	VR;
		const AT91_REG	SR;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SPI
	{
		AT91_REG		CR;
		AT91_REG		MR;
		AT91_REG		RDR;
		AT91_REG		TDR;
		AT91_REG		SR;
		AT91_REG		IER;
		AT91_REG		IDR;
		AT91_REG		IMR;
		AT91_REG		z__rsrvd1[4];
		AT91_REG		CSR[4];
		AT91_REG		z__rsrvd2[38];
		AT91_REG		WPMR;
		AT91_REG		WPSR;
	};
	 
	typedef S_SPI S_SPI0, S_SPI1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PWM
	{
		AT91_REG		CLK;
		AT91_REG		ENA;
		AT91_REG		DIS;
		AT91_REG		SR;
		AT91_REG		IER1;
		AT91_REG		IDR1;
		AT91_REG		IMR1;
		AT91_REG		ISR1;
		AT91_REG		SCM;

		AT91_REG		zrsrv1;

		AT91_REG		SCUC;
		AT91_REG		SCUP;
		AT91_REG		SCUPUPD;
		AT91_REG		IER2;
		AT91_REG		IDR2;
		AT91_REG		IMR2;
		AT91_REG		ISR2;
		AT91_REG		OOV;
		AT91_REG		OS;
		AT91_REG		OSS;
		AT91_REG		OSC;
		AT91_REG		OSSUPD;
		AT91_REG		OSCUPD;
		AT91_REG		FMR;
		AT91_REG		FSR;
		AT91_REG		FCR;
		AT91_REG		FPV;
		AT91_REG		FPE1;
		AT91_REG		FPE2;

		AT91_REG		zrsrv2[2];

		AT91_REG		ELMR0;
		AT91_REG		ELMR1;

		AT91_REG		zrsrv3[11];

		AT91_REG		SMMR;

		AT91_REG		zrsrv4[12];

		AT91_REG		WPCR;
		AT91_REG		WPSR;

		AT91_REG		zrsrv5[17];

		struct { AT91_REG VR, VUR, MR, MUR; } CMP[8];

		AT91_REG		zrsrv6[20];

		struct { AT91_REG	MR, DTY, DTYUPD, PRD, PRDUPD, CNT, DT, DTUPD; } CH[4];
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_WDT
	{
		AT91_REG		CR;
		AT91_REG		MR;
		AT91_REG		SR;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_RSTC
	{
		AT91_REG		CR;
		AT91_REG		SR;
		AT91_REG		MR;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_ADC
	{
		AT91_REG		CR;
		AT91_REG		MR;

		AT91_REG		SEQR1;
		AT91_REG		SEQR2;

		AT91_REG		CHER;
		AT91_REG		CHDR;
		AT91_REG		CHSR;

		AT91_REG		zrsrv;

		AT91_REG		LCDR;

		AT91_REG		IER;
		AT91_REG		IDR;
		AT91_REG		IMR;
		AT91_REG		ISR;

		AT91_REG		zrsrv2[2];

		AT91_REG		OVER;
		AT91_REG		EMR;
		AT91_REG		CWR;
		AT91_REG		CGR;
		AT91_REG		COR;
		AT91_REG		CDR[16];

		AT91_REG		zrsrv3;

		AT91_REG		ACR;

		AT91_REG		zrsrv4[19];

		AT91_REG		WPMR;
		AT91_REG		WPSR;

		AT91_REG		zrsrv5[5];

		S_PDC			PDC;
	};


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_RTC
	{
		AT91_REG		CR;
		AT91_REG		MR;
		AT91_REG		TIMR;
		AT91_REG		CALR;
		AT91_REG		TIMALR;
		AT91_REG		CALALR;
		AT91_REG		SR;
		AT91_REG		SCCR;
		AT91_REG		IER;
		AT91_REG		IDR;
		AT91_REG		IMR;
		AT91_REG		VER;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_MATRIX
	{
		AT91_REG		MCFG[16];
		AT91_REG		SCFG[16];
		struct { AT91_REG A; AT91_REG B; } PRS[16];
		AT91_REG		MRCR;
		AT91_REG		zreserve[4];
		AT91_REG		SYSIO;
		AT91_REG		zreserve1[52];
		AT91_REG		WPMR;
		AT91_REG		WPSR;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_EFC
	{
		AT91_REG		FMR;
		AT91_REG		FCR;
		AT91_REG		FSR;
		AT91_REG		FRR;
	};

	typedef S_EFC S_EFC0, S_EFC1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SUPC
	{
		AT91_REG		CR;
		AT91_REG		SMMR;
		AT91_REG		MR;
		AT91_REG		WUMR;
		AT91_REG		WUIR;
		AT91_REG		SR;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_DMAC
	{
		AT91_REG		GCFG;
		AT91_REG		EN;
		AT91_REG		SREQ;
		AT91_REG		CREQ;
		AT91_REG		LAST;
		AT91_REG		z__rsrvd1;
		AT91_REG		EBCIER;
		AT91_REG		EBCIDR;
		AT91_REG		EBCIMR;
		AT91_REG		EBCISR;
		AT91_REG		CHER;
		AT91_REG		CHDR;
		AT91_REG		CHSR;
		AT91_REG		z__rsrvd2[2];

		struct { AT91_PTR SADDR, DADDR, DSCR; AT91_REG CTRLA, CTRLB, CFG, z__rsrvd[4]; } CH[6];

		AT91_REG		z__rsrvd3[46];

		AT91_REG		WPMR;
		AT91_REG		WPSR;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
};


namespace HW
{
	namespace PID
	{
		enum {	MKPID(SUPC, 0),		MKPID(RSTC, 1),		MKPID(RTC, 2),		MKPID(RTT, 3),		MKPID(WDT, 4),		MKPID(PMC, 5),		MKPID(EEFC0, 6),	MKPID(EEFC1, 7),
				MKPID(UART, 8),		MKPID(SMC, 9),		MKPID(SDRAM, 10),	MKPID(PIOA, 11),	MKPID(PIOB, 12),	MKPID(PIOC, 13),	MKPID(PIOD, 14),	MKPID(PIOE, 15),
				MKPID(PIOF, 16),	MKPID(USART0, 17),	MKPID(USART1, 18),	MKPID(USART2, 19),	MKPID(USART3, 20),	MKPID(HSMCI, 21),	MKPID(TWI0, 22),	MKPID(TWI1, 23), 
				MKPID(SPI0, 24),	MKPID(SPI1, 25), 	MKPID(SSC, 26),		MKPID(TC0, 27),		MKPID(TC1, 28),		MKPID(TC2, 29),		MKPID(TC3, 30),		MKPID(TC4, 31), 
				MKPID(TC5, 32),		MKPID(TC6, 33),		MKPID(TC7, 34),		MKPID(TC8, 35), 	MKPID(PWM, 36),		MKPID(ADC, 37),		MKPID(DACC, 38),	MKPID(DMAC, 39), 
				MKPID(UOTGHS, 40),	MKPID(TRNG, 41),	MKPID(EMAC, 42),	MKPID(CAN0, 43),	MKPID(CAN1, 44) };
	};

	MK_PTR(SMC, 	0x400E0000);

	MK_PTR(MATRIX,	0x400E0400);
	MK_PTR(PMC,		0x400E0600);
	MK_PTR(UART,	0x400E0800);

	MK_PTR(EFC0,	0x400E0A00);
	MK_PTR(EFC1,	0x400E0C00);
	MK_PTR(PIOA,	0x400E0E00);
	MK_PTR(PIOB,	0x400E1000);
	MK_PTR(PIOC,	0x400E1200);
	MK_PTR(PIOD,	0x400E1400);

	MK_PTR(RSTC,	0X400E1A00);
	MK_PTR(SUPC,	0X400E1A10);
	MK_PTR(RTT,		0X400E1A30);
	MK_PTR(WDT,		0X400E1A50);
	MK_PTR(RTC,		0X400E1A60);

	MK_PTR(SPI,		0X40008000);
	MK_PTR(SPI0,	0X40008000);
	MK_PTR(SPI1,	0X4000C000);

	MK_PTR(TC0,		0X40080000);
	MK_PTR(TC1,		0X40084000);
	MK_PTR(TC2,		0X40088000);

	MK_PTR(PWM,		0X40094000);
	MK_PTR(USART0,	0x40098000);
	MK_PTR(USART1,	0x4009C000);
	MK_PTR(USART2,	0x400A0000);
	MK_PTR(USART3,	0x400A4000);

	MK_PTR(ADC,		0X400C0000);

	MK_PTR(DMAC,	0X400C4000);


//	MK_PTR(UDP,		0x400A4000);



	inline void ResetWDT() { WDT->CR = 0xA5000001; }

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


}; // namespace HW

extern T_HW::AT91_IHP VectorTableInt[16];
extern T_HW::AT91_IHP VectorTableExt[45];

#undef MK_PTR
#undef MKPID

#endif // SAM3X_H__24_12_2012__15_40
