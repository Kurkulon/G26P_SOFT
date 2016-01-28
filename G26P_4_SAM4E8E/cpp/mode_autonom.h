#ifndef MEMORY_AT91SAM7X256_MODE_AUTONOM_H
#define MEMORY_AT91SAM7X256_MODE_AUTONOM_H

/*****************************************************************************/
#define MODE_AUTONOM_MAIN_TIMEOUT_S		10 //пауза после подачи питания до установления переходных процессов, секунд
/*****************************************************************************/
#define MODE_AUTONOM_LED_WAIT_PERIOD_S		3
#define MODE_AUTONOM_LED_WAIT_DUTY_MS		50
#define MODE_AUTONOM_LED_WORK_PERIOD_S		1
#define MODE_AUTONOM_LED_WORK_DUTY_MS		950
#define MODE_AUTONOM_LED_FAIL_POWER_PERIOD_S	1
#define MODE_AUTONOM_LED_FAIL_POWER_DUTY_MS	500
#define MODE_AUTONOM_LED_FAIL_OPTIONS_PERIOD_S	2
#define MODE_AUTONOM_LED_FAIL_OPTIONS_DUTY1_MS	500
#define MODE_AUTONOM_LED_FAIL_OPTIONS_DUTY2_MS	50
/*******************************************************************************/
#define MODE_AUTONOM_SPECIAL_IDENTIFICATION_COMMAND		0x3A00
typedef struct __attribute__ ((packed))
{
	unsigned short command;
	unsigned short number;
	unsigned short version;
} mode_autonom_special_identification_type;

#define MODE_AUTONOM_SPECIAL_OPTIONS_COMMAND	0x3A10
typedef struct __attribute__ ((packed))
{
	unsigned short command;
} mode_autonom_special_options_type;

#define MODE_AUTONOM_SPECIAL_MEASURE_COMMAND	0x3A20
typedef struct __attribute__ ((packed))
{
	unsigned short command;
	short battery_voltage;	
	short line_voltage;	
	short temperature_in;
	short ax;
	short ay;
	short az;
} mode_autonom_special_measure_type;

/*******************************************************************************/
typedef enum __attribute__ ((packed)) 
{
	MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT = 0,
	MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_MAKE,
	MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_TX,
	MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_RX,
	MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_SAVE,
} mode_autonom_measure_telemetry_status_type;

/*******************************************************************************/

extern bool Mode_Autonom_Idle();
extern void Mode_Autonom_Init();
extern bool Mode_Autonom_Reset();
/*****************************************************************************/
#endif
