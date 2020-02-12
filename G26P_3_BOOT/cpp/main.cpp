#include "hardware.h"
#include "time.h"
#include "ComPort.h"
#include "crc16.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//const char build_date[] __attribute__((at(0x2E0))) = __DATE__;
//const char build_time[] __attribute__((at(0x2F0))) = __TIME__;
//const char build_date[] __attribute__((used)) = "\n" __DATE__ "\n" __TIME__ "\n";

#define US2CLK(x) ((x)*(MCK/1000000))
#define MS2CLK(x) ((x)*(MCK/1000))

#define SGUID	0xBA332660A2364233	//0x93F15E77DCC1465c
#define MGUID	0x81396E6BDAF74c83	//0x3C60B1B237804398
#define FRDY 1
#define FCMDE 2
#define FLOCKE 4
#define PAGESIZE 64
#define PAGEDWORDS (PAGESIZE>>2)
#define PAGES_IN_SECTOR 16
#define SECTORSIZE (PAGESIZE*PAGES_IN_SECTOR)
#define SECTORDWORDS (SECTORSIZE>>2)
#define PLANESIZE 0x4000
#define START_SECTOR 4
#define START_PAGE (START_SECTOR*PAGES_IN_SECTOR)

#define BOOTSIZE (SECTORSIZE*START_SECTOR)
#define FLASH0 0x000000
#define FLASH_START (FLASH0+BOOTSIZE)

//#define FLASH1 (FLASH0+PLANESIZE)

#define FLASHEND (FLASH0+PLANESIZE)

#define IAP_LOCATION 0X1FFF1FF1
static u32 command_param[5];
static u32 status_result[4];
typedef void (*IAP)(unsigned int [],unsigned int[]);
#define iap_entry ((void(*)(u32[],u32[]))IAP_LOCATION)
//#define iap_entry ((IAP)IAP_LOCATION);

enum IAP_STATUS { CMD_SUCCESS = 0,  INVALID_COMMAND,  SRC_ADDR_ERROR,  DST_ADDR_ERROR,  SRC_ADDR_NOT_MAPPED,  DST_ADDR_NOT_MAPPED,  COUNT_ERROR,  INVALID_SECTOR,  SECTOR_NOT_BLANK, 
 SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION, COMPARE_ERROR, BUSY, ERR_ISP_IRC_NO_POWER , ERR_ISP_FLASH_NO_POWER,  ERR_ISP_FLASH_NO_CLOCK  };

const unsigned __int64 masterGUID = MGUID;
const unsigned __int64 slaveGUID = SGUID;

static ComPort com;

//static ComPort* actCom = 0;


struct FL
{
	u32 id;
	u32 size;
	u32 pageSize;
	u32 nbPlane;
	u32 planeSize;
};

static FL flDscr;

static bool run;
//static u32 crcErrors = 0;
//static u32 lenErrors = 0;
//static u32 reqErrors = 0;

//static u32 lens[300] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqHS { unsigned __int64 guid; u16 crc; };
__packed struct RspHS { unsigned __int64 guid; u16 crc; };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ReqMes
{
	u32 len;

	struct { u32 func; u32 padr; u32 page[PAGEDWORDS]; u16 align; u16 crc; } wp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspMes
{
	u32 len;

	struct { u32 func; u32 padr; u32 status; u16 align; u16 crc; } wp;
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_PrepareSector(u32 sector)
{
	sector += START_SECTOR;

	command_param[0] = 50;
	command_param[1] = sector;
	command_param[2] = sector;
	command_param[3] = 0;

	__disable_irq();

	iap_entry(command_param, status_result);

	__enable_irq();

	return status_result[0] == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_WritePage(u32 pagenum, u32 *pbuf)
{
	command_param[0] = 51;
	command_param[1] = FLASH_START + pagenum*PAGESIZE;
	command_param[2] = (u32)pbuf;
	command_param[3] = PAGESIZE;
	command_param[4] = MCK/1000;

	__disable_irq();

	iap_entry(command_param, status_result);

	__enable_irq();

	return status_result[0] == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_WriteSector(u32 sector, u32 *pbuf)
{
	command_param[0] = 51;
	command_param[1] = FLASH_START + sector*SECTORSIZE;
	command_param[2] = (u32)pbuf;
	command_param[3] = SECTORSIZE;
	command_param[4] = MCK/1000;

	__disable_irq();

	iap_entry(command_param, status_result);

	__enable_irq();

	return status_result[0] == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_EraseSector(u32 sector)
{
	sector += START_SECTOR;

	command_param[0] = 52;
	command_param[1] = sector;
	command_param[2] = sector;
	command_param[3] = MCK/1000;

	__disable_irq();

	iap_entry(command_param, status_result);

	__enable_irq();

	return status_result[0] == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_ErasePage(u32 pagenum)
{
	pagenum += START_PAGE;

	command_param[0] = 59;
	command_param[1] = pagenum;
	command_param[2] = pagenum;
	command_param[3] = MCK/1000;

	__disable_irq();

	iap_entry(command_param, status_result);

	__enable_irq();

	return status_result[0] == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool VerifyPage(u32 pagenum, u32 *pbuf)
{
	bool c = true;

	u32 *p = (u32*)(FLASH_START + pagenum*PAGESIZE);

	for (u32 i = 0; i < PAGEDWORDS; i++)
	{
		if (p[i] != pbuf[i])
		{
			c = false;
			break;
		};
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool WritePage(u32 pagenum, u32 *pbuf)
{
	u32 sector = pagenum/PAGES_IN_SECTOR;

	if ((pagenum & (PAGES_IN_SECTOR-1)) == 0)
	{
		if (!IAP_PrepareSector(sector))
		{
			return false;
		};
		if (!IAP_EraseSector(sector))
		{
			return false;
		};
	};

	if (!IAP_PrepareSector(sector))
	{
		return false;
	};

	if (!IAP_WritePage(pagenum, pbuf))
	{
		return false;
	};

	if (!VerifyPage(pagenum, pbuf))
	{
		return false;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool VerifySector(u32 sector, u32 *pbuf)
{
	bool c = true;

	u32 *p = (u32*)(FLASH_START + sector*SECTORSIZE);

	for (u32 i = 0; i < SECTORDWORDS; i++)
	{
		if (p[i] != pbuf[i])
		{
			c = false;
			break;
		};
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool WriteSector(u32 sector, u32 *pbuf)
{
	if (!VerifySector(sector, pbuf))
	{
		if (!IAP_PrepareSector(sector))
		{
			return false;
		};
		if (!IAP_EraseSector(sector))
		{
			return false;
		};

		if (!IAP_PrepareSector(sector))
		{
			return false;
		};

		if (!IAP_WriteSector(sector, pbuf))
		{
			return false;
		};

		if (!VerifySector(sector, pbuf))
		{
			return false;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 bufSector[SECTORDWORDS];
static u32 pageMask = 0;
static u32 curSector = ~0;

static bool WritePageBuf(u32 pagenum, u32 *pbuf)
{
	u32 sector = pagenum/PAGES_IN_SECTOR;

	if (curSector != sector)
	{
		pageMask = 0;
		curSector = sector;
		
		u32 *p = bufSector;

		for (u32 i = ArraySize(bufSector); i > 0; i--)
		{
			*p++ = ~0;
		};
	};

	pagenum &= (PAGES_IN_SECTOR-1);

	if ((pageMask & (1<<pagenum)) == 0)
	{
		u32 *p = bufSector + pagenum*PAGEDWORDS;

		for (u32 i = PAGEDWORDS; i > 0; i--)
		{
			*p++ = *pbuf++;
		};

		pageMask |= 1<<pagenum;
	};

	if ((pageMask & (PAGES_IN_SECTOR-1)) == (PAGES_IN_SECTOR-1))
	{
		if (!WriteSector(sector, bufSector))
		{
			return false;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool HandShake()
{
	static ReqHS req;
	static RspHS rsp;

	static ComPort::WriteBuffer wb = { false, sizeof(req), &req };

	static ComPort::ReadBuffer rb = { false, 0, sizeof(rsp), &rsp };

	req.guid = slaveGUID;
	req.crc = GetCRC16(&req, sizeof(req)-2);

	com.Connect(0, 115200, 0);

	bool c = false;

	for (byte i = 0; i < 2; i++)
	{
		com.Read(&rb, MS2CLK(100), US2CLK(500));

		HW::GPIO->BSET(12);

		while(com.Update())
		{
			HW::ResetWDT();
		};

		HW::GPIO->BCLR(12);

		c = (rb.recieved && rb.len == sizeof(RspHS) && GetCRC16(rb.data, rb.len) == 0 && rsp.guid == masterGUID);

		if (c)
		{
			com.Write(&wb);

			while(com.Update()) { HW::ResetWDT() ; };

			break;
		};
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestHandler(ReqMes &req, RspMes &rsp)
{
	bool c = false;

	if ((req.wp.func & 1) && req.len == sizeof(req.wp))
	{
		c = WritePage(req.wp.padr/PAGESIZE, req.wp.page);
	};

	rsp.wp.func = req.wp.func;
	rsp.wp.padr = req.wp.padr;
	rsp.wp.status = (c) ? 1 : 0;
	rsp.wp.crc = GetCRC16(&rsp.wp, sizeof(rsp.wp) - 2);
	rsp.len = sizeof(rsp.wp);

	return (req.wp.func & 0x80000000) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static ReqMes req;
	static RspMes rsp;

	static byte i = 0;

	static bool c = true;

	static TM32 tm;

	switch (i)
	{
		case 0:

			rb.data = &req.wp;
			rb.maxLen = sizeof(req);
			
			com.Read(&rb, -1, MS2CLK(2));

			i++;

			break;

		case 1:

			if (!com.Update())
			{
				i++;
			};

			break;

		case 2:

			//lens[rb.len] += 1;

			//if (!rb.recieved) { reqErrors++; };

			//if (rb.len != sizeof(req.data.wp))
			//{
			//	lenErrors++; 
			//}
			//else if (CheckCRC32(rb.data, rb.len) != 0)
			//{
			//	crcErrors++;
			//};
			

			if (rb.recieved && rb.len > 0 && GetCRC16(rb.data, rb.len) == 0)
			{
				req.len = rb.len;

				c = RequestHandler(req, rsp);

				i++;
			}
			else
			{
				i = 0;
			};

			break;

		case 3:

			while(!tm.Check(2)) ;

			wb.data = &rsp.wp;
			wb.len = rsp.len;

			com.Write(&wb);

			i++;

			break;

		case 4:

			if (!com.Update())
			{
				i = 0;

				run = c;
			};

			break;

	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern "C" void _MainAppStart(u32 adr);

int main()
{
//	__breakpoint(0);

	HW::GPIO->BCLR(12);

	InitHardware();

	run = HandShake();

	while(run)
	{
		UpdateCom();

		HW::ResetWDT();
	};

//	__breakpoint(0);

	__disable_irq();

	_MainAppStart(FLASH0+BOOTSIZE);

	return FLASH0+BOOTSIZE;
}
