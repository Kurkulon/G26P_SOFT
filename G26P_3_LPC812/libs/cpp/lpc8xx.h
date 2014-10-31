#ifndef LPC43XX_H__23_09_2013__11_37
#define LPC43XX_H__23_09_2013__11_37

#ifndef CORETYPE_LPC8XX
#error  Must #include "core.h"
#endif 

#include "types.h"
#include "cm3.h"

#define MCK 25000000


#ifndef WIN32
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(p))
#else
extern byte core_sys_array[0x100000]; 
#define MK_PTR(n,p)  T_HW::S_##n * const n = ((T_HW::S_##n*)(core_sys_array-0x40000000+p))
#endif

#define RA32(c,s,e) z__reserved##c[(e-s)/4+1]


#define MKPID(n,i) n##_M=(1UL<<(i&31)), n##_I=i

namespace T_HW
{
	typedef volatile unsigned int LPC_REG;// Hardware register definition
	typedef volatile unsigned char LPC_R8;// Hardware register definition
	typedef volatile void * LPC_PTR;// Hardware register definition

	typedef void(*LPC_IHP)() __irq;	// Interrupt handler pointer

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_SYSCON
	{
		LPC_REG SYSMEMREMAP;            /*!< Offset: 0x000 System memory remap (R/W) */
		LPC_REG PRESETCTRL;             /*!< Offset: 0x004 Peripheral reset control (R/W) */
		LPC_REG SYSPLLCTRL;             /*!< Offset: 0x008 System PLL control (R/W) */
		LPC_REG SYSPLLSTAT;             /*!< Offset: 0x00C System PLL status (R/W ) */
		LPC_REG z_RESERVED0[4];

		LPC_REG SYSOSCCTRL;             /*!< Offset: 0x020 System oscillator control (R/W) */
		LPC_REG WDTOSCCTRL;             /*!< Offset: 0x024 Watchdog oscillator control (R/W) */
		LPC_REG z_RESERVED1[2];
		LPC_REG SYSRSTSTAT;             /*!< Offset: 0x030 System reset status Register (R/W ) */
		LPC_REG z_RESERVED2[3];
		LPC_REG SYSPLLCLKSEL;           /*!< Offset: 0x040 System PLL clock source select (R/W) */
		LPC_REG SYSPLLCLKUEN;           /*!< Offset: 0x044 System PLL clock source update enable (R/W) */
		LPC_REG z_RESERVED3[10];

		LPC_REG MAINCLKSEL;             /*!< Offset: 0x070 Main clock source select (R/W) */
		LPC_REG MAINCLKUEN;             /*!< Offset: 0x074 Main clock source update enable (R/W) */
		LPC_REG SYSAHBCLKDIV;           /*!< Offset: 0x078 System AHB clock divider (R/W) */
		LPC_REG z_RESERVED4[1];

		LPC_REG SYSAHBCLKCTRL;          /*!< Offset: 0x080 System AHB clock control (R/W) */
		LPC_REG z_RESERVED5[4];
		LPC_REG UARTCLKDIV;             /*!< Offset: 0x094 UART clock divider (R/W) */
		LPC_REG z_RESERVED6[18];

		LPC_REG CLKOUTSEL;              /*!< Offset: 0x0E0 CLKOUT clock source select (R/W) */
		LPC_REG CLKOUTUEN;              /*!< Offset: 0x0E4 CLKOUT clock source update enable (R/W) */
		LPC_REG CLKOUTDIV;              /*!< Offset: 0x0E8 CLKOUT clock divider (R/W) */
		LPC_REG z_RESERVED7;
		LPC_REG UARTFRGDIV;             /*!< Offset: 0x0F0 UART fractional divider SUB(R/W) */
		LPC_REG UARTFRGMULT;             /*!< Offset: 0x0F4 UART fractional divider ADD(R/W) */
		LPC_REG z_RESERVED8[1];
		LPC_REG EXTTRACECMD;            /*!< (@ 0x400480FC) External trace buffer command register  */
		LPC_REG PIOPORCAP0;             /*!< Offset: 0x100 POR captured PIO status 0 (R/ ) */
		LPC_REG z_RESERVED9[12];
		LPC_REG IOCONCLKDIV[7];			/*!< (@0x40048134-14C) Peripheral clock x to the IOCON block for programmable glitch filter */
		LPC_REG BODCTRL;                /*!< Offset: 0x150 BOD control (R/W) */
		LPC_REG SYSTCKCAL;              /*!< Offset: 0x154 System tick counter calibration (R/W) */
		LPC_REG z_RESERVED10[6];
		LPC_REG IRQLATENCY;             /*!< (@ 0x40048170) IRQ delay */
		LPC_REG NMISRC;                 /*!< (@ 0x40048174) NMI Source Control     */
		LPC_REG PINTSEL[8];             /*!< (@ 0x40048178) GPIO Pin Interrupt Select register 0 */
		LPC_REG z_RESERVED11[27];
		LPC_REG STARTERP0;              /*!< Offset: 0x204 Start logic signal enable Register 0 (R/W) */
		LPC_REG z_RESERVED12[3];
		LPC_REG STARTERP1;              /*!< Offset: 0x214 Start logic signal enable Register 0 (R/W) */
		LPC_REG z_RESERVED13[6];
		LPC_REG PDSLEEPCFG;             /*!< Offset: 0x230 Power-down states in Deep-sleep mode (R/W) */
		LPC_REG PDAWAKECFG;             /*!< Offset: 0x234 Power-down states after wake-up (R/W) */
		LPC_REG PDRUNCFG;               /*!< Offset: 0x238 Power-down configuration Register (R/W) */
		LPC_REG z_RESERVED14[110];
		LPC_REG DEVICE_ID;              /*!< Offset: 0x3F4 Device ID (R/ ) */

	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_IOCON
	{										/*!< (@ 0x40044000) IOCONFIG Structure     */
		LPC_REG PIO0_17;                    /*!< (@ 0x40044000) I/O configuration for pin PIO0_17 */
		LPC_REG PIO0_13;                    /*!< (@ 0x40044004) I/O configuration for pin PIO0_13 */
		LPC_REG PIO0_12;                    /*!< (@ 0x40044008) I/O configuration for pin PIO0_12 */
		LPC_REG PIO0_5;                     /*!< (@ 0x4004400C) I/O configuration for pin PIO0_5 */
		LPC_REG PIO0_4;                     /*!< (@ 0x40044010) I/O configuration for pin PIO0_4 */
		LPC_REG PIO0_3;                     /*!< (@ 0x40044014) I/O configuration for pin PIO0_3 */
		LPC_REG PIO0_2;                     /*!< (@ 0x40044018) I/O configuration for pin PIO0_2 */
		LPC_REG PIO0_11;                    /*!< (@ 0x4004401C) I/O configuration for pin PIO0_11 */
		LPC_REG PIO0_10;                    /*!< (@ 0x40044020) I/O configuration for pin PIO0_10 */
		LPC_REG PIO0_16;                    /*!< (@ 0x40044024) I/O configuration for pin PIO0_16 */
		LPC_REG PIO0_15;                    /*!< (@ 0x40044028) I/O configuration for pin PIO0_15 */
		LPC_REG PIO0_1;                     /*!< (@ 0x4004402C) I/O configuration for pin PIO0_1 */
		LPC_REG z_Reserved;                 /*!< (@ 0x40044030) I/O configuration for pin (Reserved) */
		LPC_REG PIO0_9;                     /*!< (@ 0x40044034) I/O configuration for pin PIO0_9 */
		LPC_REG PIO0_8;                     /*!< (@ 0x40044038) I/O configuration for pin PIO0_8 */
		LPC_REG PIO0_7;                     /*!< (@ 0x4004403C) I/O configuration for pin PIO0_7 */
		LPC_REG PIO0_6;                     /*!< (@ 0x40044040) I/O configuration for pin PIO0_6 */
		LPC_REG PIO0_0;                     /*!< (@ 0x40044044) I/O configuration for pin PIO0_0 */
		LPC_REG PIO0_14;                    /*!< (@ 0x40044048) I/O configuration for pin PIO0_14 */
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_FLASHCTRL
	{					                            /*!< (@ 0x40040000) FLASHCTRL Structure    */
		LPC_REG  z_RESERVED0[4];
		LPC_REG  FLASHCFG;                          /*!< (@ 0x40040010) Flash configuration register                           */
		LPC_REG  z_RESERVED1[3];
		LPC_REG  FMSSTART;                          /*!< (@ 0x40040020) Signature start address register                       */
		LPC_REG  FMSSTOP;                           /*!< (@ 0x40040024) Signature stop-address register                        */
		LPC_REG  z_RESERVED2;
		LPC_REG  FMSW0;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PMU
	{
		LPC_REG PCON;                   /*!< Offset: 0x000 Power control Register (R/W) */
		LPC_REG GPREG0;                 /*!< Offset: 0x004 General purpose Register 0 (R/W) */
		LPC_REG GPREG1;                 /*!< Offset: 0x008 General purpose Register 1 (R/W) */
		LPC_REG GPREG2;                 /*!< Offset: 0x00C General purpose Register 2 (R/W) */
		LPC_REG GPREG3;                 /*!< Offset: 0x010 General purpose Register 3 (R/W) */
		LPC_REG DPDCTRL;                /*!< Offset: 0x014 Deep power-down control register (R/W) */
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	struct S_SWM
	{
		LPC_REG		PINASSIGN[9];
		LPC_REG		z_RESERVED0[103];
		LPC_REG		PINENABLE0;
	} ;
	
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_GPIO
	{
		byte	B0[18];						/*!< (@ 0xA0000000) Byte pin registers port 0 */
		u16		z_RESERVED0[2039];
		LPC_REG W0[18];						/*!< (@ 0xA0001000) Word pin registers port 0 */
		LPC_REG z_RESERVED1[1006];
		LPC_REG DIR0;						/* 0x2000 */
		LPC_REG z_RESERVED2[31];
		LPC_REG MASK0;                      /* 0x2080 */
		LPC_REG z_RESERVED3[31];
		LPC_REG PIN0;                       /* 0x2100 */
		LPC_REG z_RESERVED4[31];
		LPC_REG MPIN0;                      /* 0x2180 */
		LPC_REG z_RESERVED5[31];
		LPC_REG SET0;                       /* 0x2200 */
		LPC_REG z_RESERVED6[31];
		LPC_REG CLR0;                       /* 0x2280 */
		LPC_REG z_RESERVED7[31];
		LPC_REG NOT0;                       /* 0x2300 */
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_PIN_INT
	{                           /*!< (@ 0xA0004000) PIN_INT Structure */
		LPC_REG ISEL;           /*!< (@ 0xA0004000) Pin Interrupt Mode register */
		LPC_REG IENR;           /*!< (@ 0xA0004004) Pin Interrupt Enable (Rising) register */
		LPC_REG SIENR;          /*!< (@ 0xA0004008) Set Pin Interrupt Enable (Rising) register */
		LPC_REG CIENR;          /*!< (@ 0xA000400C) Clear Pin Interrupt Enable (Rising) register */
		LPC_REG IENF;           /*!< (@ 0xA0004010) Pin Interrupt Enable Falling Edge / Active Level register */
		LPC_REG SIENF;          /*!< (@ 0xA0004014) Set Pin Interrupt Enable Falling Edge / Active Level register */
		LPC_REG CIENF;          /*!< (@ 0xA0004018) Clear Pin Interrupt Enable Falling Edge / Active Level address */
		LPC_REG RISE;           /*!< (@ 0xA000401C) Pin Interrupt Rising Edge register */
		LPC_REG FALL;           /*!< (@ 0xA0004020) Pin Interrupt Falling Edge register */
		LPC_REG IST;            /*!< (@ 0xA0004024) Pin Interrupt Status register */
		LPC_REG PMCTRL;         /*!< (@ 0xA0004028) GPIO pattern match interrupt control register          */
		LPC_REG PMSRC;          /*!< (@ 0xA000402C) GPIO pattern match interrupt bit-slice source register */
		LPC_REG PMCFG;          /*!< (@ 0xA0004030) GPIO pattern match interrupt bit slice configuration register */
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_CRC
	{
		LPC_REG MODE;
		LPC_REG SEED;
		LPC_REG SUM;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_MRT
	{
		struct
		{
			LPC_REG INTVAL;
			LPC_REG TIMER;
			LPC_REG CTRL;
			LPC_REG STAT;
		} Channel[4];
	   
		LPC_REG Reserved0[1];
		LPC_REG IDLE_CH;
		LPC_REG IRQ_FLAG;
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	typedef struct S_USART
	{
		LPC_REG  CFG;						/* 0x00 */
		LPC_REG  CTRL;
		LPC_REG  STAT;
		LPC_REG  INTENSET;
		LPC_REG  INTENCLR;					/* 0x10 */
		LPC_REG  RXDATA;
		LPC_REG  RXDATA_STAT;
		LPC_REG  TXDATA;
		LPC_REG  BRG;						/* 0x20 */
		LPC_REG  INTSTAT;
	} S_USART0, S_USART1, S_USART2;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	typedef struct S_SPI
	{
		LPC_REG  CFG;			/* 0x00 */
		LPC_REG  DLY;
		LPC_REG  STAT;
		LPC_REG  INTENSET;
		LPC_REG  INTENCLR;		/* 0x10 */
		LPC_REG  RXDAT;
		LPC_REG  TXDATCTL;
		LPC_REG  TXDAT;
		LPC_REG  TXCTRL;		/* 0x20 */
		LPC_REG  DIV;
		LPC_REG  INTSTAT;
	} S_SPI0, S_SPI1;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	struct S_WDT
	{
		LPC_REG MOD;                    /*!< Offset: 0x000 Watchdog mode register (R/W) */
		LPC_REG TC;                     /*!< Offset: 0x004 Watchdog timer constant register (R/W) */
		LPC_REG FEED;                   /*!< Offset: 0x008 Watchdog feed sequence register (W) */
		LPC_REG TV;                     /*!< Offset: 0x00C Watchdog timer value register (R) */
		LPC_REG z_RESERVED;             /*!< Offset: 0x010 RESERVED                          */
		LPC_REG WARNINT;                /*!< Offset: 0x014 Watchdog timer warning int. register (R/W) */
		LPC_REG WINDOW;                 /*!< Offset: 0x018 Watchdog timer window value register (R/W) */
	};

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace HW
{
	namespace CLK
	{
		enum {	MKPID(SYS, 0),		MKPID(ROM, 1),		MKPID(RAM, 2),		MKPID(FLASHREG, 3),		MKPID(FLASH, 4),	MKPID(I2C, 5),		MKPID(GPIO, 6),		MKPID(SWM, 7),
				MKPID(SCT, 8),		MKPID(WKT, 9),		MKPID(MRT, 10),		MKPID(SPI0, 11),		MKPID(SPI1, 12),	MKPID(CRC, 13),		MKPID(UART0, 14),	MKPID(UART1, 15),
				MKPID(UART2, 16),	MKPID(WWDT, 17),	MKPID(IOCON, 18),	MKPID(ACMP, 19) };
	};

	MK_PTR(WDT,				0x40000000);
	MK_PTR(MRT, 			0x40004000);
	MK_PTR(SWM,				0x4000C000);
	MK_PTR(PMU,				0x40020000);
	MK_PTR(FLASHCTRL,		0x40040000);
	MK_PTR(IOCON,			0x40044000);
	MK_PTR(SYSCON,			0x40048000);
	MK_PTR(SPI0,			0x40058000);
	MK_PTR(SPI1,			0x4005C000);
	MK_PTR(USART0,			0x40064000);
	MK_PTR(USART1,			0x40068000);
	MK_PTR(USART2,			0x4006C000);

	MK_PTR(CRC,				0x50000000);
//	MK_PTR(SCT,				0x50004000);
	MK_PTR(GPIO,			0xA0000000);
	MK_PTR(PIN_INT,			0xA0004000);

	inline void ResetWDT() { /*WDT->CR = 0xA5000001;*/ }


	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


}; // namespace HW

extern T_HW::LPC_IHP VectorTableInt[16];
extern T_HW::LPC_IHP VectorTableExt[53];

#undef MK_PTR
#undef MKPID

#endif // LPC43XX_H__23_09_2013__11_37
