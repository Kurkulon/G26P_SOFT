#ifndef HARDWARE_H__15_05_2009__14_35
#define HARDWARE_H__15_05_2009__14_35

#include "types.h"
#include "core.h"

#ifdef WIN32
#include <windows.h>
#endif


extern void InitHardware();
extern void UpdateHardware();

//inline float GetValueADC1(byte i) { extern float valueADC1[8]; return valueADC1[i]; }
//inline float GetValueADC2(byte i) { extern float valueADC2[8]; return valueADC2[i]; }
//
//inline const float* GetADCptr1(byte i) { extern float valueADC1[8]; return valueADC1+i; }
//inline const float* GetADCptr2(byte i) { extern float valueADC2[8]; return valueADC2+i; }
//
//extern bool Get_Freq(float &v);
//extern u32 Get_FreqTime();
//
//extern byte GetSwitchAddress();
//extern dword GetSwitchBaudRate();

//extern int PutString(byte x, byte y, const char *str);
//extern void PutChar(byte x, byte y, char c);
extern int Printf(byte x, byte y, const char *format, ... );

//extern void SetCursorPosition(byte x, byte y);
//extern void ShowCursor();
//extern void HideCursor();
//
//inline void SetCurLim(float v) { extern float curLim; curLim = v; }

extern void Printf16(word x, word y, word c, const char *format, ... );
extern void DrawWave(byte n, void *data);


#endif // HARDWARE_H__15_05_2009__14_35
