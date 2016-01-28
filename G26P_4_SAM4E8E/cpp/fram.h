#ifndef MEMORY_AT91SAM7X256_FRAM_H
#define MEMORY_AT91SAM7X256_FRAM_H

#include "rtc.h"

/*************************/
typedef __packed struct   
{
	float k;
	float b;
} fram_linear_coeffs_type;
/*************************/
typedef __packed enum   
{
	FRAM_MAIN_DEVICE_TYPE_MODULE_MEMORY = 0,
	FRAM_MAIN_DEVICE_TYPE_MODULE_AUTONOM,
} fram_main_device_type;

enum
{
	FRAM_MAIN_DEVICE_TELEMETRY_MANCHESTER_BIT = 0,
	FRAM_MAIN_DEVICE_TELEMETRY_USART_BIT,
};

typedef __packed struct
{
	u16 device_number;
	byte device_type;	// ��� (0 = ������, 1 =���������)
	byte device_telemetry;	// ����� ������  (0bit = manch, 1bit = USART)
} fram_main_type;
/*************************/
typedef __packed struct
{
	u16 current_session;     // �� ��� ��������� ������ �������� � ������
	i64 current_adress;
	i64 current_vector_adress;
	i64 start_adress;
} fram_memory_type;
/*************************/
typedef __packed struct   // �� ��� ������� �������
{
	short battery_setup_voltage;	//0.1V
	short battery_min_voltage;	//0.1V
	short battery_max_voltage;	//0.1V
	short line_setup_voltage;	//0.1V
	short line_min_voltage;	//0.1V
	short line_max_voltage;	//0.1V
	fram_linear_coeffs_type battery_coeffs;
	fram_linear_coeffs_type line_coeffs;
} fram_power_type;
/*************************/
typedef __packed struct     // �� ��� ������� �������
{
	fram_linear_coeffs_type ax_coeffs;
	fram_linear_coeffs_type ay_coeffs;
	fram_linear_coeffs_type az_coeffs;
} fram_sensors_type;
/*************************/
typedef __packed enum   
{
        FRAM_AUTONOM_RESULT_NONE = 0, // �� ������������ �����
        FRAM_AUTONOM_RESULT_ERROR,
        FRAM_AUTONOM_RESULT_READY,
} fram_autonom_result_type;

typedef __packed enum   
{
        FRAM_AUTONOM_VALIDATION_NONE = 0, // �� �������������
        FRAM_AUTONOM_VALIDATION_OK,
        FRAM_AUTONOM_VALIDATION_ERROR_CRC_HEADER,
        FRAM_AUTONOM_VALIDATION_ERROR_CRC_DATA,
        FRAM_AUTONOM_VALIDATION_ERROR_VERSION,
        FRAM_AUTONOM_VALIDATION_ERROR_STRUCT,
        FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR,
        FRAM_AUTONOM_VALIDATION_ERROR_COUNT,
} fram_autonom_validation_type;

#define FRAM_AUTONOM_VERSION	0x1

typedef __packed struct         
{
	byte telemetry;
	byte mode;   
	u16 offset_ms; // �������� ������������ ���������� �������
	byte tx_flags;
	u32 tx_freq_hz;
	u16 tx_size;
	byte rx_flags;
	u32 rx_freq_hz;
	u32 rx_timeout_mks; 
	u16 rx_pause_mks;
	u16 rx_size;
	u16 tx_data[]; //������� � ������
} fram_autonom_command_type;

typedef __packed struct     
{
	u32 delay_ms; // �������� � ������� ������ �������
	u32 period_min_ms; // ����������� ������ ������
	byte command_count; // ������ � �������
	byte command[]; // ������ ������ � �������
} fram_autonom_device_type;

typedef __packed struct     
{
	RTC_type start;
	RTC_type stop;
	u32 period_ms;
	byte device_count; // �������� � ������
	byte device[]; // ������ �������� � ������
} fram_autonom_session_type;

typedef __packed struct     
{
	u32 special_period_ms;
	byte session_count;
	byte device_count;
	byte command_count;
	u16 pointer[];
} fram_autonom_control_type;

typedef __packed struct     
{
	byte	version;
	u16		size;			// ������ ������ ������ � header
	byte	crc_data;		// = crc(������[size - header])
	byte	crc_header; 	// = crc(size, version, crc_data)
} fram_autonom_header_type;

typedef __packed struct     // ����������� ������ ���������
{
	fram_autonom_header_type header;
	fram_autonom_control_type control;
} fram_autonom_type;
/*************************************************/
typedef __packed struct     
{
	fram_main_type main;
	fram_memory_type memory;
	fram_power_type power;
	fram_sensors_type sensors;
	fram_autonom_type autonom;
} fram_type;              
/*************************************************/
extern void 		FRAM_Init();
extern void 		FRAM_Idle();

extern u16 			FRAM_Main_Device_Number_Get();
extern void 		FRAM_Main_Device_Number_Set(u16 n);
extern byte 		FRAM_Main_Device_Type_Get();
extern void 		FRAM_Main_Device_Type_Set(byte t);
extern byte 		FRAM_Main_Device_Telemetry_Get();
extern void 		FRAM_Main_Device_Telemetry_Set(byte t);

extern u16 			FRAM_Memory_Current_Session_Get();      	// �� ������� ������
extern void 		FRAM_Memory_Current_Session_Set(u16 s);	
extern i64 			FRAM_Memory_Current_Adress_Get();             // ������� �����
extern void 		FRAM_Memory_Current_Adress_Set(i64 a);
extern i64 			FRAM_Memory_Current_Vector_Adress_Get();	// ����� ���������� ����������� �������
extern void 		FRAM_Memory_Current_Vector_Adress_Set(i64 a);
extern i64 			FRAM_Memory_Start_Adress_Get();               // ����� ������ ������
extern void			FRAM_Memory_Start_Adress_Set(i64 a);

extern void 		FRAM_Power_Battery_Voltages_Get(short *setup, short *min, short *max);
extern void 		FRAM_Power_Battery_Voltages_Set(short setup, short min, short max);
extern void 		FRAM_Power_Battery_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Power_Battery_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Power_Line_Voltages_Get(short *setup, short *min, short *max);
extern void 		FRAM_Power_Line_Voltages_Set(short setup, short min, short max);
extern void 		FRAM_Power_Line_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Power_Line_Coeffs_Set(float *k, float *b);

extern void 		FRAM_Sensors_Ax_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Ax_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Sensors_Ay_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Ay_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Sensors_Az_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Az_Coeffs_Set(float *k, float *b);

extern u16 			FRAM_Autonom_Size_Get();
extern fram_autonom_validation_type FRAM_Autonom_Validation_Get();
extern byte 		FRAM_Autonom_Version_Get();
extern void			FRAM_Autonom_Write_Begin();
extern void			FRAM_Autonom_Write_End();
extern void			FRAM_Autonom_Write_Block(u16 offset, u16 size, byte *data);
extern void			FRAM_Autonom_Read_Block(u16 offset, u16 size, byte *data);

extern u32 			FRAM_Autonom_Special_Period_MS_Get();

extern byte 		FRAM_Autonom_Session_Count_Get();
extern RTC_type 	FRAM_Autonom_Session_Start_Get(byte session);
extern RTC_type 	FRAM_Autonom_Session_Stop_Get(byte session);
extern u32 			FRAM_Autonom_Session_Period_MS_Get(byte session);
extern byte 		FRAM_Autonom_Session_Device_Count_Get(byte session);

extern u32 	FRAM_Autonom_Session_Device_Delay_MS_Get(byte session, byte device);
extern u32 	FRAM_Autonom_Session_Device_Period_Min_MS_Get(byte session, byte device);
extern byte	FRAM_Autonom_Session_Device_Command_Count_Get(byte session, byte device);

extern byte FRAM_Autonom_Session_Device_Command_Telemetry_Get(byte session, byte device, byte command);
extern byte FRAM_Autonom_Session_Device_Command_Mode_Get(byte session, byte device, byte command);
extern u16 	FRAM_Autonom_Session_Device_Command_Offset_MS_Get(byte session, byte device, byte command);
extern byte FRAM_Autonom_Session_Device_Command_TX_Flags_Get(byte session, byte device, byte command);
extern u32 	FRAM_Autonom_Session_Device_Command_TX_Freq_HZ_Get(byte session, byte device, byte command);
extern u16 	FRAM_Autonom_Session_Device_Command_TX_Size_Get(byte session, byte device, byte command);
extern byte FRAM_Autonom_Session_Device_Command_RX_Flags_Get(byte session, byte device, byte command);
extern u32 	FRAM_Autonom_Session_Device_Command_RX_Freq_HZ_Get(byte session, byte device, byte command);
extern u32 	FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(byte session, byte device, byte command);
extern u16 	FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(byte session, byte device, byte command);
extern u16 	FRAM_Autonom_Session_Device_Command_RX_Size_Get(byte session, byte device, byte command);
//extern u16*	FRAM_Autonom_Session_Device_Command_TX_Data_Get(byte session, byte device, byte command);

extern void FRAM_Autonom_Reset();
extern void FRAM_Autonom_Result_Reset();
extern void FRAM_Autonom_Result_Set(byte session, byte device, byte command, bool result);
extern bool	FRAM_Autonom_Result_Ready_Get(byte session, byte device, byte command);
extern bool	FRAM_Autonom_Result_Error_Get(byte session, byte device, byte command);


/*****************************************************************************/

#endif
