#ifndef SAM4SA_H__27_08_2014__11_07
#define SAM4SA_H__27_08_2014__11_07

#ifndef CORETYPE_SAM4E
#error  Must #include "core.h"
#endif 

#pragma anon_unions

#include "types.h"
#include "cm4.h"

#define MCK 100000000


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
		struct S_CSR { AT91_REG	SETUP, PULSE, CYCLE, MODE;	} CSR[4];

		AT91_REG	z__reserved1[16];	

		AT91_REG	OCMS;	
		AT91_REG	KEY1;	
		AT91_REG	KEY2;	

		AT91_REG	z__reserved2[22];	

		AT91_REG	WPMR;	
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
		AT91_REG	z__reserved2;
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
		AT91_REG	z__reserved9;
		AT91_REG	OCR;
		AT91_REG	z__reserved10[7];
		AT91_REG	PMMR;
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
		AT91_REG PER;						//	PIO_PER 								
		AT91_REG PDR;						//	PIO_PDR 
		AT91_REG PSR;						//	PIO_PSR 
		AT91_REG z__reserved1;				//	�
		AT91_REG OER;						//	PIO_OER 
		AT91_REG ODR;						//	PIO_ODR 
		AT91_REG OSR;						//	PIO_OSR 
		AT91_REG z__reserved2;				//	�
		AT91_REG IFER;						//	PIO_IFER 
		AT91_REG IFDR;						//	PIO_IFDR 
		AT91_REG IFSR;						//	PIO_IFSR 
		AT91_REG z__reserved3;				//	�
		AT91_REG SODR;						//	PIO_SODR 
		AT91_REG CODR;						//	PIO_CODR 
		AT91_REG ODSR;						//	PIO_ODSR
		AT91_REG PDSR;						//	PIO_PDSR 
		AT91_REG IER;						//	PIO_IER
		AT91_REG IDR;						//	PIO_IDR
		AT91_REG IMR;						//	PIO_IMR 
		AT91_REG ISR;						//	PIO_ISR
		AT91_REG MDER;						//	PIO_MDER 
		AT91_REG MDDR;						//	PIO_MDDR 
		AT91_REG MDSR;						//	PIO_MDSR 
		AT91_REG z__reserved4;				//	�
		AT91_REG PUDR;						//	PIO_PUDR 
		AT91_REG PUER;						//	PIO_PUER
		AT91_REG PUSR;						//	PIO_PUSR
		AT91_REG z__reserved5;				//	�
		AT91_REG ABCDSR1;					//	PIO_ABCDSR1
		AT91_REG ABCDSR2;					//	PIO_ABCDSR2
		AT91_REG z__reserved6[2];			//	�									
		AT91_REG SCIFSR;					//	PIO_IFSCDR 
		AT91_REG DIFSR;						//	PIO_IFSCER 
		AT91_REG IFDGSR;					//	PIO_IFSCSR 
		AT91_REG SCDR;						//	PIO_SCDR 
		AT91_REG PPDDR;						//	PIO_PPDDR 
		AT91_REG PPDER;						//	PIO_PPDER 
		AT91_REG PPDSR;						//	PIO_PPDSR 
		AT91_REG z__reserved7;				//	�
		AT91_REG OWER;						//	PIO_OWER 
		AT91_REG OWDR;						//	PIO_OWDR 
		AT91_REG OWSR;						//	PIO_OWSR 
		AT91_REG z__reserved8;				//	�
		AT91_REG AIMER;						//	PIO_AIMER 
		AT91_REG AIMDR;						//	PIO_AIMDR 
		AT91_REG AIMMR;						//	PIO_AIMMR 
		AT91_REG z__reserved9;				//	�
		AT91_REG ESR;						//	PIO_ESR 
		AT91_REG LSR;						//	PIO_LSR 
		AT91_REG ELSR;						//	PIO_ELSR 
		AT91_REG z__reserved10;				//	�
		AT91_REG FELLSR;					//	PIO_FELLSR 
		AT91_REG REHLSR;					//	PIO_REHLSR 
		AT91_REG FRLHSTR;					//	PIO_FRLHSR 
		AT91_REG z__reserved11;				//	�
		AT91_REG LOCKSR;					//	PIO_LOCKSR 
		AT91_REG WPMR;						//	PIO_WPMR 
		AT91_REG WPSR;						//	PIO_WPSR 
		AT91_REG z__reserved12[5];			//	�
		AT91_REG SCHMITT;					//	PIO_SCHMITT
											
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

	typedef S_USART S_USART0, S_USART1;


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

	typedef S_UART S_UART0, S_UART1;

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
		AT91_REG		z__rsrvd2[41];
		AT91_REG		WPMR;
		AT91_REG		WPSR;

		AT91_REG		z__rsrvd3[5];	

		S_PDC PDC;

	};
	 
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
		AT91_REG		FPE;

		AT91_REG		zrsrv2[3];

		AT91_REG		ELMR0;
		AT91_REG		ELMR1;

		AT91_REG		zrsrv3[11];

		AT91_REG		SMMR;

		AT91_REG		zrsrv4[12];

		AT91_REG		WPCR;
		AT91_REG		WPSR;

		AT91_REG		zrsrv5[5];

		S_PDC PDC;

		AT91_REG		zrsrv6[2];

		struct { AT91_REG VR, VUR, MR, MUR; } CMP[13];

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

	struct S_AFE
	{
		AT91_REG		CR;
		AT91_REG		MR;

		AT91_REG		EMR;

		AT91_REG		SEQR1;
		AT91_REG		SEQR2;

		AT91_REG		CHER;
		AT91_REG		CHDR;
		AT91_REG		CHSR;

		AT91_REG		LCDR;

		AT91_REG		IER;
		AT91_REG		IDR;
		AT91_REG		IMR;
		AT91_REG		ISR;

		AT91_REG		zrsrv2[(0x4C-0x34)/4];

		AT91_REG		OVER;
		AT91_REG		CWR;
		AT91_REG		CGR;

		AT91_REG		zrsrv;

		AT91_REG		CDOR;
		AT91_REG		DIFFR;
		AT91_REG		CSELR;
		AT91_REG		CDR;
		AT91_REG		COCR;

		AT91_REG		TEMPMR;
		AT91_REG		TEMPCWR;

		AT91_REG		zrsrv3[7];

		AT91_REG		ACR;

		AT91_REG		zrsrv4[(0xE4-0x98)/4];

		AT91_REG		WPMR;
		AT91_REG		WPSR;

		AT91_REG		zrsrv5[(0x100-0xEC)/4];

		S_PDC			PDC;
	};

	typedef S_AFE S_AFE0, S_AFE1;

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
		struct { AT91_REG A; AT91_REG zreserve; } PRAS[16];
		AT91_REG		MRCR;
		AT91_REG		zreserve[4];
		AT91_REG		SYSIO;
		AT91_REG		zreserve1[3];
		AT91_REG		SMCNFCS;
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

		struct { AT91_PTR SADDR, DADDR, DSCR; AT91_REG CTRLA, CTRLB, CFG, z__rsrvd[4]; } CH[4];

		AT91_REG		z__rsrvd3[66];

		AT91_REG		WPMR;
		AT91_REG		WPSR;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_GMAC
	{
		union ADR
		{
			u64		U;
			u16		W[4];
			
			struct 
			{ 
				AT91_REG	B;	//	Bottom Register 
				AT91_REG	T;	//	Top Register
			};
		};

		AT91_REG		NCR;			//	Network Control Register		
		AT91_REG		NCFGR;			//	Network Configuration Register
		AT91_REG		NSR;			//	Network Status Register

		AT91_REG		UR;				//	User Register
		AT91_REG		DCFGR;			//	DMA Configuration Register

		AT91_REG		TSR;   			//	Transmit Status Register
		AT91_REG		RBQP;  			//	Receive Buffer Queue Pointer Register 
		AT91_REG		TBQP;  			//	Transmit Buffer Queue Pointer Register 
		AT91_REG		RSR;   			//	Receive Status Register
		AT91_REG		ISR;   			//	Interrupt Status Register
		AT91_REG		IER;   			//	Interrupt Enable Register
		AT91_REG		IDR;   			//	Interrupt Disable Register
		AT91_REG		IMR;   			//	Interrupt Mask Register
		AT91_REG		MAN;   			//	Phy Maintenance Register

		AT91_REG		RPQ;   			//	Received Pause Quantum Register
		AT91_REG		TPQ;   			//	Transmit Pause Quantum Register
		
		AT91_REG		z__rsrvd1[16];	//	Reserved
		
		AT91_REG		HRB;			//	Hash Register Bottom [31:0] Register
		AT91_REG		HRT;			//	Hash Register Top [63:32] Register

		ADR				SA[4];			//  Specific Address Register

		AT91_REG		TIDM[4];		//  Type ID Match Register

		AT91_REG		z__rsrvd2;		//	Reserved

		AT91_REG		IPGS;			//	IPG Stretch Register
		AT91_REG		SVLAN;			//	Stacked VLAN Register
		AT91_REG		TPFCP;			//	Transmit PFC Pause Register

		ADR				SAM;			//  Specific Address 1 Mask Register

		AT91_REG		z__rsrvd3[12];	//	Reserved

		AT91_REG		OTLO;			//	Octets Transmitted [31:0] Register GMAC_OTLO						
		AT91_REG		OTHI;			//	Octets Transmitted [47:32] Register GMAC_OTHI						
		AT91_REG		FT;				//	Frames Transmitted Register GMAC_FT									
		AT91_REG		BCFT;			//	Broadcast Frames Transmitted Register GMAC_BCFT						
		AT91_REG		MFT;			//	Multicast Frames Transmitted Register GMAC_MFT						
		AT91_REG		PFT;			//	Pause Frames Transmitted Register GMAC_PFT							
		AT91_REG		BFT64;			//	64 Byte Frames Transmitted Register GMAC_BFT64						
		AT91_REG		TBFT127;		//	65 to 127 Byte Frames Transmitted Register GMAC_TBFT127				
		AT91_REG		TBFT255;		//	128 to 255 Byte Frames Transmitted Register GMAC_TBFT255			
		AT91_REG		TBFT511;		//	256 to 511 Byte Frames Transmitted Register GMAC_TBFT511			
		AT91_REG		TBFT1023;		//	512 to 1023 Byte Frames Transmitted Register GMAC_TBFT1023			
		AT91_REG		TBFT1518;		//	1024 to 1518 Byte Frames Transmitted Register GMAC_TBFT1518			
		AT91_REG		GTBFT1518;		//	Greater Than 1518 Byte Frames Transmitted Register GMAC_GTBFT1518	
													
		AT91_REG		TUR;			//	Transmit Underruns Register GMAC_TUR								
		AT91_REG		SCF;			//	Single Collision Frames Register GMAC_SCF							
		AT91_REG		MCF;			//	Multiple Collision Frames Register GMAC_MCF 						
		AT91_REG		EC;				//	Excessive Collisions Register GMAC_EC								
		AT91_REG		LC;				//	Late Collisions Register GMAC_LC									
		AT91_REG		DTF;			//	Deferred Transmission Frames Register GMAC_DTF						
		AT91_REG		CSE;			//	Carrier Sense Errors Register  GMAC_CSE								
		AT91_REG		ORLO;			//	Octets Received [31:0] Received GMAC_ORLO							
		AT91_REG		ORHI;			//	Octets Received [47:32] Received  GMAC_ORHI							
		AT91_REG		FR;				//	Frames Received Register GMAC_FR									
		AT91_REG		BCFR;			//	Broadcast Frames Received Register GMAC_BCFR						
		AT91_REG		MFR;			//	Multicast Frames Received Register GMAC_MFR							
		AT91_REG		PFR;			//	Pause Frames Received Register GMAC_PFR								
		AT91_REG		BFR64;			//	64 Byte Frames Received Register GMAC_BFR64							
		AT91_REG		TBFR127;		//	65 to 127 Byte Frames Received Register GMAC_TBFR127				
		AT91_REG		TBFR255;		//	128 to 255 Byte Frames Received Register GMAC_TBFR255				
		AT91_REG		TBFR511;		//	256 to 511Byte Frames Received Register GMAC_TBFR511				
		AT91_REG		TBFR1023;		//	512 to 1023 Byte Frames Received Register GMAC_TBFR1023				
		AT91_REG		TBFR1518;		//	1024 to 1518 Byte Frames Received Register GMAC_TBFR1518			
		AT91_REG		TMXBFR;			//	1519 to Maximum Byte Frames Received Register GMAC_TMXBFR			
		AT91_REG		UFR;			//	Undersize Frames Received Register GMAC_UFR							
		AT91_REG		OFR;			//	Oversize Frames Received Register GMAC_OFR							
		AT91_REG		JR;				//	Jabbers Received Register GMAC_JR									
		AT91_REG		FCSE;			//	Frame Check Sequence Errors Register GMAC_FCSE						
		AT91_REG		LFFE;			//	Length Field Frame Errors Register GMAC_LFFE						
		AT91_REG		RSE;			//	Receive Symbol Errors Register GMAC_RSE								
		AT91_REG		AE;				//	Alignment Errors Register GMAC_AE									
		AT91_REG		RRE;			//	Receive Resource Errors Register GMAC_RRE							
		AT91_REG		ROE;			//	Receive Overrun Register GMAC_ROE									
		AT91_REG		IHCE;			//	IP Header Checksum Errors Register GMAC_IHCE						
		AT91_REG		TCE;			//	TCP Checksum Errors Register GMAC_TCE 								
		AT91_REG		UCE;			//	UDP Checksum Errors Register GMAC_UCE 								
													
		AT91_REG		z__rsrvd4[5];	//

		AT91_REG		TSSSL;			//	1588 Timer Sync Strobe Seconds [31:0] Register GMAC_TSSSL			
		AT91_REG		TSSN;			//	1588 Timer Sync Strobe Nanoseconds Register GMAC_TSSN				
		AT91_REG		TSL;			//	1588 Timer Seconds [31:0] Register GMAC_TSL							
		AT91_REG		TN;				//	1588 Timer Nanoseconds Register GMAC_TN								
		AT91_REG		TA;				//	1588 Timer Adjust Register GMAC_TA									
		AT91_REG		TI;				//	1588 Timer Increment Register GMAC_TI								
		AT91_REG		EFTS;			//	PTP Event Frame Transmitted Seconds GMAC_EFTS						
		AT91_REG		EFTN;			//	PTP Event Frame Transmitted Nanoseconds GMAC_EFTN					
		AT91_REG		EFRS;			//	PTP Event Frame Received Seconds GMAC_EFRS							
		AT91_REG		EFRN;			//	PTP Event Frame Received Nanoseconds GMAC_EFRN						
		AT91_REG		PEFTS;			//	PTP Peer Event Frame Transmitted Seconds GMAC_PEFTS			
		AT91_REG		PEFTN;			//	PTP Peer Event Frame Transmitted Nanoseconds GMAC_PEFTN		
		AT91_REG		PEFRS;			//	PTP Peer Event Frame Received Seconds GMAC_PEFRS			
		AT91_REG		PEFRN;			//	PTP Peer Event Frame Received Nanoseconds
	};

	struct S_CMCC
	{
		AT91_REG		TYPE;
		AT91_REG		CFG;
		AT91_REG		CTRL;
		AT91_REG		SR;
		AT91_REG		z__rsrvd[4];
		AT91_REG		MAINT0;
		AT91_REG		MAINT1;
		AT91_REG		MCFG;
		AT91_REG		MEN;
		AT91_REG		MCTRL;
		AT91_REG		MSR;
};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
};


namespace HW
{
	namespace PID
	{
		enum {	MKPID(SUPC, 0),		MKPID(RSTC, 1),		MKPID(RTC, 2),		MKPID(RTT, 3),		MKPID(WDT, 4),		MKPID(PMC, 5),		MKPID(EFC, 6),		MKPID(UART0, 7),
				MKPID(SMC, 8),		MKPID(PIOA, 9),		MKPID(PIOB, 10),	MKPID(PIOC, 11),	MKPID(PIOD, 12),	MKPID(PIOE, 13),	MKPID(USART0, 14),	MKPID(USART1, 15),
				MKPID(HSMCI, 16),	MKPID(TWI0, 17),	MKPID(TWI1, 18),	MKPID(SPI, 19),		MKPID(DMAC, 20),	MKPID(TC0, 21),		MKPID(TC1, 22),		MKPID(TC2, 23),		MKPID(TC3, 24),		MKPID(TC4, 25), 
				MKPID(TC5, 26),		MKPID(TC6, 27),		MKPID(TC7, 28),		MKPID(TC8, 29),		MKPID(AFEC0, 30),	MKPID(AFEC1, 31),	MKPID(DACC, 32),	MKPID(ACC, 33),		MKPID(ARM, 34),		MKPID(UDP, 35),		
				MKPID(PWM, 36),		MKPID(CAN0, 37),	MKPID(CAN1, 38),	MKPID(AES, 39),		MKPID(GMAC, 44),	MKPID(UART1, 45) };
	};

	MK_PTR(SMC, 	0x40060000);

	MK_PTR(MATRIX,	0x400E0200);
	MK_PTR(PMC,		0x400E0400);
	MK_PTR(UART0,	0x400E0600);
	MK_PTR(UART1,	0x40060600);

	MK_PTR(EFC,		0x400E0A00);
	MK_PTR(PIOA,	0x400E0E00);
	MK_PTR(PIOB,	0x400E1000);
	MK_PTR(PIOC,	0x400E1200);
	MK_PTR(PIOD,	0x400E1400);
	MK_PTR(PIOE,	0x400E1600);

	MK_PTR(RSTC,	0X400E1800);
	MK_PTR(SUPC,	0X400E1810);
	MK_PTR(RTT,		0X400E1830);
	MK_PTR(WDT,		0X400E1850);
	MK_PTR(RTC,		0X400E1860);

	MK_PTR(SPI,		0X40088000);

	MK_PTR(TC0,		0X40090000);
	//MK_PTR(TC1,		0X40094000);
	//MK_PTR(TC2,		0X40098000);

//	MK_PTR(PWM,		0X40094000);

	MK_PTR(USART0,	0x400A0000);
	MK_PTR(USART1,	0x400A4000);

	MK_PTR(AFE0,	0x400B0000);
	MK_PTR(AFE1,	0x400B4000);

	MK_PTR(DMAC,	0X400C0000);

	MK_PTR(GMAC,	0X40034000);
	MK_PTR(CMCC,	0X400C4000);

//	MK_PTR(UDP,		0x400A4000);

#ifndef EMAC
#define EMAC GMAC
#endif



	inline void ResetWDT() { WDT->CR = 0xA5000001; }

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


}; // namespace HW

extern T_HW::AT91_IHP VectorTableInt[16];
extern T_HW::AT91_IHP VectorTableExt[47];

#undef MK_PTR
#undef MKPID

#define CLKEN	(0x1u << 0)	/**< \brief (TC_CCR) Counter Clock Enable Command */
#define CLKDIS	(0x1u << 1)	/**< \brief (TC_CCR) Counter Clock Disable Command */
#define SWTRG	(0x1u << 2)	/**< \brief (TC_CCR) Software Trigger Command */

#define TIMER_CLOCK1 (0x0u << 0) /**< \brief (TC_CMR) Clock selected: TCLK1 */
#define TIMER_CLOCK2 (0x1u << 0) /**< \brief (TC_CMR) Clock selected: TCLK2 */
#define TIMER_CLOCK3 (0x2u << 0) /**< \brief (TC_CMR) Clock selected: TCLK3 */
#define TIMER_CLOCK4 (0x3u << 0) /**< \brief (TC_CMR) Clock selected: TCLK4 */
#define TIMER_CLOCK5 (0x4u << 0) /**< \brief (TC_CMR) Clock selected: TCLK5 */
#define XC0 (0x5u << 0) /**< \brief (TC_CMR) Clock selected: XC0 */
#define XC1 (0x6u << 0) /**< \brief (TC_CMR) Clock selected: XC1 */
#define XC2 (0x7u << 0) /**< \brief (TC_CMR) Clock selected: XC2 */
#define	CLKI (0x1u << 3) /**< \brief (TC_CMR) Clock Invert */
#define BURST_NONE (0x0u << 4) /**< \brief (TC_CMR) The clock is not gated by an external signal. */
#define BURST_XC0 (0x1u << 4) /**< \brief (TC_CMR) XC0 is ANDed with the selected clock. */
#define BURST_XC1 (0x2u << 4) /**< \brief (TC_CMR) XC1 is ANDed with the selected clock. */
#define BURST_XC2 (0x3u << 4) /**< \brief (TC_CMR) XC2 is ANDed with the selected clock. */
#define LDBSTOP (0x1u << 6) /**< \brief (TC_CMR) Counter Clock Stopped with RB Loading */
#define LDBDIS (0x1u << 7) /**< \brief (TC_CMR) Counter Clock Disable with RB Loading */
#define ETRGEDG_NONE (0x0u << 8) /**< \brief (TC_CMR) The clock is not gated by an external signal. */
#define ETRGEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */
#define ETRGEDG_FALLING (0x2u << 8) /**< \brief (TC_CMR) Falling edge */
#define ETRGEDG_EDGE (0x3u << 8) /**< \brief (TC_CMR) Each edge */
#define ABETRG (0x1u << 10) /**< \brief (TC_CMR) TIOA or TIOB External Trigger Selection */

#define CPCTRG (0x1u << 14) /**< \brief (TC_CMR) RC Compare Trigger Enable */

#define WAVE (0x1u << 15) /**< \brief (TC_CMR) Waveform Mode */
#define LDRA_NONE (0x0u << 16) /**< \brief (TC_CMR) None */
#define LDRA_RISING (0x1u << 16) /**< \brief (TC_CMR) Rising edge of TIOA */
#define LDRA_FALLING (0x2u << 16) /**< \brief (TC_CMR) Falling edge of TIOA */
#define LDRA_EDGE (0x3u << 16) /**< \brief (TC_CMR) Each edge of TIOA */
#define LDRB_NONE (0x0u << 18) /**< \brief (TC_CMR) None */
#define LDRB_RISING (0x1u << 18) /**< \brief (TC_CMR) Rising edge of TIOA */
#define LDRB_FALLING (0x2u << 18) /**< \brief (TC_CMR) Falling edge of TIOA */
#define LDRB_EDGE (0x3u << 18) /**< \brief (TC_CMR) Each edge of TIOA */
#define SBSMPLR_ONE (0x0u << 20) /**< \brief (TC_CMR) Load a Capture Register each selected edge */
#define SBSMPLR_HALF (0x1u << 20) /**< \brief (TC_CMR) Load a Capture Register every 2 selected edges */
#define SBSMPLR_FOURTH (0x2u << 20) /**< \brief (TC_CMR) Load a Capture Register every 4 selected edges */
#define SBSMPLR_EIGHTH (0x3u << 20) /**< \brief (TC_CMR) Load a Capture Register every 8 selected edges */
#define SBSMPLR_SIXTEENTH (0x4u << 20) /**< \brief (TC_CMR) Load a Capture Register every 16 selected edges */
#define CPCSTOP (0x1u << 6) /**< \brief (TC_CMR) Counter Clock Stopped with RC Compare */
#define CPCDIS (0x1u << 7) /**< \brief (TC_CMR) Counter Clock Disable with RC Compare */
#define EEVTEDG_NONE (0x0u << 8) /**< \brief (TC_CMR) None */
#define EEVTEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */
#define EEVTEDG_FALLING (0x2u << 8) /**< \brief (TC_CMR) Falling edge */
#define EEVTEDG_EDGE (0x3u << 8) /**< \brief (TC_CMR) Each edge */
#define EEVT_TIOB (0x0u << 10) /**< \brief (TC_CMR) TIOB */
#define EEVT_XC0 (0x1u << 10) /**< \brief (TC_CMR) XC0 */
#define EEVT_XC1 (0x2u << 10) /**< \brief (TC_CMR) XC1 */
#define EEVT_XC2 (0x3u << 10) /**< \brief (TC_CMR) XC2 */
#define ENETRG (0x1u << 12) /**< \brief (TC_CMR) External Event Trigger Enable */
#define WAVSEL_UP (0x0u << 13) /**< \brief (TC_CMR) UP mode without automatic trigger on RC Compare */
#define WAVSEL_UPDOWN (0x1u << 13) /**< \brief (TC_CMR) UPDOWN mode without automatic trigger on RC Compare */
#define WAVSEL_UP_RC (0x2u << 13) /**< \brief (TC_CMR) UP mode with automatic trigger on RC Compare */
#define WAVSEL_UPDOWN_RC (0x3u << 13) /**< \brief (TC_CMR) UPDOWN mode with automatic trigger on RC Compare */
#define ACPA_NONE (0x0u << 16) /**< \brief (TC_CMR) None */
#define ACPA_SET (0x1u << 16) /**< \brief (TC_CMR) Set */
#define ACPA_CLEAR (0x2u << 16) /**< \brief (TC_CMR) Clear */
#define ACPA_TOGGLE (0x3u << 16) /**< \brief (TC_CMR) Toggle */
#define   ACPC_NONE (0x0u << 18) /**< \brief (TC_CMR) None */
#define   ACPC_SET (0x1u << 18) /**< \brief (TC_CMR) Set */
#define   ACPC_CLEAR (0x2u << 18) /**< \brief (TC_CMR) Clear */
#define   ACPC_TOGGLE (0x3u << 18) /**< \brief (TC_CMR) Toggle */
#define   AEEVT_NONE (0x0u << 20) /**< \brief (TC_CMR) None */
#define   AEEVT_SET (0x1u << 20) /**< \brief (TC_CMR) Set */
#define   AEEVT_CLEAR (0x2u << 20) /**< \brief (TC_CMR) Clear */
#define   AEEVT_TOGGLE (0x3u << 20) /**< \brief (TC_CMR) Toggle */
#define   ASWTRG_NONE (0x0u << 22) /**< \brief (TC_CMR) None */
#define   ASWTRG_SET (0x1u << 22) /**< \brief (TC_CMR) Set */
#define   ASWTRG_CLEAR (0x2u << 22) /**< \brief (TC_CMR) Clear */
#define   ASWTRG_TOGGLE (0x3u << 22) /**< \brief (TC_CMR) Toggle */
#define   BCPB_NONE (0x0u << 24) /**< \brief (TC_CMR) None */
#define   BCPB_SET (0x1u << 24) /**< \brief (TC_CMR) Set */
#define   BCPB_CLEAR (0x2u << 24) /**< \brief (TC_CMR) Clear */
#define   BCPB_TOGGLE (0x3u << 24) /**< \brief (TC_CMR) Toggle */
#define   BCPC_NONE (0x0u << 26) /**< \brief (TC_CMR) None */
#define   BCPC_SET (0x1u << 26) /**< \brief (TC_CMR) Set */
#define   BCPC_CLEAR (0x2u << 26) /**< \brief (TC_CMR) Clear */
#define   BCPC_TOGGLE (0x3u << 26) /**< \brief (TC_CMR) Toggle */
#define   BEEVT_NONE (0x0u << 28) /**< \brief (TC_CMR) None */
#define   BEEVT_SET (0x1u << 28) /**< \brief (TC_CMR) Set */
#define   BEEVT_CLEAR (0x2u << 28) /**< \brief (TC_CMR) Clear */
#define   BEEVT_TOGGLE (0x3u << 28) /**< \brief (TC_CMR) Toggle */
#define   BSWTRG_NONE (0x0u << 30) /**< \brief (TC_CMR) None */
#define   BSWTRG_SET (0x1u << 30) /**< \brief (TC_CMR) Set */
#define   BSWTRG_CLEAR (0x2u << 30) /**< \brief (TC_CMR) Clear */
#define   BSWTRG_TOGGLE (0x3u << 30) /**< \brief (TC_CMR) Toggle */
/* -------- TC_SMMR : (TC Offset: N/A) Stepper Motor Mode Register -------- */
#define TC_SMMR_GCEN (0x1u << 0) /**< \brief (TC_SMMR) Gray Count Enable */
#define TC_SMMR_DOWN (0x1u << 1) /**< \brief (TC_SMMR) DOWN Count */
/* -------- TC_RAB : (TC Offset: N/A) Register AB -------- */
#define TC_RAB_RAB_Pos 0
#define TC_RAB_RAB_Msk (0xffffffffu << TC_RAB_RAB_Pos) /**< \brief (TC_RAB) Register A or Register B */
/* -------- TC_CV : (TC Offset: N/A) Counter Value -------- */
#define TC_CV_CV_Pos 0
#define TC_CV_CV_Msk (0xffffffffu << TC_CV_CV_Pos) /**< \brief (TC_CV) Counter Value */
/* -------- TC_RA : (TC Offset: N/A) Register A -------- */
#define TC_RA_RA_Pos 0
#define TC_RA_RA_Msk (0xffffffffu << TC_RA_RA_Pos) /**< \brief (TC_RA) Register A */
#define TC_RA_RA(value) ((TC_RA_RA_Msk & ((value) << TC_RA_RA_Pos)))
/* -------- TC_RB : (TC Offset: N/A) Register B -------- */
#define TC_RB_RB_Pos 0
#define TC_RB_RB_Msk (0xffffffffu << TC_RB_RB_Pos) /**< \brief (TC_RB) Register B */
#define TC_RB_RB(value) ((TC_RB_RB_Msk & ((value) << TC_RB_RB_Pos)))
/* -------- TC_RC : (TC Offset: N/A) Register C -------- */
#define TC_RC_RC_Pos 0
#define TC_RC_RC_Msk (0xffffffffu << TC_RC_RC_Pos) /**< \brief (TC_RC) Register C */
#define TC_RC_RC(value) ((TC_RC_RC_Msk & ((value) << TC_RC_RC_Pos)))
/* -------- TC_SR : (TC Offset: N/A) Status Register -------- */
#define COVFS	(0x1u << 0)		/**< \brief (TC_SR) Counter Overflow Status */
#define LOVRS	(0x1u << 1)		/**< \brief (TC_SR) Load Overrun Status */
#define CPAS 	(0x1u << 2)		/**< \brief (TC_SR) RA Compare Status */
#define CPBS 	(0x1u << 3)		/**< \brief (TC_SR) RB Compare Status */
#define CPCS 	(0x1u << 4)		/**< \brief (TC_SR) RC Compare Status */
#define LDRAS 	(0x1u << 5)		/**< \brief (TC_SR) RA Loading Status */
#define LDRBS 	(0x1u << 6)		/**< \brief (TC_SR) RB Loading Status */
#define ETRGS 	(0x1u << 7)		/**< \brief (TC_SR) External Trigger Status */
#define ENDRX 	(0x1u << 8)		/**< \brief (TC_SR) End of Receiver Transfer */
#define RXBUFF 	(0x1u << 9)		/**< \brief (TC_SR) Reception Buffer Full */
#define CLKSTA 	(0x1u << 16) 	/**< \brief (TC_SR) Clock Enabling Status */
#define MTIOA 	(0x1u << 17) 	/**< \brief (TC_SR) TIOA Mirror */
#define MTIOB 	(0x1u << 18) 	/**< \brief (TC_SR) TIOB Mirror */



#endif // SAM4SA_H__27_08_2014__11_07
