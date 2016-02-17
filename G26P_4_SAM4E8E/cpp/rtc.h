#ifndef MEMORY_AT91SAM7X256_RTC_H
#define MEMORY_AT91SAM7X256_RTC_H

#include "types.h"


//#pragma anon_unions

#define RTC_INTERRUPT_LEVEL	4

#define RTC_TIME_PERIOD_MS		1	// Дискрет времени 1мс
#define RTC_TIME_PERIOD_TICS 		(u32)(((float)(CLOCK_MCK/16) * (float)RTC_TIME_PERIOD_MS/1000.0f) - 1)

__packed struct RTC_type
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

};

/************ RDC functions *************/
extern void RTC_Init();
extern void RTC_Idle();
extern RTC_type RTC_Get();
extern bool RTC_Check(RTC_type d_t);
extern u16 RTC_Get_Millisecond();
extern void RTC_Set(RTC_type x);
extern short RTC_Get_Temperature();

/*****************************************************************************/

#endif
