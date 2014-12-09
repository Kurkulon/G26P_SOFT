#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "hardware.h"
#include "time.h"
#include "win2lcd.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void LowLevelInit();
void InitDisplay();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <fonts.h>

byte valveStatus32 = 0;

HWND  hWnd;

struct BMPINF
{
	BITMAPINFO bmi;
	RGBQUAD bmiColors[256];
} bminfo;

const word winWidth = 1500;
const word winHeight = 400;

byte screenBuffer[winWidth*winHeight];
u32 bitsToBytes[256][2];

int x,y,Depth;

HDC memdc;
HBITMAP membm;

static char pressedKey = 0;

const char lpAPPNAME[] = "Test_G26P2";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static byte pallete[48] = { 0x00, 0x00, 0x00,	0x2A, 0x00, 0x00,	0x00, 0x2A, 0x00,	0x2A, 0x2A, 0x00,
							0x00, 0x00, 0x2A,	0x2A, 0x00, 0x2A,	0x00, 0x2A, 0x2A,	0x2A, 0x2A, 0x2A,
							0x15, 0x15, 0x15,	0x3F, 0x00, 0x00,	0x00, 0x3F, 0x00,	0x3F, 0x3F, 0x00,
							0x00, 0x00, 0x3F,	0x3F, 0x00, 0x3F,	0x00, 0x3F, 0x3F,	0x3F, 0x3F, 0x3F };


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitDisplay()
{
	bminfo.bmi.bmiColors[0].rgbBlue = pallete[0] << 2;
	bminfo.bmi.bmiColors[0].rgbGreen = pallete[1] << 2;
	bminfo.bmi.bmiColors[0].rgbRed = pallete[2] << 2;

	for (dword i = 1; i < 16; i++)
	{
		bminfo.bmiColors[i-1].rgbBlue = pallete[i*3] << 2;
		bminfo.bmiColors[i-1].rgbGreen = pallete[i*3+1] << 2;
		bminfo.bmiColors[i-1].rgbRed = pallete[i*3+2] << 2;
	};

	for (dword i = 0; i < 256; i++)
	{
		dword t = i;

		for (dword j = 0; j < 2; j++)
		{
			bitsToBytes[i][j] = ((t & 0x80) ? 0xFF: 0)|((t & 0x40) ? 0xFF00 : 0)|((t & 0x20) ? 0xFF0000 : 0)|((t & 0x10) ? 0xFF000000 : 0);
			t <<= 4;
		};
	};

	bminfo.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
	bminfo.bmi.bmiHeader.biWidth = winWidth; 
	bminfo.bmi.bmiHeader.biHeight = -winHeight; 
	bminfo.bmi.bmiHeader.biPlanes = 1; 
	bminfo.bmi.bmiHeader.biBitCount = 8; 
	bminfo.bmi.bmiHeader.biCompression = BI_RGB; 
	bminfo.bmi.bmiHeader.biSizeImage = 0; 
	bminfo.bmi.bmiHeader.biXPelsPerMeter = 0; 
	bminfo.bmi.bmiHeader.biYPelsPerMeter = 0; 
	bminfo.bmi.bmiHeader.biClrUsed = 16; 
	bminfo.bmi.bmiHeader.biClrImportant = 0; 

	for (dword i = 0; i < winWidth*winHeight; i++)
	{
		screenBuffer[i] = 7;//(byte)(i&15);
	};


	WNDCLASS		    wcl;

	wcl.hInstance		= NULL;
	wcl.lpszClassName	= lpAPPNAME;
	wcl.lpfnWndProc		= WindowProc;
	wcl.style	    	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	wcl.hIcon	    	= NULL;
	wcl.hCursor	    	= LoadCursor(0, IDC_ARROW);
	wcl.lpszMenuName	= NULL;

	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hbrBackground	= NULL;

	RegisterClass (&wcl);

	int sx = screenWidth = GetSystemMetrics (SM_CXSCREEN);
	int sy = screenHeight = GetSystemMetrics (SM_CYSCREEN);

//	hWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME, WS_CAPTION|WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX, 0, 0, winWidth, winHeight, 0, 0, 0, 0);

	hWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME, WS_DLGFRAME|WS_POPUP, 0, 0, winWidth, winHeight, NULL, NULL, NULL, NULL);

	if(!hWnd) 
	{
		cputs("Error creating window\r\n");
		exit(0);
	};

	RECT rect;

	if (GetClientRect(hWnd, &rect))
	{
		MoveWindow(hWnd, 0, 0, winWidth + winWidth - (rect.right - rect.left), winHeight + winHeight - (rect.bottom - rect.top), true);
	};

	ShowWindow (hWnd, SW_SHOWNORMAL);

	GetClientRect(hWnd, &rect);
	HDC hdc = GetDC(hWnd);
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
    SelectObject(memdc, membm);
	ReleaseDC(hWnd, hdc);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern bool run;
//	WINDOWINFO wi;
//	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;

	static bool move = false;
	static int movex, movey = 0;

	POINTS &p = *((POINTS*)&lParam);

    switch (message)
	{
     //   case WM_CREATE:

     //       break;

	    //case WM_DESTROY:

		   // break;

        case WM_PAINT:

			//QueryPerformanceCounter(&prev);

			if (GetUpdateRect(hWnd, &rect, false)) 
			{
				hdc = BeginPaint(hWnd, &ps);

				BitBlt(hdc, rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top + 1, memdc, rect.left, rect.top, SRCCOPY);

				EndPaint(hWnd, &ps);
			};

            break;

        case WM_CHAR:

			_ungetch(wParam);
			run = (lParam != 0x00010001);

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

			run = false;

			break;
	};
    
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void PutString16(word x, word y, const char* str, word color)
{
	register byte c;

	if (y >= winHeight || x >= winWidth) { return; }; 

	i16 l = winWidth - x;

	word ch = winHeight - y;
	
	if (ch >= 16) { ch = 16; };

	byte *p = screenBuffer + y*winWidth+x;

	u32 fg = (byte)color;		fg |= fg << 8; fg |= fg << 16;
	u32 bg = (byte)(color>>8);	bg |= bg << 8; bg |= bg << 16;

	while ((c = *(str++)) != 0 && l > 0)
	{
		byte *s = fontWin1251_8x16 + c*16;

		u32 *d = (u32*)p; p += 8;

		for (word i = 0; i < ch; i++)
		{
			u32 t = bitsToBytes[*s][0];	*(d++) = (t & fg)|(~t & bg);
				t = bitsToBytes[*s][1];	*(d++) = (t & fg)|(~t & bg);

			d += winWidth/4 - 2;
			s++;
		};

		l -= 8;
	};				   
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Printf16(word x, word y, word c, const char *format, ... )
{
	char buf[512];

	va_list arglist;

    va_start(arglist, format);
    vsprintf(buf, format, arglist);
    va_end(arglist);

	return PutString16(x, y, buf, c);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DrawWave(byte n, void *data)
{
	const u32 wh = 100;

	static HBRUSH fgbr = CreateSolidBrush(RGB(0, 128, 0));
	static HBRUSH bgbr = CreateSolidBrush(RGB(202, 198, 190));
	static HBRUSH albr = CreateSolidBrush(RGB(192, 0, 0));
	static HBRUSH debr = CreateSolidBrush(RGB(192, 188, 180));

	RECT rect;

	static float w[500];

	u16 *pw = (u16*)data;

	for (u32 i = 0; i < ArraySize(w); i++)
	{
		w[i] = (pw[i]*100.0/65535 - 50)*3;
	};

	rect.top = wh*n; rect.left = 0; rect.bottom = wh*(n+1); rect.right = winWidth;
	FillRect(memdc, &rect, (n&1) ? debr : bgbr);

	MoveToEx(memdc, 0, wh*n+wh/2-w[0], 0);

	for (u32 i = 1; i < ArraySize(w); i++)
	{
		LineTo(memdc, i*3, wh*n+wh/2-w[i]);
	};

	//rect.left = 0; rect.top = 0; rect.right = width; rect.bottom = height;

	RedrawWindow(hWnd, &rect, 0, RDW_INVALIDATE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateDisplay()
{
	MSG msg;

	RECT rect;

	static u32 pt = 0, pt2 = 1, ptf = 0, pts = 0;

	static word x = 0;
	static byte prevValveStatus = 0;

	static u32 frames = 0;

	const word osch = 160;

	if(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		GetMessage (&msg, NULL, 0, 0);

		TranslateMessage (&msg);

		DispatchMessage (&msg);
	};

	u32 t = GetTickCount();


	if ((t-pt) >= 10)
	{
		pt = t;

		//SetDIBitsToDevice(memdc, 0, 0, winWidth, winHeight, 0, 0, 0, winHeight, screenBuffer, &bminfo.bmi, DIB_RGB_COLORS);

		//rect.left = 0; rect.right = winWidth; rect.top = osch; rect.bottom = winHeight;

		//RedrawWindow(hWnd, &rect, 0, RDW_INVALIDATE);
	}
	else if ((t-pt2) >= 25)
	{
		pt2 = t;


	}
	else if ((t-ptf) >=1000)
	{
		ptf = t;
		Printf16(0, osch, 0x0700, "%10lu", frames);
		frames = 0;
	};

	frames++;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitDisplay();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	UpdateDisplay();

	static u32 sleepFrames = 0;

	sleepFrames++;

	if (sleepFrames >= 100) { Sleep(2); sleepFrames = 0; };
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
