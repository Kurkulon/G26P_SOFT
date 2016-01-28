#ifndef MEMORY_AT91SAM7X256_MODE_ONLINE_H
#define MEMORY_AT91SAM7X256_MODE_ONLINE_H

/*******************************************************************************/
#define MODE_ONLINE_SPECIAL_PERIOD_MS			250

#define MODE_ONLINE_SPECIAL_IDENTIFICATION_COMMAND	0x3A00
typedef struct __attribute__ ((packed))
{
	unsigned short command;
	unsigned short number;
	unsigned short version;
} mode_online_special_identification_type;

#define MODE_ONLINE_SPECIAL_OPTIONS_COMMAND	0x3A10
typedef struct __attribute__ ((packed))
{
	unsigned short command;
} mode_online_special_options_type;

#define MODE_ONLINE_SPECIAL_MEASURE_COMMAND	0x3A20
typedef struct __attribute__ ((packed))
{
	unsigned short command;
	short battery_voltage;	
	short line_voltage;	
	short temperature_in;
	short ax;
	short ay;
	short az;
} mode_online_special_measure_type;
/*******************************************************************************/
#define MODE_ONLINE_RDC_SEND_MAIN_PERIOD_MS	20	// Период отсылки данных RDC (основное сообщение) на комп
#define MODE_ONLINE_RDC_SEND_STATUS_PERIOD_S	1
/*******************************************************************************/


typedef enum __attribute__ ((packed)) 
{
        MODE_ONLINE_STATUS_NONE,
        MODE_ONLINE_STATUS_OK,
        MODE_ONLINE_STATUS_ERROR_PARAMETR,
        MODE_ONLINE_STATUS_ERROR_STRUCT,
        MODE_ONLINE_STATUS_ERROR_INDEX,
} mode_online_control_status_type;

/*******************************************************************************/
typedef enum __attribute__ ((packed)) 
{
	MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT = 0,
	MODE_ONLINE_MEASURE_TELEMETRY_STATUS_MAKE,
	MODE_ONLINE_MEASURE_TELEMETRY_STATUS_TX,
	MODE_ONLINE_MEASURE_TELEMETRY_STATUS_RX,
	MODE_ONLINE_MEASURE_TELEMETRY_STATUS_SEND,
} mode_online_measure_telemetry_status_type;

/*******************************************************************************/
#define MODE_ONLINE_CONTROL_BUFFER_SIZE		4096

typedef struct __attribute__ ((packed))       
{
	unsigned char telemetry;
	unsigned char mode;  
	unsigned short offset_ms; // смещение относительно предыдущей команды
	unsigned char tx_flags;
	unsigned int tx_freq_hz;
	unsigned short tx_size;
	unsigned char rx_flags;
	unsigned int rx_freq_hz;
	unsigned int rx_timeout_mks; 
	unsigned short rx_pause_mks;
	unsigned short rx_size;
	unsigned short tx_data[]; //команды в прибор
} mode_online_control_command_type;

typedef struct __attribute__ ((packed))   
{
	unsigned int delay_ms; // задержка с момента подачи питания
	unsigned int period_min_ms; // минимальный период опроса
	unsigned char command_count; // команд в приборе
	unsigned short command_index; // номер первой команды для поиска в pointer[i]; i = command_index + device_count + n;
} mode_online_control_device_type;

typedef struct __attribute__ ((packed))   
{
	unsigned short size;
	unsigned int period_ms;
	unsigned char device_count;
	unsigned short command_count;
	unsigned short pointer[];
} mode_online_control_type;

/*******************************************************************************/
extern void Mode_Online_Vector_Enable();
extern void Mode_Online_Vector_Disable();

extern void Mode_Online_Main_Enable();
extern void Mode_Online_Main_Disable();

extern unsigned int Mode_Online_RDC_Time_Reset();
extern unsigned int Mode_Online_RDC_Time_Reset();
extern void Mode_Online_RDC_Imitation_Enable(int depth, int speed);
extern void Mode_Online_RDC_Imitation_Disable();
extern void Mode_Online_RDC_Depth_Set(int depth);
extern int Mode_Online_RDC_Depth_Get();
extern void Mode_Online_RDC_Messaging_Enable();
extern void Mode_Online_RDC_Messaging_Disable();



extern bool Mode_Online_Control_Begin();
extern bool Mode_Online_Control_Cancel();
extern bool Mode_Online_Control_End();
extern unsigned int Mode_Online_Control_Period_MS_Get();
extern unsigned char Mode_Online_Control_Device_Index_Get();
extern mode_online_control_status_type Mode_Online_Control_Period_MS_Set(unsigned int period_ms); 
extern mode_online_control_status_type Mode_Online_Control_Device_Add(unsigned int delay_ms, unsigned int period_min_ms, unsigned char command_count);
extern mode_online_control_status_type Mode_Online_Control_Device_Command_Add(unsigned char device_index, unsigned char command_index, 
	unsigned char telemetry,
	unsigned char mode,  
	unsigned short offset_ms,
	unsigned char tx_flags,
	unsigned int tx_freq_hz,
	unsigned short tx_size,
	unsigned char rx_flags,
	unsigned int rx_freq_hz,
	unsigned int rx_timeout_mks, 
	unsigned short rx_pause_mks,
	unsigned short rx_size,
	unsigned short *tx_data);
extern mode_online_control_status_type Mode_Online_Control_Device_Command_Remove(unsigned char device_index, unsigned char command_index);

extern bool Mode_Online_Idle();
extern void Mode_Online_Init();
extern bool Mode_Online_Reset();
/*****************************************************************************/
#endif
