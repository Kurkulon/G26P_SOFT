#include "common.h"
#include "main.h"
#include "adc.h"
#include "sensors.h"
#include "rtc.h"
#include "fram.h"

//-----------------------------------------
short sensors_ax = 0;
short sensors_ay = 0;
short sensors_az = 0;

short Sensors_Ax_Get()
{
	return sensors_ax;
}

short Sensors_Ay_Get()
{
	return sensors_ay;
}

short Sensors_Az_Get()
{
	return sensors_az;
}

short Sensors_Temperature_In_Get()
{
	return RTC_Get_Temperature();
}

//----------------------------------------------
unsigned short sensors_measure_time_ms = 0;

bool Sensors_Measure_Idle()
{
	unsigned short ms = RTC_Get_Millisecond();
	if((((ms + 1000) - sensors_measure_time_ms) % 1000) < SENSORS_MESURE_PERIOD_MS)  return false; 
	sensors_measure_time_ms = ms;
	float k, b;
	unsigned short value;
	if(ADC_Get(ADC_CHANNEL_AX, ADC_MEASURE_PEAK_DETECTION, &value))
	{
		FRAM_Sensors_Ax_Coeffs_Get(&k, &b);
		sensors_ax = (short)((((float)value - 512) * k + b) * 100);	// *100 т.к всё в 0.01g
	}
	else return false;
	if(ADC_Get(ADC_CHANNEL_AY, ADC_MEASURE_PEAK_DETECTION, &value))
	{
		FRAM_Sensors_Ay_Coeffs_Get(&k, &b);
		sensors_ay = (short)((((float)value - 512) * k + b) * 100);	// *100 т.к всё в 0.01g
	}
	else return false;
	if(ADC_Get(ADC_CHANNEL_AZ, ADC_MEASURE_PEAK_DETECTION, &value))
	{
		FRAM_Sensors_Az_Coeffs_Get(&k, &b);
		sensors_az = (short)((((float)value - 512) * k + b) * 100);	// *100 т.к всё в 0.01g
	}
	else return false;
	return true;
}

//----------------------------------------------

void Sensors_Idle()
{
    	Sensors_Measure_Idle();
}

void Sensors_Init()
{

}


