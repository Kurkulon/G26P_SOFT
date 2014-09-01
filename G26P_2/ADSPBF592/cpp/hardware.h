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

extern void ReadSPORT(void *dst1, void *dst2, u16 len1, u16 len2, u16 clkdiv, bool *ready0, bool *ready1);

inline void ManDisable() { *pPORTFIO = 0x50; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; }
inline void ManOne() { *pPORTFIO = 0x50; *pPORTFIO = 0x30; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0xC0; *pPORTFIO_SET = 0x30; }
inline void ManZero() { *pPORTFIO = 0x50; *pPORTFIO = 0xC0; } //{ *pPORTFIO_CLEAR = 0xA0; *pPORTFIO_SET = 0x50; *pPORTFIO_CLEAR = 0x30; *pPORTFIO_SET = 0xC0; }

extern void SendManCmd(void *data, u16 len);
extern void SendManData(void *data, u16 len);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


struct TM32
{
	u32 pt;

	TM32() : pt(0) {}
	bool Check(u32 v) { if ((GetRTT() - pt) >= v) { pt += v; return true; } else { return false; }; }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif // HARDWARE_H__15_05_2009__14_35
