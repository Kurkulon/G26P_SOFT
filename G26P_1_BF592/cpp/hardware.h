#ifndef HARDWARE_H__15_05_2009__14_35
#define HARDWARE_H__15_05_2009__14_35
  
#include "types.h"
#include "core.h"

#ifdef WIN32
#include <windows.h>
#endif


extern void InitHardware();
extern void UpdateHardware();

inline u32 GetRTT() { return *pTIMER0_COUNTER; }

extern void SyncReadSPORT(void *dst1, void *dst2, u16 len1, u16 len2, u16 clkdiv, bool *ready0, bool *ready1);
extern void ReadSPORT(void *dst1, void *dst2, u16 len1, u16 len2, u16 clkdiv, bool *ready0, bool *ready1);
extern void WritePGA(u16 v);
inline void SetGain(byte g) { WritePGA(0x2A01|(g<<4)); }
inline u16 GetADC() { extern U32u adcValue; return adcValue.w[1]; }


#endif // HARDWARE_H__15_05_2009__14_35
