#include "ComPort.h"
#include "CRC16.h"

#include "at25df021.h"

#include <bfrom.h>

static ComPort com;

static byte netAdr = 1;

static u16 flashCRC = 0;
static u32 flashLen = 0;

static u16 lastErasedBlock = ~0;

static U32u adcValue;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u32 GetRTT() { return *pTIMER0_COUNTER; }

inline u16 GetADC() { return adcValue.w[1]; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TM32
{
	u32 pt;

	TM32() : pt(0) {}
	bool Check(u32 v) { if ((GetRTT() - pt) >= v) { pt = GetRTT(); return true; } else { return false; }; }
	void Reset() { pt = GetRTT(); }
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma pack(1)

struct NewRequest
{
	byte len;
	byte adr;
	byte func;
	
	union
	{
		struct  { byte n; u16 vc; word crc; } f1;						// старт оцифровки
		struct  { byte n; byte chnl; word crc; } f2;					// чтение вектора
		struct  { u16 st[3]; u16 sl[3]; u16 sd[3]; word crc; } f3;  	// установка шага и длины оцифровки вектора
		struct  { byte ka[3]; word crc; } f4;							// установка коэффициента усиления
		struct  { word crc; } f5;										// запрос контрольной суммы и длины программы во флэш-памяти
		struct  { u16 stAdr; u16 len; word crc; byte data[258]; } f6;	// запись страницы во флэш
		struct  { word crc; } f7;										// перезагрузить блэкфин
	};
};

#pragma pack()

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Response
{
	byte adr;
	byte func;
	
	union
	{
		struct  { word crc; } f1;  								// старт оцифровки
		struct  { word crc; } f3;  								// установка периода дискретизации вектора и коэффициента усиления
		struct  { u16 maxAmp[4]; u16 power[4]; word crc; } f4;  // старт оцифровки с установкой периода дискретизации вектора и коэффициента усиления
		struct  { u16 flashLen; u16 flashCRC; word crc; } f5;	// запрос контрольной суммы и длины программы во флэш-памяти
		struct  { u16 res; word crc; } f6;						// запись страницы во флэш
	};
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte rspBuf[64];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc05(const NewRequest *req, ComPort::WriteBuffer *wb)
{
//	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

	if (req->adr == 0) return  false;

	rsp.adr = req->adr;
	rsp.func = req->func;
	rsp.f5.flashLen = flashLen;
	rsp.f5.flashCRC = flashCRC;

	rsp.f5.crc = GetCRC16(&rsp, sizeof(rsp.f5));

	wb->data = &rsp;
	wb->len = sizeof(rsp.f5)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc06(const NewRequest *req, ComPort::WriteBuffer *wb)
{
//	const Request *req = (Request*)rb->data;
	Response &rsp = *((Response*)rspBuf);

	ERROR_CODE Result = NO_ERR;

	if (req->adr == 0 || GetCRC16(req->f6.data, req->f6.len+2) != 0) return  false;

	u32 stAdr = FLASH_START_ADR + req->f6.stAdr;

	u16 block = stAdr/4096;

	if (lastErasedBlock != block)
	{
		Result = EraseBlock(block);
		lastErasedBlock = block;
	};

	if (Result == NO_ERR)
	{
		Result = at25df021_Write(req->f6.data, stAdr, req->f6.len, true);
	};

	rsp.f6.res = Result;

	rsp.adr = req->adr;
	rsp.func = req->func;

	rsp.f6.crc = GetCRC16(&rsp, sizeof(rsp.f6));

	wb->data = &rsp;
	wb->len = sizeof(rsp.f6)+2;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc07(const NewRequest *req, ComPort::WriteBuffer *wb)
{
//	bfrom_SpiBoot(FLASH_START_ADR, (BFLAG_PERIPHERAL/*|BFLAG_HOOK | BFLAG_NOAUTO | BFLAG_FASTREAD|BFLAG_TYPE3*/), 0, 0);

//	bfrom_SysControl(SYSCTRL_SOFTRESET, 0, 0);

	*pWDOG_CNT = MS2SCLK(100);
	*pWDOG_CTL = WDEV_RESET|WDEN;

	while(1)
	{

	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFunc(const ComPort::ReadBuffer *rb, ComPort::WriteBuffer *wb)
{
	static NewRequest nulReq;
	static const byte fl[7] = { sizeof(nulReq.f1)+2, sizeof(nulReq.f2)+2, sizeof(nulReq.f3)+2, sizeof(nulReq.f4)+2, sizeof(nulReq.f5)+2, sizeof(nulReq.f6)+2-sizeof(nulReq.f6.data), sizeof(nulReq.f7)+2 };

	if (rb == 0 || rb->len < 2) return false;

	bool result = false;

	u16 rlen = rb->len;

	byte *p = (byte*)rb->data;

	while(rlen > 5)
	{
		byte len = p[0];
		byte func = p[2]-1;

		if (func < 7 && len == fl[func] && len < rlen && GetCRC16(p+1, len) == 0)
		{
			NewRequest *req = (NewRequest*)p;

			if (req->adr != 0 && req->adr != netAdr) return false;

			switch(req->func)
			{
				//case 1: result = RequestFunc01 (req, wb); break;
				//case 2: result = RequestFunc02 (req, wb); break;
				//case 3: result = RequestFunc03 (req, wb); break;
				//case 4: result = RequestFunc04 (req, wb); break;
				case 5: result = RequestFunc05 (req, wb); break;
				case 6: result = RequestFunc06 (req, wb); break;
				case 7: result = RequestFunc07 (req, wb); break;
			};

			break;
		}
		else
		{
			p += 1;
			rlen -= 1;
		};
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[sizeof(NewRequest)+10];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, (u32)-1, US2SCLK(50));
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0)
				{
					if (RequestFunc(&rb, &wb))
					{
						com.Write(&wb);
						i++;
					}
					else
					{
						i = 0;
					};
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
				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CheckFlash()
{
	static BOOT_HEADER bh;

	byte *p = (byte*)&bh;

//	u32 stAdr = 0x8000;
	u32 adr = 0;

//	bool ready = false;

	while (1)
	{
		at25df021_Read(p, FLASH_START_ADR + adr, sizeof(bh));

//		while(!ready) {};

		adr += sizeof(bh);

		if ((bh.blockCode & BFLAG_FILL) == 0)
		{
			adr += bh.byteCount;	
		};

		if (bh.blockCode & BFLAG_FINAL)
		{
			break;
		};
	};

	flashLen = adr + 2;

	flashCRC = at25df021_GetCRC16(FLASH_START_ADR, flashLen);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRTT()
{
	*pTIMER0_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS;
	*pTIMER0_PERIOD = 0xFFFFFFFF;
	*pTIMER_ENABLE = TIMEN0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_PLL()
{
    ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };

	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();
								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x0300;		//  0000 0011 0000 0000
	*pPORTG_MUX = 0x0000;		//  0000 0000 0000 0000

	*pPORTF_FER = 0x180F;		//  0001 1000 0000 1111
	*pPORTG_FER = 0x070F;		//  0000 0111 0000 1111

	*pPORTFIO_DIR = 0x05A0;		//  0000 0101 1010 0000
	*pPORTGIO_DIR = 0x1800;		//  0001 1000 0000 0000

	*pPORTFIO_INEN = 0x0003;	//  0000 0000 0000 0011
	*pPORTGIO_INEN = 0x0003;	//  0000 0000 0000 0000

	*pPORTGIO_SET = 3<<11;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	static byte i = 0;

	static TM32 tm;

	u16 t;

	switch (i)
	{
		case 0:

			if (tm.Check(MS2SCLK(1)))
			{
				*pSPI1_BAUD = 50; // SCLK=16MHz
				*pSPI1_FLG = 0;//FLS5|FLS2;
				*pSPI1_STAT = 0x7F;
				*pSPI1_CTL = SPE|MSTR|CPOL|/*CPHA|*/SIZE|(TIMOD & 1);    // MSTR=1, CPOL=0, CPHA=0, LSBF=0, SIZE=1, EMISO=0, PSSE=0, GM=0, SZ=0, TIMOD=01
				*pPORTGIO_CLEAR = 1<<12;
				
				t = *pSPI1_RDBR;
				
				*pSPI1_TDBR = 0;

				i++;
			}
			else
			{
				i = 2;
			};

			break;

		case 1:

			if ((*pSPI1_STAT&1) != 0)
			{
				adcValue.d += ((i32)(*pSPI1_RDBR) - (i32)adcValue.w[1])*8192;

				*pPORTGIO_SET = 1<<12;

				*pSPI1_CTL = 0;

				i++;
			};

			break;

		case 2:

			netAdr = (GetADC() / 398) + 1;

			i = 0;

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitHardware()
{
	LowLevelInit();

	InitRTT();

	u32 t = GetRTT();

	while((GetRTT() - t) < 10000000)
	{
		UpdateADC();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void main( void )
{
	//for (u32 i = 1000000; i > 0; i--)
	//{
	//	asm("nop;");
	//};

	//bfrom_SpiBoot(0x8000, (BFLAG_PERIPHERAL/*|BFLAG_HOOK | BFLAG_NOAUTO | BFLAG_FASTREAD|BFLAG_TYPE3*/), 0, 0);

	InitHardware();

	com.Connect(6250000, 0);

	CheckFlash();

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<5;

		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateBlackFin()	);
			CALL( UpdateADC()		);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

		*pPORTFIO_TOGGLE = 1<<5;
	};

//	return 0;
}
