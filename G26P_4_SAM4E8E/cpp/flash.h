#ifndef MEMORY_AT91SAM7X256_FLASH_H
#define MEMORY_AT91SAM7X256_FLASH_H

#include "rtc.h"

#define FLWB_LEN 2048
#define FLRB_LEN 1536

enum flash_save_repeat_type
{
	FLASH_SAVE_REPEAT_NONE = 0,
	FLASH_SAVE_REPEAT_NORMAL = 2,	// ���� ����������, ������ - � ����� ����
	FLASH_SAVE_REPEAT_HIGH = 4,	// .... 3 � ����� ����
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLWB
{
	FLWB *next;

	bool	*ready;
	u16 	hdrLen;
	u16 	dataLen;

	byte data[FLWB_LEN];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLRB
{
	FLRB *next;

	bool	ready;
	u16		maxLen;
	u16		len;

	byte	*data;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SpareArea
{
	u16		validPage;	// 0xFFFF - good page
	u16		validBlock;	// 0xFFFF - good block
	u16		badPages;	// 0xFFFF - all pages good

	u16		file;  		// file number
	u32		lpn;		// logic page number
	u32		start;		// start page of file
	u32		fpn;		// file page number
	u32		prev;		// start page of previos file

	u16		vecFstOff;	// first vector header offset. 0xFFFF - no vector header 
	u16		vecFstLen;	// first vector lenght. 0 - no vector

	u16		vecLstOff;	// last vector header offset. 0xFFFF - no vector header 
	u16		vecLstLen;	// last vector lenght. 0 - no vector

	u32		vecPrPg;	// previos vector raw page
	u16		vecPrOff;	// previos vector offset

	u16		fbb;		// file bad blocks count
	u16		fbp;		// file bad pages count

	byte	chipMask;	

	u16		crc;

	enum { CRC_SKIP = 6 };
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern FLWB*	AllocFlashWriteBuffer();
extern FLRB*	AllocFlashReadBuffer();
extern void		FreeFlashReadBuffer(FLRB* b);
extern bool		RequestFlashRead(FLRB* b);
extern bool		RequestFlashWrite(FLWB* b);

extern bool NAND_Idle();
extern void NAND_FullErase();
extern void NAND_NextSession();


/*************************/
extern void FLASH_Init();
extern bool FLASH_Reset();
extern void FLASH_Idle();
extern bool FLASH_Busy();

/*****************************************************************************/
extern bool FLASH_Erase_Full();
extern bool FLASH_UnErase_Full();
extern bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat);
extern bool FLASH_Read_Vector(u64 adr, u16 *size, bool *ready, byte **vector);
//extern u32 FLASH_Vectors_Errors_Get();
//extern u32 FLASH_Vectors_Saved_Get();
//extern void FLASH_Vectors_Errors_Reset();
//extern void FLASH_Vectors_Saved_Reset();
//extern i64 FLASH_Current_Adress_Get();
extern u64 FLASH_Full_Size_Get();
extern u16 FLASH_Chip_Mask_Get();
extern u64 FLASH_Used_Size_Get();
//extern i64 FLASH_Empty_Size_Get();


/*****************************************************************************/

#endif