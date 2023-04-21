#include "hardware.h"

//#pragma O3
//#pragma Ospace

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define BOOT_START_BREAKPOINT
#define BOOT_EXIT_BREAKPOINT
#define BOOT_HANDSHAKE
//#define BOOT_COM_MODE				ComPort::ASYNC
#define BOOT_COM_SPEED				115200
#define BOOT_COM_PARITY				0
#define BOOT_COM_STOPBITS			1
#define BOOT_COM_PRETIMEOUT			(~0)
#define BOOT_COM_POSTTIMEOUT		(US2COM(390))
#define BOOT_HANDSHAKE_PRETIMEOUT	(MS2COM(100))
#define BOOT_HANDSHAKE_POSTTIMEOUT	(US2COM(390))
//#define BOOT_SGUID					0x529DE289BF5D4568
//#define BOOT_MGUID					0x892EBD176E99822B
#define BOOT_SGUID					0x32416D09EFF84585
#define BOOT_MGUID					0xBE892EF1C34CA7D4
//#define BOOT_START_SECTOR			8

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define Pin_MainLoop_Set()	HW::GPIO->BSET(12)
#define Pin_MainLoop_Clr()	HW::GPIO->BCLR(12)
#define Pin_MainLoop_Tgl()	HW::GPIO->NOT0 = 1UL<<12

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <ComPort_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ComPort com(USART0_USIC_NUM, ~0, PIN_UTX0, PIN_URX0, PIN_RTS0);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <boot_LPC824_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
