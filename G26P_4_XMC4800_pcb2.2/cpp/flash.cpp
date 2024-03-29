//#ifndef WIN32

//#include <string.h>

//#include "common.h"
#include "core.h"
#include "flash.h"
//#include "fram.h"
#include "vector.h"
#include "list.h"
#include <CRC16.h>
#include "ComPort.h"
#include "trap_def.h"
#include "trap.h"
#include "twi.h"
#include "PointerCRC.h"

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#include <stdio.h>

static HANDLE handleNandFile;
static const char nameNandFile[] = "NAND_FLASH_STORE.BIN";

static byte nandChipSelected = 0;

static u64 curNandFilePos = 0;
static u64 curNandFileBlockPos = 0;
static u32 curBlock = 0;
static u16 curPage = 0;
static u16 curCol = 0;

static OVERLAPPED	_overlapped;
static u32			_ovlReadedBytes = 0;
static u32			_ovlWritenBytes = 0;

static void* nandEraseFillArray;
static u32 nandEraseFillArraySize = 0;
static byte nandReadStatus = 0x41;
static u32 lastError = 0;

#else

#pragma diag_suppress 550,177

#endif

static void NandReadData(void *data, u16 len);
static bool CheckDataComplete();

/**************************************************/
/*

���������� ������ ������, ��� ������ ������� ��� ������� ��� �� ���������� ������,
������ �� ���������� �����, ������ - ������.
����� ������� ���� ���� ����������� � ����� ����� �������� ������ ������ �����
����� ����� � ������ ����� ������ ������� �� ������ � ������ � �� ������ �� ��� ������, �� ��� ������� �������������

*/
/**************************************************/

#pragma push
#pragma O3
#pragma Otime

static bool __memcmp(ConstDataPointer s, ConstDataPointer d, u32 len)
{
	while (len > 3)
	{
		if (*s.d++ != *d.d++) return false;
		len -= 4;
	};

	while (len > 0)
	{
		if (*s.b++ != *d.b++) return false;
		len -= 1;
	};

	return true;
}

#pragma pop

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define FLASH_SAVE_BUFFER_SIZE		8400
#define FLASH_READ_BUFFER_SIZE		8400

static FLWB flashWriteBuffer[4];
static FLRB flashReadBuffer[4];

static List<FLWB> freeFlWrBuf;
static List<FLWB> writeFlBuf;

static List<FLRB> freeFlRdBuf;
static List<FLRB> readFlBuf;

static FLWB *curWrBuf = 0;
static FLRB *curRdBuf = 0;

//static SpareArea spareRead;
//static SpareArea spareWrite;
//static SpareArea spareErase;

static u32 pagesRead = 0;
static u32 pagesReadOK = 0;
static u32 pagesReadErr = 0;

static bool cmdCreateNextFile = false;
static bool cmdFullErase = false;
static bool cmdSendSession = false;

static bool writeFlashEnabled = false;
static bool flashFull = false;
static bool flashEmpty = false;

static bool testWriteFlash = false;	

static const bool verifyWritePage = true; // �������� ��������� ��������, ���� ������ �������� � ��������� � �������


//__packed struct SI
//{
//	u16			session;
//	i64			size; //���� 0 �� ������ ������� ���������
//	RTC_type	start_rtc; //���� 0 �� ������ ������� ���������
//	RTC_type	stop_rtc;  
//	FLADR		start_adress; 
//	FLADR		last_adress; 
//	byte		flags;
////	u16			crc;
//};	

static SessionInfo lastSessionInfo;

//static u32 vecCount[8] = {0};

static u64 flashUsedSize = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NVV // NonVolatileVars  
{
	u16 numDevice;

	FileDsc f;

	u16 index;

	u32 prevFilePage;

	u32 badBlocks[8];
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NVSI // NonVolatileSessionInfo  
{
	FileDsc f;

	u16 crc;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NVV nvv;

static NVSI nvsi[128];

static byte buf[sizeof(nvv)*2+4];

byte savesCount = 0;

byte savesSessionsCount = 0;

byte eraseSessionsCount = 0;

//static TWI	twi;

static void SaveVars();

static bool loadVarsOk = false;
static bool loadSessionsOk = false;

static NVSI bbsi[128]; // Black box sessions info

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const SessionInfo* GetLastSessionInfo()
{
	return &lastSessionInfo;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

enum flash_status_type
{
	FLASH_STATUS_WAIT = 0,
	FLASH_STATUS_WRITE,
	FLASH_STATUS_READ,
//	FLASH_STATUS_ERASE
};

static byte flashStatus = FLASH_STATUS_WAIT;

//enum flash_status_operation_type
//{
//	FLASH_STATUS_OPERATION_WAIT = 0,
//	FLASH_STATUS_OPERATION_SAVE_WAIT,
//	FLASH_STATUS_OPERATION_SAVE_WRITE,
//	FLASH_STATUS_OPERATION_SAVE_VERIFY,
//	FLASH_STATUS_OPERATION_READ_WAIT,
//	FLASH_STATUS_OPERATION_READ_HEADER,
//	FLASH_STATUS_OPERATION_READ_DATA,
//};

enum NandState
{
	NAND_STATE_WAIT = 0,
	NAND_STATE_WRITE_START,
	NAND_STATE_READ_START,
	NAND_STATE_FULL_ERASE_START,
	NAND_STATE_FULL_ERASE_0,
	NAND_STATE_CREATE_FILE,
	NAND_STATE_SEND_SESSION
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NandState nandState = NAND_STATE_WAIT;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct NandMemSize
{
 	u64 ch;	// chip
 	u64 fl;	// full
 	u32 bl;	// block
//	u32 row;
	u16 pg;	// page
	u16 mask;
	byte shPg; //(1 << x)
	byte shBl; //(1 << x)
	byte shCh;
	//byte shRow;

	byte bitCol;
	byte bitPage; // 
	byte bitBlock;

	u16	pagesInBlock;


	u16		maskPage;
	u32		maskBlock;

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NandMemSize nandSize;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//static FLADR wr(0, 0, 0, 0);
//static FLADR er(-1, -1, -1, -1);

//static u64	cur_adr;
//static u32 		rd_row = 0;
//static u32 		rd_block = 0;
//static u16 		rd_page = 0;
//static u16		rd_col = 0;
//static byte		rd_chip = 0;

//static byte		er_chip = 0xFF;
//static byte		wr_chip = 0;

//static u16		wr_col = 0;
//static u32 		wr_row = 0;
//static u32 		wr_page = 0;

//static u32 		wr_block = 0;
//static u32 		er_block = 0xFFFFFFFF;

static u32		invalidBlocks = 0;


//static u16	cur_col;
//static u32 	cur_row;
//static byte	cur_chip;
//static u16	cur_size;
//static u16	cur_count;
//static byte*cur_data;
//static u16	cur_verify_errors;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ComPort com1;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetNumDevice(u16 num)
{
	nvv.numDevice = num;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern u16 GetNumDevice()
{
	return nvv.numDevice;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitFlashBuffer()
{
	for (byte i = 0; i < ArraySize(flashWriteBuffer); i++)
	{
		freeFlWrBuf.Add(&flashWriteBuffer[i]);
	};							  

	for (byte i = 0; i < ArraySize(flashReadBuffer); i++)
	{
		freeFlRdBuf.Add(&flashReadBuffer[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FLWB* AllocFlashWriteBuffer()
{
	return freeFlWrBuf.Get();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FLRB* AllocFlashReadBuffer()
{
	return freeFlRdBuf.Get();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FreeFlashReadBuffer(FLRB* b)
{
	freeFlRdBuf.Add(b);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFlashRead(FLRB* b)
{
	if ((b != 0)/* && (b->data != 0) && (b->maxLen > 0)*/)
	{
		b->ready = false;
		b->len = 0;

		readFlBuf.Add(b);

		//b->ready = true;
		//
		//if (b->vecStart)
		//{
		//	b->hdr.crc = 0;
		//	b->hdr.dataLen = 4112;
		//};

		//b->len = b->maxLen;

		return true;
	}
	else
	{
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RequestFlashWrite(FLWB* b)
{
	if ((b != 0) && (b->dataLen > 0))
	{
//		b->ready[0] = false;

		writeFlBuf.Add(b);

		return true;
	}
	else
	{
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_FullErase()
{
	cmdFullErase = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_NextSession()
{
	cmdCreateNextFile = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//inline u32 GetBlock(u32 row, byte chip)
//{
//	return (row >> (nandSize.shBl - nandSize.shPg)) + ((u32)chip << (nandSize.shCh - nandSize.shBl));
//}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define NAND_BASE_PIO_RB 	AT91C_BASE_PIOA
//#define NAND_PIO_RB 		AT91C_PIO_PA27
//#define NAND_BASE_PIO_RE 	AT91C_BASE_PIOB
//#define NAND_PIO_RE 		AT91C_PIO_PB20
//#define NAND_BASE_PIO_CE 	AT91C_BASE_PIOA
//#define NAND_PIO_CE 		AT91C_PIO_PA28
//#define NAND_BASE_PIO_CE2 	AT91C_BASE_PIOA
//#define NAND_PIO_CE2 		AT91C_PIO_PA29
//#define NAND_BASE_PIO_CLE 	AT91C_BASE_PIOB
//#define NAND_PIO_CLE 		AT91C_PIO_PB24
//#define NAND_BASE_PIO_ALE 	AT91C_BASE_PIOB
//#define NAND_PIO_ALE 		AT91C_PIO_PB23
//#define NAND_BASE_PIO_WE 	AT91C_BASE_PIOB
//#define NAND_PIO_WE 		AT91C_PIO_PB22
//#define NAND_BASE_PIO_WP	AT91C_BASE_PIOB
//#define NAND_PIO_WP 		AT91C_PIO_PB21

#define NAND_BASE_PIO_DATA 		AT91C_BASE_PIOA
#define NAND_PIO_DATA_OFFSET	10
#define NAND_PIO_DATA_MASK		0xFF



#define NAND_READ_PACK_BYTES	512
#define NAND_WRITE_PACK_BYTES	256

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define NAND_CMD_RESET			0xFF
#define NAND_CMD_READ_ID		0x90
#define NAND_CMD_READ_1			0x00
#define NAND_CMD_READ_2			0x30
#define NAND_CMD_RANDREAD_1		0x05
#define NAND_CMD_RANDREAD_2		0xE0
#define NAND_CMD_PAGE_PROGRAM_1	0x80
#define NAND_CMD_PAGE_PROGRAM_2	0x10
#define NAND_CMD_READ_STATUS	0x70
#define NAND_CMD_BLOCK_ERASE_1	0x60
#define NAND_CMD_BLOCK_ERASE_2	0xD0

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_SR_FAIL				0x01	// 0 = Pass, 1 = Fail
#define NAND_SR_FAILC				0x02	// 0 = Pass, 1 = Fail
#define NAND_SR_ARDY				0x20	// 0 = Busy, 1 = Ready
#define NAND_SR_RDY					0x40	// 0 = Busy, 1 = Ready
#define NAND_SR_WP					0x80	// 0 = Protected, 1 = Not Protected

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NandID
{
 	byte marker;
 	byte device;
 	byte data[3];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte	FLADR::chipValidNext[NAND_MAX_CHIP]; // ���� ��� �����, �� �� ������� ��������� ��������� ������� ���
byte	FLADR::chipValidPrev[NAND_MAX_CHIP]; // ���� ��� �����, �� �� ������� ��������� ���������� ������� ���
u32		FLADR::chipOffsetNext[NAND_MAX_CHIP]; // ���� ��� �����, �� �� ������� ��������� �������� ������ �� ��������� ������� ���
u32		FLADR::chipOffsetPrev[NAND_MAX_CHIP]; // ���� ��� �����, �� �� ������� ��������� �������� ������ �� ���������� ������� ���

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLADR::InitVaildTables(u16 mask)
{
	u32 blocksize = 1UL << (NAND_COL_BITS + NAND_PAGE_BITS);;

	for(byte chip = 0; chip < NAND_MAX_CHIP; chip++)
	{
		chipValidNext[chip] = 0;
		chipValidPrev[chip] = 0;

		u32 offset = 0;

		for (byte i = 0; i < NAND_MAX_CHIP; i++)
		{
			byte cn = chip+i; if (cn >= NAND_MAX_CHIP) cn = 0;

			if (mask & (1<<cn))
			{
				chipValidNext[chip] = cn;
				chipOffsetNext[chip] = offset;
				break;
			};

			offset += blocksize;
		};

		offset = 0;

		for (byte i = 0; i < NAND_MAX_CHIP; i++)
		{
			byte cp = chip-i; if (cp >= NAND_MAX_CHIP) cp = NAND_MAX_CHIP - 1;

			if (mask & (1<<cp))
			{
				chipValidPrev[chip] = cp;
				chipOffsetPrev[chip] = offset;
				break;
			};
			
			offset += blocksize;
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

volatile byte * const FLC = (byte*)0x60000008;	
volatile byte * const FLA = (byte*)0x60000010;	
volatile byte * const FLD = (byte*)0x60000000;	
//volatile u16 * const FLD16 = (u16*)0x60000000;	
//volatile u32 * const FLD32 = (u32*)0x60000000;	

#else

static byte reg_FLC;
static byte reg_FLA;
static byte reg_FLD;

byte * const FLC = &reg_FLC;	
byte * const FLA = &reg_FLA;	
byte * const FLD = &reg_FLD;	

#endif

static u32 chipSelect[NAND_MAX_CHIP] = { 1<<2, 1<<0, 1<<8, 1<<9, 1<<6, 1<<1, 1<<3, 1<<7 };
static const u32 maskChipSelect = (0xF<<0)|(0xF<<6);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define CMD_LATCH(cmd) { *FLC = cmd; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_LATCH_COL(col) { *FLA = col; *FLA = col >> 8; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_LATCH_ROW(row) { *FLA = row; *FLA = row >> 8; *FLA = row >> 16; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_LATCH_COL_ROW(col, row)	{ ADR_LATCH_COL(col); ADR_LATCH_ROW(row) } 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_READ_CYCLE_PREPARE() /**/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_READ_CYCLE(data) {	data = *FLD; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_WRITE_CYCLE_PREPARE()/* */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_WRITE_CYCLE(data) { *FLD = data; }       

////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashReady()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) != 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashBusy()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) == 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

#define NAND_BUSY() (HW::P14->TBCLR(7))
//#define NAND_BUSY() ((CmdReadStatus() & (1<<6)) == 0)

#else

//#define NAND_BUSY() (false)
#define NAND_BUSY() (!HasOverlappedIoCompleted(&_overlapped))

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void EnableWriteProtect()
{
	HW::P3->BCLR(3);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void DisableWriteProtect()
{
	HW::P3->BSET(3);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte CmdReadStatus()
{
#ifndef WIN32

	CMD_LATCH(NAND_CMD_READ_STATUS);
	return *FLD;

#else

	return (nandReadStatus & ~0x40) | ((NAND_BUSY()) ? 0 : 0x40);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CmdNandBusy()
{
	return NAND_BUSY() || ((CmdReadStatus() & (1<<6)) == 0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Chip_Select(byte chip) 
{    
#ifndef WIN32
	if(chip < NAND_MAX_CHIP)                   
	{ 				
		HW::P1->SET(maskChipSelect ^ chipSelect[chip]);
		HW::P1->CLR(chipSelect[chip]);
	};
#else
	nandChipSelected = chip;
#endif
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Chip_Disable() 
{    
#ifndef WIN32
	HW::P1->SET(maskChipSelect);
#else
	nandChipSelected = ~0;
#endif
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ResetNandState()
{
	nandState = NAND_STATE_WAIT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NandState GetNandState()
{
	return nandState;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ResetNand()
{
#ifndef WIN32

	while(NAND_BUSY());
	CMD_LATCH(NAND_CMD_RESET);
	while(NAND_BUSY());

	for (u32 i = 0; i < 1000; i++) { __nop(); };

#endif

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read_ID(NandID *id)
{
	CMD_LATCH(NAND_CMD_READ_ID);
	*FLA = 0;

	//byte *p = (byte*)id;

	//for(byte i = 0; i < sizeof(NandID); i++) 
	//{ 
	//	*p++ = *FLD;
	//}

	NandReadData(id, sizeof(NandID));

	while (!CheckDataComplete());

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 ROW(u32 block, u16 page)
{
	return (block << NAND_PAGE_BITS) + page;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

static void SetCurAdrNandFile(u16 col, u32 bl, u16 pg)
{
	curBlock = bl;
	curPage = pg;
	curCol = col;

	//bl = ROW(bl, pg);

	u64 adr = bl;
	
	adr = (adr << NAND_CHIP_BITS) + nandChipSelected;

	adr *= (NAND_PAGE_SIZE + NAND_SPARE_SIZE) << NAND_PAGE_BITS;

	adr += (u32)(NAND_PAGE_SIZE + NAND_SPARE_SIZE) * pg;

	adr += col & ((1 << (NAND_COL_BITS+1))-1);

	curNandFilePos = adr;

	//SetFilePointer(handleNandFile, _overlapped.Offset, (i32*)&_overlapped.OffsetHigh, FILE_BEGIN);
}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdEraseBlock(u32 bl)
{
#ifndef WIN32

	bl = ROW(bl, 0);
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_1);
	ADR_LATCH_ROW(bl);
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_2);

#else

	SetCurAdrNandFile(0, bl, 0);

	*((u32*)nandEraseFillArray) = (bl << NAND_CHIP_BITS) + nandChipSelected;

	_overlapped.Offset = (u32)curNandFilePos;
	_overlapped.OffsetHigh = (u32)(curNandFilePos>>32);
	_overlapped.hEvent = 0;
	_overlapped.Internal = 0;
	_overlapped.InternalHigh = 0;

	WriteFile(handleNandFile, nandEraseFillArray, (NAND_PAGE_SIZE+NAND_SPARE_SIZE) << NAND_PAGE_BITS, 0, &_overlapped);

	nandReadStatus = 0;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdRandomRead(u16 col)
{
#ifndef WIN32

	CMD_LATCH(NAND_CMD_RANDREAD_1);
	ADR_LATCH_COL(col);
	CMD_LATCH(NAND_CMD_RANDREAD_2);

#else

	SetCurAdrNandFile(col, curBlock, curPage);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdReadPage(u16 col, u32 bl, u16 pg)
{
#ifndef WIN32

	bl = ROW(bl, pg);
	CMD_LATCH(NAND_CMD_READ_1);
	ADR_LATCH_COL_ROW(col, bl);
	CMD_LATCH(NAND_CMD_READ_2);

#else

	SetCurAdrNandFile(col, bl, pg);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma push
#pragma O3
#pragma Otime

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
#ifndef WIN32

	using namespace HW;

	register u32 t __asm("r0");

	HW::GPDMA1->DMACFGREG = 1;

	HW::GPDMA1_CH3->CTLL = DST_INC|SRC_INC|TT_FC(0)|DEST_MSIZE(0)|SRC_MSIZE(0);
	HW::GPDMA1_CH3->CTLH = BLOCK_TS(len);

	HW::GPDMA1_CH3->SAR = (u32)src;
	HW::GPDMA1_CH3->DAR = (u32)dst;
	HW::GPDMA1_CH3->CFGL = 0;
	HW::GPDMA1_CH3->CFGH = PROTCTL(1);

	HW::GPDMA1->CHENREG = 0x101<<3;

#else

	DataPointer s((void*)src);
	DataPointer d((void*)dst);

	while (len > 3) *d.d++ = *s.d++, len -= 4;

	while (len > 0) *d.b++ = *s.b++, len -= 1;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NandReadData(void *data, u16 len)
{
#ifndef WIN32

	//CopyDataDMA(FLD, data, len);

	using namespace HW;

	HW::GPDMA1->DMACFGREG = 1;

	HW::GPDMA1_CH3->CTLL = DST_INC|SRC_NOCHANGE|TT_FC(0);
	HW::GPDMA1_CH3->CTLH = BLOCK_TS(len);

	HW::GPDMA1_CH3->SAR = (u32)FLD;
	HW::GPDMA1_CH3->DAR = (u32)data;
	HW::GPDMA1_CH3->CFGL = 0;
	HW::GPDMA1_CH3->CFGH = PROTCTL(1);

	HW::GPDMA1->CHENREG = 0x101<<3;

#else

	_overlapped.Offset = (u32)curNandFilePos;
	_overlapped.OffsetHigh = (u32)(curNandFilePos>>32);
	_overlapped.hEvent = 0;
	_overlapped.Internal = 0;
	_overlapped.InternalHigh = 0;

	ReadFile(handleNandFile, data, len, 0, &_overlapped);

	curNandFilePos += len;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NandWriteData(void *data, u16 len)
{
#ifndef WIN32

	using namespace HW;

	HW::GPDMA1->DMACFGREG = 1;

	HW::GPDMA1_CH3->CTLL = DST_NOCHANGE|SRC_INC|TT_FC(0);
	HW::GPDMA1_CH3->CTLH = BLOCK_TS(len);

	HW::GPDMA1_CH3->SAR = (u32)data;
	HW::GPDMA1_CH3->DAR = (u32)FLD;
	HW::GPDMA1_CH3->CFGL = 0;
	HW::GPDMA1_CH3->CFGH = PROTCTL(1);

	HW::GPDMA1->CHENREG = 0x101<<3;

#else

	_overlapped.Offset = (u32)curNandFilePos;
	_overlapped.OffsetHigh = (u32)(curNandFilePos>>32);
	_overlapped.hEvent = 0;
	_overlapped.Internal = 0;
	_overlapped.InternalHigh = 0;

	WriteFile(handleNandFile, data, len, 0, &_overlapped);

	curNandFilePos += len;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CheckDataComplete()
{
#ifndef WIN32

	return (HW::GPDMA1->CHENREG & (1<<3)) == 0;

#else

	return HasOverlappedIoCompleted(&_overlapped);

#endif
}

#pragma pop

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdWritePage(u16 col, u32 bl, u16 pg)
{
#ifndef WIN32

	bl = ROW(bl, pg);
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_1);
	ADR_LATCH_COL_ROW(col, bl);

#else

	SetCurAdrNandFile(col, bl, pg);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdWritePage2()
{
#ifndef WIN32
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_2);
#else

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct EraseBlock
{
	enum {	WAIT = 0,ERASE_START,ERASE_0,ERASE_1,ERASE_2,ERASE_3,ERASE_4,ERASE_5 };

	SpareArea spare;
	
	FLADR er;

	byte state;
	bool force;		// ������� ��������
	bool check;		// ��������� ��������� ��������
	u16	errBlocks;

	EraseBlock() : state(WAIT), force(false), check(true), errBlocks(0), er(-1, -1, -1, -1) {}

	void Start(const FLADR &rd, bool frc, bool chk);
	bool Update();
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void EraseBlock::Start(const FLADR &rd, bool frc, bool chk)
{
	er = rd;

	force = frc;
	check = chk;

	errBlocks = 0;

	state = ERASE_START;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool EraseBlock::Update()
{
	byte t;

	switch(state)
	{
		case WAIT:
			
			return false;

		case ERASE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++		
																																
			NAND_Chip_Select(er.chip);

			if (force)
			{
				CmdEraseBlock(er.block);																						
																															
				state = ERASE_2;																				
			}
			else
			{
				CmdReadPage(er.pg, er.block, 0);																					
																																	
				state = ERASE_0;																						
			};
																																
			break;																												
																																
		case ERASE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++				
																																
			if(!NAND_BUSY())																									
			{																													
				NandReadData(&spare, spare.CRC_SKIP);	

				state = ERASE_1;																					
			};																													
																																
			break;																												
																																
		case ERASE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++				
																																
			if (CheckDataComplete())																							
			{																													
				if (spare.validPage != 0xFFFF && spare.validBlock != 0xFFFF)									
				{																												
					errBlocks += 1;																							
					nvv.badBlocks[er.chip] += 1;
																																
					er.NextBlock();	

					state = ERASE_START; 
				}	
				else if (spare.badPages != 0xFFFF)
				{
					errBlocks += 1;	
																																
					CmdWritePage(er.pg, er.block, 0);																			
																																
					*FLD = 0; *FLD = 0; *FLD = 0; *FLD = 0;	// spareErase.validPage = 0; spareErase.validBlock = 0;																
																																
					CmdWritePage2();																							
																																
					state = ERASE_3;																				
				}
				else																											
				{																												
					CmdEraseBlock(er.block);																						
																																
					state = ERASE_2;																				
				};																												
			};																													
																																
			break;																												
																																
																																
		case ERASE_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++				
																																
			if(!NAND_BUSY()/* && ((t = CmdReadStatus()) & (1<<6))*/)																									
			{																													
				byte status = CmdReadStatus();

				if ((status & (NAND_SR_FAIL|NAND_SR_RDY)) != NAND_SR_RDY && check) // erase error																	
				{																												
					errBlocks += 1;	
					nvv.badBlocks[er.chip] += 1;

					CmdWritePage(er.pg, er.block, 0);																			
																																
					*FLD = 0; *FLD = 0; *FLD = 0; *FLD = 0;	// spareErase.validPage = 0; spareErase.validBlock = 0;																
																																
					CmdWritePage2();																							
																																
					state = ERASE_3;																				
				}																												
				else																											
				{																												
					//er_block = wr.block;														
					//er_chip = wr.chip;																							
																																
					state = WAIT;		

					return false;
				};																												
			};																													
																																
			break;																												
																																
		case ERASE_3:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++				
																																
			if(!NAND_BUSY())																									
			{																													
				er.NextBlock();	

				state = ERASE_START; 
			};

			break;
	};

	return true;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Write
{
	enum {	WAIT = 0,				WRITE_START,			WRITE_BUFFER,			WRITE_PAGE,				WRITE_PAGE_0,	WRITE_PAGE_1,
			WRITE_PAGE_2,			WRITE_PAGE_3,			WRITE_PAGE_4,			WRITE_PAGE_5,			WRITE_PAGE_6,	WRITE_PAGE_7,	WRITE_PAGE_8,			
			ERASE,			
			WRITE_CREATE_FILE_0,	WRITE_CREATE_FILE_1,	WRITE_CREATE_FILE_2,	WRITE_CREATE_FILE_3,	WRITE_CREATE_FILE_4 };

	FLADR wr;

	//u16		wr_cur_col;
	//u32 	wr_cur_pg;

	byte	wr_pg_error;
	u16		wr_count;
	byte*	wr_data	;
	void*	wr_ptr	;

	u64		prWrAdr;
	u32		rcvVec ;
	u32		rejVec ;
	u32		errVec ;

	SpareArea spare;

	SpareArea rspare;

	byte state;

	bool createFile;

	EraseBlock eraseBlock;

	u32	wrBuf[2048/4];
	//u32	rdBuf[2112/4];


	bool Start();
	bool Update();
	void Vector_Make(VecData *vd, u16 size);
	void Finish();
	void Init();
	void Init(u32 bl, u32 file, u32 prfile);

	void SaveSession();

//	void BufWriteData(void *data, u16 len) { CopyDataDMA(data, (byte*)wrBuf+wr.col, len); }

	void CreateNextFile() { state = WRITE_CREATE_FILE_0; }


};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Write write;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Write::Init()
{
	wr.SetRawPage(nvv.f.lastPage);

	spare.file = nvv.f.session;  

	spare.prev = nvv.prevFilePage;		

	spare.start = nvv.f.startPage;		
	spare.fpn = 0;	

	spare.vectorCount = 0;

	spare.vecFstOff = -1;
	spare.vecFstLen = 0;

	spare.vecLstOff = -1;
	spare.vecLstLen = 0;

	spare.fbb = 0;		
	spare.fbp = 0;		

	spare.chipMask = nandSize.mask;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Write::Init(u32 bl, u32 file, u32 prfile)
{
	wr.SetRawBlock(bl);

	spare.file = file;  
//	spare.lpn = wr.GetRawPage();

	spare.prev = prfile;		

	spare.start = wr.GetRawPage();		
	spare.fpn = 0;	

	spare.vectorCount = 0;

	spare.vecFstOff = -1;
	spare.vecFstLen = 0;

	spare.vecLstOff = -1;
	spare.vecLstLen = 0;

	spare.fbb = 0;		
	spare.fbp = 0;		

	spare.chipMask = nandSize.mask;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Write::SaveSession()
{
	nvv.index = (nvv.index+1) & 127;

	FileDsc &s = nvv.f;
	NVSI &d = nvsi[nvv.index];

	d.f = nvv.f;

	d.crc = GetCRC16(&d.f, sizeof(d.f));

	s.session = spare.file;
	s.startPage = s.lastPage = wr.GetRawPage();
	s.size = 0;
	GetTime(&s.start_rtc);
	s.stop_rtc = s.start_rtc;
	s.flags = 0;

	SaveParams();
	savesSessionsCount = 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Write::Vector_Make(VecData *vd, u16 size)
{
	vd->h.session = spare.file;
	vd->h.device = 0;
	GetTime(&vd->h.rtc);

	vd->h.prVecAdr = prWrAdr; 
	vd->h.flags = 0;
	vd->h.dataLen = size;
	vd->h.crc = GetCRC16(&vd->h, sizeof(vd->h) - sizeof(vd->h.crc));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Write::Start()
{
	if ((curWrBuf = writeFlBuf.Get()) != 0)
	{
		rcvVec += 1;

		if (!writeFlashEnabled || flashFull)
		{
//			rejVec += 1;

//			curWrBuf->ready[0] = true;
			freeFlWrBuf.Add(curWrBuf);
			return false;
		};

		Vector_Make(&curWrBuf->vd, curWrBuf->dataLen); 

//		curWrBuf->dataLen += sizeof(curWrBuf->vd.vec);
//		curWrBuf->hdrLen = 0;

		
		prWrAdr = wr.GetRawAdr();

		wr_data = (byte*)&curWrBuf->vd;
		wr_count = curWrBuf->dataLen + sizeof(curWrBuf->vd.h);

//		wr_cur_col = wr.col;
//		wr_cur_pg = wr.GetRawPage();

		if (spare.vecFstOff == 0xFFFF)
		{
			spare.vecFstOff = wr.col;
			spare.vecFstLen = wr_count;
		};

		spare.vecLstOff = wr.col;
		spare.vecLstLen = wr_count;

		spare.vectorCount += 1;

		state = WRITE_START;

		return true;
	}
	else
	{
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Write::Finish()
{
	NAND_Chip_Disable();

	if (curWrBuf != 0)
	{
		nvv.f.size += curWrBuf->vd.h.dataLen;
		nvv.f.stop_rtc = curWrBuf->vd.h.rtc;
		nvv.f.lastPage = spare.rawPage;

		SaveParams();

//		curWrBuf->ready[0] = true;

		freeFlWrBuf.Add(curWrBuf);

		curWrBuf = 0;

		//wr_prev_col = wr_cur_col;
		//wr_prev_pg = wr_cur_pg;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Write::Update()
{
//	u32 t;

	switch(state)
	{
		case WAIT:	return false;

		case WRITE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
			
//			NAND_Chip_Select(wr.chip);

			{
				register u16 c = wr.pg - wr.col;


				if (wr.col == 0 && wr_count >= wr.pg) // ������ ����� �� ����
				{
					wr_ptr = wr_data;
					wr_count -= wr.pg;
					wr_data += wr.pg;

					state = WRITE_PAGE;
				}
				else // ������ � �����
				{
					if (wr_count < c ) c = wr_count;

		HW::P5->BSET(1);

					CopyDataDMA(wr_data, (byte*)wrBuf+wr.col, c);	// BufWriteData(wr_data, c);

					wr.col += c;
					wr_data += c;
					wr_count -= c;

					state = WRITE_BUFFER;
				};
			};

			break;
	
		case WRITE_BUFFER:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{

		HW::P5->BCLR(1);

				if (wr.col == 0)
				{
					wr_ptr = wrBuf;

					state = WRITE_PAGE;
				}
				else if (wr_count > 0)
				{
					state = WRITE_START;
				}
				else
				{
					Finish();

					state = WAIT;

					return false;
				};
			};

			break;

		case WRITE_PAGE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(eraseBlock.er.GetRawBlock() != wr.GetRawBlock())	// ����� ����
			{
				wr_pg_error = 0;

				eraseBlock.Start(wr, true, true);

	            state = ERASE;

				break;
			};

		case WRITE_PAGE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			NAND_Chip_Select(wr.chip);

			CmdWritePage(0, wr.block, wr.page);

			NandWriteData(wr_ptr, wr.pg);
			
			state = WRITE_PAGE_1;

			break;

		case WRITE_PAGE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				spare.validPage = -1;
				spare.validBlock = -1;
				spare.badPages = -1;
				spare.rawPage = wr.GetRawPage();
				spare.chipMask = nandSize.mask;

				spare.crc = GetCRC16((void*)&spare.file, sizeof(spare) - spare.CRC_SKIP - sizeof(spare.crc));

				NandWriteData(&spare, sizeof(spare));

				state = WRITE_PAGE_2;
			};

			break;

		case WRITE_PAGE_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				CmdWritePage2();

				state = WRITE_PAGE_3;
			};

			break;

		case WRITE_PAGE_3:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				byte status = CmdReadStatus();

				if ((status & (NAND_SR_FAIL|NAND_SR_RDY)) != NAND_SR_RDY) // program error
				{
					spare.fbp += 1;

					CmdWritePage(wr.pg, wr.block, wr.page);

					*FLD = 0; *FLD = 0;  // spareErase.validPage = 0;

					CmdWritePage2();

					state = WRITE_PAGE_4;
				}
				else if (verifyWritePage)
				{
					CmdReadPage(wr.pg, wr.block, wr.page);
					
					state = WRITE_PAGE_6;
				}
				else
				{
					state = WRITE_PAGE_8;
				};
			};

			break;

		case WRITE_PAGE_4:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			

			if(!NAND_BUSY())																									
			{																													
				if (wr_pg_error == 2) // �������� ���� ��� ������� ������� ��������
				{
					CmdWritePage(wr.pg + sizeof(spare.validPage) + sizeof(spare.validBlock), wr.block, 0);

					*FLD = 0; *FLD = 0;  // spare.badPages = 0;

					CmdWritePage2();

					state = WRITE_PAGE_5;
				}
				else
				{
					state = WRITE_PAGE;																			
				};

				wr_pg_error += 1;

				wr.NextPage();		
			};																													

			break;

		case WRITE_PAGE_5:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			
																																
			if(!NAND_BUSY())																									
			{																													
				state = WRITE_PAGE;																			
			};
																																
			break;	

		case WRITE_PAGE_6:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				NandReadData(&rspare, sizeof(rspare));

				state = WRITE_PAGE_7;
			};

			break;	

		case WRITE_PAGE_7:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			

			if (CheckDataComplete())
			{
				if (!__memcmp(&spare, &rspare, sizeof(spare)))
				{
					spare.fbp += 1;

					CmdWritePage(wr.pg, wr.block, wr.page);

					*FLD = 0; *FLD = 0;  // spareErase.validPage = 0;

					CmdWritePage2();

					state = WRITE_PAGE_4;
				}
				else
				{
					state = WRITE_PAGE_8;
				};
			};

			break;

		case WRITE_PAGE_8:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (wr_count == 0)
			{
				Finish();

				state = (createFile) ? WRITE_CREATE_FILE_1 : WAIT;
			}
			else
			{
				state = WRITE_START;
			};

			wr.NextPage();

			spare.fpn += 1;

			spare.vecFstOff = -1;
			spare.vecFstLen = 0;

			spare.vecLstOff = -1;
			spare.vecLstLen = 0;

			break;	

		case ERASE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			
					
			if (!eraseBlock.Update())
			{
				wr = eraseBlock.er;

				spare.fbb += eraseBlock.errBlocks;

				state = WRITE_PAGE_0;																			
			};
																																
			break;																												

		case WRITE_CREATE_FILE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (wr.col > 0)
			{
				wr_ptr = wrBuf;
				wr_count = 0;

				byte *p = (byte*)wrBuf;

				p += wr.col;

				for (u16 i = wr.col&3; i > 0; i--)
				{
					*(p++) = 0xff;
				};

				u32 *d = wrBuf + (wr.col+3)/4;

				for (u16 i = (sizeof(wrBuf)-wr.col)/4; i > 0; i--)
				{
					*(d++) = -1;
				};

				createFile = true;

				state = WRITE_PAGE_0;
				
				break;
			};


		case WRITE_CREATE_FILE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			spare.file += 1;  		

			spare.prev = spare.start;		

			if (wr.page > 0)
			{
				wr.NextBlock();
			};

			spare.start = wr.GetRawPage();		
			spare.fpn = 0;
			spare.vectorCount = 0;

			//wr_prev_pg = -1;
			//wr_prev_col = 0;

			spare.fbb = 0;		
			spare.fbp = 0;		

			spare.chipMask = nandSize.mask;	

			SaveSession();

			createFile = false;

			state = WAIT;

			return false;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ReadSpare
{
	enum { WAIT = 0, START, READ_1, READ_2, READ_3 };

	SpareArea	*spare;
	FLADR		*rd;

	u32			badBlocks[8];

	byte state;

	ReadSpare() : spare(0), rd(0) {}

	bool Start(SpareArea *sp, FLADR *frd);
	bool Update();
	void ClearBadBlocks() { for (u32 i = 0; i < ArraySize(badBlocks); i++) badBlocks[i] = 0; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ReadSpare::Start(SpareArea *sp, FLADR *frd)
{
	if (sp == 0 || frd == 0)
	{
		return false;
	};

	spare = sp;
	rd = frd;

	state = START;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ReadSpare::Update()
{
	switch(state)
	{
		case WAIT:

			return false;

		case START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			NAND_Chip_Select(rd->chip);

			CmdReadPage(rd->pg, rd->block, rd->page);

			state = READ_1;

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				NandReadData(spare, sizeof(*spare));

				state = READ_2;
			};

			break;

		case READ_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (spare->validBlock != 0xFFFF)
				{
					rd->NextBlock();

					NAND_Chip_Select(rd->chip);
					CmdReadPage(rd->pg, rd->block, rd->page);

					state = READ_1;
				}				
				else if (spare->validPage != 0xFFFF)
				{
					rd->NextPage();

					NAND_Chip_Select(rd->chip);
					CmdReadPage(rd->pg, rd->block, rd->page);

					state = READ_1;
				}
				else
				{
					spare->crc = GetCRC16((void*)&spare->file, sizeof(*spare) - spare->CRC_SKIP);
				
					state = WAIT;

					return false;
				};
			};

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace Read
{
	enum {	WAIT = 0,READ_START,READ_1, /*READ_2,*/ READ_PAGE,READ_PAGE_1,FIND_START,FIND_1,/*FIND_2,*/FIND_3,FIND_4};

	static FLADR rd(0, 0, 0, 0);
	static byte*	rd_data = 0;
	static u16		rd_count = 0;
	static u16		findTryCount;

	static u32 		sparePage = -1;

	static SpareArea spare;

	static ReadSpare readSpare;

	static bool vecStart = false;

	static byte state;

	static bool Start();
//	static bool Start(FLRB *flrb, FLADR *adr);
	static bool Update();
	static void End() { NAND_Chip_Disable(); curRdBuf->ready = true; curRdBuf = 0; state = WAIT; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read::Start()
{
	if ((curRdBuf = readFlBuf.Get()) != 0)
	{
		if (curRdBuf->useAdr) { rd.SetRawAdr(curRdBuf->adr); };

		vecStart = curRdBuf->vecStart;

		if (vecStart)
		{
			rd_data = (byte*)&curRdBuf->hdr;
			rd_count = sizeof(curRdBuf->hdr);
			curRdBuf->len = 0;	
		}
		else
		{
			rd_data = curRdBuf->data;
			rd_count = curRdBuf->maxLen;
			curRdBuf->len = 0;	
		};

		state = READ_START;

		return true;
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma push
#pragma O0
//#pragma Otime

static bool Read::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case READ_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			NAND_Chip_Select(rd.chip);

			if (rd.GetRawPage() != sparePage)
			{
				readSpare.Start(&spare, &rd);

				state = READ_1;
			}
			else
			{
				CmdRandomRead(rd.col);
				//CmdReadPage(rd.col, rd.block, rd.page);

				state = READ_PAGE;
			};

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (!readSpare.Update()) //(!NAND_BUSY())
			{
				sparePage = rd.GetRawPage();

				CmdRandomRead(rd.col);

				state = READ_PAGE;
			};

			break;

		case READ_PAGE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				register u16 c = rd.pg - rd.col;

				if (rd_count < c) c = rd_count;

				NandReadData(rd_data, c);

				rd_count -= c;
				rd.raw += c;
				rd_data += c;
				curRdBuf->len += c;

				state = READ_PAGE_1;
			};

			break;

		case READ_PAGE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd_count == 0)
				{
					if (vecStart)
					{
						curRdBuf->hdr.crc = GetCRC16(&curRdBuf->hdr, sizeof(curRdBuf->hdr));

						if (curRdBuf->hdr.crc == 0)
						{
							rd_data = curRdBuf->data;
							rd_count = (curRdBuf->hdr.dataLen > curRdBuf->maxLen) ? curRdBuf->maxLen : curRdBuf->hdr.dataLen;
							curRdBuf->len = 0;	
							vecStart = false;

							if (rd_data == 0 || rd_count == 0)
							{
								End();

								return false;
							}
							else
							{
								state = READ_START;
							};
						}
						else
						{
							// ������ ������

							findTryCount = 1024;

							state = FIND_START;
						};
					}
					else
					{
						End();

						return false;
					};
				}
				else
				{
					state = READ_START;
				};
			};

			break;

		case FIND_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

//			__breakpoint(0);

			if (spare.start == -1 || spare.fpn == -1)
			{
				if (findTryCount == 0)
				{
					// ������� ���������
					state = FIND_3;
				}
				else
				{
					findTryCount -= 1;

					rd.NextPage();

					readSpare.Start(&spare, &rd);

					state = FIND_1;
				};
			}
			else if (spare.crc != 0 || spare.vecFstOff == 0xFFFF || spare.vecLstOff == 0xFFFF || rd.col > spare.vecLstOff)
			{
				rd.NextPage();

				readSpare.Start(&spare, &rd);

				state = FIND_1;
			}
			else 
			{
				if (rd.col <= spare.vecFstOff)
				{
					rd.col = spare.vecFstOff;
				}
				else if (rd.col <= (spare.vecFstOff+spare.vecFstLen))
				{
					rd.col = spare.vecFstOff+spare.vecFstLen;
				}
				else if (rd.col <= spare.vecLstOff)
				{
					rd.col = spare.vecLstOff;
				};

				rd_data = (byte*)&curRdBuf->hdr;
				rd_count = sizeof(curRdBuf->hdr);
				curRdBuf->len = 0;	

				state = READ_START;
			};

			break;

		case FIND_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (!readSpare.Update())	//(!NAND_BUSY())
			{
				sparePage = rd.GetRawPage();

				state = FIND_START;
			};

			break;

		case FIND_3:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			curRdBuf->len = 0;
			curRdBuf->hdr.dataLen = 0;

			End();

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pop



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
struct BuildFileTable
{
	enum {	WAIT = 0,START,READ_1, READ_2, READ_3,READ_4,NO_FILE};

	FLADR rd;
	//FLADR rs(nandSize, 0, 0, 0, 0);
	//FLADR re(nandSize, 0, 0, 0, 0);

	u32 bs;
	u32 be;
	u32 bm;
	u32 endBlock;

	SpareArea spare;

	byte state;

	FileDsc curf;
	FileDsc prf;
	FileDsc lastf;

	ReadSpare readSpare;
		
	BuildFileTable(NandMemSize &v) : rd(v, 0, 0, 0, 0) {}

	bool Start();
	bool Update();

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool BuildFileTable::Start()
{
	//rd.block = 1 << (nandSize.bitBlock-1);
	//rd.page = 0;
	//rd.chip = 0;

	bs = 0;
	be = endBlock = ((rd.sz.ch << NAND_CHIP_BITS) >> rd.sz.shBl) - 1;
	bm = (be+bs)/2;

	rd.SetRawBlock(bm);

	state = START;

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool BuildFileTable::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			readSpare.Start(&spare, &rd);

			state = READ_1;

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!readSpare.Update())
			{
				bm = rd.GetRawBlock();

				if (be <= bs || bm > be || bm < bs)
				{
					rd.SetRawBlock(bs);
					readSpare.Start(&spare, &rd);

					state = READ_2;
				}
				else
				{
					if (spare.lpn == -1 || spare.start == -1 || spare.fpn == -1 )
					{
						be = bm;
					}
					else
					{
						bs = bm;
					};

					bm = (bs + be) / 2;

					rd.SetRawBlock(bm);

					state = START;
				};
			};

			break;

		case READ_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!readSpare.Update())
			{
				if (bs >= endBlock || bm < bs)
				{
					// ����� ��������� ���������
				}
				else if (spare.lpn == -1 || spare.start == -1 || spare.fpn == -1)
				{
					// ����� ������

					Write::spare.file = -1;
					Write::spare.start = -1;
					Write::spare.fpn = 0;	
					Write::spare.prev = -1;

					state = WAIT;

					return false;

				}
				else if (spare.crc == 0)
				{
					Write::spare.file = spare.file;
					Write::spare.start = spare.start;
					Write::spare.fpn = spare.fpn;	
					Write::spare.prev = spare.prev;

					state = WAIT;

					return false;

					//curf.num = spare.file;
					//curf.start = spare.start;
					//curf.prev = spare.prev;
					//curf.vecCount = 0;

					//rd.SetRawPage(curf.start);

					//state = READ_2;

					//state = READ_3;
				}
				else
				{

				};
			}

			break;

		case READ_3:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!readSpare.Update())
			{
				if (spare.crc != 0)
				{
					state = START;
				}
				else
				{
					if (spare.file == curf.num)
					{
						rd.SetRawPage(curf.prev);

						state = READ_2;
					}
					else
					{
						lastf.num = spare.file;
						lastf.prev = spare.prev;
						lastf.start = spare.start;

						state = READ_4;
					};
				};
			};

			break;

		case READ_4:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd.col >= rd.pg)
				{
					rd.NextPage();

					state = START;
				}
				else
				{
					curRdBuf->ready = true;

					curRdBuf = 0;

					state = WAIT;

					return false;
				};
			};

			break;

		case NO_FILE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static BuildFileTable bft(nandSize);
*/
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReadSpareNow(SpareArea *spare, FLADR *rd, bool crc)
{
	//SpareArea *spare;
	//FLADR *rd;

	NAND_Chip_Select(rd->chip);

	CmdReadPage(rd->pg, rd->block, rd->page);

//	while(!NAND_BUSY());
	while(NAND_BUSY()) { HW::WDT->Update(); };

	NandReadData(spare, sizeof(*spare));

	while (!CheckDataComplete()) { HW::WDT->Update(); };

	if (crc)
	{
		spare->crc = GetCRC16((void*)&spare->file, sizeof(*spare) - spare->CRC_SKIP);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReadDataNow(void *dst, u16 len, FLADR *rd)
{
	NAND_Chip_Select(rd->chip);

	CmdReadPage(rd->col, rd->block, rd->page);

//	while(!NAND_BUSY());
	while(NAND_BUSY()) { HW::WDT->Update(); };

	NandReadData(dst, len);

	while (!CheckDataComplete()) { HW::WDT->Update(); };
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReadVecHdrNow(VecData::Hdr *h, FLADR *rd)
{
	ReadDataNow(h, sizeof(*h), rd);

	h->crc = GetCRC16(h, sizeof(*h));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*static void Test()
{
	FLADR rd(0, 0, 0, 0);

	SpareArea spare;

	rd.SetRawPage(0);

	while (1)
	{
		ReadSpareNow(&spare, &rd, false);

		if (spare.validBlock != 0xFFFF)
		{
			rd.NextBlock();
		}
		else if (spare.validPage != 0xFFFF)
		{
			rd.NextPage();
		}
		else
		{
			if (spare.rawPage != rd.GetRawPage())
			{
				__breakpoint(0);
			};

			rd.NextPage();

			//if (rd.overflow != 0)
			//{
			//	__breakpoint(0);
			//	break;
			//};
		};
	};

}*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void Test2()
//{
//	FLADR rd(0, 0, 0, 0);
//
//	SpareArea spare;
//
//	rd.SetRawPage(0);
//
//	while (1)
//	{
//		ReadSpareStart(&spare, &rd);
//
//		while (ReadSpareUpdate());
//
//		if (spare.rawPage != rd.GetRawPage())
//		{
//			__breakpoint(0);
//		};
//
//		rd.NextPage();
//
//		//if (rd.overflow != 0)
//		//{
//		//	__breakpoint(0);
//		//	break;
//		//};
//	};
//
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitSessions()
{
//	__breakpoint(0);

	write.Init();

	if (nvv.f.size > 0)
	{
//		NAND_NextSession();

		write.CreateNextFile();

		while (write.Update()) ;
	};

	u32 ms = 0, me = 0, ls = -1;
	u32 sp = 0;

	bool bm = false, bl = false;

	//FLADR sb(0), eb(-1); // free start and end block

	for (u16 i = 128, ind = nvv.index; i > 0; i--, ind = (ind-1)&127)
	{
		FileDsc &f = nvsi[ind].f;

		if (f.size == 0) continue;

		if (bl)
		{
			if (f.lastPage < f.startPage)
			{
				f.size = 0;
			}
			else
			{
				if (f.lastPage < ls)
				{
					ls = f.startPage;

					if (bm)
					{
						if (f.lastPage <= me)
						{
							f.size = 0;
						}
						else if (f.startPage < me)
						{
							f.startPage = me+1;
						};
					};
				}
				else
				{
					f.size = 0;
				};
			};
		}
		else
		{
			if (f.size >= FLASH_Full_Size_Get())
			{
				bl = true;
				ls = 0;
			}
			else if (f.lastPage < f.startPage)
			{
				bl = true;
				ls = f.startPage;
				sp = 0;
			}
			else
			{
				sp = f.startPage;
			};

			if (!bm)
			{
				bm = true;
				ms = sp;
				me = f.lastPage;
			}
			else if (f.lastPage < ms)
			{
				ms = sp;
			}
			else if (f.lastPage > me)
			{
				f.startPage = me+1;
				me = f.lastPage;
			}
			else
			{
				f.size = 0;
			};
		};



	}; // 	for (u16 i = 128, ind = nvv.index; i > 0; i--, ind = (ind-1)&127)
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitSessionsNew()
{
	//__breakpoint(0);

	SpareArea spare;
	ReadSpare rdspr;

	write.Init();

	if (nvv.f.size > 0)
	{
		write.CreateNextFile();

		while (write.Update()) ;
	};

	FLADR firstSessionAdr = write.wr;

	u16 firstSessionNum = 0;
	bool firstSessionValid = false;

	u16 count = 100;

	while (count--)
	{
		rdspr.Start(&spare, &firstSessionAdr);	while (rdspr.Update()) HW::WDT->Update();

		if (spare.crc == 0)
		{ 
			firstSessionNum = spare.file; 
			firstSessionValid = true; 
			break; 
		};
	};

	bool c = false;

	flashUsedSize = 0;

	for (u16 i = 128, ind = nvv.index; i > 0; i--, ind = (ind-1)&127)
	{
		FileDsc &f = nvsi[ind].f;

		if (firstSessionValid && f.session == firstSessionNum)
		{
			f.startPage = firstSessionAdr.GetRawPage();
			//f.size = ???;

			firstSessionValid = false;
			c = true;
		}
		else if (c)
		{
			f.size = 0;
		}
		else if (f.size != 0)
		{
			FLADR adr(f.startPage);

			count = 100;

			while (count--)
			{
				rdspr.Start(&spare, &adr);	while (rdspr.Update()) HW::WDT->Update();

				if (spare.crc == 0 ) break;

				adr.NextPage();
			};

			if (spare.crc != 0 || f.session != spare.file)
			{
				f.size = 0;
			};
		};

		flashUsedSize += f.size;
	}; 

	u64 fullSize = FLASH_Full_Size_Get();

	if (flashUsedSize > fullSize) flashUsedSize = fullSize;

	NAND_Chip_Disable();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//static void InitSessions()
//{
//	__breakpoint(0);
//
//	write.Init();
//
//	if (nvv.f.size > 0)
//	{
////		NAND_NextSession();
//
//		write.CreateNextFile();
//
//		while (write.Update()) ;
//	};
//
//	SpareArea spare;
//	FLADR rd;
//
//	u16 ps = 0;
//
//	for (u16 i = 128, ind = nvv.index; i > 0; i--, ind = (ind-1)&127)
//	{
//		FileDsc &f = nvsi[ind].f;
//
//		if (f.size == 0) continue;
//
//		bool c = false;
//
//		rd.SetRawPage(f.lastPage);
//
//		for (u16 j = 1024; j > 0; j--)
//		{
//			ReadSpareNow(&spare, &rd, true);
//
//			if (spare.validPage != 0xFFFF || spare.crc != 0)
//			{
//				rd.PrevPage();
//			}
//			else
//			{
//				if (f.session != spare.file)
//				{
//					f.size = 0;
//
//					c = true;
//				};
//
//				break;
//			};
//		};
//
//		if (c) continue;
//
//		rd.SetRawPage(f.startPage);
//
//		c = false;
//
//		for (u16 j = 1024; j > 0; j--)
//		{
//			ReadSpareNow(&spare, &rd, true);
//
//			if (spare.validBlock != 0xFFFF )
//			{
//				rd.NextBlock();
//			}
//			if (spare.validPage != 0xFFFF || spare.crc != 0)
//			{
//				rd.NextPage();
//			}
//			else
//			{
//				if (f.session == spare.file)
//				{
//					f.startPage = rd.GetRawPage();
//
//					c = true;
//
//					break;
//				}
//				else
//				{
//					if (!c)
//					{
//						ps = spare.file;
//						c = true;
//					}
//					else if (ps != spare.file)
//					{
//						f.size = 0;
//						break;
//					};
//
//					rd.NextBlock();
//				};
//			};
//		};
//	};
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool UpdateBlackBoxSendSessions()
{
	//enum {	WAIT = 0, FIND_LAST_USED_BLOCK, READ_START, READ_1, READ_2, READ_PAGE,READ_PAGE_1,FIND_START,FIND_1,FIND_2,FIND_3,FIND_4, READ_END};

	static byte state = 0;

	static FLADR rd;
	static FLADR adr;

	static u32 bs;
	static u32 be;
	static u32 bm;

	static u32 ps;
	static u32 pe;
	static u32 pm;

	static SpareArea spare;

	static ReadSpare readSpare;

	static FLRB flrb;

	//static u32 sessionFirstPage = -1;
	//static u32 sessionLastBlock = -1;
	//static u32 sessionLastPage = -1;
	//static u64 sessionFirsVector = -1;
	//static u64 sessionLastVector = -1;

	//static u32 lastValidBlock = ~0;
	//static u16 lastFileNum = ~0;
	//static u32 lastFileStartPage = ~0;

	static u32	firstSessionBlock = ~0;
	static u32	firstSessionLastBlock = ~0;
	static u16	firstSessionNum = ~0;
	static bool firstSessionValid = false;
	static u64	firstSessionStartAdr = 0;
	static u64	firstSessionLastAdr = 0;
	static bool firstSessionSended = false;

	//static u32	curFileFirstBlock = ~0;
//	static u32	curFileLastBlock = ~0;
	static u16	curFileNum = ~0;
	static u64	curFileStartAdr = ~0;
	static u64	curFileEndAdr = ~0;
	//static u32	curFileFPN = ~0;
	static bool	sendedFileRes = false;
	static u16	sendedFileNum = 0;

	static u32	lastSessionBlock = ~0;
	//static u16	lastSessionNum = ~0;
	//static bool lastSessionValid = false;

	static u32	findFileLastBlock = ~0;
	static u64	findFileStartAdr = 0;
	static u64	findFileEndAdr = 0;
	static u16	findFileNum = ~0;

	//static u32 initFileNum = 0;
	//static u32 initFileStartPage = 0;
	static u32 prgrss = 0;

	static RTC start_rtc;
	static RTC stop_rtc;

	static TM32 tm;

	static u32 count = 0;
	static u32 countFindVec = 0;

	static bool findVector = false;

	const u32 * const bb = readSpare.badBlocks;

	bool result = true;

	switch (state)
	{
		case 0: // ����� ��������� �� ����� ���� � ���������

			if (cmdSendSession)
			{
				prgrss = 0;

				state++;
			}
			else
			{
				return false;
			};

			break;

		case 1: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


			rd.SetRawBlock(0);

			readSpare.ClearBadBlocks();

			readSpare.Start(&spare, &rd);

			firstSessionBlock = ~0;
			firstSessionLastBlock = ~0;
			firstSessionNum = ~0;
			firstSessionValid = false;
			firstSessionStartAdr = 0;
			firstSessionLastAdr = 0;
			firstSessionSended = false;

//			curFileFirstBlock = ~0;
//			curFileLastBlock = ~0;
			curFileNum = ~0;
			curFileStartAdr = ~0;
			curFileEndAdr = ~0;
//			curFileFPN = ~0;
			sendedFileRes = false;
			sendedFileNum = 0;

			lastSessionBlock = ~0;
			//lastSessionNum = ~0;

			findVector = false;

			count = (NAND_RAWBLOCK_MASK+1)*2;

			state++;

			break;

		case 2: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!readSpare.Update())
			{
				if (spare.crc == 0)
				{
					if (!firstSessionValid)
					{
						if (firstSessionBlock == ~0)
						{
							firstSessionBlock = rd.GetRawBlock();
							firstSessionNum = spare.file;
							firstSessionStartAdr = rd.GetRawAdr();
						}
						else if (firstSessionNum != spare.file)
						{
							firstSessionLastBlock = lastSessionBlock;
							firstSessionValid = true;
							firstSessionLastAdr = curFileStartAdr = rd.GetRawAdr();
//							curFileFPN = spare.fpn;
						};
					}
					else
					{
						if (curFileNum != spare.file)
						{
							findVector = true;

							findFileNum = curFileNum;
							findFileStartAdr = curFileStartAdr;
							//findFileEndAdr = rd.GetRawAdr();
							findFileLastBlock = lastSessionBlock;

							curFileStartAdr	= rd.GetRawAdr();
//							curFileFPN = spare.fpn;
						};
					};

					lastSessionBlock	= rd.GetRawBlock();
					curFileNum			= spare.file;
					curFileEndAdr		= rd.GetRawAdr();

					rd.NextBlock();
				}
				else if (spare.start == ~0 || spare.fpn == ~0 || spare.rawPage == ~0)
				{
					rd.NextBlock();
				}
				else
				{
					rd.NextPage(); count++;
				};

				if (findVector)
				{
					state++;
				}
				else if (count == 0 || rd.overflow)
				{
					state = 9;
				}
				else 
				{
					readSpare.Start(&spare, &rd);

					count--;
				};
			};

			break;

		case 3: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			findVector = false;

			flrb.data = 0;
			flrb.maxLen = 0;
			flrb.vecStart = true;
			flrb.useAdr = true;
			flrb.adr = findFileStartAdr;

			RequestFlashRead(&flrb);

			state++;

			break;

		case 4: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (Read::Start())
			{
				state++;
			};
				
			break;

		case 5: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Read::Update())
			{
				if (flrb.ready && flrb.hdr.session == findFileNum && flrb.hdr.crc == 0)
				{
					start_rtc = flrb.hdr.rtc;
				};

				adr.SetRawBlock(findFileLastBlock+1);

				findFileEndAdr = adr.GetRawAdr();

				adr.SetRawBlock(findFileLastBlock);

				flrb.data = 0;
				flrb.maxLen = 0;
				flrb.vecStart = true;
				flrb.useAdr = true;
				flrb.adr = adr.GetRawAdr();

				RequestFlashRead(&flrb);

				countFindVec = 10;

				state++;
			};
				
			break;

		case 6: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (Read::Start())
			{
				state++;
			};
				
			break;

		case 7: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Read::Update())
			{
				if (flrb.ready && flrb.hdr.session == findFileNum && flrb.hdr.crc == 0)
				{
					stop_rtc = flrb.hdr.rtc;

					state++;
				}
				else if (countFindVec > 0)
				{
					countFindVec--;
	
					adr.PrevBlock(); 

					flrb.data = 0;
					flrb.maxLen = 0;
					flrb.vecStart = true;
					flrb.useAdr = true;
					flrb.adr = adr.GetRawAdr();
					
					RequestFlashRead(&flrb);

					state--;
				}
				else
				{
					state++;
				};
			};
				
			break;

		case 8: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (TRAP_MEMORY_SendSession(sendedFileNum = findFileNum, (findFileEndAdr - findFileStartAdr) & NAND_RAWADR_MASK, findFileStartAdr, start_rtc, stop_rtc, 0))
			{
				if (count == 0 || rd.overflow)
				{
					state++;
				}
				else
				{
					readSpare.Start(&spare, &rd);

					count--;
					
					state = 2;
				};
			};
				
			break;

		case 9: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!firstSessionSended)
			{
				if (curFileNum == firstSessionNum)
				{
					firstSessionStartAdr = curFileStartAdr;
				};

				findFileNum = firstSessionNum;
				findFileStartAdr = firstSessionStartAdr;
				findFileEndAdr = firstSessionLastAdr;
				findFileLastBlock = firstSessionLastBlock;

				firstSessionSended = true;

				state = 3;
			}
			else if (curFileNum != sendedFileNum)
			{
				findFileNum = curFileNum;
				findFileStartAdr = curFileStartAdr;
				findFileEndAdr = rd.GetRawAdr();
				findFileLastBlock = lastSessionBlock;
				
				state = 3;
			}
			else
			{
				state++;
			};

			break;

		case 10: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (TRAP_MEMORY_SendStatus(-1, FLASH_STATUS_READ_SESSION_READY))
			{
				state++;
			};

			break;

		case 11: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (TRAP_TRACE_PrintString("NAND chip mask: 0x%02hX; Bad Blocks: %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu", nandSize.mask, bb[0], bb[1], bb[2], bb[3], bb[4], bb[5], bb[6], bb[7]))
			{
				cmdSendSession = false;

				state = 0;
			};

			break;
	};

	if (tm.Check(100))
	{
		prgrss = rd.GetRawBlock() * (0x100000000/(NAND_RAWBLOCK_MASK+1));

		TRAP_MEMORY_SendStatus(prgrss, FLASH_STATUS_READ_SESSION_IDLE);
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void StartSendSession()
{
	cmdSendSession = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool UpdateSendSession()
{
//	enum {	WAIT = 0, READ_START, READ_1, READ_2, READ_PAGE,READ_PAGE_1,FIND_START,FIND_1,FIND_2,FIND_3,FIND_4, READ_END};

	static byte i = 0;
	
	static u16 ind = 0;
	static u32 prgrss = 0;
	static u16 count = 0;
	static u32 lp = 0;
	static u32 sum = 0;
	static FLADR a(0);
	static TM32 tm;

	FileDsc &s = nvsi[ind].f;
	
	switch (i)
	{
		case 0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (cmdSendSession)
			{
				ind = nvv.index;
				prgrss = 0;
				count = 128;

				i++;
			}
			else
			{
				return false;
			};

			break;

		case 1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (s.size > 0)
			{
				if (!TRAP_MEMORY_SendSession(s.session, s.size, (u64)s.startPage * FLADR::pg, s.start_rtc, s.stop_rtc, s.flags))
				{
					break;
				};
			};

			ind = (ind-1)&127;

			prgrss += 0x100000000/128;

			count--;

			i++;

			break;

		case 2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (TRAP_MEMORY_SendStatus(prgrss, FLASH_STATUS_READ_SESSION_IDLE))
			{
				if (count > 0)
				{
					i = 1;
				}
				else
				{
					tm.Reset(); count = 3;

					i++;
				};
			};

			break;

		case 3:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (tm.Check(50))
			{
				TRAP_MEMORY_SendStatus(~0, FLASH_STATUS_READ_SESSION_IDLE);

				if (count > 0) count--; else count = 3, i++;
			};

			break;

		case 4:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (tm.Check(50))
			{
				TRAP_MEMORY_SendStatus(~0, FLASH_STATUS_READ_SESSION_READY);

				if (count > 0) count--; else  cmdSendSession = false, i = 0;
			};

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FileDsc* GetSessionInfo(u16 session, u64 adr)
{
	u16 ind = nvv.index;

	FileDsc *s = 0;

	for (u16 i = 128; i > 0; i--)
	{
		s = &nvsi[ind].f;

		if (s->session == session)
		{
			return s;
		};

		ind = (ind-1)&127;
	};

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static SessionInfo *readSessionInfoPtr = 0;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void ReadSessionInfoStart(SessionInfo *si)
//{
//	readSessionInfoPtr = si;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool ReadSessionInfoUpdate()
//{
//	enum {	WAIT = 0,START,READ_1,READ_2,READ_3};
//
//	static SpareArea spare;
//	static VecData::Hdr hdr;
//	static SessionInfo *si;
//
//	static FLADR rd(0,0,0,0);
//
//	static byte state = WAIT;
//
//	switch (state)
//	{
//		case WAIT:
//
//			if (readSessionInfoPtr != 0)
//			{
//				si = readSessionInfoPtr;
//
//				rd.raw = adrLastVector;
//
//				ReadSpareStart(&spare, &rd);
//			}
//			else
//			{
//				return false;
//			};
//
//			break;
//
//		case START:
//
//			if (!ReadSpareUpdate())
//			{
//				si->size = (u64)spare.fpn << NAND_COL_BITS;
//
//				si->last_adress = adrLastVector;
//				si->session = spare.file;
//
//			ReadVecHdrNow(&hdr, &rd);
//
//			if (hdr.crc == 0)
//			{
//				si->stop_rtc = hdr.rtc;
//			}
//			else
//			{
//
//			};
//
//			rd.SetRawPage(spare.start);
//
//			ReadVecHdrNow(&hdr, &rd);
//
//			if (hdr.crc == 0)
//			{
//				si->start_rtc = hdr.rtc;
//			}
//			else
//			{
//
//			};
//	};
//
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Idle()
{
//	register u32 t;

	static i32 t = 0;
	static i32 eb = 0;
	static TM32 tm;
	static FLADR er(0);
	static EraseBlock eraseBlock;
	static bool blackBox = false;

	switch (nandState)
	{
		case NAND_STATE_WAIT: 

			if (cmdCreateNextFile)
			{
				write.CreateNextFile();
				cmdCreateNextFile = false;
				nandState = NAND_STATE_CREATE_FILE;

				break;
			};

			if (cmdFullErase)
			{
				cmdFullErase = false;
				eraseSessionsCount = 1;
				//nandState = NAND_STATE_FULL_ERASE_START;

				break;
			};

			if (cmdSendSession)
			{
				nandState = NAND_STATE_SEND_SESSION;

				break;
			};

			if (write.Start())
			{
				nandState = NAND_STATE_WRITE_START;
			}
			else if (Read::Start())
			{
				nandState = NAND_STATE_READ_START;
			};

			if (EmacIsConnected() && tm.Check(500)) { TRAP_MEMORY_SendStatus(-1, FLASH_STATUS_NONE); };

			break;

		case NAND_STATE_WRITE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!write.Update())
			{
				nandState = NAND_STATE_WAIT;
			};

			break;

		case NAND_STATE_READ_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Read::Update())
			{
				nandState = NAND_STATE_WAIT;
			};

			break;

		case NAND_STATE_FULL_ERASE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			t = eb = nandSize.fl >> (NAND_COL_BITS + NAND_PAGE_BITS); // blocks count

			er.SetRawAdr(0);

			eraseBlock.Start(er, true, true);

			nandState = NAND_STATE_FULL_ERASE_0;

			break;

		case NAND_STATE_FULL_ERASE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!eraseBlock.Update())
			{
//				lastError = GetLastError();

				invalidBlocks += eraseBlock.errBlocks;

				t -= eraseBlock.errBlocks;
				t -= 1;

				write.errVec = t;

				if (t > 0)
				{
					er.NextBlock();

					eraseBlock.Start(er, true, true);

					if (tm.Check(500)) { TRAP_MEMORY_SendStatus((eb-t)*((u64)0x100000000)/eb, FLASH_STATUS_BUSY); };
				}
				else
				{
					flashEmpty = true;

					write.Init(0, 1, -1);

//					adrLastVector = -1;

					TRAP_MEMORY_SendStatus(-1, FLASH_STATUS_ERASE);
					nandState = NAND_STATE_WAIT;
				};
			};

			break;

		case NAND_STATE_CREATE_FILE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!write.Update())
			{
				nandState = NAND_STATE_WAIT;
			};

			break;

		case NAND_STATE_SEND_SESSION:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (blackBox)
			{
				if (!UpdateBlackBoxSendSessions())
				{
					blackBox = !blackBox;

					nandState = NAND_STATE_WAIT;
				};
			}
			else
			{
				if (!UpdateSendSession())
				{
					blackBox = !blackBox;

					nandState = NAND_STATE_WAIT;
				};
			};
	
			//if (tm.Check(200) && IsComputerFind() && EmacIsConnected()) TRAP_MEMORY_SendStatus(lastFlashProgress, lastFlashStatus);

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Init()
{
#ifndef WIN32

	using namespace HW;

	nandSize.shPg = 0;
	nandSize.shBl = 0;
	nandSize.shCh = 0;
//	nandSize.shRow = 0;

	nandSize.pg = 0;
	nandSize.bl = 0;
	nandSize.ch = 0;
	nandSize.fl = 0;
//	nandSize.row = 0;
	nandSize.mask = 0;

	nandSize.bitBlock	= 0;
	nandSize.bitCol		= 0;
	nandSize.bitPage	= 0;

	nandSize.maskBlock	= 0;
	nandSize.maskPage	= 0;

	nandSize.pagesInBlock	= 0;


	HW::EBU_Enable(0);

	HW::Peripheral_Enable(PID_DMA1);

	HW::GPDMA1->DMACFGREG = 1;

	EBU->CLC = 0x110000;
	EBU->MODCON = EBU_ARBSYNC|EBU_ARBMODE(3);
	EBU->USERCON = 0x3FF<<16;

	HW::EBU->ADDRSEL0 = EBU_REGENAB/*|EBU_ALTENAB*/;

	EBU->BUSRCON0 = EBU_AGEN(4)|EBU_WAIT(0)|EBU_PORTW(1);
	EBU->BUSRAP0 = EBU_ADDRC(0)|EBU_CMDDELAY(0)|EBU_WAITRDC(NS2CLK(60))|EBU_DATAC(0)|EBU_RDRECOVC(NS2CLK(20))|EBU_RDDTACS(0);

	EBU->BUSWCON0 = EBU_AGEN(4)|EBU_WAIT(0)|EBU_PORTW(1);

//				 = |			|				 |		tWP		 |			   |			   |				;
	EBU->BUSWAP0 = EBU_ADDRC(0)|EBU_CMDDELAY(0)|EBU_WAITWRC(NS2CLK(40))|EBU_DATAC(0)|EBU_WRRECOVC(NS2CLK(20))|EBU_WRDTACS(0);

//	PMC->PCER0 = PID::SMC_M;

//	SMC->CSR[0].SETUP = 1;
//	SMC->CSR[0].PULSE = 0x0F0F0F0F;
//	SMC->CSR[0].CYCLE = 0x00200020;
//	SMC->CSR[0].MODE = 0x00000003;


	for(byte chip = 0; chip < NAND_MAX_CHIP; chip ++)
	{
		NAND_Chip_Select(chip);
		ResetNand();
		NandID k9k8g08u_id;
		Read_ID(&k9k8g08u_id);
		
		if((k9k8g08u_id.marker == 0xEC) && (k9k8g08u_id.device == 0xD3))
		{
			if (nandSize.shCh == 0)
			{
				nandSize.pg = 1 << (nandSize.bitCol = nandSize.shPg = ((k9k8g08u_id.data[1] >> 0) & 0x03) + 10);
				nandSize.bl = 1 << (nandSize.shBl = ((k9k8g08u_id.data[1] >> 4) & 0x03) + 16);
				nandSize.ch = 1 << (nandSize.shCh = (((k9k8g08u_id.data[2] >> 4) & 0x07) + 23) + (((k9k8g08u_id.data[2] >> 2) & 0x03) + 0));
//				nandSize.row = 1 << (nandSize.shRow = nandSize.shCh - nandSize.shPg);
				
				nandSize.pagesInBlock = 1 << (nandSize.bitPage = nandSize.shBl - nandSize.shPg);

				nandSize.maskPage = nandSize.pagesInBlock - 1;
				nandSize.maskBlock = (1 << (nandSize.bitBlock = nandSize.shCh - nandSize.shBl)) - 1;
			};
			
			nandSize.fl += nandSize.ch;
			
			nandSize.mask |= (1 << chip);
		};
	};

	FLADR::InitVaildTables(nandSize.mask);

	NAND_Chip_Disable();

	DisableWriteProtect();

#else

	printf("Open file %s ... ", nameNandFile);

	handleNandFile = CreateFile(nameNandFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED , 0);
	//handleNandFile = CreateFile(nameNandFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0 , 0);

	cputs((handleNandFile == INVALID_HANDLE_VALUE) ? "!!! ERROR !!!\n" : "OK\n");

	nandSize.pg = 1 << (nandSize.bitCol = NAND_COL_BITS);
	nandSize.bl = 1 << (nandSize.shBl = NAND_COL_BITS+NAND_PAGE_BITS);
	nandSize.ch = 1 << (nandSize.shCh = NAND_COL_BITS+NAND_PAGE_BITS+NAND_BLOCK_BITS);
	
	nandSize.pagesInBlock = 1 << (nandSize.bitPage = nandSize.shBl - nandSize.shPg);

	nandSize.maskPage = nandSize.pagesInBlock - 1;
	nandSize.maskBlock = (1 << (nandSize.bitBlock = nandSize.shCh - nandSize.shBl)) - 1;
			
	nandSize.fl = nandSize.ch * NAND_MAX_CHIP;
			
	nandSize.mask = (1 << NAND_MAX_CHIP) - 1;

	cputs("Alloc nandEraseFillArray ... ");

	nandEraseFillArray = VirtualAlloc(0, nandEraseFillArraySize = nandSize.bl*2, MEM_COMMIT, PAGE_READWRITE);

	cputs((nandEraseFillArray == NULL) ? "!!! ERROR !!!\n" : "OK\n");

	if (nandEraseFillArray != 0) for (u32 i = nandEraseFillArraySize/4, *p = (u32*)nandEraseFillArray; i != 0; i--) *(p++) = ~0;

#endif
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Errors_Get()
{
	return write.errVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Saved_Get()
{
	return write.spare.vectorCount;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Recieved_Get()
{
	return write.rcvVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Rejected_Get()
{
	return write.rejVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Session_Get()
{
	return write.spare.file;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Current_Adress_Get()
{
	return write.wr.GetRawAdr();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Full_Size_Get()
{
	return nandSize.fl;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Used_Size_Get()
{
	return flashUsedSize;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i64 FLASH_Empty_Size_Get()
{
	return FLASH_Full_Size_Get() - FLASH_Used_Size_Get();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 FLASH_Chip_Mask_Get()
{
	return nandSize.mask;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// ������� 
bool FLASH_Erase_Full()
{
	//if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	//FRAM_Memory_Start_Adress_Set(FRAM_Memory_Current_Adress_Get());
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// ������������
bool FLASH_UnErase_Full()
{
	//if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	//FRAM_Memory_Start_Adress_Set(-1);
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req
{
	u16 rw; 
	u32 cnt; 
	u16 gain; 
	u16 st; 
	u16 len; 
	u16 delay; 
	u16 data[1024*4]; 
	u16 crc;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp
{
	byte	adr;
	byte	func;
	
	__packed union
	{
		__packed struct  { word crc; } f1;  // ����� ����� ������
		__packed struct  { word crc; } f2;  // ������ �������
		__packed struct  { word crc; } f3;  // 
		__packed struct  { word crc; } fFE;  // ������ CRC
		__packed struct  { word crc; } fFF;  // ������������ ������
	};
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Rsp rspData;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRsp01(ComPort::WriteBuffer *wb)
{
	static Rsp rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.adr = 1;
	rsp.func = 1;
	rsp.f1.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRsp02(ComPort::WriteBuffer *wb)
{
	static Rsp rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.adr = 1;
	rsp.func = 2;
	rsp.f2.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRsp03(ComPort::WriteBuffer *wb)
{
	static Rsp rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.adr = 1;
	rsp.func = 3;
	rsp.f3.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRspErrCRC(ComPort::WriteBuffer *wb)
{
	static Rsp rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.adr = 1;
	rsp.func = 0xFE;
	rsp.fFE.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateRspErrReq(ComPort::WriteBuffer *wb)
{
	static Rsp rsp;

	if (wb == 0)
	{
		return false;
	};

	rsp.adr = 1;
	rsp.func = 0xFF;
	rsp.fFF.crc = GetCRC16(&rsp, sizeof(rsp)-2);

	wb->data = &rsp;
	wb->len = sizeof(rsp);

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc02(FLWB *fwb, ComPort::WriteBuffer *wb)
{
	VecData &vd = fwb->vd;

//	Req &req = *((Req*)vd.data);

	//byte n = vd.data[0] & 7;

	//vecCount[n] += 1;

	if (!RequestFlashWrite(fwb))
	{
		freeFlWrBuf.Add(fwb);
//		return false;
	};


	return CreateRsp02(wb);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc(FLWB *fwb, ComPort::WriteBuffer *wb)
{
	bool result = false;

	VecData &vd = fwb->vd;

	Req &req = *((Req*)vd.data);

	if (fwb == 0)
	{
//		freeReqList.Add(req);
	}
	else if (fwb->dataLen < 2)
	{
		result = CreateRspErrReq(wb);
		
		freeFlWrBuf.Add(fwb);
	}
	else
	{
		if (GetCRC16(fwb->vd.data, fwb->dataLen) == 0) // (req.rw & 0xFF00) == 0xAA00) // 
		{
			result = RequestFunc02 (fwb, wb);
		}
		else
		{
			freeFlWrBuf.Add(fwb);
			write.rejVec++;
		};
	};

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestTestWrite(FLWB *fwb)
{
	__packed struct Req { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[1024*4]; u16 crc; };

	if (fwb == 0)
	{
		return;
	};

	static byte nr = 0;
	static byte nf = 0;
	static u32 count = 0;

	VecData &vd = fwb->vd;

	Req &req = *((Req*)vd.data);

	req.rw = 0xAA30 + ((nf & (3))<<4) + (nr & 7);
	req.cnt = count;
	req.gain = 7;
	req.st = 10;
	req.len = ArraySize(req.data)/4;
	req.delay = 0;

	u16 v = 0;

	//for (u16 i = 0; i < ArraySize(req.data); i++)
	//{
	//	req.data[i] = v++;
	//};

	if (nr < 7)
	{ 
		nr += 1; 
	}
	else
	{
		nr = 0;

		count += 1;

		if (nf < 2)
		{
			nf += 1;
		}
		else
		{
			nf = 0;
		};
	};

	fwb->dataLen = sizeof(req);

	req.crc = GetCRC16(fwb->vd.data, fwb->dataLen - 2);
	
	RequestFlashWrite(fwb);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitCom()
{
	com1.ConnectAsyn(1, 6250000, 0, 1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
//	__packed struct Req { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[512*4]; };

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static byte state = 0;

	static FLWB *fwb;
//	static Req *req;
	static VecData *vd;

	static u32 count = 0;
	static u16 v = 0;
	static byte b = 0;

	static TM32 tm;

	if (testWriteFlash)
	{
		state = 0;
	};

	switch (state)
	{
		case 0:

			fwb = freeFlWrBuf.Get();

			if (fwb != 0)
			{
				if (testWriteFlash)
				{
					if (!writeFlashEnabled && tm.Check(2000))
					{
						FLASH_WriteEnable();
					}
					else if (writeFlashEnabled && (nvv.f.size > 10000000000))
					{	
						FLASH_WriteDisable();
						tm.Reset();
					};

					RequestTestWrite(fwb);
				}
				else
				{
#ifndef WIN32
					state++;
#else
					freeFlWrBuf.Add(fwb); fwb = 0;
#endif
				};
			};

			break;

		case 1:

			vd = &fwb->vd;

			rb.data = vd->data;
			rb.maxLen = sizeof(vd->data);

			HW::P1->BSET(13);

			com1.Read(&rb, MS2RT(500), US2RT(50));

			state++;

			break;

		case 2:

			if (!com1.Update())
			{
				HW::P1->BCLR(13);

				if (rb.recieved)
				{
					fwb->dataLen = rb.len;

					//HW::PIOB->SODR = 1<<13;

					if (RequestFunc(fwb, &wb))
					{
						state++;
					}
					else
					{
						state = 0;
					};

					//HW::PIOB->CODR = 1<<13;
				}
				else
				{
					state = 1;
				};
			};

			break;

		case 3:

			if (!freeFlWrBuf.Empty())
			{
				com1.Write(&wb);

				state++;
			};

			break;

		case 4:
			
			if (!com1.Update())
			{
				state = 0;
			};

			break;
	};

//	HW::PIOB->CODR = 1<<13;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_WriteEnable()
{
	if (!writeFlashEnabled)
	{
		nvv.f.size = 0;
		GetTime(&nvv.f.start_rtc);
	};

	writeFlashEnabled = true;
	flashStatus = FLASH_STATUS_WRITE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_WriteDisable()
{
	if (writeFlashEnabled)
	{
		NAND_NextSession();
	};

	writeFlashEnabled = false;

	flashStatus = FLASH_STATUS_WAIT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte FLASH_Status()
{
	return flashStatus;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FLASH_Reset()
{
	if(!ResetNand()) return false;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadVars()
{
	//Init_TWI();

	PointerCRC p(buf);

	static DSCTWI dsc;

	//dsc.MMR = 0x500200;
	//dsc.IADR = 0;
	//dsc.CWGR = 0x7575;
	//dsc.data = buf;
	//dsc.len = sizeof(buf);

	u16 adr = 0;

	dsc.adr = 0x50;
	dsc.wdata = &adr;
	dsc.wlen = sizeof(adr);
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;
	dsc.next = 0;
	dsc.rdata = buf;
	dsc.rlen = sizeof(buf);

	if (AddRequest_TWI(&dsc))
	{
		while (!dsc.ready)
		{ 
			Update_TWI();
			HW::WDT->Update(); 
		};
	};

//	bool c = false;

	loadVarsOk = false;

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&nvv, sizeof(nvv)+2);

		if (p.CRC.w == 0) { loadVarsOk = true; break; };
	};

	if (!loadVarsOk)
	{
		nvv.numDevice = 0;
		nvv.index = 0;
		nvv.prevFilePage = -1;

		nvv.f.session = 0;
		nvv.f.size = 0;
		nvv.f.startPage = 0;
		nvv.f.lastPage = 0;
		GetTime(&nvv.f.start_rtc);
		GetTime(&nvv.f.stop_rtc);
		nvv.f.flags = 0;

		for (u32 i = 0; i < ArraySize(nvv.badBlocks); i++)
		{
			nvv.badBlocks[i] = 0;
		};

		savesCount = 2;
	};

#ifdef WIN32

	cputs(loadVarsOk ? "Load Vars ... OK\n" : "Load Vars ... ERROR\n");

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	const u16 sa = 0x100;

	PointerCRC p(buf);

	static DSCTWI dsc;

	static byte i = 0;
	static u16 adr;
	static TM32 tm;

	switch (i)
	{
		case 0:

			if (tm.Check(200))
			{
				if (savesCount > 0)
				{
					savesCount--;
					i++;
				}
				else if (savesSessionsCount > 0)
				{
					savesSessionsCount--;
					i = 3;
				}
				else if (eraseSessionsCount > 0)
				{
					eraseSessionsCount--;

					for (u16 n = 0; n < ArraySize(nvsi); n++)
					{
						nvsi[n].f.size = 0;
						nvsi[n].crc = 0;
					};

					for (u16 n = 0; n < ArraySize(nvv.badBlocks); n++)
					{
						nvv.badBlocks[n] = 0;
					};

					nvv.f.session += 1;
					nvv.f.size = 0;
					//nvv.f.startPage = 0;
					//nvv.f.lastPage = 0;
					nvv.index = 0;

					savesCount = 1;

					i = 4;
				};
			};

			break;

		case 1:

			adr = 0;

			dsc.adr = 0x50;
			dsc.wdata = &adr;
			dsc.wlen = sizeof(adr);
			dsc.wdata2 = buf;
			dsc.wlen2 = sizeof(buf);
			dsc.next = 0;
			dsc.rdata = 0;
			dsc.rlen = 0;

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&nvv, sizeof(nvv));
				p.WriteW(p.CRC.w);
			};

			i = (AddRequest_TWI(&dsc)) ? (i+1) : 0;

			break;

		case 2:

			if (dsc.ready)
			{
				i = 0;
			};

			break;

		case 3:

			{
				NVSI &si = nvsi[nvv.index];

				adr = ReverseWord(sa+sizeof(si)*nvv.index);

				dsc.adr = 0x50;
				dsc.wdata = &adr;
				dsc.wlen = sizeof(adr);
				dsc.wdata2 = buf;
				dsc.wlen2 = sizeof(si);
				dsc.next = 0;
				dsc.rdata = 0;
				dsc.rlen = 0;

				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&si, sizeof(si.f));
				p.WriteW(p.CRC.w);

				i = (AddRequest_TWI(&dsc)) ? 2 : 0;
			};

			break;

		case 4:
			
			adr = ReverseWord(sa);

			dsc.adr = 0x50;
			dsc.wdata = &adr;
			dsc.wlen = sizeof(adr);
			dsc.wdata2 = &nvsi;
			dsc.wlen2 = sizeof(nvsi);
			dsc.next = 0;
			dsc.rdata = 0;
			dsc.rlen = 0;

			i = (AddRequest_TWI(&dsc)) ? 2 : 0;

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadSessions()
{
	PointerCRC p(buf);

	static DSCTWI dsc;

	const u16 sa = 0x100;

	loadSessionsOk = true;

	for (u16 i = 0; i < ArraySize(nvsi); i++)
	{
		NVSI &si = nvsi[i];

		u16 adr = ReverseWord(sa+sizeof(si)*i);

		dsc.adr = 0x50;
		dsc.wdata = &adr;
		dsc.wlen = sizeof(adr);
		dsc.rdata = &si;
		dsc.rlen = sizeof(si);
		dsc.next = 0;
		dsc.wdata2 = 0;
		dsc.wlen2 = 0;

		if (AddRequest_TWI(&dsc))
		{
			while (!dsc.ready)
			{ 
				Update_TWI();
				HW::WDT->Update(); 
			};
		};

		if (GetCRC16(&si, sizeof(si)) != 0)
		{
			loadSessionsOk = false;

			si.f.session = 0;
			si.f.size = 0;
			si.f.startPage = 0;
			si.f.lastPage = 0;
			si.f.flags = 0;
			si.f.start_rtc.date = 0;
			si.f.start_rtc.time = 0;
			si.f.stop_rtc.date = 0;
			si.f.stop_rtc.time = 0;
			si.crc = GetCRC16(&si, sizeof(si.f));

			dsc.adr = 0x50;
			dsc.wdata = &adr;
			dsc.wlen = sizeof(adr);
			dsc.wdata2 = &si;
			dsc.wlen2 = sizeof(si);
			dsc.next = 0;
			dsc.rdata = 0;
			dsc.rlen = 0;

			AddRequest_TWI(&dsc);

			while (!dsc.ready)
			{ 
				Update_TWI();
				HW::WDT->Update(); 
			};
		};
	};

#ifdef WIN32

	cputs(loadSessionsOk ? "Load Sessions ... OK\n" : "Load Sessions ... ERROR\n");

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_Init()
{
	LoadVars();

	LoadSessions();

	InitFlashBuffer();

	NAND_Init();

	//FLASH_Reset();

	InitCom();

//	BlackBoxSessionsInit();

	InitSessionsNew();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma push
#pragma O3
#pragma Otime

void FLASH_Update()
{
	static byte i = 0;
	static u32 t = 0;
	static volatile byte *p = FLD;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateCom();	);
		CALL( SaveVars();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL

//	HW::P5->BSET(1);
	
	//t = FLD[0]+FLD[2]+FLD[4]+FLD[6];
	//*FLA = 0x55;
	//*FLC = 0xAA;

//	CmdReadStatus();

	//if (CheckDataComplete())
	//{
	//	NandReadData(buf, 8);
	//};

//	HW::P5->BCLR(1);
}

#pragma pop

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#endif // #ifndef WIN32
