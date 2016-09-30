//#include "common.h"
//#include "fram.h"
//#include "ds32c35.h"
//#include "telemetry.h"
//
//#define FRAM_VARIABLE_CHECK(n, x) (fram_ ## n ## _buf.x != fram_ ## n .x) 
//#define FRAM_VARIABLE_WRITE(n, x) \
//	{                                                 \
// 		if(FRAM_Write(offsetof(fram_ ## n ## _type, x) + offsetof(fram_type, n), (byte *)&fram_ ## n ## _buf.x, sizeof(fram_ ## n ## _buf.x))) \
//		{ \
//			fram_ ## n .x = fram_ ## n ## _buf.x; \
//		} \
//	}
//
///*****************************************************************************/
//
//bool FRAM_Write(u16 adr, byte *data, u16 size)
//{
//	if(adr + size > DS32C35_FRAM_SIZE) return false;
//	return DS32C35_FRAM_Write(adr, data, size);
//}
//
//bool FRAM_Read(u16 adr, byte *data, u16 size)
//{
//	if(adr + size > DS32C35_FRAM_SIZE) return false;
//	return DS32C35_FRAM_Read(adr, data, size);
//}
//
///*****************************************************************************/
//fram_main_type fram_main;
//fram_main_type fram_main_buf;
//bool fram_main_change = false;
//
//void FRAM_Main_Init()
//{
//	FRAM_Read(offsetof(fram_type, main), (byte *)&fram_main, sizeof(fram_main_type));
//	fram_main_buf = fram_main;
//	fram_main_change = false;
//}
//
//void FRAM_Main_Idle()
//{
//	if(!fram_main_change) return;
//	if(FRAM_VARIABLE_CHECK(main, device_number)) FRAM_VARIABLE_WRITE(main, device_number)
//	else 
//	if(FRAM_VARIABLE_CHECK(main, device_type)) FRAM_VARIABLE_WRITE(main, device_type)
//	else 
//	if(FRAM_VARIABLE_CHECK(main, device_telemetry)) FRAM_VARIABLE_WRITE(main, device_telemetry)
//	else fram_main_change = false;
//}
//
//u16 FRAM_Main_Device_Number_Get()
//{
//	return fram_main_buf.device_number;	
//}
//
//void FRAM_Main_Device_Number_Set(u16 n)
//{	
//	fram_main_buf.device_number = n;
//	fram_main_change = true;
//}
//
//byte FRAM_Main_Device_Type_Get()
//{
//	return fram_main_buf.device_type;	
//}
//
//void FRAM_Main_Device_Type_Set(byte n)
//{	
//	fram_main_buf.device_type = n;
//	fram_main_change = true;
//}
//
//byte FRAM_Main_Device_Telemetry_Get()
//{
//	return fram_main_buf.device_telemetry;	
//}
//
//void FRAM_Main_Device_Telemetry_Set(byte n)
//{	
//	fram_main_buf.device_telemetry = n;
//	fram_main_change = true;
//}
//
///*****************************************************************************/
//fram_memory_type fram_memory;
//fram_memory_type fram_memory_buf;
//bool fram_memory_change = false;
//
//void FRAM_Memory_Init()
//{
//	FRAM_Read(offsetof(fram_type, memory), (byte *)&fram_memory, sizeof(fram_memory_type));
//	fram_memory_buf = fram_memory;
//	fram_memory_change = false;
//}
//
//void FRAM_Memory_Idle()
//{
//	if(!fram_memory_change) return;
//	if(FRAM_VARIABLE_CHECK(memory, current_adress)) FRAM_VARIABLE_WRITE(memory, current_adress)
//	else
//	if(FRAM_VARIABLE_CHECK(memory, current_vector_adress)) FRAM_VARIABLE_WRITE(memory, current_vector_adress)
//	else
//	if(FRAM_VARIABLE_CHECK(memory, current_session)) FRAM_VARIABLE_WRITE(memory, current_session)
//	else
//	if(FRAM_VARIABLE_CHECK(memory, start_adress)) FRAM_VARIABLE_WRITE(memory, start_adress)
//	else fram_memory_change = false;
//}
//
//u16 FRAM_Memory_Current_Session_Get()
//{
//	return fram_memory_buf.current_session;	
//}
//
//void FRAM_Memory_Current_Session_Set(u16 s)
//{	
//	fram_memory_buf.current_session = s;	
//	fram_memory_change = true;
//}
//
//i64 FRAM_Memory_Current_Adress_Get()
//{
//	return fram_memory_buf.current_adress;	
//}
//
//void FRAM_Memory_Current_Adress_Set(i64 a)
//{	
//	fram_memory_buf.current_adress = a;
//	fram_memory_change = true;
//}
//
//i64 FRAM_Memory_Current_Vector_Adress_Get()
//{
//	return fram_memory_buf.current_vector_adress;	
//}
//
//void FRAM_Memory_Current_Vector_Adress_Set(i64 a)
//{	
//	fram_memory_buf.current_vector_adress = a;
//	fram_memory_change = true;
//}
//
//i64 FRAM_Memory_Start_Adress_Get()
//{
//	return fram_memory_buf.start_adress;	
//}
//
//void FRAM_Memory_Start_Adress_Set(i64 a)
//{	
//	fram_memory_buf.start_adress = a;
//	fram_memory_change = true;
//}
//
///*****************************************************************************/
//fram_power_type fram_power;
//fram_power_type fram_power_buf;
//bool fram_power_change = false;
//
//void FRAM_Power_Init()
//{
//	FRAM_Read(offsetof(fram_type, power), (byte *)&fram_power, sizeof(fram_power_type));
//	fram_power_buf = fram_power;
//	fram_power_change = false;
//}
//
//void FRAM_Power_Idle()
//{
//	if(!fram_power_change) return;
//	if(FRAM_VARIABLE_CHECK(power, battery_setup_voltage)) FRAM_VARIABLE_WRITE(power, battery_setup_voltage) 
//	else
//	if(FRAM_VARIABLE_CHECK(power, battery_min_voltage)) FRAM_VARIABLE_WRITE(power, battery_min_voltage)
//	else
//	if(FRAM_VARIABLE_CHECK(power, battery_max_voltage)) FRAM_VARIABLE_WRITE(power, battery_max_voltage)
//	else
//	if(FRAM_VARIABLE_CHECK(power, battery_coeffs.k)) FRAM_VARIABLE_WRITE(power, battery_coeffs.k)
//	else
//	if(FRAM_VARIABLE_CHECK(power, battery_coeffs.b)) FRAM_VARIABLE_WRITE(power, battery_coeffs.b)
//	else
//	if(FRAM_VARIABLE_CHECK(power, line_setup_voltage)) FRAM_VARIABLE_WRITE(power, line_setup_voltage) 
//	else
//	if(FRAM_VARIABLE_CHECK(power, line_min_voltage)) FRAM_VARIABLE_WRITE(power, line_min_voltage)
//	else
//	if(FRAM_VARIABLE_CHECK(power, line_max_voltage)) FRAM_VARIABLE_WRITE(power, line_max_voltage)
//	else
//	if(FRAM_VARIABLE_CHECK(power, line_coeffs.k)) FRAM_VARIABLE_WRITE(power, line_coeffs.k)
//	else
//	if(FRAM_VARIABLE_CHECK(power, line_coeffs.b)) FRAM_VARIABLE_WRITE(power, line_coeffs.b)
//	else fram_power_change = false;
//}
//
//void FRAM_Power_Battery_Voltages_Get(i16 *setup, i16 *min, i16 *max)
//{
//	*setup = fram_power_buf.battery_setup_voltage;
//	*min = fram_power_buf.battery_min_voltage;
//	*max = fram_power_buf.battery_max_voltage;
//}
//
//void FRAM_Power_Battery_Voltages_Set(i16 setup, i16 min, i16 max)
//{
//	fram_power_buf.battery_setup_voltage = setup;
// 	fram_power_buf.battery_min_voltage = min;
//	fram_power_buf.battery_max_voltage = max;
//	fram_power_change = true;
//}
//
//void FRAM_Power_Battery_Coeffs_Get(float *k, float *b)
//{
//	*k = fram_power_buf.battery_coeffs.k;
//	*b = fram_power_buf.battery_coeffs.b;
//}
//
//void FRAM_Power_Battery_Coeffs_Set(float k, float b)
//{
//	fram_power_buf.battery_coeffs.k = k;
//	fram_power_buf.battery_coeffs.b = b;
//	fram_power_change = true;
//}
//
//void FRAM_Power_Line_Voltages_Get(i16 *setup, i16 *min, i16 *max)
//{
//	*setup = fram_power_buf.line_setup_voltage;
//	*min = fram_power_buf.line_min_voltage;
//	*max = fram_power_buf.line_max_voltage;
//}
//
//void FRAM_Power_Line_Voltages_Set(i16 setup, i16 min, i16 max)
//{
//	fram_power_buf.line_setup_voltage = setup;
//	fram_power_buf.line_min_voltage = min;
//	fram_power_buf.line_max_voltage = max;
//	fram_power_change = true;
//}
//
//void FRAM_Power_Line_Coeffs_Get(float *k, float *b)
//{
//	*k = fram_power_buf.line_coeffs.k;
//	*b = fram_power_buf.line_coeffs.b;
//}
//
//void FRAM_Power_Line_Coeffs_Set(float k, float b)
//{
//	fram_power_buf.line_coeffs.k = k;
//	fram_power_buf.line_coeffs.b = b;
//	fram_power_change = true;
//}
//
///*****************************************************************************/
//fram_sensors_type fram_sensors;
//fram_sensors_type fram_sensors_buf;
//bool fram_sensors_change = false;
//
//void FRAM_Sensors_Init()
//{
//	FRAM_Read(offsetof(fram_type, sensors), (byte *)&fram_sensors, sizeof(fram_sensors_type));
//	fram_sensors_buf = fram_sensors;
//	fram_sensors_change = false;
//}
//
//void FRAM_Sensors_Idle()
//{
//	if(!fram_sensors_change) return;
//	if(FRAM_VARIABLE_CHECK(sensors, ax_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, ax_coeffs.k)
//	else
//	if(FRAM_VARIABLE_CHECK(sensors, ax_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, ax_coeffs.b)
//	else
//	if(FRAM_VARIABLE_CHECK(sensors, ay_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, ay_coeffs.k)
//	else
//	if(FRAM_VARIABLE_CHECK(sensors, ay_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, ay_coeffs.b)
//	else
//	if(FRAM_VARIABLE_CHECK(sensors, az_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, az_coeffs.k)
//	else
//	if(FRAM_VARIABLE_CHECK(sensors, az_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, az_coeffs.b)
//	else fram_sensors_change = false;
//}
//
//void FRAM_Sensors_Ax_Coeffs_Get(float *k, float *b)
//{
//	*k = fram_sensors_buf.ax_coeffs.k;
//	*b = fram_sensors_buf.ax_coeffs.b;
//}
//
//void FRAM_Sensors_Ax_Coeffs_Set(float k, float b)
//{
//	fram_sensors_buf.ax_coeffs.k = k;
//	fram_sensors_buf.ax_coeffs.b = b;
//	fram_sensors_change = true;
//}
//
//void FRAM_Sensors_Ay_Coeffs_Get(float *k, float *b)
//{
//	*k = fram_sensors_buf.ay_coeffs.k;
//	*b = fram_sensors_buf.ay_coeffs.b;
//}
//
//void FRAM_Sensors_Ay_Coeffs_Set(float k, float b)
//{
//	fram_sensors_buf.ay_coeffs.k = k;
//	fram_sensors_buf.ay_coeffs.b = b;
//	fram_sensors_change = true;
//}
//
//void FRAM_Sensors_Az_Coeffs_Get(float *k, float *b)
//{
//	*k = fram_sensors_buf.az_coeffs.k;
//	*b = fram_sensors_buf.az_coeffs.b;
//}
//
//void FRAM_Sensors_Az_Coeffs_Set(float k, float b)
//{
//	fram_sensors_buf.az_coeffs.k = k;
//	fram_sensors_buf.az_coeffs.b = b;
//	fram_sensors_change = true;
//}
//     
///*****************************************************************************/
//byte fram_autonom_buffer[DS32C35_FRAM_SIZE - offsetof(fram_type, autonom)]; // последняя в структуре
//fram_autonom_type *fram_autonom = (fram_autonom_type *)fram_autonom_buffer;
//fram_autonom_validation_type fram_autonom_validation = FRAM_AUTONOM_VALIDATION_NONE;
//bool fram_autonom_change = false;
//
//fram_autonom_result_type fram_autonom_result[256]; //отработавшие команды
//
//byte Fram_Automon_CRC(byte *data, u16 size)
//{
//	byte crc = 0xFF;
//	byte i;
//
//	while(size --)
//   	{
//		crc ^= *data++;
//		for (i = 0; i < 8; i++) crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
//	};
//
//	return crc;
//}
//
//fram_autonom_validation_type FRAM_Autonom_Validation()
//{
//	byte crc_header = fram_autonom->header.crc_header;
//	fram_autonom->header.crc_header = 0;
//	if(Fram_Automon_CRC((byte *)fram_autonom, sizeof(fram_autonom->header)) != crc_header) 
//	{
//	        fram_autonom->header.crc_header = crc_header;
//		return FRAM_AUTONOM_VALIDATION_ERROR_CRC_HEADER;
//	}
//        fram_autonom->header.crc_header = crc_header;
//	if(Fram_Automon_CRC((byte *)fram_autonom + sizeof(fram_autonom->header), fram_autonom->header.size - sizeof(fram_autonom->header)) != fram_autonom->header.crc_data) return FRAM_AUTONOM_VALIDATION_ERROR_CRC_DATA;
//	if(fram_autonom->header.version != FRAM_AUTONOM_VERSION) return FRAM_AUTONOM_VALIDATION_ERROR_VERSION;
//	if(fram_autonom->control.special_period_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//	byte i, j;
//	for(i = 0; i < fram_autonom->control.session_count; i ++)
//	{
//		if(((fram_autonom_session_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->period_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		for(j = 0; j < ((fram_autonom_session_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->device_count; j ++)
//			if(((fram_autonom_session_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->device[j] >= fram_autonom->control.device_count) return FRAM_AUTONOM_VALIDATION_ERROR_STRUCT;
//		if(j == 0) return FRAM_AUTONOM_VALIDATION_ERROR_COUNT;
//	}
//	for(i = fram_autonom->control.session_count; i < fram_autonom->control.session_count + fram_autonom->control.device_count; i ++)
//	{
//		if(((fram_autonom_device_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->delay_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		if(((fram_autonom_device_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->period_min_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		for(j = 0; j < ((fram_autonom_device_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->command_count; j ++)
//			if(((fram_autonom_device_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->command[j] >= fram_autonom->control.command_count) return FRAM_AUTONOM_VALIDATION_ERROR_STRUCT;
//		if(j == 0) return FRAM_AUTONOM_VALIDATION_ERROR_COUNT;
//	}
//	for(i = fram_autonom->control.session_count + fram_autonom->control.device_count; i < fram_autonom->control.session_count + fram_autonom->control.device_count + fram_autonom->control.command_count; i ++)
//	{
//		if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->telemetry >= TELEMETRY_UNKNOWN) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->tx_freq_hz < 1000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->rx_freq_hz < 1000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->telemetry == TELEMETRY_MANCHESTER)
//		{
//			if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->tx_freq_hz > 50000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//			if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->rx_freq_hz > 50000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		}
//		if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->telemetry == TELEMETRY_USART)
//		{
//			if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->tx_freq_hz > 1000000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//			if(((fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[i]))->rx_freq_hz > 1000000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
//		}
//		
//	}
//	return FRAM_AUTONOM_VALIDATION_OK;
//}
//
////-------------------------------------------------------------
//
//void FRAM_Autonom_Write_Begin()
//{
//	FRAM_Autonom_Result_Reset();
//       	fram_autonom_validation = FRAM_AUTONOM_VALIDATION_NONE;
//}
//
//void FRAM_Autonom_Write_End()
//{
//	fram_autonom_validation = FRAM_Autonom_Validation(/*(byte *)fram_autonom*/);
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) fram_autonom_change = true; else fram_autonom_change = false;
//}
//
//void FRAM_Autonom_Write_Block(u16 offset, u16 size, byte *data)
//{
//	if((offsetof(fram_type, autonom) + offset + size) < sizeof(fram_autonom_buffer)) memmove((char *)fram_autonom + offset, (char *)data, size);
//}
//
//void FRAM_Autonom_Read_Block(u16 offset, u16 size, byte *data)
//{
//	if((offsetof(fram_type, autonom) + offset + size) < sizeof(fram_autonom_buffer)) memmove((char *)data, (char *)fram_autonom + offset, size);
//}
//
////-------------------------------------------------------------
//
//void FRAM_Autonom_Reset()
//{
//	FRAM_Autonom_Result_Reset();
//}
//
//void FRAM_Autonom_Init()
//{
//	FRAM_Read(offsetof(fram_type, autonom), (byte *)fram_autonom, sizeof(fram_autonom_buffer));		
//	fram_autonom_validation = FRAM_Autonom_Validation(/*(byte *)fram_autonom*/);
//	fram_autonom_change = false;
//	FRAM_Autonom_Reset();
//}
//
//bool FRAM_Autonom_Idle()
//{
//	if(!fram_autonom_change) return false;
//	if(fram_autonom_validation != FRAM_AUTONOM_VALIDATION_OK) return false; 
//	FRAM_Write(offsetof(fram_type, autonom), (byte *)fram_autonom, fram_autonom->header.size);
//	fram_autonom_change = false;
//	return true;
//}
//
////-----------------------------------------------
///*inline*/ u16 FRAM_Autonom_Size_Get()
//{
//	return sizeof(fram_autonom_buffer);
//}
//
///*inline*/ byte FRAM_Autonom_Version_Get()
//{
//	return FRAM_AUTONOM_VERSION;
//}
//
///*inline*/ fram_autonom_validation_type FRAM_Autonom_Validation_Get()
//{
//	return fram_autonom_validation;
//}
//
//u32 FRAM_Autonom_Special_Period_MS_Get()
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) return fram_autonom->control.special_period_ms;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Count_Get()
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) return fram_autonom->control.session_count;
//	return 0;
//}
////-----------------------------------------------
//inline fram_autonom_session_type *FRAM_Autonom_Session_Pointer_Get(byte session)
//{
//	return (fram_autonom_session_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[session]);
//}
//
//inline fram_autonom_device_type *FRAM_Autonom_Device_Pointer_Get(byte device)
//{
//	return (fram_autonom_device_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[fram_autonom->control.session_count + device]);
//}
//
//inline fram_autonom_command_type *FRAM_Autonom_Command_Pointer_Get(byte command)
//{
//	return (fram_autonom_command_type *)((u32)fram_autonom + (u32)fram_autonom->control.pointer[fram_autonom->control.session_count + fram_autonom->control.device_count + command]);
//}
//
////-----------------------------------------------
//RTC_type FRAM_Autonom_Session_Start_Get(byte session)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//        return FRAM_Autonom_Session_Pointer_Get(session)->start;
//	RTC_type rtc;
//	rtc.date = 0;
//	rtc.time = 0;
//	return rtc;
//}
//
//RTC_type FRAM_Autonom_Session_Stop_Get(byte session)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//        return FRAM_Autonom_Session_Pointer_Get(session)->stop;
//	RTC_type rtc;
//	rtc.date = 0;
//	rtc.time = 0;
//	return rtc;
//}
//
//u32 FRAM_Autonom_Session_Period_MS_Get(byte session)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Session_Pointer_Get(session)->period_ms;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Device_Count_Get(byte session)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Session_Pointer_Get(session)->device_count;
//	return 0;
//}
//
////-----------------------------------------------
//u32 FRAM_Autonom_Session_Device_Delay_MS_Get(byte session, byte device)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->delay_ms;
//	return 0;
//}
//
//u32 FRAM_Autonom_Session_Device_Period_Min_MS_Get(byte session, byte device)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->period_min_ms;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Device_Command_Count_Get(byte session, byte device)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command_count;
//	return 0;
//}
//
////-----------------------------------------------
//byte FRAM_Autonom_Session_Device_Command_Telemetry_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->telemetry;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Device_Command_Mode_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->mode;
//	return 0;
//}
//
//u16 FRAM_Autonom_Session_Device_Command_Offset_MS_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->offset_ms;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Device_Command_TX_Flags_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_flags;
//	return 0;
//}
//
//u32 FRAM_Autonom_Session_Device_Command_TX_Freq_HZ_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_freq_hz;
//	return 0;
//}
//
//u16 FRAM_Autonom_Session_Device_Command_TX_Size_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_size;
//	return 0;
//}
//
//byte FRAM_Autonom_Session_Device_Command_RX_Flags_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_flags;
//	return 0;
//}
//
//u32 FRAM_Autonom_Session_Device_Command_RX_Freq_HZ_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_freq_hz;
//	return 0;
//}
//
//u32 FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_timeout_mks;
//	return 0;
//}
//
//u16 FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_pause_mks;
//	return 0;
//}
//
//u16 FRAM_Autonom_Session_Device_Command_RX_Size_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_size;
//	return 0;
//}
//
//u16 *FRAM_Autonom_Session_Device_Command_TX_Data_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	{
//		return (u16*)FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_data;
//	};
//
//	return 0;
//}
////-------------------------------------------------------------
//
//void FRAM_Autonom_Result_Reset()
//{
//	u16 i;
//	for(i = 0; i < sizeof(fram_autonom_result) / sizeof(fram_autonom_result_type); i ++) fram_autonom_result[i] = FRAM_AUTONOM_RESULT_NONE;
//}
//
//void FRAM_Autonom_Result_Set(byte session, byte device, byte command, bool result)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	{
//		if(result) fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] = FRAM_AUTONOM_RESULT_READY;
//		else fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] = FRAM_AUTONOM_RESULT_ERROR;
//	}
//}
//
//bool FRAM_Autonom_Result_Ready_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	{
//		if(fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] == FRAM_AUTONOM_RESULT_READY) return true;
//	}
//	return false;
//}
//
//bool FRAM_Autonom_Result_Error_Get(byte session, byte device, byte command)
//{
//	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
//	{
//		if(fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] == FRAM_AUTONOM_RESULT_ERROR) return true;
//	}
//	return false;
//}
//
///*****************************************************************************/
//
//void FRAM_Init()
//{
//	FRAM_Main_Init();
//	FRAM_Memory_Init();
//	FRAM_Power_Init();
//	FRAM_Sensors_Init();
//	FRAM_Autonom_Init();
//}
//
//void FRAM_Idle()
//{
//	FRAM_Memory_Idle();
//	FRAM_Power_Idle();
//	FRAM_Sensors_Idle();
//	FRAM_Autonom_Idle();	
//	FRAM_Main_Idle();
//}
