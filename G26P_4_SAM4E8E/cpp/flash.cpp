//#include "common.h"
#include "flash.h"
#include "fram.h"
#include "vector.h"
#include "core.h"
#include "list.h"
#include <CRC16.h>

#pragma diag_suppress 546

/**************************************************/
/*
Технология записи такова, что первая попытка при провале идёт по следующему адресу,
вторая по следующему блоку, трижды - ошибка.
можно сделать этот цикл бесконечным в целях более быстрого поиска живого места
можно также в случае битых блоков считать ИД памяти и понять а не сдохла ли она совсем, но это муторно реализовывать
*/
/**************************************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define NAND_MAX_CHIP		8
#define NAND_CHIP_MASK		7
#define NAND_CHIP_BITS		3

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

static bool cmdCreateNextFile = false;
static bool cmdFullErase = false;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 	spareRd_block = -1;
static u16 	spareRd_page = -1;
static byte	spareRd_chip = -1;

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

	u16		maskPage;
	u32		maskBlock;

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NandMemSize nandSize;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLADR
{
	u32 	block;
	u16 	page;
	u16		col;
	byte	chip;

	const NandMemSize& sz;

	FLADR(NandMemSize &v, u32 bl, u16 pg, u16 cl, byte ch) : block(bl), page(pg), col(cl), chip(ch), sz(v) {}

	void	NextPage();
	void	NextBlock();
	u32		GetRawPage() { return (((block << NAND_CHIP_BITS) | chip) << sz.bitPage) | page; };
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLADR::NextPage()
{
	col = 0;

	page += 1;

	chip += page >> sz.bitPage;

	page &= sz.maskPage;

	block += chip >> NAND_CHIP_BITS;

	chip &= NAND_CHIP_MASK;

	block &= sz.maskBlock;

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLADR::NextBlock()
{
	col = 0;

	page = 0;

	chip += 1;

	block += chip >> NAND_CHIP_BITS;

	chip &= NAND_CHIP_MASK;

	block &= sz.maskBlock;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static FLADR rd(nandSize, 0, 0, 0, 0);
static FLADR wr(nandSize, 0, 0, 0, 0);
static FLADR er(nandSize, -1, -1, -1, -1);

//static u64	cur_adr;
static byte*	rd_data = 0;
//static u32 		rd_row = 0;
//static u32 		rd_block = 0;
//static u16 		rd_page = 0;
static u16		rd_count = 0;
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


static u16	cur_col;
static u32 	cur_row;
static byte	cur_chip;
static u16	cur_size;
static u16	cur_count;
static byte*cur_data;
static u16	cur_verify_errors;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

enum flash_status_type
{
	FLASH_STATUS_WAIT = 0,
	FLASH_STATUS_READY,
	FLASH_STATUS_ERROR,
} ;


enum flash_status_operation_type
{
	FLASH_STATUS_OPERATION_WAIT = 0,
	FLASH_STATUS_OPERATION_SAVE_WAIT,
	FLASH_STATUS_OPERATION_SAVE_WRITE,
	FLASH_STATUS_OPERATION_SAVE_VERIFY,
	FLASH_STATUS_OPERATION_READ_WAIT,
	FLASH_STATUS_OPERATION_READ_HEADER,
	FLASH_STATUS_OPERATION_READ_DATA,
};

enum NandState
{
	NAND_STATE_WAIT = 0,
	NAND_STATE_WRITE_START,
	NAND_STATE_READ_START,
	NAND_STATE_FULL_ERASE_START,
	NAND_STATE_FULL_ERASE_0,
	NAND_STATE_CREATE_FILE
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static NandState nandState = NAND_STATE_WAIT;

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

void NAND_Chip_Select(byte chip) 
{    
	if(chip < 8)                   
	{ 				
		HW::PIOA->SODR = maskChipSelect ^ chipSelect[chip];
		HW::PIOA->CODR = chipSelect[chip];
	};
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetNandAdr(u64 a)
{
//	while(a < 0) a += ((i64)1 << (nandSize.full)) * (((-a) >> (nandSize.full)) + 1);
	a &= nandSize.fl - 1;
	cur_chip = a >> nandSize.shCh;
	cur_row = a >> nandSize.shPg;
	cur_col = a & (nandSize.pg-1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Adress_Set_Next(u64 a)
{
//	while(a < 0) a += ((i64)1 << (nandSize.full)) * (((-a) >> (nandSize.full)) + 1);

	a += nandSize.bl;
	a &= (nandSize.fl - 1);

	cur_chip = a >> nandSize.shCh;
	cur_row = a >> nandSize.shPg;
	cur_col = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 GetNandAdr()
{
	return (nandSize.ch * cur_chip) + ((u64)nandSize.pg * cur_row) + cur_col;
}

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

bool NAND_Read_Data(u64 *adr, byte *data, u16 size)
{
	if(nandState != NAND_STATE_WAIT) return false;
	SetNandAdr(*adr);	
	cur_size = size;
	cur_count = 0;
	cur_data = data;
	nandState = NAND_STATE_READ_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_Verify_Data(u64 *adr, byte *data, u16 size)
{
	if(nandState != NAND_STATE_WAIT) return false;
	SetNandAdr(*adr);	
	cur_size = size;
	cur_count = 0;
	cur_data = data;
	cur_verify_errors = 0;
//	nandState = NAND_STATE_VERIFY_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool NAND_Write_Data(bool next, u64 *adr, byte *data, u16 size)
{
	if(nandState != NAND_STATE_WAIT) return false;
	if(next)
	{
		NAND_Adress_Set_Next(*adr);
		*adr = GetNandAdr();
	}
	else SetNandAdr(*adr);
	cur_size = size;
	cur_count = 0;
	cur_data = data;         
	nandState = NAND_STATE_WRITE_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u32 ROW(u32 block, u16 page)
{
	return (block << nandSize.bitPage) + page;
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
	static bool force = false;	// стереть полюбасу
	static bool check = true;	// Проверить результат стирания
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
				CmdReadPage(nandSize.pg, er.block, 0);																					
																																	
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

					state = ERASE_START; 																			
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
																																
					CmdWritePage(er.sz.pg, er.block, 0);																			
																																
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
																																
				state = ERASE_START; 
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

	static SpareArea spare;

	static byte state;

	static bool createFile = false;

	static u16	wr_cur_col = 0;
	static u32 	wr_cur_pg = -1;
	static u16	wr_prev_col = 0;
	static u32 	wr_prev_pg = -1;
	static byte	wr_pg_error = 0;

	static u16		wr_count = 0;
	static byte*	wr_data = 0;
	static byte*	wr_ptr = 0;

	static bool Start();
	static bool Update();
	static void CreateNextFile();


};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Write::Start()
{
	if ((curWrBuf = writeFlBuf.Get()) != 0)
	{
		wr_data = curWrBuf->data + curWrBuf->hdrLen;
		wr_count = curWrBuf->dataLen;

		wr_cur_col = wr.col;
		wr_cur_pg = wr.GetRawPage();

		if (spare.vecFstOff == 0xFFFF)
		{
			spare.vecFstOff = wr.col;
			spare.vecFstLen = wr_count;
		};

		spare.vecLstOff = wr.col;
		spare.vecLstLen = wr_count;

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

static bool Write::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case WRITE_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
			
//			NAND_Chip_Select(wr.chip);

			register u16 c = wr.sz.pg - wr.col;

			if (wr.col == 0 && wr_count >= wr.sz.pg) // писать сразу во флеш
			{
				wr_ptr = wr_data;
				wr_count -= wr.sz.pg;
				wr_data += wr.sz.pg;

				state = WRITE_PAGE;
			}
			else // писать в буфер
			{
				if (wr_count < c ) c = wr_count;

				BufWriteData(wr_data, c);

				wr.col += c;
				wr_data += c;
				wr_count -= c;

				state = WRITE_BUFFER;
			};

			break;
	
		case WRITE_BUFFER:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (wr.col >= wr.sz.pg)
				{
					wr_ptr = wrBuf;
					wr.col = 0;

					state = WRITE_PAGE;
				}
				else if (wr_count > 0)
				{
					state = WRITE_START;
				}
				else
				{
					curWrBuf->ready[0] = true;

					freeFlWrBuf.Add(curWrBuf);

					curWrBuf = 0;

					state = WAIT;

					return false;
				};
			};

			break;

		case WRITE_PAGE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(er.block != wr.block || er.chip != wr.chip)	// новый блок
			{
				wr_pg_error = 0;

				Erase::Start(wr.chip, wr.block, false, true);

	            state = ERASE;

				break;
			};

		case WRITE_PAGE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			NAND_Chip_Select(wr.chip);

			CmdWritePage(0, wr.block, wr.page);

			NandWriteData(wr_ptr, wr.sz.pg);
			
			state = WRITE_PAGE_1;

			break;

		case WRITE_PAGE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				spare.validPage = -1;
				spare.validBlock = -1;
				spare.badPages = -1;

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
					__breakpoint(0);

					spare.fbp += 1;

					CmdWritePage(nandSize.pg, wr.block, wr.page);

					*(u16*)FLD = 0; // spareErase.validPage = 0;

					CmdWritePage2();

					state = WRITE_PAGE_4;
				}
				else if (wr_count > 0)
				{
					state = WRITE_START;
				}
				else
				{
					curWrBuf->ready[0] = true;

					freeFlWrBuf.Add(curWrBuf);

					curWrBuf = 0;

					wr_prev_col = wr_cur_col;
					wr_prev_pg = wr_cur_pg;

					state = (createFile) ? WRITE_CREATE_FILE_1 : WAIT;
				};

				spare.lpn += 1;
				spare.fpn += 1;

				spare.vecFstOff = -1;
				spare.vecFstLen = 0;

				spare.vecLstOff = -1;
				spare.vecLstLen = 0;

				spare.vecPrPg = wr_prev_pg;
				spare.vecPrOff = wr_prev_col;

				wr.NextPage();
			};

			break;

		case WRITE_PAGE_4:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++			

			if(!NAND_BUSY())																									
			{																													
				if (wr_pg_error == 2) // пометить блок как имеющий сбойные страницы
				{
					CmdWritePage(wr.sz.pg + sizeof(spare.validPage) + sizeof(spare.validBlock), wr.block, 0);

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
				wr.block = er.block;
				wr.chip = er.chip;

				spare.fbb += Erase::errBlocks;

				state = WRITE_PAGE_0;																			
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
			};

			spare.start = wr.GetRawPage();		
			spare.fpn = 0;		

			wr_prev_pg = -1;
			wr_prev_col = 0;

			spare.fbb = 0;		
			spare.fbp = 0;		

			spare.chipMask = nandSize.mask;	

			state = WAIT;

			return false;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace Read
{
	enum {	WAIT = 0,READ_START,READ_1, READ_2, READ_PAGE,READ_PAGE_1};

	static SpareArea spare;

	static byte state;

	static bool Start();
	static bool Update();

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read::Start()
{
	if ((curRdBuf = readFlBuf.Get()) != 0)
	{
		rd_data = curRdBuf->data;
		rd_count = curRdBuf->maxLen;
		curRdBuf->len = 0;	
		return true;
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Read::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case READ_START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			NAND_Chip_Select(rd.chip);

			if ((rd.block != spareRd_block) || (rd.chip != spareRd_chip) || (rd.page != spareRd_page))
			{
				CmdReadPage(rd.sz.pg, rd.block, rd.page);

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
				if (spare.validPage != 0xFFFF)
				{
					rd.NextPage();

					CmdReadPage(nandSize.pg, rd.block, rd.page);

					state = READ_1;
				}
				else
				{
					spareRd_block = rd.block;
					spareRd_chip = rd.chip;
					spareRd_page = rd.page;

					CmdRandomRead(rd.col);

					state = READ_PAGE;
				};
			};

			break;

		case READ_PAGE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				register u16 c = rd.sz.pg - rd.col;

				if (rd_count < c) c = rd_count;

				NandReadData(rd_data, c);

				rd_count -= c;
				rd.col += c;
				rd_data += c;
				curRdBuf->len += c;

				state = READ_PAGE_1;
			};

			break;

		case READ_PAGE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd.col >= rd.sz.pg)
				{
					rd.NextPage();

					state = READ_START;
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
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace ReadPage
{
	enum {	WAIT = 0,START,READ_1};

	static byte state;
	static void *dst;
	static u16 len;

	static bool Start(void *d, u16 l, byte chip, u32 block, u16 page, u16 col);
	static bool Update();

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReadPage::Start(void *d, u16 l, byte chip, u32 block, u16 page, u16 col)
{
	dst = d;
	len = l;

	NAND_Chip_Select(chip);

	CmdReadPage(col, block, page);

	state = START;

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReadPage::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if(!NAND_BUSY())
			{
				NandReadData(dst, len);

				state = READ_1;
			};

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				state = WAIT;

				return false;
			};

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

namespace BuildFileTable
{
	enum {	WAIT = 0,START,READ_1, READ_2, READ_PAGE,READ_PAGE_1};

	static FLADR rd(nandSize, 0, 0, 0, 0);

	static SpareArea spare;

	static byte state;

	static bool Start();
	static bool Update();

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool BuildFileTable::Start()
{
	rd.block = 1 << (nandSize.bitBlock-1);
	rd.page = 0;
	rd.chip = 0;

	state = START;

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool BuildFileTable::Update()
{
	switch(state)
	{
		case WAIT:	return false;

		case START:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			ReadPage::Start(&spare, sizeof(spare), rd.chip, rd.block, rd.page, rd.sz.pg);

			state = READ_1;

			break;

		case READ_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!ReadPage::Update())
			{
				state = READ_2;
			};

			break;

		case READ_2:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (spare.validBlock != 0xFFFF)
				{
					rd.NextBlock();

					NAND_Chip_Select(rd.chip);
					
					CmdReadPage(nandSize.pg, rd.block, rd.page);

					state = READ_1;
				}				
				else if (spare.validPage != 0xFFFF)
				{
					rd.NextPage();

					CmdReadPage(nandSize.pg, rd.block, rd.page);

					state = READ_1;
				}
				else
				{
					spareRd_block = rd.block;
					spareRd_chip = rd.chip;
					spareRd_page = rd.page;

					CmdRandomRead(rd.col);

					state = READ_PAGE;
				};
			};

			break;

		case READ_PAGE:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if(!NAND_BUSY())
			{
				register u16 c = rd.sz.pg - rd.col;

				if (rd_count < c) c = rd_count;

				NandReadData(rd_data, c);

				rd_count -= c;
				rd.col += c;
				rd_data += c;
				curRdBuf->len += c;

				state = READ_PAGE_1;
			};

			break;

		case READ_PAGE_1:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

			if (CheckDataComplete())
			{
				if (rd.col >= rd.sz.pg)
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
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool NAND_Idle()
{
//	register u32 t;

	static i32 t = 0;

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

			t = nandSize.fl >> nandSize.shBl; // blocks count

			Erase::Start(0, 0, true, true);

			nandState = NAND_STATE_FULL_ERASE_0;

			break;

		case NAND_STATE_FULL_ERASE_0:	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			if (!Erase::Update())
			{
				invalidBlocks += Erase::errBlocks;

				t -= Erase::errBlocks;
				t -= 1;

				if (t > 0)
				{
					er.NextBlock();

					Erase::Start(er.chip, er.block, true, true);
				}
				else
				{
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
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void NAND_Init()
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
				nandSize.maskPage = (1 << (nandSize.bitPage = nandSize.shBl - nandSize.shPg)) - 1;
				nandSize.maskBlock = (1 << (nandSize.bitBlock = nandSize.shCh - nandSize.shBl)) - 1;
			};
			
			nandSize.fl += nandSize.ch;
			
			nandSize.mask |= (1 << chip);
		};
	};

	NAND_Chip_Select(0); // пока один

	DisableWriteProtect();
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

flash_status_type		flash_read_status = FLASH_STATUS_WAIT;          // результат последней операции
flash_status_type 		flash_write_status = FLASH_STATUS_WAIT;
flash_status_type 		flash_verify_status = FLASH_STATUS_WAIT;

i64 	flash_current_adress;

u32 	flash_vectors_errors = 0;
u32 	flash_vectors_saved = 0;

u16 	flash_save_size = 0;
byte	flash_save_buffer[FLASH_SAVE_BUFFER_SIZE];
byte	flash_save_repeat_counter = 0;

flash_save_repeat_type		flash_save_repeat = FLASH_SAVE_REPEAT_NONE;

byte	flash_read_buffer[FLASH_READ_BUFFER_SIZE];
u64 	flash_read_vector_adress;
u16 	*flash_read_vector_size_p;
bool	*flash_read_vector_ready_p;
byte	*flash_read_vector_p;


flash_status_operation_type 	flash_status_operation = FLASH_STATUS_OPERATION_WAIT;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 inline FLASH_Vectors_Errors_Get()
{
	return flash_vectors_errors;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 inline FLASH_Vectors_Saved_Get()
{
	return flash_vectors_saved;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void inline FLASH_Vectors_Errors_Reset()
{
	flash_vectors_errors = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void inline FLASH_Vectors_Saved_Reset()
{
	flash_vectors_saved = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Read_Is_Ready()
{
	if(flash_read_status == FLASH_STATUS_READY)
	{
		flash_read_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Read_Is_Error()
{
	if(flash_read_status == FLASH_STATUS_ERROR)
	{
		flash_read_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Write_Is_Ready()
{
	if(flash_write_status == FLASH_STATUS_READY)
	{
		flash_write_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Write_Is_Error()
{
	if(flash_write_status == FLASH_STATUS_ERROR)
	{
		flash_write_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Verify_Is_Ready()
{
	if(flash_verify_status == FLASH_STATUS_READY)
	{
		flash_verify_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool inline FLASH_Verify_Is_Error()
{
	if(flash_verify_status == FLASH_STATUS_ERROR)
	{
		flash_verify_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool /*inline*/ FLASH_Busy()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return true;
	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i64 FLASH_Current_Adress_Get()
{
	return flash_current_adress;
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
	if(FRAM_Memory_Start_Adress_Get() != -1)
	{
		size = FRAM_Memory_Current_Adress_Get() - FRAM_Memory_Start_Adress_Get();
		if(size < 0) size += FLASH_Full_Size_Get();
	}
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

// удалить 
bool FLASH_Erase_Full()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	FRAM_Memory_Start_Adress_Set(FRAM_Memory_Current_Adress_Get());
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// восстановить
bool FLASH_UnErase_Full()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	FRAM_Memory_Start_Adress_Set(-1);
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// читает по указанному адресу
bool FLASH_Read(u64 *adr, byte *data, u16 size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!NAND_Read_Data(adr, data, size)) return false;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FLASH_Read_Vector(u64 adr, u16 *size, bool *ready, byte **vector)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT)
	{
		return false;
	}
	flash_read_vector_adress = adr;
	flash_read_vector_size_p = size;
	*flash_read_vector_size_p = 0;
	flash_read_vector_ready_p = ready;
	*flash_read_vector_ready_p = false;
	*vector = flash_read_buffer;
	flash_status_operation = FLASH_STATUS_OPERATION_READ_WAIT;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// проверяет по указанному адресу

bool FLASH_Verify(u64 *adr, byte *data, u16 size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!NAND_Verify_Data(adr, data, size)) return false;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// пишет по указанному адресу

bool FLASH_Write(u64 *adr, byte *data, u16 size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!NAND_Write_Data(false, adr, data, size)) return false;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// пишет по указанному адресу смещаясь на новы блок, возвращает исправленный адрес

bool FLASH_Write_Next(u64 *adr, byte *data, u16 size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!NAND_Write_Data(true, adr, data, size)) return false;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// пишет вектор

bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat)
{
	if((flash_status_operation != FLASH_STATUS_OPERATION_WAIT) || (size + sizeof(vector_type) > FLASH_SAVE_BUFFER_SIZE))
	{
		flash_vectors_errors ++;
		return false;
	}
	flash_save_size = Vector_Make(session, device, rtc, flags, data, flash_save_buffer, size);
	if(flash_save_size == 0)
	{
		flash_vectors_errors ++;
		return false;
	}
	flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;
	flash_save_repeat = repeat;
	flash_save_repeat_counter = 0;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FLASH_Reset()
{
	if(!ResetNand()) return false;
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	flash_current_adress = FRAM_Memory_Current_Adress_Get();
	SetNandAdr(flash_current_adress);
	FRAM_Memory_Current_Adress_Set(GetNandAdr()); // проверка
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_Init()
{
	InitFlashBuffer();
	NAND_Init();
	FLASH_Reset();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FLASH_Idle()
{
	switch (GetNandState())	// сделано так, чтобы можно было отсюда дублировать запись в случае неудачи
	{
		//case NAND_STATE_WAIT:	// для скорости
		//	break;
		//case NAND_STATE_READ_READY: 
		//	flash_read_status = FLASH_STATUS_READY;
		//	ResetNandState();	
		//	break;
		//case NAND_STATE_READ_ERROR: 
		//	flash_read_status = FLASH_STATUS_ERROR;
		//	ResetNandState();	
		//	break;
		//case NAND_STATE_WRITE_READY: 
		//	flash_write_status = FLASH_STATUS_READY;
		//	ResetNandState();
		//	break;
		//case NAND_STATE_WRITE_ERROR: 
		//	flash_write_status = FLASH_STATUS_ERROR;
		//	ResetNandState();	
		//	break;
		//case NAND_STATE_VERIFY_READY: 
		//	flash_verify_status = FLASH_STATUS_READY;
		//	ResetNandState();
		//	break;
		//case NAND_STATE_VERIFY_ERROR: 
		//	flash_verify_status = FLASH_STATUS_ERROR;
		//	ResetNandState();	
		//	break;
	}

	if(GetNandState() == NAND_STATE_WAIT) flash_current_adress = GetNandAdr();	

	// операция записи вектора
	i64 current_adress_old = FRAM_Memory_Current_Adress_Get();
	if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_WRITE)
	{
		if(FLASH_Write_Is_Ready()) 
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			u64 adr = FRAM_Memory_Current_Adress_Get();
			if(FLASH_Verify(&adr, flash_save_buffer, flash_save_size))
			{
				flash_status_operation = FLASH_STATUS_OPERATION_SAVE_VERIFY;
			}
			else
			{	
				FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
				flash_vectors_errors ++;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		if(FLASH_Write_Is_Error())
		{
			FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
			flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;		
		}
	}
	else
		if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_VERIFY)
		{
			if(FLASH_Verify_Is_Ready()) 
			{
				FRAM_Memory_Current_Vector_Adress_Set(FRAM_Memory_Current_Adress_Get());
				FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());	
				flash_vectors_saved ++;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
			if(FLASH_Verify_Is_Error())
			{
				FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
				flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;		
			}
		}

		if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_WAIT)
		{
			if(flash_save_repeat_counter <= 1)	// певичный, повтор по сл.адресу
			{
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				u64 adr = FRAM_Memory_Current_Adress_Get();
				if(FLASH_Write(&adr, flash_save_buffer, flash_save_size))
				{
					FRAM_Memory_Current_Adress_Set(adr);
					flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WRITE;		
				} 
				else
				{
					flash_vectors_errors ++;
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				}
			}
			else
				if(flash_save_repeat_counter <= flash_save_repeat)
				{
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
					u64 adr = FRAM_Memory_Current_Adress_Get();
					if(FLASH_Write_Next(&adr, flash_save_buffer, flash_save_size))
					{
						FRAM_Memory_Current_Adress_Set(adr);
						flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WRITE;		
					} 
					else
					{
						flash_vectors_errors ++;
						flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
					}
				}
				else
				{
					flash_vectors_errors ++;
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				}
				flash_save_repeat_counter ++;
		}	
		if(FRAM_Memory_Start_Adress_Get() != -1) 	//check full size
		{
			u64 current_adress_new = FRAM_Memory_Current_Adress_Get();
			if(current_adress_old != current_adress_new) // чтобы не считать за зря
			{
				i64 size_old = current_adress_old - FRAM_Memory_Start_Adress_Get();
				if(size_old < 0) size_old += FLASH_Full_Size_Get();
				i64 size_new = current_adress_new - FRAM_Memory_Start_Adress_Get();
				if(size_new < 0) size_new += FLASH_Full_Size_Get();
				if(size_new < size_old) FRAM_Memory_Start_Adress_Set(-1);
			}
		}

		// операция чтения вектора
		if(flash_status_operation == FLASH_STATUS_OPERATION_READ_HEADER)
		{
			if(FLASH_Read_Is_Ready()) 
			{
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				u16 size_vector;
				if(Vector_Check_Header(flash_read_buffer, &size_vector))
				{
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;
					
					if((size_vector <= FLASH_READ_BUFFER_SIZE) && (FLASH_Read(&flash_read_vector_adress, flash_read_buffer, size_vector)))
					{
						*flash_read_vector_size_p = size_vector;
						flash_status_operation = FLASH_STATUS_OPERATION_READ_DATA;		
					}      	
					else
					{
						*flash_read_vector_size_p = 0;
						*flash_read_vector_ready_p = true;
						flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
					}
				}
				else
				{	
					*flash_read_vector_size_p = 0;
					*flash_read_vector_ready_p = true;
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				}
			}
			if(FLASH_Read_Is_Error())
			{
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = true;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		else if(flash_status_operation == FLASH_STATUS_OPERATION_READ_DATA)
		{
			if(FLASH_Read_Is_Ready()) 
			{
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				if(Vector_Check_Data(flash_read_buffer))
				{
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;
					*flash_read_vector_ready_p = true;
				}
				else
				{	
					*flash_read_vector_size_p = 0;
					*flash_read_vector_ready_p = true;
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				}
			}
			if(FLASH_Read_Is_Error())
			{
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = true;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}

		if(flash_status_operation == FLASH_STATUS_OPERATION_READ_WAIT)
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			if(FLASH_Read(&flash_read_vector_adress, flash_read_buffer, sizeof(vector_header_type)))
			{
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = false;
				flash_status_operation = FLASH_STATUS_OPERATION_READ_HEADER;		
			} 
			else
			{
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = true;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}	
}
