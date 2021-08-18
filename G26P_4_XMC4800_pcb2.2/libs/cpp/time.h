#ifndef TIME_H__04_08_2009__17_35
#define TIME_H__04_08_2009__17_35

#ifdef WIN32
#include <windows.h>
#else
#include "core.h"
#endif

#include "types.h"

#define RTC_type RTC

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RTC
{
	__packed union
	{
		__packed struct
		{
			u32 msec:10;     // mili second value - [0,999] 
			u32 sec:6;     // Second value - [0,59] 
			u32 min:6;     // Minute value - [0,59] 
			u32 hour:5;    // Hour value - [0,23] 
		};

		u32 time;
	};
	__packed union
	{
		__packed struct
		{
			u32 day:5;    // Day of the month value - [1,31] 
			u32 mon:4;     // Month value - [1,12] 
			u32 year:12;    // Year value - [0,4095] 
		};

		u32 date;
	};

	inline void operator=(const RTC &r) { time = r.time; date = r.date; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void Init_time();
extern void RTT_Init();

extern bool SetTime(const RTC &t);
extern void GetTime(RTC *t);

//extern const u32 msec;

inline i32 GetMilliseconds()
{
#ifndef WIN32
	extern u32 msec;
	return msec;
#else
	return GetTickCount();
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u16 GetMillisecondsLow()
{
#ifndef WIN32
	extern u32 msec;
	return (u16)msec;
#else
	return (u16)(GetTickCount());
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32
#define US2RT(x) (((x)*MCK_MHz+1024)/2048)
#define MS2RT(x) (((x)*MCK_MHz*1000+1024)/2048)
#else
#define US2RT(x) (x)
#define MS2RT(x) (x)
#endif

#ifndef WIN32
inline u16 GetRTT() { return HW::CCU43_CC43->TIMER; }
#else
inline u16 GetRTT() { return 0; }
#endif

struct RTM
{
	u16 pt;

	//RTM16() : pt(0) {}
	bool Check(u16 v) { if ((u16)(GetRTT() - pt) >= v) { pt = GetRTT(); return true; } else { return false; }; }
	bool Timeout(u16 v) { return (GetRTT() - pt) >= v; }
	void Reset() { pt = GetRTT(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TM32
{
	u32 pt;

	//TM32() : pt(0) {}
	bool Check(u32 v) { u32 t = GetMilliseconds(); if ((t - pt) >= v) { pt = t; return true; } else { return false; }; }
	bool Timeout(u32 v) { return (GetMilliseconds() - pt) >= v; }
	void Reset() { pt = GetMilliseconds(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Dbt
{
//	bool	stat;
	u32		pt;
	u32		dt;

	Dbt(u32 t = 500) : pt(0), dt(t) {}

	bool Check(bool c)
	{
		if (!c)
		{ 
			pt = GetMilliseconds(); 
		} 
		else if ((GetMilliseconds() - pt) < dt)
		{ 
			c = false; 
		}; 

		return c;
	}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Deb
{
	bool	stat;
	u32		pt;
	u32		dt;

	Deb(bool s = false, u32 t = 500) : stat(s), pt(0), dt(t) {}

	bool Check(bool c)
	{
		if (stat == c)
		{ 
			pt = GetMilliseconds(); 
		} 
		else if ((GetMilliseconds() - pt) >= dt)
		{ 
			stat = c; 
		}; 

		return stat;
	}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // TIME_H__04_08_2009__17_35
