/*********************************************************************************************************************
 * @file     system_XMC4800.h
 * @brief    Device specific initialization for the XMC4800-Series according to CMSIS
 * @version  V1.0
 * @date     22 May 2015
 *
 * @cond
 *********************************************************************************************************************
 * Copyright (c) 2015-2016, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the 
 * following conditions are met:   
 *                                                                              
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer.                        
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided with the distribution.                       
 * 
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written permission.                                           
 *                                                                              
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR  
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                  
 *                                                                              
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes with 
 * Infineon Technologies AG dave@infineon.com).                                                          
 *********************************************************************************************************************
 *
 **************************** Change history *********************************
 * V1.0, 22 May 2015, JFT, Initial version
 *****************************************************************************
 * @endcond 
 */

#ifndef SYSTEM_XMC4800_H
#define SYSTEM_XMC4800_H

/*******************************************************************************
 * HEADER FILES
 *******************************************************************************/

#include <stdint.h>

/*******************************************************************************
 * MACROS
 *******************************************************************************/

#define	OFI_FREQUENCY        (24000000UL)  /**< 24MHz Backup Clock (fOFI) frequency. */
#define OSI_FREQUENCY        (32768UL)    /**< 32KHz Internal Slow Clock source (fOSI) frequency. */  

#define SCU_CLK_CLKCLR_ENABLE_USBCLK SCU_CLK_CLKCLR_USBCDI_Msk
#define SCU_CLK_CLKCLR_ENABLE_MMCCLK SCU_CLK_CLKCLR_MMCCDI_Msk
#define SCU_CLK_CLKCLR_ENABLE_ETHCLK SCU_CLK_CLKCLR_ETH0CDI_Msk
#define SCU_CLK_CLKCLR_ENABLE_EBUCLK SCU_CLK_CLKCLR_EBUCDI_Msk
#define SCU_CLK_CLKCLR_ENABLE_CCUCLK SCU_CLK_CLKCLR_CCUCDI_Msk

#define SCU_CLK_SYSCLKCR_SYSSEL_OFI      (0U << SCU_CLK_SYSCLKCR_SYSSEL_Pos)
#define SCU_CLK_SYSCLKCR_SYSSEL_PLL      (1U << SCU_CLK_SYSCLKCR_SYSSEL_Pos)

#define SCU_CLK_USBCLKCR_USBSEL_USBPLL   (0U << SCU_CLK_USBCLKCR_USBSEL_Pos)
#define SCU_CLK_USBCLKCR_USBSEL_PLL      (1U << SCU_CLK_USBCLKCR_USBSEL_Pos)

#define SCU_CLK_ECATCLKCR_ECATSEL_USBPLL (0U << SCU_CLK_ECATCLKCR_ECATSEL_Pos)
#define SCU_CLK_ECATCLKCR_ECATSEL_PLL    (1U << SCU_CLK_ECATCLKCR_ECATSEL_Pos)

#define SCU_CLK_WDTCLKCR_WDTSEL_OFI      (0U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)
#define SCU_CLK_WDTCLKCR_WDTSEL_STANDBY  (1U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)
#define SCU_CLK_WDTCLKCR_WDTSEL_PLL      (2U << SCU_CLK_WDTCLKCR_WDTSEL_Pos)

#define SCU_CLK_EXTCLKCR_ECKSEL_SYS      (0U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)
#define SCU_CLK_EXTCLKCR_ECKSEL_USBPLL   (2U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)
#define SCU_CLK_EXTCLKCR_ECKSEL_PLL      (3U << SCU_CLK_EXTCLKCR_ECKSEL_Pos)

/*******************************************************************************
 * GLOBAL VARIABLES
 *******************************************************************************/

//extern uint32_t SystemCoreClock;     /*!< System Clock Frequency (Core Clock)  */
//extern uint8_t g_chipid[16];

//typedef void(*ARM_IHP)() __irq;
//extern ARM_IHP VectorTableInt[16]; //0x1FFE8000
//extern ARM_IHP VectorTableExt[112];

//ARM_IHP * const VectorTableInt = (ARM_IHP*)0x1FFE8000;
//ARM_IHP * const VectorTableExt = (ARM_IHP*)0x1FFE8040;

/*******************************************************************************
 * API PROTOTYPES
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the system
 *
 */
void SystemInit(void);

/**
 * @brief Initialize CPU settings
 *
 */
//uint32_t OSCHP_GetFrequency(void);

#ifdef __cplusplus
}
#endif

#endif
