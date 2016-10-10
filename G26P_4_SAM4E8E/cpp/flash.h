#ifndef MEMORY_AT91SAM7X256_FLASH_H
#define MEMORY_AT91SAM7X256_FLASH_H

#include "rtc.h"
#include "vector.h"
#include "trap_def.h"


#define FLWB_LEN (8192+32)
#define FLRB_LEN 1536

enum  
{
	FLASH_STATUS_NONE = 0,
	FLASH_STATUS_BUSY,				// зан€т неизвестной дл€ компьютера операцией (например записью в пам€ть)
	FLASH_STATUS_STOP,
	FLASH_STATUS_PAUSE,
	FLASH_STATUS_RESUME,
	FLASH_STATUS_ERASE,
	FLASH_STATUS_UNERASE,
	FLASH_STATUS_READ_SESSION_START,
	FLASH_STATUS_READ_SESSION_IDLE,
	FLASH_STATUS_READ_SESSION_FIND, // поиск вектора по несв€занной области
	FLASH_STATUS_READ_SESSION_ERROR,
	FLASH_STATUS_READ_SESSION_READY,
	FLASH_STATUS_READ_VECTOR_START,
	FLASH_STATUS_READ_VECTOR_IDLE,
	FLASH_STATUS_READ_VECTOR_FIND, // поиск вектора по несв€занной области
	FLASH_STATUS_READ_VECTOR_ERROR,
	FLASH_STATUS_READ_VECTOR_READY
};


enum flash_save_repeat_type
{
	FLASH_SAVE_REPEAT_NONE = 0,
	FLASH_SAVE_REPEAT_NORMAL = 2,	// одна перезапись, втора€ - в новый блок
	FLASH_SAVE_REPEAT_HIGH = 4,	// .... 3 в новый блок
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct VecData
{
	__packed struct Hdr
	{
		u16 		session;
		u16 		device;
		RTC_type	rtc;
		byte		flags;		// флажки
		u64			prVecAdr;	// адрес предыдущего бл€довектора
		u16			dataLen;
		u16			crc;		// CRC16 всей этой хрени
	} h;

	byte		data[FLWB_LEN]; // ѕоследние 2 байта CRC16
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLWB
{
	FLWB *next;

	bool	*ready;
//	u16 	hdrLen;
	u16 	dataLen;

	VecData vd;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLRB
{
	FLRB *next;

	bool	ready;
	bool	vecStart; // Ќайти начало вектора и считать hdr

	VecData::Hdr hdr;

	u16		maxLen;
	u16		len;

	byte	*data;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FileDsc
{
	u16		num;		// file number
	u32		start;		// start page of file
	u32		prev;		// start page of previos file
	u32		vecCount;	// vectors count
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SpareArea
{
	u16		validPage;	// 0xFFFF - good page
	u16		validBlock;	// 0xFFFF - good block
	u16		badPages;	// 0xFFFF - all pages good

	u16		file;  		// file number
//	u32		lpn;		// logic page number
	u32		start;		// start page of file
	u32		fpn;		// file page number
	u32		prev;		// start page of previos file
	u32		rawPage;	// raw page num

	u32		vectorCount; // vector count in file

	u16		vecFstOff;	// first vector header offset. 0xFFFF - no vector header 
	u16		vecFstLen;	// first vector lenght. 0 - no vector

	u16		vecLstOff;	// last vector header offset. 0xFFFF - no vector header 
	u16		vecLstLen;	// last vector lenght. 0 - no vector

	//u32		vecPrPg;	// previos vector raw page
	//u16		vecPrOff;	// previos vector offset

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

extern void NAND_Idle();
extern void NAND_FullErase();
//extern void NAND_NextSession();

extern void StartSendSession();

extern const SessionInfo* GetLastSessionInfo();

inline void SaveParams() { extern byte savesCount; savesCount = 1; }

/*************************/
extern void FLASH_Init();
extern bool FLASH_Reset();
extern void FLASH_Update();
extern bool FLASH_Busy();

extern void FLASH_WriteEnable();
extern void FLASH_WriteDisable();
extern byte FLASH_Status();

/*****************************************************************************/
//extern bool FLASH_Erase_Full();
//extern bool FLASH_UnErase_Full();
//extern bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat);
//extern bool FLASH_Read_Vector(u64 adr, u16 *size, bool *ready, byte **vector);

extern u32 FLASH_Vectors_Errors_Get();
extern u32 FLASH_Vectors_Saved_Get();
extern u32 FLASH_Vectors_Recieved_Get();
extern u32 FLASH_Vectors_Rejected_Get();
extern u32 FLASH_Session_Get();


//extern void FLASH_Vectors_Errors_Reset();
//extern void FLASH_Vectors_Saved_Reset();

extern u64 FLASH_Current_Adress_Get();
extern u64 FLASH_Full_Size_Get();
extern u16 FLASH_Chip_Mask_Get();
extern u64 FLASH_Used_Size_Get();

//extern i64 FLASH_Empty_Size_Get();


/*****************************************************************************/

#endif
