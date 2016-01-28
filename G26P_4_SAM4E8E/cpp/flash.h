#ifndef MEMORY_AT91SAM7X256_FLASH_H
#define MEMORY_AT91SAM7X256_FLASH_H

#include "rtc.h"

#define FLASH_SAVE_BUFFER_SIZE		8400
#define FLASH_READ_BUFFER_SIZE		8400

typedef enum __attribute__ ((packed)) 
{
	FLASH_SAVE_REPEAT_NONE = 0,
	FLASH_SAVE_REPEAT_NORMAL = 2,	// одна перезапись, вторая - в новый блок
	FLASH_SAVE_REPEAT_HIGH = 4,	// .... 3 в новый блок
} flash_save_repeat_type;

typedef enum __attribute__ ((packed)) 
{
	FLASH_STATUS_WAIT = 0,
	FLASH_STATUS_READY,
	FLASH_STATUS_ERROR,
} flash_status_type;

typedef enum __attribute__ ((packed)) 
{
	FLASH_STATUS_OPERATION_WAIT = 0,
	FLASH_STATUS_OPERATION_SAVE_WAIT,
	FLASH_STATUS_OPERATION_SAVE_WRITE,
	FLASH_STATUS_OPERATION_SAVE_VERIFY,
	FLASH_STATUS_OPERATION_READ_WAIT,
	FLASH_STATUS_OPERATION_READ_HEADER,
	FLASH_STATUS_OPERATION_READ_DATA,
} flash_status_operation_type;


/*************************/
extern void FLASH_Init();
extern bool FLASH_Reset();
extern void FLASH_Idle();
extern bool FLASH_Busy();

/*****************************************************************************/
extern bool FLASH_Erase_Full();
extern bool FLASH_UnErase_Full();
extern bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat);
extern bool FLASH_Read_Vector(i64 adr, u16 *size, bool *ready, byte **vector);
extern u32 FLASH_Vectors_Errors_Get();
extern u32 FLASH_Vectors_Saved_Get();
extern void FLASH_Vectors_Errors_Reset();
extern void FLASH_Vectors_Saved_Reset();
extern i64 FLASH_Current_Adress_Get();
extern i64 FLASH_Full_Size_Get();
extern u16 FLASH_Chip_Mask_Get();
extern i64 FLASH_Used_Size_Get();
extern i64 FLASH_Empty_Size_Get();


/*****************************************************************************/

#endif
