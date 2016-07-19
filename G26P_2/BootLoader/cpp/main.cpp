//#include "hardware.h"
//#include "AllocMem.h"
#include "ComPort.h"
#include "CRC32.h"
#include "time.h"

extern "C" void SystemInit (void);

#define SGUID	0x284A7A9899E04c96
#define MGUID	0x704F5B478AC9470d
#define FRDY 1
#define FCMDE 2
#define FLOCKE 4
#define PAGESIZE 0x200
#define PAGEDWORDS (PAGESIZE>>2)
#define PLANESIZE 0x100000
#define BOOTSIZE 0x4000
#define FLASH0 0x400000
//#define FLASH1 (FLASH0+PLANESIZE)
#define FLASHEND (FLASH0+PLANESIZE)

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
static u32 crcErrors = 0;
static u32 lenErrors = 0;
static u32 reqErrors = 0;

static u32 lens[300] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqHS { unsigned __int64 guid; u32 crc; };
__packed struct RspHS { unsigned __int64 guid; u32 crc; };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ReqMes
{
	u32 len;

	union A
	{
		struct { u32 func; u32 padr; u32 page[PAGEDWORDS]; u32 crc; } wp;
	} data;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspMes
{
	u32 len;

	union A
	{
		struct { u32 func; u32 padr; u32 status; u32 crc; } wp;
	} data;
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool FCMD(T_HW::S_EFC *efc, byte cmd, u16 farg)
{
	while ((efc->FSR & FRDY) == 0) ;

	efc->FCR = cmd | (farg << 8) | (0x5A << 24);

	while ((efc->FSR & FRDY) == 0) ;

	return (efc->FSR & 6) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool GetFlashDescriptor(FL *dsc)
{
	using namespace HW;

	if (FCMD(EFC0, 0, 0))
	{
		dsc->id = EFC0->FRR;
		dsc->size = EFC0->FRR;
		dsc->pageSize = EFC0->FRR;
		dsc->nbPlane = EFC0->FRR;
		dsc->planeSize = EFC0->FRR;

		return true;
	}
	else
	{
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool VerifyPage(u32 *p, u32 *pbuf)
{
	bool c = true;

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

static bool WritePage(u32 padr, u32 *pbuf)
{
	T_HW::S_EFC *efc = 0; 
	
//	u32 *wb = 0;

	padr &= ~0xFF;

	u32 *p = (u32*)padr;

	if(padr < FLASH0+BOOTSIZE)
	{
		return false;
	}
	if (padr < FLASHEND)
	{
		efc = HW::EFC0;
	}
	else
	{
		return false;
	};

	u16 page = (padr - FLASH0) / PAGESIZE;

	//if (VerifyPage(p, pbuf))
	//{
	//	return true;
	//};

	if ((page & 0x7) == 0) // Erase 8 pages
	{
		FCMD(efc, 7, 1|(page<<2));
	};

	for (u32 i = 0; i < PAGEDWORDS; i++)
	{
		p[i] = pbuf[i];
	};

	FCMD(efc, 1, page);

	//if (!VerifyPage(p, pbuf))
	//{
	//	for (u32 i = 0; i < PAGEDWORDS; i++)
	//	{
	//		p[i] = pbuf[i];
	//	};

	//	FCMD(efc, 1, page);

	//	return VerifyPage(p, pbuf);
	//}

	return VerifyPage(p, pbuf);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool HandShake()
{
	static ReqHS req;
	static RspHS rsp;

	static ComPort::WriteBuffer wb = { false, sizeof(req), &req };

	static ComPort::ReadBuffer rb = { false, false, 0, 0, sizeof(rsp), &rsp };

	req.guid = slaveGUID;
	req.crc = GetCRC32(&req, sizeof(req)-4);

	com.Connect(0, 115200, 0);

	com.Write(&wb);

	while(com.Update())
	{
		HW::ResetWDT();
	};

	com.Read(&rb, MS2RT(50), US2RT(500));

	while(com.Update())
	{
		HW::ResetWDT();
	};

	bool c = (rb.recieved && rb.len == sizeof(RspHS) && CheckCRC32(rb.data, rb.len) == 0 && rsp.guid == masterGUID);

	if (c)
	{
		com.Write(&wb);

		while(com.Update()) { HW::ResetWDT() ; };
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestHandler(ReqMes &req, RspMes &rsp)
{
	bool c = false;

	if ((req.data.wp.func & 1) == 1 && req.len == sizeof(req.data.wp))
	{
		c = WritePage(req.data.wp.padr, req.data.wp.page);
	};

	rsp.data.wp.func = req.data.wp.func;
	rsp.data.wp.padr = req.data.wp.padr;
	rsp.data.wp.status = (c) ? 1 : 0;
	rsp.data.wp.crc = GetCRC32(&rsp.data.wp, sizeof(rsp.data.wp) - 4);
	rsp.len = sizeof(rsp.data.wp);

	return (req.data.wp.func & 0x80000000) == 0;
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

	static RTM32 tm;

	switch (i)
	{
		case 0:

			rb.data = &req.data;
			rb.maxLen = sizeof(req);
			
			com.Read(&rb, -1, MS2RT(2));

			i++;

			break;

		case 1:

			if (!com.Update())
			{
				i++;
			};

			break;

		case 2:

			lens[rb.len] += 1;

			if (!rb.recieved) { reqErrors++; };

			if (rb.len != sizeof(req.data.wp))
			{
				lenErrors++; 
			}
			else if (CheckCRC32(rb.data, rb.len) != 0)
			{
				crcErrors++;
			};
			

			if (rb.recieved && rb.len > 0 && CheckCRC32(rb.data, rb.len) == 0)
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

			while(!tm.Check(MS2RT(2))) ;

			wb.data = &rsp.data;
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
/*
static void UpdateCom2()
{
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;

	static ReqMes req;
	static RspMes rsp;

	static byte i = 0;

	static bool c = true;
	static u32 pt = 0;

	switch (i)
	{
		case 0:

			wb.data = &req.data;
			wb.len = 256;

			actCom->Write(&wb);

			i++;

			break;

		case 1:

			if (!actCom->Update())
			{
				i++;
			};

			break;

		case 2:

			if ((HW::RTT->VR - pt) > MS2RT(100))
			{
				i = 0;
			};

			break;
	};

}
*/
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
//	__breakpoint(0);

	SystemInit();

	RTT_Init();

	//	InitHardware();

//	GetFlashDescriptor(&flDscr);

	run = HandShake();

	while(run)
	{
//		UpdateHardware(); 

		UpdateCom();
	};

//	__breakpoint(0);

	__disable_irq();


	//u32 t = FLASH0+BOOTSIZE;

	//__asm
	//{
	//	LDR		R14, [t]
	//	LDR		t, [t, #4]
	//	BX		t
	//};

	return FLASH0+BOOTSIZE;
}
