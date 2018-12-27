#include "time.h"
#include "core.h"

#include <stdlib.h>

#ifdef WIN32

#include <windows.h>
#include <time.h>

#endif

#pragma O3
#pragma Otime

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

dword msec = 0;

//static U32u __hsec(0);

u32 TM32::ipt = 0;

RTC timeBDC;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//static bool sync_Loc_RTC = false; // Синхронизация локальных часов с часами реального времени
//static bool sync_RTC_Loc = false; // Синхронизация часов реального времени с локальными часами 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const byte daysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static const word __diyr[] = {									/* days in normal year array */
//    0,                                                          /* Jan */
//    31,                                                         /* Feb */
//    31 + 28,                                                    /* Mar */
//    31 + 28 + 31,                                               /* Apr */
//    31 + 28 + 31 + 30,                                          /* May */
//    31 + 28 + 31 + 30 + 31,                                     /* Jun */
//    31 + 28 + 31 + 30 + 31 + 30,                                /* Jul */
//    31 + 28 + 31 + 30 + 31 + 30 + 31,                           /* Aug */
//    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,                      /* Sep */
//    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 /* Oct */
//    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            /* Nov */
//    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       /* Dec */
//    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31   /* Jan, next year */
//};
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//static const word __dilyr[] = {									/* days in leap year array */
//    0,                                                          /* Jan */
//    31,                                                         /* Feb */
//    31 + 29,                                                    /* Mar */
//    31 + 29 + 31,                                               /* Apr */
//    31 + 29 + 31 + 30,                                          /* May */
//    31 + 29 + 31 + 30 + 31,                                     /* Jun */
//    31 + 29 + 31 + 30 + 31 + 30,                                /* Jul */
//    31 + 29 + 31 + 30 + 31 + 30 + 31,                           /* Aug */
//    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,                      /* Sep */
//    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 /* Oct */
//    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            /* Nov */
//    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       /* Dec */
//    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31   /* Jan, next year */
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//inline bool __leapyear(word year)
//{
//    return (year&3) == 0;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// #define DAYS_IN_4_YRS   ( 365 + 365 + 365 + 366 )
// #define DAYS_IN_400_YRS ( ( 100 * DAYS_IN_4_YRS ) - 3 )

//  #define SECONDS_PER_DAY ( 24 * 60 * 60 )
//  extern  short   __diyr[], __dilyr[];

/*
 The number of leap years from year 1 to year 1900 is 460.
 The number of leap years from year 1 to current year is
 expressed by "years/4 - years/100 + years/400". To determine
 the number of leap years from current year to 1900, we subtract
 460 from the formula result. We do this since "days" is the
 number of days since 1900.
*/

//static dword __DaysToJan1(word year)
//{
//    unsigned    years = 1900 + year - 1;
//    unsigned    leap_days = years / 4 - years / 100 + years / 400 - 460;
//
//    return( year * 365UL + leap_days );
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define SECONDS_FROM_1900_TO_1970       2208988800UL
#define SECONDS_FROM_1972_TO_2000       883612800UL
#define SECONDS_FROM_1970_TO_1972       63072000UL
#define SECONDS_FROM_1970_TO_2000		(SECONDS_FROM_1970_TO_1972+SECONDS_FROM_1972_TO_2000)      

#define SECONDS_PER_DAY                 (24 * 60 * 60)
#define DAYS_FROM_1900_TO_1970          ( ( long ) ( SECONDS_FROM_1900_TO_1970 / SECONDS_PER_DAY ) ) 

#define MONTH_YR        ( 12 )
#define DAY_YR          ( 365 )
#define HOUR_YR         ( DAY_YR * 24 )
#define MINUTE_YR       ( HOUR_YR * 60 )
#define SECOND_YR       ( MINUTE_YR * 60 )
#define __MONTHS        ( INT_MIN / MONTH_YR )
#define __DAYS          ( INT_MIN / DAY_YR )

//// these ones can underflow in 16bit environments,
//// so check the relative values first
//#if ( HOUR_YR ) < ( INT_MAX / 60 )
// #define __MINUTES      ( INT_MIN / MINUTE_YR )
// #if ( MINUTE_YR ) < ( INT_MAX / 60 )
//  #define __SECONDS     ( INT_MIN / SECOND_YR )
// #else
//  #define __SECONDS     ( 0 )
// #endif
//#else
// #define __MINUTES      ( 0 )
// #define __SECONDS      ( 0 )
//#endif

#define SMALLEST_YEAR_VALUE ( __MONTHS + __DAYS + __MINUTES + __SECONDS )

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GetTime(RTC *t)
{
	if (t == 0) return;

	__disable_irq();

	t->date = timeBDC.date;
	t->time = timeBDC.time;

	__enable_irq();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//time_utc GetTimeAll(time_bdc *t)
//{
//#ifndef WIN32
//
//	time_utc tu;
//
//	HW::AIC->IDCR = HW::PID::TC0|HW::PID::SYSC;
//
//	*t = timeBDC;
//	tu = timeUTC;
//
//	//*(dword*)t = *(dword*)&timeBDC;
//	//*((dword*)t+1) = *((dword*)&timeBDC+1);
//	//*((word*)t+4) = *((word*)&timeBDC+4);
//
//	HW::AIC->IECR = HW::PID::TC0|HW::PID::SYSC;
//
//	return tu;
//
//#else
//
//	GetTime(t);
//	
//	return mktime_utc(*t);
//
//#endif
//}
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


bool CheckTime(const RTC &t)
{
	if (t.sec > 59 || t.min > 59 || t.hour > 23 || t.day < 1 || (t.mon-1) > 11 || (t.year-2000) > 99) { return false; };

	byte d = daysInMonth[t.mon] + ((t.mon == 2 && (t.year&3) == 0) ? 1 : 0);

	if (t.day > d) { return false; };

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SetTime(const RTC &t)
{
#ifndef WIN32

	if (!CheckTime(t)) { return false; };

	__disable_irq();

	timeBDC = t;

	__enable_irq();

	return true;

#else

	SYSTEMTIME lt;

	lt.wMilliseconds	=	t.hsecond*10;
	lt.wSecond			=	t.second	;
	lt.wMinute			=	t.minute	;
	lt.wHour			=	t.hour		;
	lt.wDay				=	t.day		;
	lt.wDayOfWeek		=	t.dayofweek;
	lt.wMonth			=	t.month	;
	lt.wYear			=	t.year		;

	return SetLocalTime(&lt);

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void Timer_Handler (void)
{
	msec++;

	if (timeBDC.msec < 999)
	{
		timeBDC.msec += 1;
	}
	else
	{
		timeBDC.msec = 0;

		if (timeBDC.sec < 59)
		{
			timeBDC.sec += 1;
		}
		else
		{
			timeBDC.sec = 0;

			if (timeBDC.min < 59)
			{
				timeBDC.min += 1;
			}
			else
			{
				timeBDC.min = 0;

				if (timeBDC.hour < 23)
				{
					timeBDC.hour += 1;
				}
				else
				{
					timeBDC.hour = 0;

					byte day = daysInMonth[timeBDC.mon] + ((timeBDC.mon == 2 && (timeBDC.year&3) == 0) ? 1 : 0);

//					if ((timeBDC.dayofweek += 1) > 6) timeBDC.dayofweek = 0;

					if (timeBDC.day < day)
					{
						timeBDC.day += 1;
					}
					else
					{
						timeBDC.day = 1;

						if (timeBDC.mon < 12)
						{
							timeBDC.mon += 1;
						}
						else
						{
							timeBDC.mon = 1;

							timeBDC.year += 1;
						};
					};
				};
			};
		};
	};

	HW::PIOA->ODSR ^= 1<<0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitTimer()
{
	enum { freq = 1000 };

	timeBDC.day = 1;
	timeBDC.mon = 1;
	timeBDC.year = 2000;
	timeBDC.time = 0;

	CM4::SysTick->LOAD = (MCK+freq/2)/freq;
	VectorTableInt[15] = Timer_Handler;
	CM4::SysTick->CTRL = 7;
	__enable_irq();

	HW::PIOA->PER = 1<<0;
	HW::PIOA->OER = 1<<0;
	HW::PIOA->OWER = 1<<0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RTT_Init()
{
	using namespace HW;

	PMC->PCER0 = PID::TC0_M;
	TC0->C0.CMR = 4; // SCLK
	TC0->C0.CCR = 5;

	RTT->MR = 0x40001;
//	RTT->MR = 0x100000;


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


