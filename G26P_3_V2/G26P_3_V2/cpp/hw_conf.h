#ifndef HW_CONF_H__25_11_2022__16_53
#define HW_CONF_H__25_11_2022__16_53

#include <types.h>
#include <core.h>

#define BOOTLOADER

#define CLKIN_MHz	25
#define PLL_MSEL	2		// 1...32
#define PLL_PSEL	1		// 0...3
//#define PLL_MHz		(CLKIN_MHz*PLL_MSEL)		
//#define FCCO_MHz	(PLL_MHz*(2UL<<PLL_PSEL))		// 156...320

#if defined(FCCO_MHz) && ((FCCO_MHz < 156) || (FCCO_MHz > 320))
#error  FCCO_MHz must be 156...320
#endif

#define MCK_DIV			1
#define UARTCLK_DIV		1

#ifdef PLL_MHz
#define MCK_MHz ((float)PLL_MHz/MCK_DIV)
#else
#define MCK_MHz ((float)CLKIN_MHz/MCK_DIV)
#endif

#define MCK			((u32)(MCK_MHz*1000000UL))
#define NS2CLK(x) 	((u32)(((x)*MCK_MHz+500)/1000))
#define US2CLK(x) 	((u32)((x)*MCK_MHz))
#define MS2CLK(x) 	((u32)((x)*MCK_MHz*1000))

// ++++++++++++++	USIC	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define USART_USIC_NUM		0
//#define USART1_USIC_NUM	1
//#define USART2_USIC_NUM	2
//#define SPI0_USIC_NUM		3
//#define SPI1_USIC_NUM		4
#define I2C_USIC_NUM		5
//#define I2C1_USIC_NUM		6
//#define I2C2_USIC_NUM		7
//#define I2C3_USIC_NUM		8

// ++++++++++++++	USART	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define PIN_UTX			6 
#define PIN_URX			14 
#define PIN_RTS			0

#define UTX				(1UL<<PIN_UTX)
#define URX				(1UL<<PIN_URX)
#define RTS				(1UL<<PIN_RTS)

// ++++++++++++++	I2C	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define PIN_SCL			6 
#define PIN_SDA			14 
#define I2C_BAUDRATE	400000

// ++++++++++++++	MOTOR	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define PIN_FX1			17 
#define PIN_FX2			13 
#define PIN_FY1			12 
#define PIN_FY2			4 
#define PIN_EN			11
#define PIN_CHARGE		8
#define PIN_TR1			7

//#define PIN_SCK			9
//#define PIN_MISO		15
//#define PIN_ADCS		16

#define FX1				(1<<PIN_FX1) 
#define FX2				(1<<PIN_FX2) 
#define FY1				(1<<PIN_FY1) 
#define FY2				(1<<PIN_FY2) 
#define EN				(1<<PIN_EN) 
#define CHARGE			(1<<PIN_CHARGE) 
#define TR1				(1<<PIN_TR1) 


// ++++++++++++++	PIO INIT	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

																			
#define GPIO_INIT_DIR0		RTS|FX1|FX2|FY1|FY2|EN|CHARGE|TR1				
#define GPIO_INIT_PIN0		EN					


#endif // HW_CONF_H__25_11_2022__16_53
