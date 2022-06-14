#ifndef HARDWARE_H__15_05_2009__14_35
#define HARDWARE_H__15_05_2009__14_35

#include "types.h"
//#include "core.h"
#include "time.h"
#include "i2c.h"
#include "spi.h"
#include "hw_nand.h"
#include "hw_rtm.h"
#include "manch.h"

extern void InitHardware();
extern void UpdateHardware();

extern bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init);
extern bool CRC_CCITT_DMA_CheckComplete(u16* crc);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void SetClock(const RTC &t);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__forceinline u32 Push_IRQ()
{
	register u32 t;

#ifndef WIN32

	register u32 primask __asm("primask");

	t = primask;

	__disable_irq();

#endif

	return t;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__forceinline void Pop_IRQ(u32 t)
{
#ifndef WIN32

	register u32 primask __asm("primask");

	primask = t;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifdef WIN32

extern void UpdateDisplay();
extern int PutString(u32 x, u32 y, byte c, const char *str);
extern int Printf(u32 x, u32 y, byte c, const char *format, ... );

#endif

#endif // HARDWARE_H__15_05_2009__14_35
