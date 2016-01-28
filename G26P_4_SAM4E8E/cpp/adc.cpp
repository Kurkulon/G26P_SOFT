#include <stdlib.h>

#include "common.h"
#include "main.h"
#include "adc.h"
#include "rtc.h"


static u16 adc_min[8];
static u16 adc_max[8];
static u32 adc_value[8];
static u16 adc_count[8];
static u16 adc_time_ms;

void ADC_Idle()
{
	u32 ms = RTC_Get_Millisecond();
	if((((ms + 1000) - adc_time_ms) % 1000) >= ADC_PERIOD_MS)
	{
		//adc_time_ms = ms;
  //     		u32 status = AT91C_BASE_ADC->ADC_SR;
	 //       if((status & ADC_CHANNELS_MASK) == ADC_CHANNELS_MASK)
		//{
		//	byte i = 1;
		//	for(i = 0; i < 8; i++)
		//	{
	 //      	         	if(status & (1 << i))
		//		{
		//			if(adc_count[i] < 0xFFFF)
		//			{
		//				u16 adc = (*(u32 *)((u32)(&AT91C_BASE_ADC->ADC_CDR0) + i * sizeof(u32))) & AT91C_ADC_DATA;
		//				adc_value[i] += adc; 
		//				adc_count[i] ++;
		//				if(adc_min[i] > adc) adc_min[i] = adc; 
		//				if(adc_max[i] < adc) adc_max[i] = adc; 
		//			}	
		//		}
		//	}
		//}
		//AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
	}
}

bool ADC_Get(byte channel, adc_measure_type type, u16 *value)
{
	if(adc_count[channel] == 0) return false;
	u16 average = (u16)(adc_value[channel] / adc_count[channel]);
	switch (type)
	{
		case ADC_MEASURE_AVERAGING:
			*value = average;
			break;
		case ADC_MEASURE_PEAK_DETECTION:
			if(abs((i16)average - (i16)adc_min[channel]) >= abs((i16)average - (i16)adc_max[channel])) *value = adc_min[channel]; else *value = adc_max[channel];
			break;
		case ADC_MEASURE_MAX:
			*value = adc_max[channel];
			break;
		case ADC_MEASURE_MIN:
			*value = adc_min[channel];
			break;
	}
	adc_value[channel] = 0;
	adc_count[channel] = 0;
	adc_min[channel] = 0;//AT91C_ADC_DATA;
	adc_max[channel] = 0;
	return true;
}

void ADC_Init()
{
	byte i;
	for(i = 0; i < 8; i++)
	{
		adc_value[i] = 0;
		adc_count[i] = 0;
	}
	//AT91C_BASE_ADC->ADC_CR = AT91C_ADC_SWRST;
	//AT91C_BASE_ADC->ADC_MR = AT91C_ADC_TRGEN_DIS | AT91C_ADC_LOWRES_10_BIT | AT91C_ADC_SLEEP_NORMAL_MODE | (AT91C_ADC_PRESCAL) | (AT91C_ADC_STARTUP); // частота по-минимуму, стартап тоже
	//AT91C_BASE_ADC->ADC_CHER = ADC_CHANNELS_MASK;
	//AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
}


