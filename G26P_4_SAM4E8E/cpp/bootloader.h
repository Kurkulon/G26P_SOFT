#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#define BOOT_CHECK_REGISTER			(*(volatile unsigned int *)(AT91C_ISRAM + AT91C_ISRAM_SIZE - sizeof(unsigned int))) // последний регистр памяти
#define BOOT_CHECK_REGISTER_VALUE_NULL		0
#define BOOT_CHECK_REGISTER_VALUE_BOOTLOADER	0xAAAAAAAA
#define BOOT_CHECK_REGISTER_VALUE_PROGRAMM	0x55555555

#define BOOTLOADER_BUTTON_PIO			AT91C_PIO_PA22
#define BOOTLOADER_BUTTON_BASE_PIO		AT91C_BASE_PIOA

#define BOOTLOADER_START_DELAY_CYCLES		16

extern void BootLoader_Init();
extern void BootLoader_Idle();
extern void BootLoader_Start();
extern void BootLoader_Start_Delay();


#endif
