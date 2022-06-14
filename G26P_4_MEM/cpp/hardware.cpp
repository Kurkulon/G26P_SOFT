#include "core.h"
#include "time.h"
#include "hardware.h"
#include "hw_conf.h"
#include "hw_rtm.h"
#include "SEGGER_RTT.h"
#include "CRC16.h"
#include "DMA.h"

#ifndef WIN32

//#include "system_XMC4800.h"

#else

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>

#endif 

//#pragma O3
//#pragma Otime


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define D_FIRE		(1<<16)
//#define SYNC		(1<<20)
//#define SPI			HW::SPI5
//#define L1			(1<<0)
//#define H1			(1<<1)
//#define L2			(1<<9)
//#define H2			(1<<10)
//#define RXD			(1<<5) 
//#define ERR			(1<<4) 
//#define ENVCORE		(1<<6) 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void EnableVCORE() { PIO_ENVCORE->CLR(ENVCORE); }
inline void DisableVCORE() { PIO_ENVCORE->SET(ENVCORE); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_XMC48

u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init)
{
	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	__packed const byte *s = (__packed const byte*)data;

	for ( ; len > 0; len--)
	{
		CRC_FCE->IR = *(s++);
	};

	//if (len > 0)
	//{
	//	CRC_FCE->IR = *(s++)&0xFF;
	//}

	__dsb(15);

	return CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	HW::P6->BSET(5);

	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	CRC_DMA.MemCopySrcInc(data, &CRC_FCE->IR, len);

	while (!CRC_DMA.CheckMemCopyComplete());

	HW::P6->BCLR(5);

	__dsb(15);

	return (byte)CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
	HW::P6->BSET(5);

	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	CRC_DMA.MemCopySrcInc(data, &CRC_FCE->IR, len);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	if (CRC_DMA.CheckMemCopyComplete())
	{
		__dsb(15);

		*crc = (byte)CRC_FCE->RES;

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Init_CRC_CCITT_DMA ... ");

	HW::Peripheral_Enable(PID_FCE);

	HW::FCE->CLC = 0;
	CRC_FCE->CFG = 0;

	SEGGER_RTT_WriteString(0, "OK\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#elif defined(CPU_SAME53)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	CRC_DMA.CRC_CCITT(data, len, init);

	while (!CRC_DMA.CheckComplete());

	__dsb(15);

	return CRC_DMA.Get_CRC_CCITT_Result();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
	if (busy_CRC_CCITT_DMA) return false;

	busy_CRC_CCITT_DMA = true;

	CRC_DMA.CRC_CCITT(data, len, init);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	if (CRC_DMA.CheckComplete())
	{
		__dsb(15);

		*crc = CRC_DMA.Get_CRC_CCITT_Result();

		busy_CRC_CCITT_DMA = false;

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	//T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	//HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x3F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#elif defined(WIN32)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	return 0;//GetCRC16_CCIT(data, len, init);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
	crc_ccit_result = 0;//GetCRC16_CCIT(data, len, init);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	if (crc != 0)
	{
		*crc = crc_ccit_result;

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	//T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	//HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x3F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void Init_ERU()
{
	using namespace HW;

	HW::CCU_Enable(PID_ERU1);

	// Event Request Select (ERS)
	
	ERU1->EXISEL = 0;
	
	// Event Trigger Logic (ETL)

	ERU1->EXICON[0] = 0;
	ERU1->EXICON[1] = 0;
	ERU1->EXICON[2] = 0;
	ERU1->EXICON[3] = 0;

	// Cross Connect Matrix

	// Output Gating Unit (OGU)

	ERU1->EXOCON[0] = 0;
	ERU1->EXOCON[1] = 0;
	ERU1->EXOCON[2] = 0;
	ERU1->EXOCON[3] = 0;
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetClock(const RTC &t)
{
#ifndef WIN32

	static DSCI2C dsc;

	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];

	buf[0] = 0;
	buf[1] = ((t.sec/10) << 4)|(t.sec%10);
	buf[2] = ((t.min/10) << 4)|(t.min%10);
	buf[3] = ((t.hour/10) << 4)|(t.hour%10);
	buf[4] = 1;
	buf[5] = ((t.day/10) << 4)|(t.day%10);
	buf[6] = ((t.mon/10) << 4)|(t.mon%10);

	byte y = t.year % 100;

	buf[7] = ((y/10) << 4)|(y%10);

	dsc.adr = 0x68;
	dsc.wdata = buf;
	dsc.wlen = 8;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	if (SetTime(t))
	{
		I2C_AddRequest(&dsc);
	};
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static void InitClock()
{
	DSCI2C dsc;

	byte reg = 0;
	byte buf[10];
	
	RTC t;

	buf[0] = 0x0F;
	buf[1] = 0x88;
	dsc.adr = 0x68;
	dsc.wdata = buf;
	dsc.wlen = 2;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	dsc.adr = 0x68;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	I2C_AddRequest(&dsc);

	while (!dsc.ready) I2C_Update();

	t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
	t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
	t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
	t.day	= (buf[4]&0xF) + ((buf[4]>>4)*10);
	t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
	t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

	SetTime(t);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WDT_Init()
{
	HW::WDT_Enable();

	HW::WDT->WLB = OFI_FREQUENCY/2;
	HW::WDT->WUB = (3 * OFI_FREQUENCY)/2;
	HW::SCU_CLK->WDTCLKCR = 0|SCU_CLK_WDTCLKCR_WDTSEL_OFI;

	#ifndef _DEBUG
	HW::WDT->CTR = WDT_CTR_ENB_Msk|WDT_CTR_DSP_Msk;
	#else
	HW::WDT->CTR = WDT_CTR_ENB_Msk;
	#endif

	//HW::ResetWDT();
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

//extern dword DI;
//extern dword DO;
char pressedKey;
//extern  dword maskLED[16];

//byte _array1024[0x60000]; 

HWND  hWnd;

HCURSOR arrowCursor;
HCURSOR handCursor;

HBRUSH	redBrush;
HBRUSH	yelBrush;
HBRUSH	grnBrush;
HBRUSH	gryBrush;

RECT rectLed[10] = { {20, 41, 33, 54}, {20, 66, 33, 79}, {20, 91, 33, 104}, {21, 117, 22, 118}, {20, 141, 33, 154}, {218, 145, 219, 146}, {217, 116, 230, 129}, {217, 91, 230, 104}, {217, 66, 230, 79}, {217, 41, 230, 54}  }; 
HBRUSH* brushLed[10] = { &yelBrush, &yelBrush, &yelBrush, &gryBrush, &grnBrush, &gryBrush, &redBrush, &redBrush, &redBrush, &redBrush };

int x,y,Depth;

HFONT font1;
HFONT font2;

HDC memdc;
HBITMAP membm;

//HANDLE facebitmap;

static const u32 secBufferWidth = 80;
static const u32 secBufferHeight = 12;
static const u32 fontWidth = 12;
static const u32 fontHeight = 16;


static char secBuffer[secBufferWidth*secBufferHeight*2];

static u32 pallete[16] = {	0x000000,	0x800000,	0x008000,	0x808000,	0x000080,	0x800080,	0x008080,	0xC0C0C0,
							0x808080,	0xFF0000,	0x00FF00,	0xFFFF00,	0x0000FF,	0xFF00FF,	0x00FFFF,	0xFFFFFF };

const char lpAPPNAME[] = "8юд73_лдл_соп";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



static __int64		tickCounter = 0;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

static void InitDisplay()
{
	WNDCLASS		    wcl;

	wcl.hInstance		= NULL;
	wcl.lpszClassName	= lpAPPNAME;
	wcl.lpfnWndProc		= WindowProc;
	wcl.style	    	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	wcl.hIcon	    	= NULL;
	wcl.hCursor	    	= NULL;
	wcl.lpszMenuName	= NULL;

	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hbrBackground	= NULL;

	RegisterClass (&wcl);

	int sx = screenWidth = GetSystemMetrics (SM_CXSCREEN);
	int sy = screenHeight = GetSystemMetrics (SM_CYSCREEN);

	hWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME,	WS_DLGFRAME|WS_POPUP, 0, 0,	640, 480, NULL,	NULL, NULL, NULL);

	if(!hWnd) 
	{
		cputs("Error creating window\r\n");
		exit(0);
	};

	RECT rect;

	if (GetClientRect(hWnd, &rect))
	{
		MoveWindow(hWnd, 0, 0, 641 + 768 - rect.right - 2, 481 - rect.bottom - 1 + 287, true);
	};

	ShowWindow (hWnd, SW_SHOWNORMAL);

	font1 = CreateFont(30, 14, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");
	font2 = CreateFont(fontHeight, fontWidth-2, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");

	if (font1 == 0 || font2 == 0)
	{
		cputs("Error creating font\r\n");
		exit(0);
	};

	GetClientRect(hWnd, &rect);
	HDC hdc = GetDC(hWnd);
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, rect.right - rect.left + 1, rect.bottom - rect.top + 1 + secBufferHeight*fontHeight);
    SelectObject(memdc, membm);
	ReleaseDC(hWnd, hdc);


	arrowCursor = LoadCursor(0, IDC_ARROW);
	handCursor = LoadCursor(0, IDC_HAND);

	if (arrowCursor == 0 || handCursor == 0)
	{
		cputs("Error loading cursors\r\n");
		exit(0);
	};

	LOGBRUSH lb;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = RGB(0xFF, 0, 0);

	redBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0xFF, 0xFF, 0);

	yelBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0x7F, 0x7F, 0x7F);

	gryBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0, 0xFF, 0);

	grnBrush = CreateBrushIndirect(&lb);

	for (u32 i = 0; i < sizeof(secBuffer); i+=2)
	{
		secBuffer[i] = 0x20;
		secBuffer[i+1] = 0xF0;
	};
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

void UpdateDisplay()
{
	static byte curChar = 0;
	static byte c = 0, i = 0;
	//static byte flashMask = 0;
	static const byte a[4] = { 0x80, 0x80+0x40, 0x80+20, 0x80+0x40+20 };

	MSG msg;

	static dword pt = 0;

	if(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		GetMessage (&msg, NULL, 0, 0);

		TranslateMessage (&msg);

		DispatchMessage (&msg);
	};

	static char buf[80];
	static char buf1[sizeof(secBuffer)];

	u32 t = GetTickCount();

	if ((t-pt) > 10)
	{
		pt = t;

		bool rd = true;

		//flashMask += 1;

		//for (u32 i = 0; i < sizeof(buf1); i++)
		//{
		//	char t = secBuffer[i];

		//	if (buf1[i] != t) 
		//	{ 
		//		buf1[i] = t; rd = true; 
		//	};
		//};

		if (rd)
		{
			//SelectObject(memdc, font1);
			//SetBkColor(memdc, 0x074C00);
			//SetTextColor(memdc, 0x00FF00);
			//TextOut(memdc, 443, 53, buf, 20);
			//TextOut(memdc, 443, 30*1+53, buf+20, 20);
			//TextOut(memdc, 443, 30*2+53, buf+40, 20);
			//TextOut(memdc, 443, 30*3+53, buf+60, 20);

			SelectObject(memdc, font2);

			for (u32 j = 0; j < secBufferHeight; j++)
			{
				for (u32 i = 0; i < secBufferWidth; i++)
				{
					u32 n = (j*secBufferWidth+i)*2;

					u8 t = secBuffer[n+1];

					SetBkColor(memdc, pallete[(t>>4)&0xF]);
					SetTextColor(memdc, pallete[t&0xF]);
					TextOut(memdc, i*fontWidth, j*fontHeight+287, secBuffer+n, 1);
				};
			};

			RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE);

		}; // if (rd)

		//if (!rd) { RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE); };


	}; // if ((t-pt) > 10)

}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	bool c;
	static int x, y;
	int x0, y0, r0;
	static char key = 0;
	static char pkey = 0;

	RECT rect;

	static bool move = false;
	static int movex, movey = 0;

//	char *buf = (char*)screenBuffer;

    switch (message)
	{
        case WM_CREATE:

            break;

	    case WM_DESTROY:

		    break;

        case WM_PAINT:

			if (GetUpdateRect(hWnd, &rect, false)) 
			{
				hdc = BeginPaint(hWnd, &ps);

				//printf("Update RECT: %li, %li, %li, %li\r\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

				c = BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left + 1, ps.rcPaint.bottom - ps.rcPaint.top + 1, memdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	 
				EndPaint(hWnd, &ps);
			};

            break;

        case WM_CHAR:

			pressedKey = wParam;

			if (pressedKey == '`')
			{
				GetWindowRect(hWnd, &rect);

				if ((rect.bottom-rect.top) > 340)
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-fontHeight*secBufferHeight, true); 
				}
				else
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top+fontHeight*secBufferHeight, true); 
				};
			};

            break;

		case WM_MOUSEMOVE:

			x = LOWORD(lParam); y = HIWORD(lParam);

			if (move)
			{
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, HWND_TOP, rect.left+x-movex, rect.top+y-movey, 0, 0, SWP_NOSIZE); 
			};

			return 0;

			break;

		case WM_MOVING:

			return TRUE;

			break;

		case WM_SYSCOMMAND:

			return 0;

			break;

		case WM_LBUTTONDOWN:

			move = true;
			movex = x; movey = y;

			SetCapture(hWnd);

			return 0;

			break;

		case WM_LBUTTONUP:

			move = false;

			ReleaseCapture();

			return 0;

			break;

		case WM_MBUTTONDOWN:

			ShowWindow(hWnd, SW_MINIMIZE);

			return 0;

			break;

		case WM_ACTIVATE:

			if (HIWORD(wParam) != 0 && LOWORD(wParam) != 0)
			{
				ShowWindow(hWnd, SW_NORMAL);
			};

			break;

		case WM_CLOSE:

			//run = false;

			break;
	};
    
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif		
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

int PutString(u32 x, u32 y, byte c, const char *str)
{
	char *dst = secBuffer+(y*secBufferWidth+x)*2;
	dword i = secBufferWidth-x;

	while (*str != 0 && i > 0)
	{
		*(dst++) = *(str++);
		*(dst++) = c;
		i -= 1;
	};

	return secBufferWidth-x-i;
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef	WIN32

int Printf(u32 x, u32 y, byte c, const char *format, ... )
{
	char buf[1024];

	va_list arglist;

    va_start(arglist, format);
    vsprintf(buf, format, arglist);
    va_end(arglist);

	return PutString(x, y, c, buf);
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	Init_time(MCK);
	RTT_Init();
	I2C_Init();
	//SPI_Init();

#ifndef WIN32

	InitClock();

	InitManTransmit();
	InitManRecieve();

	Init_CRC_CCITT_DMA();
	
	EnableVCORE();

	WDT_Init();

#else

	InitDisplay();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
#ifndef WIN32

	static byte i = 0;

	static Deb db(false, 20);

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( I2C_Update();		);
		CALL( SPI_Update();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

