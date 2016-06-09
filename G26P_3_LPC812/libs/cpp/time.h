#ifndef TIME_H__04_08_2009__17_35
#define TIME_H__04_08_2009__17_35

#ifdef WIN32
#include <windows.h>
#endif

#include "types.h"
#include "core.h"


extern void Init_time();
extern void RTT_Init();

extern const u32 msec;

inline u32 GetMilliseconds()
{
#ifndef WIN32
//	extern u32 msec;
	return msec;
#else
	return GetTickCount();
#endif
}

inline word GetMillisecondsLow()
{
#ifndef WIN32
//	extern u32 msec;
	return (u16)msec;
#else
	return (word)(GetTickCount());
#endif
}

#define US2RT(x) ((x*32768+500000)/1000000)
#define MS2RT(x) ((x*32768+500)/1000)

inline u32 GetRTT() { return 0; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TM32
{
	u32 pt;

	TM32() : pt(0) {}
	bool Check(u32 v) { if ((GetMilliseconds() - pt) >= v) { pt = GetMilliseconds(); return true; } else { return false; }; }
	void Reset() { pt = GetMilliseconds(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RTM32
{
	u32 pt;

	RTM32() : pt(0) {}
	bool Check(u32 v) { if ((GetRTT() - pt) >= v) { pt = GetRTT(); return true; } else { return false; }; }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // TIME_H__04_08_2009__17_35
