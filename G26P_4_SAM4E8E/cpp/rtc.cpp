//#include "common.h"
#include "rtc.h"
//#include "ds32c35.h"


RTC_type rtc;
RTC_type rtc_set;
bool rtc_set_need = false;
short rtc_temperature;
unsigned char rtc_event = 0;

/*inline*/ RTC_type RTC_Get()
{
	return rtc;	
}

bool RTC_Check(RTC_type d_t)
{
	if(d_t.year < rtc.year) return true;
	if(d_t.year > rtc.year) return false;
	if(d_t.mon < rtc.mon) return true;
	if(d_t.mon > rtc.mon) return false;
	if(d_t.day < rtc.day) return true;
	if(d_t.day > rtc.day) return false;
	if(d_t.hour < rtc.hour) return true;
	if(d_t.hour > rtc.hour) return false;
	if(d_t.min < rtc.min) return true;
	if(d_t.min > rtc.min) return false;
	if(d_t.sec < rtc.sec) return true;
	if(d_t.sec > rtc.sec) return false;
	if(d_t.msec < rtc.msec) return true;
	if(d_t.msec > rtc.msec) return false;
	return true;
}


/*inline*/ unsigned short RTC_Get_Millisecond()
{
	return rtc.msec;	
}

//void RTC_Set(RTC_type x)
//{
//	rtc_set = x;
//	rtc_set_need = true;
//	rtc_event = 0; // миллисекунды сбрасывать нет смысла, т.к DS32C35 работает с секундами
//}

short RTC_Get_Temperature()
{
	return rtc_temperature;
}

void RTC_Isr()
{
	//volatile unsigned short temp = AT91C_BASE_PITC->PITC_PIVR; // read and reset
	//rtc_event = 0;
}

void RTC_Init()
{
	//AT91C_BASE_PITC->PITC_PIMR = RTC_TIME_PERIOD_TICS;
	//AT91C_BASE_PITC->PITC_PIMR |= AT91C_PITC_PITEN;	      // использовать библеотечную функцию нельзя, т.к. там всё округлено.
	//AT91F_PITC_CfgPMC();
	//AT91F_PMC_EnablePeriphClock(AT91C_BASE_PMC, ((unsigned int) 1 << AT91C_ID_IRQ0));
	//AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, ((unsigned int) AT91C_PA30_IRQ0), 0);
	//AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_IRQ0, RTC_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, RTC_Isr);
	//AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_IRQ0);
}

void RTC_Idle()
{
 //       rtc.msec = ((AT91C_BASE_PITC->PITC_PIIR & AT91C_PITC_PICNT) >> 20);  // read and no reset
	//unsigned char t;
	//unsigned char old;	
	//switch	(rtc_event)
	//{
	//        case 0: if(rtc_set_need) if(!DS32C35_RTC_Set_Second(rtc_set.sec)) break;
	//		if(DS32C35_RTC_Get_Second(&t))
	//		{
	//			rtc.msec %= 1000;
	//			old = rtc.sec;
	//			rtc.sec = t;
	//			rtc_event ++;
	//		}
	//		if(((t > 0) || (old == t)) && (!rtc_set_need)) break;
	//        case 1: if(rtc_set_need) if(!DS32C35_RTC_Set_Minute(rtc_set.min)) break;
	//		if(DS32C35_RTC_Get_Minute(&t))
	//		{
	//			old = rtc.min;
	//			rtc.min = t;	
	//			rtc_event ++;
	//		}
	//		if(((t > 0) || (old == t)) && (!rtc_set_need)) break;
	//        case 2: if(rtc_set_need) if(!DS32C35_RTC_Set_Hour(rtc_set.hour)) break;
	//		if(DS32C35_RTC_Get_Hour(&t))
	//		{
	//			old = rtc.hour;
	//			rtc.hour = t;
	//			rtc_event ++;
	//		}
	//		if(((t > 0) || (old == t)) && (!rtc_set_need)) break;
	//        case 3: if(rtc_set_need) if(!DS32C35_RTC_Set_Day(rtc_set.day)) break;
	//		if(DS32C35_RTC_Get_Day(&t)) 
	//		{
	//			old = rtc.day;
	//			rtc.day = t;
	//			rtc_event ++;
	//		}
	//		if(((t > 1) || (old == t)) && (!rtc_set_need)) break;
	//        case 4: if(rtc_set_need) if(!DS32C35_RTC_Set_Month(rtc_set.mon)) break;
	//		if(DS32C35_RTC_Get_Month(&t)) 
	//		{
	//			old = rtc.mon;
	//			rtc.mon = t;
	//			rtc_event ++;
	//		}
	//		if(((t > 1) || (old == t)) && (!rtc_set_need)) break;
	//        case 5: if(rtc_set_need) if(!DS32C35_RTC_Set_Year(rtc_set.year - 2000)) break;
	//		if(DS32C35_RTC_Get_Year(&t)) 
	//		{
	//			rtc.year = (unsigned short)t + 2000;
	//			rtc_event ++;
	//		}
	//		break;
	//	case 6: 
	//		if(DS32C35_RTC_Get_Temperature((signed char *)&t))
	//		{
	//			rtc_temperature = ((short)t) * 10;	// в данном слечае датчик даёт точность 1гр, нам надо 0.1гр
	//                	rtc_event ++;
	//		}
	//		break;
	//	case 7: rtc_set_need = false;
	//		bool busy = true;
	//		if(DS32C35_RTC_Get_Busy(&busy))
	//		{
	//			rtc_event ++;
	//                	if(busy) rtc_event ++;
	//		}
 //                       break;
	//	case 8: 
	//		if(DS32C35_RTC_Start_Temperature_Conversion())
	//		{
	//                	rtc_event ++;
	//		}
	//                break;
	//	default: break;
	//}
	//if(rtc.msec > 999) rtc.msec = 999;
}
