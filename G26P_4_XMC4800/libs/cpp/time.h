#ifndef TIME_H__04_08_2009__17_35
#define TIME_H__04_08_2009__17_35

#ifdef WIN32
#include <windows.h>
#endif

#include "types.h"
#include "core.h"

//typedef dword time_utc;

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

//struct time_bdc 
//{ 
//	byte hsecond; byte second; byte minute; byte hour; byte day; byte month; byte dayofweek; word year; time_utc t; 
//
//	//time_bdc(const time_bdc& t) 
//	//{
//	//	*(dword*)this = *(dword*)&t;
//	//	*((dword*)this+1) = *((dword*)&t+1);
//	//	*((word*)this+4) = *((word*)&t+4);
//	//}
//
//	//void operator=(const time_bdc& t) 
//	//{
//	//	*(dword*)this = *(dword*)&t;
//	//	*((dword*)this+1) = *((dword*)&t+1);
//	//	*((dword*)this+2) = *((dword*)&t+2);
//	//	*((word*)this+6) = *((word*)&t+6);
//	//}
//};
//
//extern time_bdc timeBDC;

//extern time_utc mktime_utc(const time_bdc &t);
//extern time_utc mktime_utc(time_bdc *t);
//extern bool mktime_bdc(time_utc t, time_bdc *tbdc);
//extern void Init_time();

extern void RTT_Init();
extern void InitTimer();


//extern bool SetTime(time_utc t);
extern bool SetTime(const RTC &t);
//extern time_utc GetTime();
extern void GetTime(RTC *t);
//extern bool CheckTime(const time_bdc &t);
//extern int CompareTime(const time_bdc &t1, const time_bdc &t2);
//extern void NextDay(time_bdc *t);
//extern void NextHour(time_bdc *t);
//extern void PrevDay(time_bdc *t);
//extern void PrevHour(time_bdc *t);
//extern dword msec;

inline dword GetMilliseconds()
{
#ifndef WIN32
	extern dword msec;
	return msec;
#else
	return GetTickCount();
#endif
}

inline word GetMillisecondsLow()
{
#ifndef WIN32
	extern dword msec;
	return (word)msec;
#else
	return (word)(GetTickCount());
#endif
}

#define US2RT(x) (((x)*25+256)/512)
#define MS2RT(x) (((x)*3125+32)/64)

inline u16 GetRTT() { return HW::CCU43_CC43->TIMER; }

//inline u32 GetRTT()
//{
//	u32 t1 = HW::RTT->VR;
//	u32 t2;
//
//	while ((t2 = HW::RTT->VR) != t1) { t1 = t2; };
//
//	return t2;
//} 

//inline u32 GetRTT() { return HW::RTT->VR|HW::RTT->VR; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TM32
{
	u32 pt;

	static u32 ipt;

	TM32() : pt(ipt++) {}
	bool Check(u32 v) { if ((GetMilliseconds() - pt) >= v) { pt = GetMilliseconds(); return true; } else { return false; }; }
	void Reset() { pt = GetMilliseconds(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RTM16
{
	u16 pt;

	RTM16() : pt(0) {}
	bool Check(u16 v) { if ((u16)(GetRTT() - pt) >= v) { pt = GetRTT(); return true; } else { return false; }; }
	void Reset() { pt = GetRTT(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // TIME_H__04_08_2009__17_35
