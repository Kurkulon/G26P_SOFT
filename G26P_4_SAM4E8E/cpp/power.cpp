#include "common.h"
#include "main.h"
#include "trap.h"
#include "power.h"
#include "adc.h"
#include "fram.h"

//-----------------------------------------
unsigned int power_status_mask = (1u << POWER_STATUS_MASK_TRANSMISSION_ENABLED_BIT);

unsigned int Power_Status_Mask_Get()
{
	return power_status_mask;
}

bool Power_Transmission_Get()
{
	return (bool)((power_status_mask >> POWER_STATUS_MASK_TRANSMISSION_ENABLED_BIT) & 1);
}

void Power_Switch_Set(bool s)
{
	power_status_mask = (power_status_mask & ~(1u << POWER_STATUS_MASK_SWITCHED_ON_BIT)) | ((s & 0x1) << POWER_STATUS_MASK_SWITCHED_ON_BIT);
}

void Power_Transmission_Set(bool t)
{
	power_status_mask = (power_status_mask & ~(1u << POWER_STATUS_MASK_TRANSMISSION_ENABLED_BIT)) | ((t & 0x1) << POWER_STATUS_MASK_TRANSMISSION_ENABLED_BIT);
}

//------------------------------------------
short power_battery_voltage = 0;
power_battery_status_type power_battery_status = POWER_BATTERY_STATUS_WAIT;

power_battery_status_type Power_Battery_Status_Get()
{
	return power_battery_status;
}

short Power_Battery_Voltage_Get()
{
	return power_battery_voltage;
}

//----------------------------------------------
short power_line_voltage = 0;
power_line_status_type power_line_status = POWER_LINE_STATUS_WAIT;

power_line_status_type Power_Line_Status_Get()
{
	return power_line_status;
}

short Power_Line_Voltage_Get()
{
	return power_line_voltage;
}

//----------------------------------------------

unsigned short power_measure_time_ms;

bool Power_Measure_Idle()
{
	unsigned short ms = RTC_Get_Millisecond();
	if((((ms + 1000) - power_measure_time_ms) % 1000) < POWER_MESURE_PERIOD_MS)  return false; 
	power_measure_time_ms = ms;
	float k, b;
	unsigned short value;
	if(ADC_Get(ADC_CHANNEL_BATTERY, ADC_MEASURE_MIN, &value))
	{
		FRAM_Power_Battery_Coeffs_Get(&k, &b);
		power_battery_voltage = (short)(((float)value * k + b) * 10);	// *10 т.к всё в 0.1В
	}
	else return false;
	if(ADC_Get(ADC_CHANNEL_LINE, ADC_MEASURE_MIN, &value))
	{
		FRAM_Power_Line_Coeffs_Get(&k, &b);
		power_line_voltage = (short)(((float)value * k + b) * 10);	// *10 т.к всё в 0.1В
	}  return false;
//	return true;
}

//----------------------------------------------
unsigned short power_control_time_ms;
unsigned short power_control_time_enable_ms;

void Power_Control_Idle()
{
	//unsigned short msec = RTC_Get_Millisecond();
	//if((((msec + 1000) - power_control_time_ms) % 1000) < POWER_CONTROL_PERIOD_MS)  return; // раз в 10 мс, чаще не надо
	//power_control_time_ms = msec;

	//short setup, min, max;
	//// проверяем батарейку, здесь же добавим ERROR с отключением
	//FRAM_Power_Battery_Voltages_Get(&setup, &min, &max);
	//if(power_battery_voltage >= max) power_battery_status = POWER_BATTERY_STATUS_WARNING_MAX;
	//else if(power_battery_voltage <= min) power_battery_status = POWER_BATTERY_STATUS_WARNING_MIN;
	//else power_battery_status = POWER_BATTERY_STATUS_ON;
	//// работа линии
	//if(power_status_mask & (1u << POWER_STATUS_MASK_SWITCHED_ON_BIT)) 
	//{
	//	if(!(AT91C_BASE_PIOB->PIO_ODSR & AT91C_PIO_PB29))
	//	{	// включение линии
	//		AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB29;
	//		power_line_status = POWER_LINE_STATUS_ON;
	//		power_control_time_enable_ms = msec;
	//	}
	//	else
	//	{
	//		if((AT91C_BASE_PIOB->PIO_ODSR & AT91C_PIO_PB19))
	//		{	
	//	                if((((msec + 1000) - power_control_time_enable_ms) % 1000) >= POWER_CONTROL_ENABLE_MS)	AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB19;
	//		}
	//		// проверяем линию, здесь же добавим ERROR с отключением
	//		FRAM_Power_Line_Voltages_Get(&setup, &min, &max);
	//		if(power_line_voltage >= max) power_line_status = POWER_LINE_STATUS_WARNING_MAX;
	//		else if(power_line_voltage <= min) power_line_status = POWER_LINE_STATUS_WARNING_MIN;
	//		else power_line_status = POWER_LINE_STATUS_ON;
	//	}
	//}
	//else 
	//{
	//	if(!(AT91C_BASE_PIOB->PIO_ODSR & AT91C_PIO_PB19))
	//	{
	//		AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB19;
	//	}
	//	if((AT91C_BASE_PIOB->PIO_ODSR & AT91C_PIO_PB29))
	//	{
	//		AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB29;
	//	}
	//	power_line_status = POWER_LINE_STATUS_OFF;
	//}
}

//----------------------------------------------

void Power_Idle()
{
    	Power_Measure_Idle();
    	Power_Control_Idle();
}

void Power_Init()
{

	//AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB29;	// Включитть линию
 //       AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB29;
	//AT91C_BASE_PIOB->PIO_OWER = AT91C_PIO_PB29;
	//AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB29;

	//AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB19;	// Включитть линию
 //       AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB19;
	//AT91C_BASE_PIOB->PIO_OWER = AT91C_PIO_PB19;
	//AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB19;


}


