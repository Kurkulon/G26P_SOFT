#include "common.h"
#include "bootloader.h"

/*****************************************************************************/
unsigned int bootloader_time = 0;

void BootLoader_Start_Delay()
{
	bootloader_time = BOOTLOADER_START_DELAY_CYCLES;
}

void BootLoader_Start()
{
	//BOOT_CHECK_REGISTER = BOOT_CHECK_REGISTER_VALUE_BOOTLOADER; 
	//AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;
 //   	AT91C_BASE_AIC->AIC_ICCR = 0xFFFFFFFF;
	//AT91C_BASE_RSTC->RSTC_RMR = 0xA5000000 | AT91C_RSTC_ERSTL&(0x01<<8) | AT91C_RSTC_URSTEN;
	//AT91C_BASE_RSTC->RSTC_RCR = 0xA5000000 | AT91C_RSTC_EXTRST | AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST;
}

void BootLoader_Init()
{
 //      	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;
	//AT91F_PIO_Enable(BOOTLOADER_BUTTON_BASE_PIO, BOOTLOADER_BUTTON_PIO);
	//AT91F_PIO_CfgInput(BOOTLOADER_BUTTON_BASE_PIO, BOOTLOADER_BUTTON_PIO);
	//BOOTLOADER_BUTTON_BASE_PIO->PIO_IFER = BOOTLOADER_BUTTON_PIO;	// set filter noise
	//BOOTLOADER_BUTTON_BASE_PIO->PIO_PPUER = BOOTLOADER_BUTTON_PIO;	// set pullup
	//unsigned int timeout = 0x500000; // ~5 sec
	//while((timeout) && (!(AT91F_PIO_GetInput(BOOTLOADER_BUTTON_BASE_PIO) & BOOTLOADER_BUTTON_PIO))) 
	//{
	//	timeout --;
	//	AT91F_WDTRestart(AT91C_BASE_WDTC); // на всяк случай
	//}
	//if(timeout == 0) BootLoader_Start();
	//if(BOOT_CHECK_REGISTER != BOOT_CHECK_REGISTER_VALUE_PROGRAMM)
	//{
	//	BOOT_CHECK_REGISTER = BOOT_CHECK_REGISTER_VALUE_PROGRAMM; 
	//	AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;
	//	AT91C_BASE_AIC->AIC_ICCR = 0xFFFFFFFF;
	//	AT91C_BASE_RSTC->RSTC_RMR = 0xA5000000 | AT91C_RSTC_ERSTL&(0x01<<8) | AT91C_RSTC_URSTEN;
	//	AT91C_BASE_RSTC->RSTC_RCR = 0xA5000000 | AT91C_RSTC_EXTRST | AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST;
	//}
}

void BootLoader_Idle()
{
 //       if(bootloader_time)
	//{
	//	bootloader_time --;
	//	if(!bootloader_time) BootLoader_Start();
	//}
}
