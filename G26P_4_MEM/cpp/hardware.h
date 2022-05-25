#ifndef HARDWARE_H__15_05_2009__14_35
#define HARDWARE_H__15_05_2009__14_35

#include "types.h"
//#include "core.h"
#include "time.h"
#include "i2c.h"
#include "spi.h"
#include "hw_nand.h"
#include "hw_rtm.h"

extern void InitHardware();
extern void UpdateHardware();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u16 CheckParity(u16 x)
{
	u16 y = x ^ (x >> 1);

	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	
	return (y ^ (y >> 8))^1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MRB
{
	volatile bool	ready;
	volatile bool	OK;
	volatile u16	len;
	u16				maxLen;
	u16				*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MTB
{
	bool		ready;
	u16			baud;
	u16			len1;
	const u16	*data1;
	u16			len2;
	const u16	*data2;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool RcvManData(MRB *mrb);
extern bool SendManData(MTB *mtb);
extern void SetTrmBoudRate(byte i);
extern void ManRcvUpdate();
extern void ManRcvStop();

extern bool SendMLT3(MTB *mtb);
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
