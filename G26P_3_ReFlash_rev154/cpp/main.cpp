#include "ComPort.h"
#include "CRC16.h"

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <stdio.h>
#include <commctrl.h>

#include "types.h"

#define SGUID	0xBA332660A2364233	//0x93F15E77DCC1465c
#define MGUID	0x81396E6BDAF74c83	//0x3C60B1B237804398

const unsigned __int64 masterGUID = MGUID;
const unsigned __int64 slaveGUID = SGUID;
//const u32	initCRC32 = 0x94979e45;
//const u32	xoutCRC32 = 0x33605738;

//#define BOOTSIZE 0x4000
//#define FLASH0 (0x00400000+BOOTSIZE)
#define PAGESIZE 64

static u32 flashPages[] = {
#include "G26P3LPC812.bin.h"
};



HWND	hWnd;
HWND	btAccept;
HWND	cbCom;
HWND	prBar;

static u32 crcErr = 0;

static bool run = true;
static bool start = false;


const word winWidth = 768;
const word winHeight = 400;

const COLORREF bgColor = RGB(212, 208, 200);
const COLORREF txtColor = RGB(0, 0, 0);

HFONT font1;
HFONT font2;


int x,y,Depth;

HDC memdc;
HBITMAP membm;

static char pressedKey = 0;

const char lpAPPNAME[] = "G26P_3_LPC812 Flash Loader";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static byte pallete[256*3] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct ReqHS { unsigned __int64 guid; u16 crc; };
struct RspHS { unsigned __int64 guid; u16 crc; };

struct Request
{
	union
	{
		u32 func;
		struct { u32 func; u32 padr; u32 page[PAGESIZE>>2]; u16 pad; u16 crc; } f1;
		struct { u32 func; u32 data; u16 pad; u16 crc; } fx;
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Response
{
	union
	{
		u32 func;
		struct { u32 func; u32 padr; u32 status; u16 pad; u16 crc; } f1;
		struct { u32 func; u32 padr; u32 status; u16 pad; u16 crc; } fx;
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ComPort com1;
static ComPort::ReadBuffer rb = {};
static ComPort::WriteBuffer wb;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ReqHS reqHS;
static RspHS rspHS;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CreateRequest_01(Request* req);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Printf(int x, int y, const char *format, ... )
{
	char buf[512];

	va_list arglist;

    va_start(arglist, format);
    int l = vsnprintf_s(buf, sizeof(buf), sizeof(buf)-1, format, arglist);
    va_end(arglist);

	if (l > 0)
	{
		SetBkColor(memdc, bgColor);
		SetTextColor(memdc, txtColor);
		TextOut(memdc, x, y, buf, l);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CreateComboCom()
{
	cbCom =	CreateWindowEx(0, "COMBOBOX", "1", CBS_DROPDOWNLIST|WS_CHILDWINDOW|WS_VSCROLL, 10, 10, 200, 100, hWnd, 0, 0, 0); 
//	SendMessage(cbCom, CB_INITSTORAGE, 0, 0);

	com1.BuildPortTable();

	u32 size = com1.GetPortTableSize();
	const u32* p = com1.GetPortTable();

	if (size == 0)
	{
		return false;
	};

    char buf[256];

	for (u32 i = 0; i < size; i++)
	{
        wsprintf(buf, "\\\\.\\COM%lu", p[i]);

		if (SendMessage(cbCom, CB_ADDSTRING, 0, (LPARAM)buf) == CB_ERR)
		{
			return false;
		};
	};

	SendMessage(cbCom, CB_SETCURSEL, 0, 0);
	ShowWindow (cbCom, SW_SHOWNORMAL);


	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitDisplay()
{
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

	hWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME, WS_CAPTION|WS_POPUP|WS_SYSMENU, 0, 0, winWidth, winHeight, 0, 0, 0, 0);

	if(!hWnd) 
	{
		exit(0);
	};

	font1 = CreateFont(14, 8, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");
	font2 = CreateFont(16, 8, 0, 0, FW_DONTCARE, false, false, false, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH|FF_DONTCARE, "System");

	RECT rect;

	if (GetClientRect(hWnd, &rect))
	{
		MoveWindow(hWnd, 0, 0, winWidth + winWidth - (rect.right - rect.left), winHeight + winHeight - (rect.bottom - rect.top), true);
	};

	ShowWindow (hWnd, SW_SHOWNORMAL);

	btAccept = CreateWindowEx(0, "BUTTON", "Старт", BS_PUSHBUTTON|WS_CHILDWINDOW, 662, 365, 96, 23, hWnd, 0, 0, 0);

	CreateComboCom();

	ShowWindow (btAccept, SW_SHOWNORMAL);

	InitCommonControls();

	prBar = CreateWindowEx(0, PROGRESS_CLASS, "pb", PBS_SMOOTH|WS_CHILDWINDOW|WS_VISIBLE, 10, 50, 300, 16, hWnd, 0, 0, 0); 

	SendMessage(prBar, PBM_SETRANGE, 0, MAKELPARAM (0, 2048));


	GetClientRect(hWnd, &rect);
	HDC hdc = GetDC(hWnd);
	SelectObject(hdc, CreateSolidBrush(RGB(0,0,0)));
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, winWidth, winHeight);
    SelectObject(memdc, membm);
	ReleaseDC(hWnd, hdc);

	SelectObject(memdc, font1);

	rect.left = 0; rect.top = 0; rect.right = winWidth; rect.bottom = winHeight;

	FillRect(memdc, &rect, CreateSolidBrush(bgColor));


	//for (dword i = 0; i < 1900; i++) { screenBuffer[i] = 0xFF; };

	//screenBuffer[0] = 0;
	//screenBuffer[32] = 0;


	//SetDIBitsToDevice(memdc, 0, 0, width, height, 0, 0, 0, height, screenBuffer, &bminfo.bmi, DIB_RGB_COLORS);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

LRESULT CALLBACK WindowProc(HWND h, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;

    switch (message)
	{
        case WM_PAINT:

			if (h == hWnd && GetUpdateRect(h, &rect, false))
			{
				hdc = BeginPaint(h, &ps);

				BitBlt(hdc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, memdc, rect.left, rect.top, SRCCOPY);

				EndPaint(h, &ps);

				return 0;
			};

            break;

		case WM_CLOSE:

			run = false;

            break;

		case WM_COMMAND:

			if (HIWORD(wParam) == BN_CLICKED)
			{
				if ((HWND)lParam == btAccept)
				{
					start = true;
					EnableWindow(btAccept, FALSE);
					EnableWindow(cbCom, FALSE);
				}
				else
				{
//					Get_Cond();
				};

				return 0;
			};

			break;
	};
    
	return DefWindowProc(h, message, wParam, lParam);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateDisplay()
{
	MSG msg;

	if(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		GetMessage (&msg, NULL, 0, 0);

		TranslateMessage (&msg);

		DispatchMessage (&msg);
	}
	else
	{
		Sleep(1);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateRequest()
{
	static byte i = 0;

	static Request req;
	static Response rsp;
	static u32 *p = flashPages;
	static u32 pa = 0;
	static u32 count = ArraySize(flashPages);
	static u32 packetnum = 0;
	static u32 oknum = 0;
	static u32 errornum = 0;
	static u32 lenErrors = 0;
	static u32 crcErrors = 0;
	static u32 pageErrors = 0;


	char buf[1024];

	switch(i)
	{
		case 0:

			req.func = 1;
			req.f1.padr = pa;

			
			for (i32 j = 0, t = count; j < ArraySize(req.f1.page); j++, t--)
			{
				req.f1.page[j] = (t > 0) ? p[j] : ~0;
			};

			req.f1.crc = GetCRC16(&req, sizeof(req) - sizeof(req.f1.crc));

			wb.data = &req;
			wb.len = sizeof(req.f1);
			Sleep(5);
			com1.Write(&wb);
			i++;

			break;

		case 1:

			if (!com1.Update())
			{
				rb.data = &rsp;
				rb.maxLen = sizeof(rsp.f1);
				com1.Read(&rb, 1000, 2);
				i++;
			};

			break;

		case 2:

			if (!com1.Update())
			{
				packetnum++;

				if (!rb.recieved)
				{ 
					errornum++; 
				}
				else if (rb.len != sizeof(rsp.f1))
				{
					lenErrors++;
				}
				else if (GetCRC16(&rsp, sizeof(rsp.f1)) != 0)
				{
					crcErrors++;
				}
				else if (rsp.f1.padr != req.f1.padr)
				{
					pageErrors++;
				};


				if (rb.recieved && rb.len == sizeof(rsp.f1) && rsp.f1.padr == req.f1.padr && GetCRC16(&rsp, sizeof(rsp.f1)) == 0)
				{
					oknum++;

					if (rsp.f1.status == 1)
					{
						pa += PAGESIZE;
						p += PAGESIZE>>2;

						if (count > PAGESIZE>>2)
						{
							count -= PAGESIZE>>2; 
							i = 0;
						}
						else
						{
							i++;
						};

						SendMessage(prBar, PBM_SETPOS,  2048 * (ArraySize(flashPages) - count) / ArraySize(flashPages), 0);
					}
					else
					{
						start = false;
						i = 0;
						sprintf_s(buf, ArraySize(buf), "Ошибка программирования по адресу 0x%08lX", pa);
						MessageBox(hWnd, buf, "Ахтунг", MB_OK); 
					};
				}
				else
				{
					i = 0;
				};

				RECT rect = { 10, 80, 480, 180 };

				Printf(10, 80, "Запросов: %lu Таймаут: %lu Длина: %lu CRC: %lu PAGE: %lu", packetnum, errornum, lenErrors, crcErrors, pageErrors);
				Printf(10, 96, "OK: %lu      ", oknum);
				Printf(10, 112, "Адрес: 0x%08lX    ", pa);
				RedrawWindow(hWnd, &rect, 0, RDW_INVALIDATE);
			};

			break;

		case 3:

			req.func = 0x80000000;
			req.fx.data = 0x12345678;
			req.fx.crc = GetCRC16(&req, sizeof(req.fx) - sizeof(req.fx.crc));

			wb.data = &req;
			wb.len = sizeof(req.fx);
			com1.Write(&wb);
			i++;

			break;

		case 4:

			if (!com1.Update())
			{
				MessageBox(hWnd, "Программирование завершено", "Ахтунг", MB_OK); 
				run = false;
				start = false;
				i++;
			};

			break;

		case 5:


			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
	static byte i = 0;
	static u32 pt = 0;

	u32 n;

	if (start) switch(i)
	{
		case 0:

			n = (u32)SendMessage(cbCom, CB_GETCURSEL, 0, 0);

			char buf[1024];

			SendMessage(cbCom, CB_GETLBTEXT, n, (LPARAM)buf);

			if (!com1.Connect(buf, 115200, 0))
			{
				MessageBox(hWnd, "Не удалось открыть COM-порт", "", MB_OK); 
				start = false;
			}
			else
			{
				i++;
			};

			break;

		case 1:

			reqHS.guid = masterGUID;
			reqHS.crc = GetCRC16(&reqHS, sizeof(reqHS) - sizeof(reqHS.crc));
			wb.data = &reqHS;
			wb.len = sizeof(reqHS);
			com1.Write(&wb);
			i++;

		case 2:

			if (!com1.Update())
			{
				rb.data = &rspHS;
				rb.maxLen = sizeof(rspHS);
				com1.Read(&rb, 50, 2);
				i++;
			};

			break;

		case 3:

			if (!com1.Update())
			{
				if (rb.recieved && rb.len == sizeof(rspHS) && GetCRC16(&rspHS, sizeof(rspHS)) == 0 && rspHS.guid == slaveGUID)
				{
					pt = GetTickCount();
					i++;
				}
				else
				{
					i = 1;
				};
			};

			break;

		case 4:

			if ((GetTickCount() - pt) > 100)
			{
				i++;
			};

			break;

		case 5:

			UpdateRequest();

			break;

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//int main()
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	static bool pb = false;
	static dword pt = 0;

	InitDisplay();

	while (run)
	{
		UpdateCom();

		UpdateDisplay();

	};

	return 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

