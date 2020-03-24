#include "core.h"
//#include "xtrap.h"
//#include "flash.h"
#include "time.h"
#include "ComPort.h"
//#include "twi.h"
#include "hardware.h"
#include "main.h"
#include "crc16.h"
//#include "emac.h"

#pragma diag_suppress 546,550,177

#define US2CLK(x) ((x)*(MCK/1000000))
#define MS2CLK(x) ((x)*(MCK/1000))

#define SGUID	0xCA38D95A98E7447f	
#define MGUID	0x9CDC54C162D06925	
#define FRDY 1
#define FCMDE 2
#define FLOCKE 4
#define PAGESIZE 256
#define PAGEDWORDS (PAGESIZE>>2)
#define PAGES_IN_SECTOR 64
#define SECTORSIZE (PAGESIZE*PAGES_IN_SECTOR)
#define SECTORDWORDS (SECTORSIZE>>2)
#define PLANESIZE 0x200000
#define START_SECTOR 4
#define START_PAGE (START_SECTOR*PAGES_IN_SECTOR)

#define BOOTSIZE 0x10000	//(SECTORSIZE*START_SECTOR)
#define FLASH0_ADR 0x0C000000
#define FLASH_START (FLASH0_ADR+BOOTSIZE)

//#define FLASH1 (FLASH0+PLANESIZE)

#define FLASH_END (FLASH0_ADR+PLANESIZE)

#define IAP_LOCATION 0X1FFF1FF1
static u32 command_param[5];
static u32 status_result[4];
typedef void (*IAP)(unsigned int [],unsigned int[]);
#define iap_entry ((void(*)(u32[],u32[]))IAP_LOCATION)
//#define iap_entry ((IAP)IAP_LOCATION);

const u32 sectorAdr[] = {0x010000, 0x020000, 0x040000, 0x080000, 0x0C0000, 0x100000, 0x140000, 0x180000, 0x1C0000, 0x200000 };

//enum IAP_STATUS { CMD_SUCCESS = 0,  INVALID_COMMAND,  SRC_ADDR_ERROR,  DST_ADDR_ERROR,  SRC_ADDR_NOT_MAPPED,  DST_ADDR_NOT_MAPPED,  COUNT_ERROR,  INVALID_SECTOR,  SECTOR_NOT_BLANK, 
// SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION, COMPARE_ERROR, BUSY, ERR_ISP_IRC_NO_POWER , ERR_ISP_FLASH_NO_POWER,  ERR_ISP_FLASH_NO_CLOCK  };

const unsigned __int64 masterGUID = MGUID;
const unsigned __int64 slaveGUID = SGUID;

volatile u32* const _CMD_553C = (u32*)(FLASH0_ADR + 0x553C);
volatile u32* const _CMD_5554 = (u32*)(FLASH0_ADR + 0x5554);
volatile u32* const _CMD_55F0 = (u32*)(FLASH0_ADR + 0x55F0);
volatile u32* const _CMD_55F4 = (u32*)(FLASH0_ADR + 0x55F4);
volatile u32* const _CMD_AAA8 = (u32*)(FLASH0_ADR + 0xAAA8);

static void CMD_ResetToRead()		{ *_CMD_5554 = 0xF0; }
static void CMD_EnterPageMode()		{ *_CMD_5554 = 0x50; }
static void CMD_ClearStatus()		{ *_CMD_5554 = 0xF5; }

static TM32 timeOut;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 GetSectorAdrLen(u32 sadr, u32 *radr)
{
	sadr += BOOTSIZE;

	if (sadr >= PLANESIZE)
	{
		return 0;
	};

	u32 len = 0;

	for (u32 i = 0; i < (ArraySize(sectorAdr)-1); i++)
	{
		if (sadr >= sectorAdr[i] && sadr < sectorAdr[i+1])
		{
			sadr = sectorAdr[i] - BOOTSIZE;
			len = sectorAdr[i+1] - sectorAdr[i];

			break;
		};
	};

	if (len != 0 && radr != 0)
	{
		*radr = sadr;
	};

	return len;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CMD_LoadPage(const u32* data, u16 len)
{ 
	u16 l = len&1;

	len >>= 1;

	while (len > 0)
	{
		*_CMD_55F0 = *(data++);
		*_CMD_55F4 = *(data++);
		len -= 1;
	};

	if (l)
	{
		*_CMD_55F0 = *(data++);
		*_CMD_55F4 = 0;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CMD_WritePage(u32 pa)
{
	*_CMD_5554 = 0xAA; *_CMD_AAA8 = 0x55; *_CMD_5554 = 0xA0; *((u32*)(FLASH_START+pa)) = 0xAA;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CMD_ErasePhysicalSector(u32 sa)
{
	*_CMD_5554 = 0xAA; *_CMD_AAA8 = 0x55; *_CMD_5554 = 0x80; *_CMD_5554 = 0xAA; *_CMD_AAA8 = 0x55; *((u32*)(FLASH_START+sa)) = 0x40;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CMD_ResumeProtection()	{ *_CMD_5554 = 0x5E; }


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
	
	union
	{
		struct { u32 func; u32 sadr; u32 len;				u16 align; u16 crc; }	F1; // Get CRC
		struct { u32 func; u32 sadr;						u16 align; u16 crc; }	F2; // Erase sector
		struct { u32 func; u32 padr; u32 page[PAGEDWORDS];	u16 align; u16 crc; }	F3; // Programm page
	};

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspMes
{
	u32 len;

	union
	{
		struct { u32 func; u32 sadr; u32 len;	u16 sCRC;	u16 crc; }	F1; // Get CRC
		struct { u32 func; u32 sadr; u32 status; u16 align;	u16 crc; } 	F2; // Erase sector
		struct { u32 func; u32 padr; u32 status; u16 align;	u16 crc; } 	F3; // Programm page
	};
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_WritePage(u32 pa, u32 *pbuf)
{
	CMD_ResetToRead();

	CMD_ClearStatus();

	CMD_EnterPageMode();

	while ((HW::FLASH0->FSR & (FLASH_FSR_PFPAGE_Msk|FLASH_FSR_SQER_Msk)) == 0);// { HW::WDT->Update(); };

	CMD_LoadPage(pbuf, PAGEDWORDS);

	CMD_WritePage(pa);

	while ((HW::FLASH0->FSR & (FLASH_FSR_PROG_Msk|FLASH_FSR_SQER_Msk)) == 0);// { HW::WDT->Update(); };

	while (HW::FLASH0->FSR & FLASH_FSR_PBUSY_Msk);// { HW::WDT->Update(); };

	return (HW::FLASH0->FSR & (FLASH_FSR_VER_Msk|FLASH_FSR_SQER_Msk)) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool IAP_EraseSector(u32 sa)
{
	CMD_ResetToRead();
	
	CMD_ClearStatus();

	CMD_ErasePhysicalSector(sa);

	while ((HW::FLASH0->FSR & (FLASH_FSR_ERASE_Msk|FLASH_FSR_SQER_Msk)) == 0);// { HW::WDT->Update(); };

	while (HW::FLASH0->FSR & FLASH_FSR_PBUSY_Msk);// { HW::WDT->Update(); };

	return (HW::FLASH0->FSR & (FLASH_FSR_VER_Msk|FLASH_FSR_SQER_Msk)) == 0;
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

	com.ConnectAsyn(1, 115200, 0, 1);

	static byte i = 0;

	static TM32 tm;

	bool c = false;

	tm.Reset();

	while (!tm.Check(200) && !c)
	{
		HW::WDT->Update();

		switch (i)
		{
			case 0:

				com.Read(&rb, MS2RT(100), US2RT(500));

				i++;

				break;

			case 1:

				if (!com.Update())
				{
					if (rb.recieved && rb.len == sizeof(RspHS) && GetCRC16(rb.data, rb.len) == 0 && rsp.guid == masterGUID)
					{
						com.Write(&wb);

						i++;
					}
					else
					{
						i = 0;
					};
				};

				break;

			case 2:

				if (!com.Update())
				{
					c = true;

					timeOut.Reset();
				};

				break;
		};

		//UpdateEMAC();

		//if (EmacIsConnected())
		//{
		//	c = true;
		//};
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request_F1_GetCRC(ReqMes &req, RspMes &rsp)
{
	u32 len = 0;
	u32 adr = 0;

	if (req.len == sizeof(req.F1))
	{
		len = GetSectorAdrLen(req.F1.sadr, &adr);

		if (len != 0)
		{
			if (len > req.F1.len) len = req.F1.len;

			rsp.F1.sCRC = GetCRC16((void*)(FLASH_START+adr), len);
		};
	};

	rsp.F1.func = req.F1.func;
	rsp.F1.sadr = adr;
	rsp.F1.len = len;
	rsp.F1.crc = GetCRC16(&rsp.F1, sizeof(rsp.F1) - 2);
	rsp.len = sizeof(rsp.F1);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request_F2_EraseSector(ReqMes &req, RspMes &rsp)
{
	bool c = false;

	if (req.len == sizeof(req.F2))
	{
		c = IAP_EraseSector(req.F2.sadr);
	};

	rsp.F2.func = req.F2.func;
	rsp.F2.sadr = req.F2.sadr;
	rsp.F2.status = c;
	rsp.F2.crc = GetCRC16(&rsp.F2, sizeof(rsp.F2) - 2);
	rsp.len = sizeof(rsp.F2);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool Request_F3_WritePage(ReqMes &req, RspMes &rsp)
{
	bool c = false;

	if (req.len == sizeof(req.F3))
	{
		c = IAP_WritePage(req.F3.padr, req.F3.page);
	};

	rsp.F3.func = req.F3.func;
	rsp.F3.padr = req.F3.padr;
	rsp.F3.status = c;
	rsp.F3.crc = GetCRC16(&rsp.F3, sizeof(rsp.F3) - 2);
	rsp.len = sizeof(rsp.F3);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestHandler(ReqMes &req, RspMes &rsp)
{
	bool c = false;

	switch (req.F1.func)
	{
		case 1: c = Request_F1_GetCRC(req, rsp);		break;
		case 2: c = Request_F2_EraseSector(req, rsp);	break;
		case 3: c = Request_F3_WritePage(req, rsp);		break;
	};

	return c;
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

			rb.data = &req.F1;
			rb.maxLen = sizeof(req);
			
			com.Read(&rb, MS2RT(200), MS2RT(2));

			i++;

			break;

		case 1:

			if (!com.Update())
			{
				i++;
			};

			break;

		case 2:

			if (rb.recieved && rb.len > 0 && GetCRC16(rb.data, rb.len) == 0)
			{
				req.len = rb.len;

				c = RequestHandler(req, rsp);

				timeOut.Reset();

				i++;
			}
			else
			{
				if (timeOut.Check(2000))
				{
					run = false;
				};

				i = 0;
			};

			break;

		case 3:

			if (tm.Check(2))
			{
				wb.data = &rsp.F1;
				wb.len = rsp.len;

				com.Write(&wb);

				i++;
			};

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
	//__breakpoint(0);

	//IAP_EraseSector(0);

	//IAP_WritePage(0, 0);

	InitHardware();

//	InitEMAC();

	run = HandShake();

	HW::SCU_RESET->ResetEnable(PID_WDT); HW::SCU_CLK->CLKCLR = SCU_CLK_CLKCLR_WDTCDI_Msk; HW::SCU_CLK->ClockDisable(PID_WDT);

	while(run)
	{
		HW::P5->BSET(7);

		UpdateCom();

//		UpdateEMAC();

		HW::P5->BCLR(7);

//		HW::WDT->Update();
	};

	//__breakpoint(0);

	__disable_irq();

	CM4::SysTick->CTRL = 0;

	HW::Peripheral_Disable(PID_DMA0);
	HW::Peripheral_Disable(PID_DMA1);

	HW::Peripheral_Disable(PID_USIC0);
	HW::Peripheral_Disable(PID_USIC1);

	_MainAppStart(FLASH_START);

	return FLASH_START;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

