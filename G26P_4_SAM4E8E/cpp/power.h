#ifndef MEMORY_AT91SAM7X256_POWER_H
#define MEMORY_AT91SAM7X256_POWER_H

#define POWER_MESURE_PERIOD_MS		200
#define POWER_CONTROL_PERIOD_MS		10
#define POWER_CONTROL_ENABLE_MS		100 // пауза от включения CM12 до CM33

enum
{
	POWER_STATUS_MASK_SWITCHED_ON_BIT = 0,
	POWER_STATUS_MASK_TRANSMISSION_ENABLED_BIT,
};

typedef enum __attribute__ ((packed)) 
{
	POWER_BATTERY_STATUS_WAIT = 0,
	POWER_BATTERY_STATUS_ON,
	POWER_BATTERY_STATUS_OFF,
	POWER_BATTERY_STATUS_WARNING_MIN,   	// без отключения
	POWER_BATTERY_STATUS_WARNING_MAX,
	POWER_BATTERY_STATUS_ERROR_MIN,		// с отключением
	POWER_BATTERY_STATUS_ERROR_MAX,
} power_battery_status_type;

typedef enum __attribute__ ((packed)) 
{
	POWER_LINE_STATUS_WAIT = 0,
	POWER_LINE_STATUS_ON,
	POWER_LINE_STATUS_OFF,
	POWER_LINE_STATUS_WARNING_MIN,   	// без отключения
	POWER_LINE_STATUS_WARNING_MAX,
	POWER_LINE_STATUS_ERROR_MIN,		// с отключением
	POWER_LINE_STATUS_ERROR_MAX,
} power_line_status_type;

extern void Power_Idle();
extern void Power_Init();
extern unsigned int Power_Status_Mask_Get();
extern bool Power_Transmission_Get();
extern void Power_Switch_Set(bool s);
extern void Power_Transmission_Set(bool t);

extern short Power_Battery_Voltage_Get();
extern power_battery_status_type Power_Battery_Status_Get();

extern short Power_Line_Voltage_Get();
extern power_line_status_type Power_Line_Status_Get();


#endif
