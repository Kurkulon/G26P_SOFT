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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MRB
{
	bool	ready;
	bool	OK;
	u16		len;
	u16		maxLen;
	void	*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MTB
{
	bool	ready;
	u16		len;
	void	*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool RcvManData(MRB *mrb);
extern bool SendManData(MTB *mtb);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define US2CLK(x) ((u32)(x*100+0.5))


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TM32
{
	u32 pt;

	TM32() : pt(0) {}
	bool Check(u32 v) { if ((GetRTT() - pt) >= v) { pt += v; return true; } else { return false; }; }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



#endif // HARDWARE_H__15_05_2009__14_35
