#ifndef MEMORY_AT91SAM7X256_K9K8G08U_H
#define MEMORY_AT91SAM7X256_K9K8G08U_H

#include "types.h"

/**************************************************/
/*
“ехнологи€ записи такова, что перва€ попытка при провале идЄт по следующему адресу,
втора€ по следующему блоку, трижды - ошибка.
можно сделать этот цикл бесконечным в цел€х более быстрого поиска живого места
можно также в случае битых блоков считать »ƒ пам€ти и пон€ть а не сдохла ли она совсем, но это муторно реализовывать
*/
/**************************************************/
#define K9K8G08U_BASE_PIO_RB 		AT91C_BASE_PIOA
#define K9K8G08U_PIO_RB 		AT91C_PIO_PA27
#define K9K8G08U_BASE_PIO_RE 		AT91C_BASE_PIOB
#define K9K8G08U_PIO_RE 		AT91C_PIO_PB20
#define K9K8G08U_BASE_PIO_CE 		AT91C_BASE_PIOA
#define K9K8G08U_PIO_CE 		AT91C_PIO_PA28
#define K9K8G08U_BASE_PIO_CE2 		AT91C_BASE_PIOA
#define K9K8G08U_PIO_CE2 		AT91C_PIO_PA29
#define K9K8G08U_BASE_PIO_CLE 		AT91C_BASE_PIOB
#define K9K8G08U_PIO_CLE 		AT91C_PIO_PB24
#define K9K8G08U_BASE_PIO_ALE 		AT91C_BASE_PIOB
#define K9K8G08U_PIO_ALE 		AT91C_PIO_PB23
#define K9K8G08U_BASE_PIO_WE 		AT91C_BASE_PIOB
#define K9K8G08U_PIO_WE 		AT91C_PIO_PB22
#define K9K8G08U_BASE_PIO_WP	 	AT91C_BASE_PIOB
#define K9K8G08U_PIO_WP 		AT91C_PIO_PB21

#define K9K8G08U_BASE_PIO_DATA 		AT91C_BASE_PIOA
#define K9K8G08U_PIO_DATA_OFFSET	10
#define K9K8G08U_PIO_DATA_MASK		0xFF

#define K9K8G08U_COMMAND_RESET		0xFF
#define K9K8G08U_COMMAND_READ_ID	0x90
#define K9K8G08U_COMMAND_READ_1		0x00
#define K9K8G08U_COMMAND_READ_2		0x30
#define K9K8G08U_COMMAND_PAGE_PROGRAM_1	0x80
#define K9K8G08U_COMMAND_PAGE_PROGRAM_2	0x10
#define K9K8G08U_COMMAND_READ_STATUS	0x70
#define K9K8G08U_COMMAND_BLOCK_ERASE_1	0x60
#define K9K8G08U_COMMAND_BLOCK_ERASE_2	0xD0

#define K9K8G08U_MAX_CHIP		8

#define K9K8G08U_READ_PACK_BYTES	512
#define K9K8G08U_WRITE_PACK_BYTES	256
/**************************************************/
typedef enum __attribute__ ((packed)) 
{
	K9K8G08U_STATUS_OPERATION_WAIT = 0,
	K9K8G08U_STATUS_OPERATION_READ_START,
	K9K8G08U_STATUS_OPERATION_READ_IDLE,
	K9K8G08U_STATUS_OPERATION_READ_READY,
	K9K8G08U_STATUS_OPERATION_READ_ERROR,
	K9K8G08U_STATUS_OPERATION_VERIFY_START,
	K9K8G08U_STATUS_OPERATION_VERIFY_IDLE,
	K9K8G08U_STATUS_OPERATION_VERIFY_READY,
	K9K8G08U_STATUS_OPERATION_VERIFY_ERROR,
	K9K8G08U_STATUS_OPERATION_WRITE_START,
	K9K8G08U_STATUS_OPERATION_WRITE_START_ERASED,
	K9K8G08U_STATUS_OPERATION_WRITE_IDLE,
	K9K8G08U_STATUS_OPERATION_WRITE_CHECK,
	K9K8G08U_STATUS_OPERATION_WRITE_ERASE_CHECK,
	K9K8G08U_STATUS_OPERATION_WRITE_READY,
	K9K8G08U_STATUS_OPERATION_WRITE_ERROR,
} k9k8g08u_status_operation_type;


typedef struct	__attribute__ ((packed))
{
 	byte marker;
 	byte device;
 	byte data[3];
} k9k8g08u_id_type;

typedef struct	__attribute__ ((packed))
{
 	byte page; //(1 << x)
 	byte block; //(1 << x)
 	byte chip;
 	byte full;
	u16 mask;
} k9k8g08u_memory_size_type;
/**************************************************/
extern void K9K8G08U_Init();
extern bool K9K8G08U_Idle();
/*********************************************************************************/
extern k9k8g08u_memory_size_type K9K8G08U_Size_Get();
extern k9k8g08u_status_operation_type K9K8G08U_Status_Operation_Get();
extern void K9K8G08U_Status_Operation_Reset();
/*********************************************************************************/
extern void K9K8G08U_Adress_Set(i64 a);
extern i64 K9K8G08U_Adress_Get();
/*********************************************************************************/
extern bool K9K8G08U_Reset();
extern bool K9K8G08U_Read_ID(k9k8g08u_id_type *id);
extern bool K9K8G08U_Read_Data(i64 *adr, byte *data, u16 size);
extern bool K9K8G08U_Verify_Data(i64 *adr, byte *data, u16 size);
extern bool K9K8G08U_Write_Data(bool next, i64 *adr, byte *data, u16 size);

/**************************************************/

#endif
