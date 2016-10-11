//#include "common.h"
#include "flash.h"
//#include "fram.h"
#include "vector.h"
#include "core.h"
#include "list.h"
#include <CRC16.h>
#include "ComPort.h"
#include "trap_def.h"
#include "trap.h"
#include "twi.h"
#include "PointerCRC.h"


#pragma diag_suppress 550,177

/**************************************************/
/*

���������� ������ ������, ��� ������ ������� ��� ������� ��� �� ���������� ������,
������ �� ���������� �����, ������ - ������.
����� ������� ���� ���� ����������� � ����� ����� �������� ������ ������ �����
����� ����� � ������ ����� ������ ������� �� ������ � ������ � �� ������ �� ��� ������, �� ��� ������� �������������

*/
/**************************************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define FLASH_SAVE_BUFFER_SIZE		8400
#define FLASH_READ_BUFFER_SIZE		8400

static FLWB flwb[4];
static FLRB flrb[4];

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

static u64 adrLastVector = -1;
static u32 lastSessionBlock = -1;
static u32 lastSessionPage = -1;

static bool cmdCreateNextFile = false;
static bool cmdFullErase = false;
static bool cmdSendSession = false;

static bool writeFlashEnabled = false;
static bool flashFull = false;
static bool flashEmpty = false;

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NVV // NonVolatileVars  
{
	u16 numDevice;

	SessionInfo si;

	u16 index;

	u32 prevFilePage;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NVSI // NonVolatileSessionInfo  
{
	SessionInfo si;

	u16 crc;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NVV nvv;

static NVSI nvsi[128];

static byte buf[sizeof(nvv)*2+4];

byte savesCount = 0;

byte savesSessionsCount = 0;

static TWI	twi;

static void SaveVars();

static bool loadVarsOk = false;
static bool loadSessionsOk = false;

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


static FLADR wr(0, 0, 0, 0);
static FLADR er(-1, -1, -1, -1);

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

static byte		wrBuf[2112];

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

static void InitFlashBuffer()
{
	for (byte i = 0; i < ArraySize(flwb); i++)
	{
		freeFlWrBuf.Add(&flwb[i]);
	};

	for (byte i = 0; i < ArraySize(flrb); i++)
	{
		freeFlRdBuf.Add(&flrb[i]);
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
	if ((b != 0) && (b->data != 0) && (b->maxLen > 0))
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
		b->ready[0] = false;

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

__packed struct NandID
{
 	byte marker;
 	byte device;
 	byte data[3];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

volatile byte * const FLC = (byte*)0x60400000;	
volatile byte * const FLA = (byte*)0x60200000;	
volatile byte * const FLD = (byte*)0x60000000;	

static u32 chipSelect[8] = { 1<<13, 1<<16, 1<<23, 1<<15, 1<<14, 1<<24, 1<<25, 1<<26 };
static const u32 maskChipSelect = (0xF<<13)|(0xF<<23);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define CMD_LATCH(cmd) { *FLC = cmd; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_LATCH_COL(col) { *FLA = col; *FLA = col >>= 8; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ADR_LATCH_ROW(row) { *FLA = row; *FLA = row >>= 8; *FLA = row >>= 8; }

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

#define NAND_BUSY() ((HW::PIOC->PDSR & (1UL<<31)) == 0)
//#define NAND_BUSY() ((CmdReadStatus() & (1<<6)) == 0)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void EnableWriteProtect()
{
	HW::PIOD->CODR = 1UL<<18;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void DisableWriteProtect()
{
	HW::PIOD->SODR = 1UL<<18;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline byte CmdReadStatus()
{
	CMD_LATCH(NAND_CMD_READ_STATUS);
	return *FLD;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool CmdNandBusy()
{
	return ((CmdReadStatus() & (1<<6)) == 0) || NAND_BUSY();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Chip_Select(byte chip) 
{    
	if(chip < 8)                   
	{ 				
		HW::PIOA->SODR = maskChipSelect ^ chipSelect[chip];
		HW::PIOA->CODR = chipSelect[chip];
	};
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void SetNandAdr(u64 a)
//{
////	while(a < 0) a += ((i64)1 << (nandSize.full)) * (((-a) >> (nandSize.full)) + 1);
//	a &= nandSize.fl - 1;
//	cur_chip = a >> nandSize.shCh;
//	cur_row = a >> nandSize.shPg;
//	cur_col = a & (nandSize.pg-1);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void NAND_Adress_Set_Next(u64 a)
//{
////	while(a < 0) a += ((i64)1 << (nandSize.full)) * (((-a) >> (nandSize.full)) + 1);
//
//	a += nandSize.bl;
//	a &= (nandSize.fl - 1);
//
//	cur_chip = a >> nandSize.shCh;
//	cur_row = a >> nandSize.shPg;
//	cur_col = 0;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//u64 GetNandAdr()
//{
//	return (nandSize.ch * cur_chip) + ((u64)nandSize.pg * cur_row) + cur_col;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ResetNandState()
{
	nandState = NAND_STATE_WAIT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NandState GetNandState()
{
	return nandState;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ResetNand()
{
	while(NAND_BUSY());
	CMD_LATCH(NAND_CMD_RESET);
	while(NAND_BUSY());
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read_ID(NandID *id)
{
	CMD_LATCH(NAND_CMD_READ_ID);
	*FLA = 0;

	byte *p = (byte*)id;

	for(byte i = 0; i < sizeof(NandID); i++) 
	{ 
		*p++ = *FLD;
	}

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool NAND_Read_Data(u64 *adr, byte *data, u16 size)
//{
//	if(nandState != NAND_STATE_WAIT) return false;
//	SetNandAdr(*adr);	
//	cur_size = size;
//	cur_count = 0;
//	cur_data = data;
//	nandState = NAND_STATE_READ_START;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool NAND_Verify_Data(u64 *adr, byte *data, u16 size)
//{
//	if(nandState != NAND_STATE_WAIT) return false;
//	SetNandAdr(*adr);	
//	cur_size = size;
//	cur_count = 0;
//	cur_data = data;
//	cur_verify_errors = 0;
////	nandState = NAND_STATE_VERIFY_START;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool NAND_Write_Data(bool next, u64 *adr, byte *data, u16 size)
//{
//	if(nandState != NAND_STATE_WAIT) return false;
//	if(next)
//	{
//		NAND_Adress_Set_Next(*adr);
//		*adr = GetNandAdr();
//	}
//	else SetNandAdr(*adr);
//	cur_size = size;
//	cur_count = 0;
//	cur_data = data;         
//	nandState = NAND_STATE_WRITE_START;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u32 ROW(u32 block, u16 page)
{
	return (block << NAND_PAGE_BITS) + page;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void CmdEraseBlock(u32 bl)
{
	bl = ROW(bl, 0);
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_1);
	ADR_LATCH_ROW(bl);
	CMD_LATCH(NAND_CMD_BLOCK_ERASE_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void CmdRandomRead(u16 col)
{
	CMD_LATCH(NAND_CMD_RANDREAD_1);
	ADR_LATCH_COL(col);
	CMD_LATCH(NAND_CMD_RANDREAD_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void CmdReadPage(u16 col, u32 bl, u16 pg)
{
	bl = ROW(bl, pg);
	CMD_LATCH(NAND_CMD_READ_1);
	ADR_LATCH_COL_ROW(col, bl);
	CMD_LATCH(NAND_CMD_READ_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma push
#pragma O3
#pragma Otime

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
	using namespace HW;

	register u32 t __asm("r0");

	DMAC->EN = 1;

	t = DMAC->EBCISR;

	DMAC->CH[0].SADDR = src;
	DMAC->CH[0].DADDR = dst;
	DMAC->CH[0].DSCR = 0;
	DMAC->CH[0].CTRLA = len;
	DMAC->CH[0].CTRLB = (1<<16)|(1<<20);
	DMAC->CH[0].CFG = 0;
	DMAC->CHER = 1;
}

#pragma pop

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void NandReadData(void *data, u16 len)
{
	CopyDataDMA(FLD, data, len);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void NandWriteData(void *data, u16 len)
{
	CopyDataDMA(data, FLD, len);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void BufWriteData(void *data, u16 len)
{
	CopyDataDMA(data, wrBuf+wr.col, len);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool CheckDataComplete()
{
	return (HW::DMAC->CHSR & 1) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void CmdWritePage(u16 col, u32 bl, u16 pg)
{
	bl = ROW(bl, pg);
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_1);
	ADR_LATCH_COL_ROW(col, bl);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void CmdWritePage2()
{
	CMD_LATCH(NAND_CMD_PAGE_PROGRAM_2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace Erase
{
	enum {	WAIT = 0,ERASE_START,ERASE_0,ERASE_1,ERASE_2,ERASE_3,ERASE_4,ERASE_5 };

	static SpareArea spare;

	static byte state = WAIT;
	static bool force = false;	// ������� ��������
	static bool check = true;	// ��������� ��������� ��������
	static u16	errBlocks = 0;
//	static u32	fullEraseBlockCount = 0;


	static bool Start(byte chip, u32 block, bool frc, bool chk);
//	static bool FullEraseStart(u32 blockCount, bool frc, bool chk);
	static bool Update();
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Erase::Start(byte chip, u32 block, bool frc, bool chk)
{
	er.chip = chip;
	er.block = block;

	force = frc;
	check = chk;

	errBlocks = 0;

	state = ERASE_START;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Erase::Update()
{
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
																																
					er.NextBlock();	

					//if (er.GetRawBlock() == 0)
					//{
					//	flashFull = true;

					//	state = WAIT;		

					//	return false;
					//}
					//else
					//{
						state = ERASE_START; 
					//};
				}																												
				else																											
				{																												
					CmdEraseBlock(er.block);																						
																																
					state = ERASE_2;																				
				};																												
			};																													
																																
			break;																												
																																
																																
		case ERASE_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++				
																																
			if(!NAND_BUSY())																									
			{																													
				if (((CmdReadStatus() & 1) != 0 /*|| (spare.badPages != 0xFFFF && !force)*/) && check) // erase error																	
				{																												
					errBlocks += 1;	

//					__breakpoint(0);																							
																																
					CmdWritePage(er.pg, er.block, 0);																			
																																
					*(u32*)FLD = 0;		// spareErase.validPage = 0; spareErase.validBlock = 0;																
																																
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

				//if (er.GetRawBlock() == 0)
				//{
				//	flashFull = true;

				//	state = WAIT;		

				//	return false;
				//}
				//else
				//{
					state = ERASE_START; 
				//};
			};

			break;
	};

	return true;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace Write
{
	enum {	WAIT = 0,				WRITE_START,			WRITE_BUFFER,			WRITE_PAGE,				WRITE_PAGE_0,	WRITE_PAGE_1,
			WRITE_PAGE_2,			WRITE_PAGE_3,			WRITE_PAGE_4,			WRITE_PAGE_5,			ERASE,			
			WRITE_CREATE_FILE_0,	WRITE_CREATE_FILE_1,	WRITE_CREATE_FILE_2,	WRITE_CREATE_FILE_3,	WRITE_CREATE_FILE_4 };

	static u16		wr_cur_col	= 0;
	static u32 		wr_cur_pg	= -1;
	//static u16		wr_prev_col = 0;
	//static u32 		wr_prev_pg	= -1;
	static byte		wr_pg_error = 0;
	static u16		wr_count	= 0;
	static byte*	wr_data		= 0;
	static byte*	wr_ptr		= 0;

	static u64		prWrAdr = -1;
	static u32		rcvVec = 0;
	static u32		rejVec = 0;
	static u32		errVec = 0;

	static SpareArea spare;

	static byte state;

	static bool createFile = false;



	static bool Start();
	static bool Update();
	static void CreateNextFile();
	static void	Vector_Make(VecData *vd, u16 size);
	static void Finish();
	static void Init();
	static void Init(u32 bl, u32 file, u32 prfile);

	static void SaveSession();
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Write::Init()
{
	wr.SetRawAdr(nvv.si.last_adress+nvv.si.size);

	Write::spare.file = nvv.si.session;  

	Write::spare.prev = nvv.prevFilePage;		

	Write::spare.start = nvv.si.last_adress;		
	Write::spare.fpn = 0;	

	Write::spare.vectorCount = 0;

	Write::spare.vecFstOff = -1;
	Write::spare.vecFstLen = 0;

	Write::spare.vecLstOff = -1;
	Write::spare.vecLstLen = 0;

	Write::spare.fbb = 0;		
	Write::spare.fbp = 0;		

	Write::spare.chipMask = nandSize.mask;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Write::Init(u32 bl, u32 file, u32 prfile)
{
	wr.SetRawBlock(bl);

	Write::spare.file = file;  
//	Write::spare.lpn = wr.GetRawPage();

	Write::spare.prev = prfile;		

	Write::spare.start = wr.GetRawPage();		
	Write::spare.fpn = 0;	

	Write::spare.vectorCount = 0;

	Write::spare.vecFstOff = -1;
	Write::spare.vecFstLen = 0;

	Write::spare.vecLstOff = -1;
	Write::spare.vecLstLen = 0;

	//Write::wr_prev_pg = -1;
	//Write::wr_prev_col = 0;

	Write::spare.fbb = 0;		
	Write::spare.fbp = 0;		

	Write::spare.chipMask = nandSize.mask;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Write::SaveSession()
{
	nvv.index = (nvv.index+1) & 127;

	SessionInfo &s = nvv.si;
	SessionInfo &d = nvsi[nvv.index].si;

	d = nvv.si;

	nvsi[nvv.index].crc = GetCRC16(&d, sizeof(d));

	s.session = spare.file;
	s.last_adress = wr.GetRawAdr();
	s.size = 0;
	GetTime(&s.start_rtc);
	s.stop_rtc = s.start_rtc;
	s.flags = 0;

	SaveParams();
	savesSessionsCount = 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Write::Vector_Make(VecData *vd, u16 size)
{
	vd->h.session = spare.file;
	vd->h.device = 0;
	GetTime(&vd->h.rtc);

	//if (vd->h.rtc.msec == 0)
	//{
	//	__breakpoint(0);
	//};

	vd->h.prVecAdr = prWrAdr; 
	vd->h.flags = 0;
	vd->h.dataLen = size;
	vd->h.crc = GetCRC16(&vd->h, sizeof(vd->h) - sizeof(vd->h.crc));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Write::Start()
{
	if ((curWrBuf = writeFlBuf.Get()) != 0)
	{
		rcvVec += 1;

		if (!writeFlashEnabled || flashFull)
		{
			rejVec += 1;

			curWrBuf->ready[0] = true;
			freeFlWrBuf.Add(curWrBuf);
			return false;
		};

		Vector_Make(&curWrBuf->vd, curWrBuf->dataLen); 

//		curWrBuf->dataLen += sizeof(curWrBuf->vd.vec);
//		curWrBuf->hdrLen = 0;

		nvv.si.size += curWrBuf->vd.h.dataLen;
		nvv.si.stop_rtc = curWrBuf->vd.h.rtc;

		SaveParams();
		
		prWrAdr = wr.GetRawAdr();

		wr_data = (byte*)&curWrBuf->vd;
		wr_count = curWrBuf->dataLen + sizeof(curWrBuf->vd.h);

		wr_cur_col = wr.col;
		wr_cur_pg = wr.GetRawPage();

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

static void Write::CreateNextFile()
{
	state = WRITE_CREATE_FILE_0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Write::Finish()
{
	if (curWrBuf != 0)
	{
		curWrBuf->ready[0] = true;

		freeFlWrBuf.Add(curWrBuf);

		curWrBuf = 0;

		//wr_prev_col = wr_cur_col;
		//wr_prev_pg = wr_cur_pg;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Write::Update()
{
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

					BufWriteData(wr_data, c);

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
				if (wr.col == 0)
				{
					wr_ptr = wrBuf;
//					wr.col = 0;

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

			if(er.block != wr.block || er.chip != wr.chip)	// ����� ����
			{
				wr_pg_error = 0;

				Erase::Start(wr.chip, wr.block, false, true);

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
				if ((CmdReadStatus() & 1) != 0) // program error
				{
//					__breakpoint(0);

					spare.fbp += 1;

					CmdWritePage(wr.pg, wr.block, wr.page);

					*(u16*)FLD = 0; // spareErase.validPage = 0;

					CmdWritePage2();

					state = WRITE_PAGE_4;
				}
				else 
				{
					wr.NextPage();

//					flashFull = wr.overflow != 0;  // ����� �� ���������

					if (wr_count == 0/* || flashFull*/)
					{
						Finish();

						if (!createFile && (nvv.si.size >= 1024*1024*1536))
						{
							NAND_NextSession();
						};

						//if (wr.GetRawPage() >= 0xC0000)
						//{
						//	flashFull = true;
						//};

						//if (!createFile && spare.fpn >= 0xC0000)
						//{
						//	cmdCreateNextFile = true;
						//};

						state = (createFile) ? WRITE_CREATE_FILE_1 : WAIT;
					}
					else
					{
						state = WRITE_START;
					}

//					spare.lpn += 1;
					spare.fpn += 1;

					spare.vecFstOff = -1;
					spare.vecFstLen = 0;

					spare.vecLstOff = -1;
					spare.vecLstLen = 0;

					//spare.vecPrPg = wr_prev_pg;
					//spare.vecPrOff = wr_prev_col;

				};
			};

			break;

		case WRITE_PAGE_4:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			

			if(!NAND_BUSY())																									
			{																													
				if (wr_pg_error == 2) // �������� ���� ��� ������� ������� ��������
				{
					CmdWritePage(wr.pg + sizeof(spare.validPage) + sizeof(spare.validBlock), wr.block, 0);

					*(u16*)FLD = 0; // spare.badPages = 0;

					CmdWritePage2();

					state = WRITE_PAGE_5;
				}
				else
				{
					state = WRITE_PAGE;																			
				};

				wr_pg_error += 1;

				wr.NextPage();		

				//if (wr.overflow != 0)
				//{
				//	flashFull = true;

				//	Finish();

				//	state = WAIT;
				//};
			};																													

			break;

		case WRITE_PAGE_5:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			
																																
			if(!NAND_BUSY())																									
			{																													
				state = WRITE_PAGE;																			
			};
																																
			break;						

		case ERASE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			
					
			if (!Erase::Update())
			{
				//if (flashFull)
				//{
				//	Finish();

				//	state = WAIT;
				//}
				//else
				{
					wr.block = er.block;
					wr.chip = er.chip;

					spare.fbb += Erase::errBlocks;

					state = WRITE_PAGE_0;																			
				};
			};
																																
			break;																												

		case WRITE_CREATE_FILE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (wr.col > 0)
			{
				wr_ptr = wrBuf;
				wr_count = 0;

				for (u16 i = wr.col; i < sizeof(wrBuf); i++)
				{
					wrBuf[i] = 0xff;
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

				//if (wr.overflow != 0)
				//{
				//	flashFull = true;
				//};
			};

			spare.start = wr.GetRawPage();		
			spare.fpn = 0;		

			//wr_prev_pg = -1;
			//wr_prev_col = 0;

			spare.fbb = 0;		
			spare.fbp = 0;		

			spare.chipMask = nandSize.mask;	

			SaveSession();

			state = WAIT;

			return false;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace Read
{
	enum {	WAIT = 0,READ_START,READ_1, READ_2, READ_PAGE,READ_PAGE_1,FIND_START,FIND_1,FIND_2,FIND_3,FIND_4};

	static FLADR rd(0, 0, 0, 0);
	static byte*	rd_data = 0;
	static u16		rd_count = 0;

	static u32 		sparePage = -1;

	static SpareArea spare;

	static bool vecStart = false;

	static byte state;

	static bool Start();
//	static bool Start(FLRB *flrb, FLADR *adr);
	static bool Update();
	static void End() { curRdBuf->ready = true; curRdBuf = 0; state = WAIT; }
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

//static bool Read::Start(FLRB *flrb, FLADR *adr)
//{
//	if (flrb != 0)
//	{
//		curRdBuf = flrb;
//
//		if (adr != 0) { rd = *adr; };
//
//		vecStart = curRdBuf->vecStart;
//
//		if (vecStart)
//		{
//			rd_data = (byte*)&curRdBuf->hdr;
//			rd_count = sizeof(curRdBuf->hdr);
//			curRdBuf->len = 0;	
//		}
//		else
//		{
//			rd_data = curRdBuf->data;
//			rd_count = curRdBuf->maxLen;
//			curRdBuf->len = 0;	
//		};
//
//		state = READ_START;
//
//		return true;
//	};
//
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case READ_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			NAND_Chip_Select(rd.chip);

			if (rd.GetRawPage() != sparePage)
			{
				CmdReadPage(rd.pg, rd.block, rd.page);

				state = READ_1;
			}
			else
			{
				CmdReadPage(rd.col, rd.block, rd.page);

				state = READ_PAGE;
			};

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				NandReadData(&spare, sizeof(spare));

				state = READ_2;
			};

			break;

		case READ_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd.page == 0 && spare.validBlock != 0xFFFF)
				{
					rd.NextBlock();

					//if (rd.overflow != 0)
					//{
					//	End();
					//}
					//else
					{
						CmdReadPage(rd.pg, rd.block, rd.page);

						state = READ_1;
					};
				}
				else if (spare.validPage != 0xFFFF)
				{
					rd.NextPage();

					//if (rd.overflow != 0)
					//{
					//	End();
					//}
					//else
					{
						CmdReadPage(rd.pg, rd.block, rd.page);

						state = READ_1;
					};
				}
				else
				{
					sparePage = rd.GetRawPage();

					if (sparePage != spare.rawPage)
					{
//						__breakpoint(0);
					};

					CmdRandomRead(rd.col);

					state = READ_PAGE;
				};
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

			if (/*spare.lpn == -1 ||*/ spare.start == -1 || spare.fpn == -1 )
			{
				if (rd.page == 0)
				{
					// ������� ���������
					state = FIND_3;
				}
				else
				{
					//rd.NextBlock();

					//if (rd.GetRawBlock() == 0)
					//{
					//	state = FIND_3;
					//	break;
					//};

					rd.NextPage();

					//if (rd.overflow != 0)
					//{
					//	state = FIND_3;
					//	break;
					//};

					CmdReadPage(rd.pg, rd.block, rd.page);

					state = FIND_1;
				};
			}
			else if (spare.vecFstOff == 0xFFFF || spare.vecLstOff == 0xFFFF || rd.col > spare.vecLstOff)
			{
				rd.NextPage();

				//if (rd.overflow != 0)
				//{
				//	state = FIND_3;
				//	break;
				//};

				CmdReadPage(rd.pg, rd.block, rd.page);

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

			if(!NAND_BUSY())
			{
				NandReadData(&spare, sizeof(spare));

				state = FIND_2;
			};

			break;

		case FIND_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd.page == 0 && spare.validBlock != 0xFFFF)
				{
					rd.NextBlock();

					//if (rd.overflow != 0)
					//{
					//	state = FIND_3;
					//	break;
					//};

					CmdReadPage(rd.pg, rd.block, rd.page);

					state = FIND_1;
				}
				else if (spare.validPage != 0xFFFF)
				{
					rd.NextPage();

					//if (rd.overflow != 0)
					//{
					//	state = FIND_3;
					//	break;
					//};

					CmdReadPage(rd.pg, rd.block, rd.page);

					state = FIND_1;
				}
				else
				{
					sparePage = rd.GetRawPage();

					state = FIND_START;
				};
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

static SpareArea *spareReadPtr = 0;
static FLADR *spareAdrPtr = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ReadSpareStart(SpareArea *sp, FLADR *frd)
{
	spareReadPtr = sp;
	spareAdrPtr = frd;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ReadSpareUpdate()
{
	enum {	WAIT = 0,START,READ_1,READ_2,READ_3};

	static byte state = WAIT;
	static SpareArea *spare = 0;
	static FLADR *rd = 0;

	switch(state)
	{
		case WAIT:	
			
			if (spareReadPtr != 0 && spareAdrPtr != 0)
			{
				spare = spareReadPtr;
				rd = spareAdrPtr;

				state = START;
			}
			else
			{
				return false;
			};

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

					state = START;
				}				
				else if (spare->validPage != 0xFFFF)
				{
					rd->NextPage();

					state = START;
				}
				else
				{
					spare->crc = GetCRC16((void*)&spare->file, sizeof(*spare) - spare->CRC_SKIP);
				
					spareReadPtr = 0;
					spareAdrPtr = 0;

					state = WAIT;

					return false;

				};
			};

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


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

	while(!NAND_BUSY());
	while(NAND_BUSY());

	NandReadData(spare, sizeof(*spare));

	while (!CheckDataComplete());

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

	while(!NAND_BUSY());
	while(NAND_BUSY());

	NandReadData(dst, len);

	while (!CheckDataComplete());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReadVecHdrNow(VecData::Hdr *h, FLADR *rd)
{
	ReadDataNow(h, sizeof(*h), rd);

	h->crc = GetCRC16(h, sizeof(*h));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Test()
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

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Test2()
{
	FLADR rd(0, 0, 0, 0);

	SpareArea spare;

	rd.SetRawPage(0);

	while (1)
	{
		ReadSpareStart(&spare, &rd);

		while (ReadSpareUpdate());

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

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitSessions()
{
	Write::Init();

	if (nvv.si.size > 0)
	{
		NAND_NextSession();
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SimpleBuildFileTable()
{
	FLADR rd(0, 0, 0, 0);

	u32 bs;
	u32 be;
	u32 bm;
	u32 endBlock;

	SpareArea spare;

	byte state;

	FileDsc curf;
	FileDsc prf;
	FileDsc lastf;

	//rd.block = 1 << (nandSize.bitBlock-1);
	//rd.page = 0;
	//rd.chip = 0;

//	__breakpoint(0);

//	rd.SetRawBlock(-1);
	rd.SetRawPage(0xC0000);
	endBlock = rd.GetRawBlock();

	bs = 0;
	be = endBlock;
	bm = (be+bs)/2;

	rd.SetRawBlock(bm);

	while (be > bs)// && bm < be && bm > bs)
	{
		ReadSpareNow(&spare, &rd, false);

		if (spare.validBlock != 0xFFFF)
		{
			rd.NextBlock();
			bm = rd.GetRawBlock();
		}
		else if (spare.validPage != 0xFFFF)
		{
			rd.NextPage();
			bm = rd.GetRawBlock();
		}
		else
		{
			if (/*spare.lpn == -1 ||*/ spare.start == -1 || spare.fpn == -1 )
			{
				be = bm-1;
			}
			else
			{
				bs = bm;
			};

			bm = (bs + be + 1) / 2;

			rd.SetRawBlock(bm);
		};
	};

	rd.SetRawBlock(bs);

	ReadSpareNow(&spare, &rd, true);

	if (bs >= endBlock || bm < bs)
	{
		// ����� ��������� ���������
		
		flashFull = true;

		Write::Init(0, 1, -1);

		//wr.SetRawBlock(0);

		//Write::spare.file = 1;
		//Write::spare.lpn = 0;
		//Write::spare.start = 0;
		//Write::spare.fpn = 0;	
		//Write::spare.prev = -1;

		//Write::spare.vectorCount = 0;

		//Write::spare.vecFstOff = -1;
		//Write::spare.vecFstLen = 0;

		//Write::spare.vecLstOff = -1;
		//Write::spare.vecLstLen = 0;

		//Write::wr_prev_pg = -1;
		//Write::wr_prev_col = 0;

		//Write::spare.fbb = 0;		
		//Write::spare.fbp = 0;		

		//Write::spare.chipMask = nandSize.mask;	

	}
	else if (spare.validBlock != 0xFFFF || /*spare.lpn == -1 ||*/ spare.start == -1 || spare.fpn == -1)
	{
		// ����� ������

		flashEmpty = true;

		wr.SetRawBlock(0);

		Write::Init(0, 1, -1);

		//Write::spare.file = 1;
		//Write::spare.lpn = 0;
		//Write::spare.start = 0;
		//Write::spare.fpn = 0;	
		//Write::spare.prev = -1;

		//Write::spare.vectorCount = 0;

		//Write::spare.vecFstOff = -1;
		//Write::spare.vecFstLen = 0;

		//Write::spare.vecLstOff = -1;
		//Write::spare.vecLstLen = 0;

		//Write::wr_prev_pg = -1;
		//Write::wr_prev_col = 0;

		//Write::spare.fbb = 0;		
		//Write::spare.fbp = 0;		

		//Write::spare.chipMask = nandSize.mask;	
		
		Erase::Start(wr.chip, wr.block, true, false);
		while (Erase::Update());

		adrLastVector = -1;

		return;
	}
	else if (spare.crc == 0)
	{
		// ��������� ���

		//wr.SetRawBlock(bs);
		//wr.NextBlock();

		Write::Init(bs+1, 1, -1);

		//Write::spare.file = spare.file + 1;  
		//Write::spare.lpn = wr.GetRawPage();

		//Write::spare.prev = spare.start;		

		//Write::spare.start = wr.GetRawPage();		
		//Write::spare.fpn = 0;	

		//Write::spare.vectorCount = 0;

		//Write::spare.vecFstOff = -1;
		//Write::spare.vecFstLen = 0;

		//Write::spare.vecLstOff = -1;
		//Write::spare.vecLstLen = 0;

		//Write::wr_prev_pg = -1;
		//Write::wr_prev_col = 0;

		//Write::spare.fbb = 0;		
		//Write::spare.fbp = 0;		

		//Write::spare.chipMask = nandSize.mask;

		Erase::Start(wr.chip, wr.block, true, false);
		while (Erase::Update());



		//Write::spare.file = spare.file;
		//Write::spare.start = spare.start;
		//Write::spare.fpn = spare.fpn;	
		//Write::spare.prev = spare.prev;
	}
	else
	{

	};

	// ����� ��������� ���������� �������� � �����

	lastSessionBlock = bs;

	bs = rd.GetRawPage();
	be = bs + (1<<NAND_PAGE_BITS) - 1;

	bm = (be+bs)/2;

	rd.SetRawPage(bm);

	while (be > bs)
	{
		ReadSpareNow(&spare, &rd, false);

		if (spare.validPage != 0xFFFF)
		{
			rd.NextPage();
			bm = rd.GetRawPage();
		}
		else
		{
			if (/*spare.lpn == -1 ||*/ spare.start == -1 || spare.fpn == -1 )
			{
				be = bm-1;
			}
			else
			{
				bs = bm;
			};

			bm = (bs + be + 1) / 2;

			rd.SetRawPage(bm);
		};
	};

	lastSessionPage = bs;

	rd.SetRawPage(bs);

	ReadSpareNow(&spare, &rd, true);

	while (spare.vecLstOff == 0xFFFF || spare.crc != 0)
	{
		rd.PrevPage();
		
		ReadSpareNow(&spare, &rd, true);
	}

	adrLastVector = rd.GetRawAdr() + spare.vecLstOff;



	static VecData::Hdr hdr;

	rd.raw = adrLastVector;

	ReadSpareNow(&spare, &rd, true);

	lastSessionInfo.size = (u64)lastSessionPage << NAND_COL_BITS;

	lastSessionInfo.last_adress = adrLastVector;

	ReadVecHdrNow(&hdr, &rd);

	if (hdr.crc == 0)
	{
		lastSessionInfo.stop_rtc = hdr.rtc;
	}
	else
	{

	};

	rd.SetRawPage(0);

	ReadSpareNow(&spare, &rd, true);

	while (spare.validPage != 0xFFFF || spare.validBlock != 0xFFFF)
	{
		if (spare.validBlock != 0xFFFF)
		{
			rd.NextBlock();
			ReadSpareNow(&spare, &rd, true);
		}
		else if (spare.validPage != 0xFFFF)
		{
			rd.NextPage();
			ReadSpareNow(&spare, &rd, true);
		};
	};

	lastSessionInfo.session = spare.file;
	lastSessionInfo.flags = 0;

	ReadVecHdrNow(&hdr, &rd);

	if (hdr.crc == 0)
	{
		lastSessionInfo.start_rtc = hdr.rtc;
	}
	else
	{

	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool GetSessionNow(SessionInfo *si)
{
	static SpareArea spare;

	static VecData::Hdr hdr;

	FLADR rd(0,0,0,0);

	rd.raw = adrLastVector;

	ReadSpareNow(&spare, &rd, true);

	si->size = (u64)spare.fpn << NAND_COL_BITS;

	si->last_adress = adrLastVector;
	si->session = spare.file;

	ReadVecHdrNow(&hdr, &rd);

	if (hdr.crc == 0)
	{
		si->stop_rtc = hdr.rtc;
	}
	else
	{

	};

	rd.SetRawPage(spare.start);

	ReadVecHdrNow(&hdr, &rd);

	if (hdr.crc == 0)
	{
		si->start_rtc = hdr.rtc;
	}
	else
	{

	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void StartSendSession()
{
	cmdSendSession = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool UpdateSendSession()
{
	enum {	WAIT = 0, READ_START, READ_1, READ_2, READ_PAGE,READ_PAGE_1,FIND_START,FIND_1,FIND_2,FIND_3,FIND_4, READ_END};

	static byte i = WAIT;
	
	static FLADR rs(0), re(0);
	static u32 bs = 0, be = 0;

	static SpareArea spare;
	static u16 sid = 0;

	static FLRB flrb;

	static RTC srtc, ertc;

	static u16 ind = 0;
	static u32 prgrss = 0;
	static u16 count = 0;

	SessionInfo &s = nvsi[ind].si;

	switch (i)
	{
		case 0:

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

		case 1:

			if (TRAP_MEMORY_SendSession(s.session, s.size, s.last_adress, s.start_rtc, s.stop_rtc, s.flags))
			{
				ind = (ind-1)&127;

				prgrss += 0x100000000/128;

				count--;

				i++;
			};

			break;

		case 2:

			if (TRAP_MEMORY_SendStatus(prgrss, FLASH_STATUS_READ_SESSION_IDLE))
			{
				if (s.size > 0 && count > 0)
				{
					i = 1;
				}
				else
				{
					i++;
				};
			};

			break;

		case 3:

			if (TRAP_MEMORY_SendStatus(-1, FLASH_STATUS_READ_SESSION_READY))
			{
				cmdSendSession = false;

				i = 0;
			};

			break;
	};

	return true;
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

	switch (nandState)
	{
		case NAND_STATE_WAIT: 

			if (cmdCreateNextFile)
			{
				Write::CreateNextFile();
				cmdCreateNextFile = false;
				nandState = NAND_STATE_CREATE_FILE;

				break;
			};

			if (cmdFullErase)
			{
				cmdFullErase = false;
				nandState = NAND_STATE_FULL_ERASE_START;

				break;
			};

			if (cmdSendSession)
			{
				nandState = NAND_STATE_SEND_SESSION;

				break;
			};

			if (Write::Start())
			{
				nandState = NAND_STATE_WRITE_START;
			}
			else if (Read::Start())
			{
				nandState = NAND_STATE_READ_START;
			};

			break;

		case NAND_STATE_WRITE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Write::Update())
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

			Erase::Start(0, 0, true, true);

			nandState = NAND_STATE_FULL_ERASE_0;

			break;

		case NAND_STATE_FULL_ERASE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Erase::Update())
			{
				invalidBlocks += Erase::errBlocks;

				t -= Erase::errBlocks;
				t -= 1;

				Write::errVec = t;

				if (t > 0)
				{
					er.NextBlock();

					Erase::Start(er.chip, er.block, true, true);

					if (tm.Check(500)) { TRAP_MEMORY_SendStatus((eb-t)*((u64)0x100000000)/eb, FLASH_STATUS_BUSY); };
				}
				else
				{
					flashEmpty = true;

					Write::Init(0, 1, -1);

					adrLastVector = -1;

					TRAP_MEMORY_SendStatus(-1, FLASH_STATUS_ERASE);
					nandState = NAND_STATE_WAIT;
				};
			};

			break;

		case NAND_STATE_CREATE_FILE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Write::Update())
			{
				nandState = NAND_STATE_WAIT;
			};

			break;

		case NAND_STATE_SEND_SESSION:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!UpdateSendSession())
			{
				nandState = NAND_STATE_WAIT;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void NAND_Init()
{
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

	NAND_Chip_Select(0); // ���� ����

	DisableWriteProtect();
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//flash_status_type		flash_read_status = FLASH_STATUS_WAIT;          // ��������� ��������� ��������
//flash_status_type 		flash_write_status = FLASH_STATUS_WAIT;
//flash_status_type 		flash_verify_status = FLASH_STATUS_WAIT;
//
//i64 	flash_current_adress;
//
//u32 	flash_vectors_errors = 0;
//u32 	flash_vectors_saved = 0;
//
//u16 	flash_save_size = 0;
//byte	flash_save_buffer[FLASH_SAVE_BUFFER_SIZE];
//byte	flash_save_repeat_counter = 0;
//
//flash_save_repeat_type		flash_save_repeat = FLASH_SAVE_REPEAT_NONE;
//
//byte	flash_read_buffer[FLASH_READ_BUFFER_SIZE];
//u64 	flash_read_vector_adress;
//u16 	*flash_read_vector_size_p;
//bool	*flash_read_vector_ready_p;
//byte	*flash_read_vector_p;


//flash_status_operation_type 	flash_status_operation = FLASH_STATUS_OPERATION_WAIT;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Errors_Get()
{
	return Write::errVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Saved_Get()
{
	return Write::spare.vectorCount;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Recieved_Get()
{
	return Write::rcvVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Vectors_Rejected_Get()
{
	return Write::rejVec;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 FLASH_Session_Get()
{
	return Write::spare.file;
}

//void inline FLASH_Vectors_Errors_Reset()
//{
//	flash_vectors_errors = 0;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void inline FLASH_Vectors_Saved_Reset()
//{
//	flash_vectors_saved = 0;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Read_Is_Ready()
//{
//	if(flash_read_status == FLASH_STATUS_READY)
//	{
//		flash_read_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Read_Is_Error()
//{
//	if(flash_read_status == FLASH_STATUS_ERROR)
//	{
//		flash_read_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Write_Is_Ready()
//{
//	if(flash_write_status == FLASH_STATUS_READY)
//	{
//		flash_write_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Write_Is_Error()
//{
//	if(flash_write_status == FLASH_STATUS_ERROR)
//	{
//		flash_write_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Verify_Is_Ready()
//{
//	if(flash_verify_status == FLASH_STATUS_READY)
//	{
//		flash_verify_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool inline FLASH_Verify_Is_Error()
//{
//	if(flash_verify_status == FLASH_STATUS_ERROR)
//	{
//		flash_verify_status = FLASH_STATUS_WAIT;
//		return true;
//	}	
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool /*inline*/ FLASH_Busy()
//{
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return true;
//	return false;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Current_Adress_Get()
{
	return wr.GetRawAdr();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Full_Size_Get()
{
	return nandSize.fl;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 FLASH_Used_Size_Get()
{
	i64 size = FLASH_Full_Size_Get();

	//if(FRAM_Memory_Start_Adress_Get() != -1)
	//{
	//	size = FRAM_Memory_Current_Adress_Get() - FRAM_Memory_Start_Adress_Get();
	//	if(size < 0) size += FLASH_Full_Size_Get();
	//}

	return size;
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

// ������ �� ���������� ������
//bool FLASH_Read(u64 *adr, byte *data, u16 size)
//{
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
//	if(!NAND_Read_Data(adr, data, size)) return false;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//bool FLASH_Read_Vector(u64 adr, u16 *size, bool *ready, byte **vector)
//{
	//if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT)
	//{
	//	return false;
	//}
	//flash_read_vector_adress = adr;
	//flash_read_vector_size_p = size;
	//*flash_read_vector_size_p = 0;
	//flash_read_vector_ready_p = ready;
	//*flash_read_vector_ready_p = false;
	//*vector = flash_read_buffer;
	//flash_status_operation = FLASH_STATUS_OPERATION_READ_WAIT;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ��������� �� ���������� ������

//bool FLASH_Verify(u64 *adr, byte *data, u16 size)
//{
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
//	if(!NAND_Verify_Data(adr, data, size)) return false;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ����� �� ���������� ������

//bool FLASH_Write(u64 *adr, byte *data, u16 size)
//{
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
//	if(!NAND_Write_Data(false, adr, data, size)) return false;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// ����� �� ���������� ������ �������� �� ���� ����, ���������� ������������ �����

//bool FLASH_Write_Next(u64 *adr, byte *data, u16 size)
//{
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
//	if(!NAND_Write_Data(true, adr, data, size)) return false;
//	return true;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// ����� ������

//bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat)
//{
//	if((flash_status_operation != FLASH_STATUS_OPERATION_WAIT) || (size + sizeof(vector_type) > FLASH_SAVE_BUFFER_SIZE))
//	{
//		flash_vectors_errors ++;
//		return false;
//	}
//	flash_save_size = Vector_Make(session, device, rtc, flags, data, flash_save_buffer, size);
//	if(flash_save_size == 0)
//	{
//		flash_vectors_errors ++;
//		return false;
//	}
//	flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;
//	flash_save_repeat = repeat;
//	flash_save_repeat_counter = 0;
//	return true;
//}

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

//static bool RequestFunc01(FLWB *fwb, ComPort::WriteBuffer *wb)
//{
//	VecData &vd = fwb->vd;
//
//	Req &req = *((Req*)vd.data);
//
//
//
//
//
//
//
//
//
//	freeFlWrBuf.Add(fwb);
//
//	return CreateRsp01(wb);
//}

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

//static bool RequestFunc03(FLWB *fwb, ComPort::WriteBuffer *wb)
//{
//	VecData &vd = fwb->vd;
//
//	Req &req = *((Req*)vd.data);
//
//
//
//
//
//
//	freeFlWrBuf.Add(fwb);
//
//	return CreateRsp03(wb);
//}

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
		if (GetCRC16(&flwb->vd, flwb->dataLen) == 0)//((req.rw & 0xFF00) == 0xAA00)
		{
			result = RequestFunc02 (fwb, wb);
		}
		else
		{
			freeFlWrBuf.Add(fwb);
			Write::rejVec++;
		};
	};

	return result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitCom()
{
	com1.Connect(1, 6250000, 0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
	__packed struct Req { u16 rw; u32 cnt; u16 gain; u16 st; u16 len; u16 delay; u16 data[512*4]; };

	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static byte state = 0;

	static FLWB *fwb;
	static Req *req;
	static VecData *vd;

	static u32 count = 0;
	static u16 v = 0;
	static byte b = 0;

	//static byte buf[100];

	//wb.data = buf;
	//wb.len = sizeof(buf);

	//HW::PIOA->SODR = 1<<27;

	//if (!com1.Update())
	//{
	//	com1.Write(&wb);
	//};


//	HW::PIOB->SODR = 1<<13;

	switch (state)
	{
		case 0:

			fwb = freeFlWrBuf.Get();

			if (fwb != 0)
			{
				state++;
			};

			break;

		case 1:

			vd = &fwb->vd;
			req = (Req*)vd->data;

			rb.data = vd->data;
			rb.maxLen = sizeof(vd->data);

//			rb.len = sizeof(*req);

			//req->rw = 0xAA30 + ((b & (~7))<<1) + (b & 7);
			//req->cnt = count++;
			//req->gain = 7;
			//req->st = 10;
			//req->len = ArraySize(req->data)/4;
			//req->delay = 0;

			//if (writeFlashEnabled)
			//{
			//	v = 0;

			//	for (u16 i = 0; i < ArraySize(req->data); i++)
			//	{
			//		req->data[i] = v++;
			//	};
			//};

			HW::PIOB->SODR = 1<<13;

			com1.Read(&rb, -1, 100);

			//b += 1;

			//if (b >= 24) { b = 0; };

			state++;

			break;

		case 2:

			if (!com1.Update())
			{
				HW::PIOB->CODR = 1<<13;

				if (rb.recieved)
				{
					fwb->dataLen = rb.len;

					HW::PIOB->SODR = 1<<13;

					if (RequestFunc(fwb, &wb))
					{
						state++;
					}
					else
					{
						state = 0;
					};

					HW::PIOB->CODR = 1<<13;
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
		nvv.si.size = 0;
		GetTime(&nvv.si.start_rtc);
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
//	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	//flash_current_adress = FRAM_Memory_Current_Adress_Get();
	//SetNandAdr(flash_current_adress);
	//FRAM_Memory_Current_Adress_Set(GetNandAdr()); // ��������
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadVars()
{
	twi.Init(1);

	PointerCRC p(buf);

	static DSCTWI dsc;

	dsc.MMR = 0x500200;
	dsc.IADR = 0;
	dsc.CWGR = 0x7575;
	dsc.data = buf;
	dsc.len = sizeof(buf);

	if (twi.Read(&dsc))
	{
		while (twi.Update());
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

		nvv.si.session = 0;
		nvv.si.size = 0;
		nvv.si.last_adress = 0;
		GetTime(&nvv.si.start_rtc);
		GetTime(&nvv.si.stop_rtc);
		nvv.si.flags = 0;

		savesCount = 2;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	const u16 sa = 0x100;

	PointerCRC p(buf);

	static DSCTWI dsc;

	static byte i = 0;
	static RTM32 tm;

	switch (i)
	{
		case 0:

			if (savesCount > 0)
			{
				savesCount--;
				i++;
			}
			else if (savesSessionsCount > 0)
			{
				savesSessionsCount--;
				i = 3;
			};

			break;

		case 1:

			dsc.MMR = 0x500200;
			dsc.IADR = 0;
			dsc.CWGR = 0x07575; 
			dsc.data = buf;
			dsc.len = sizeof(buf);

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&nvv, sizeof(nvv));
				p.WriteW(p.CRC.w);
			};

			i = (twi.Write(&dsc)) ? (i+1) : 0;

			break;

		case 2:

			if (!twi.Update())
			{
				i = 0;
			};

			break;

		case 3:

			NVSI &si = nvsi[nvv.index];

			u32 adr = sa+sizeof(si)*nvv.index;

			dsc.MMR = 0x500200;
			dsc.IADR = adr;
			dsc.CWGR = 0x7575;
			dsc.data = buf;
			dsc.len = sizeof(si);

			p.CRC.w = 0xFFFF;
			p.WriteArrayB(&si, sizeof(si.si));
			p.WriteW(p.CRC.w);

			i = (twi.Write(&dsc)) ? 2 : 0;

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

		u32 adr = sa+sizeof(si)*i;

		dsc.MMR = 0x500200;
		dsc.IADR = adr;
		dsc.CWGR = 0x7575;
		dsc.data = &si;
		dsc.len = sizeof(si);

		if (twi.Read(&dsc))
		{
			while (twi.Update());
		};

		if (GetCRC16(&si, sizeof(si)) != 0)
		{
			loadSessionsOk = false;

			si.si.session = 0;
			si.si.size = 0;
			si.si.last_adress = 0;
			si.si.flags = 0;
			si.si.start_rtc.date = 0;
			si.si.start_rtc.time = 0;
			si.si.stop_rtc.date = 0;
			si.si.stop_rtc.time = 0;
			si.crc = GetCRC16(&si, sizeof(si.si));

			dsc.MMR = 0x500200;
			dsc.IADR = adr;
			dsc.CWGR = 0x07575; 
			dsc.data = &si;
			dsc.len = sizeof(si);

			twi.Write(&dsc);

			while (twi.Update());
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_Init()
{
	LoadVars();

	LoadSessions();

	InitFlashBuffer();

	NAND_Init();

	FLASH_Reset();

	InitCom();

//	__breakpoint(0);

//	cmdFullErase = true;
//	Test2();
 
	InitSessions();

//	SimpleBuildFileTable();

	//static SessionInfo si;

	//GetSessionNow(&si);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_Update()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateCom();	);
		CALL( SaveVars();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
