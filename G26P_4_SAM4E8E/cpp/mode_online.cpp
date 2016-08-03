//#include "common.h"
//#include "main.h"
//#include "fram.h"
//#include "flash.h"
////#include "manchester.h"
////#include "usart1.h"
//#include "rtc.h"
//#include "power.h"
//#include "telemetry.h"
//#include "mode_online.h"
//#include "vector.h"
//#include "trap.h"
//#include "sensors.h"
//
//#pragma diag_suppress 546,550,177
//
///*********************************************************************/
//bool Mode_Online_Special_Busy();
//bool Mode_Online_Measure_Busy();
//bool Mode_Online_Special_Reset();
//bool Mode_Online_Measure_Reset();	
//
//bool mode_online_main_enable_need;    
//bool mode_online_main_enable;    
//
//bool Mode_Online_Main_Reset()
//{
//	mode_online_main_enable_need = false;
//	if(mode_online_main_enable) return false;
//	else return true;
//}
//
//void Mode_Online_Main_Init()
//{
//	Mode_Online_Main_Reset();
//}
//
//void Mode_Online_Main_Idle()
//{       
//	if(mode_online_main_enable != mode_online_main_enable_need)
//	{
//		if((!Mode_Online_Special_Busy()) && (!Mode_Online_Measure_Busy()))
//		{
//
//			mode_online_main_enable = mode_online_main_enable_need;
//			Mode_Online_Special_Reset();	// юсэєы ■ тёх яю ╠╧
//			Mode_Online_Measure_Reset();	// юсэєы ■ яю яЁшсюЁрь
//		}		
//	}
//}
// 
//inline bool Mode_Online_Main_Enable_Get()
//{
//	return mode_online_main_enable;
//}
//
///*inline*/ void Mode_Online_Main_Enable()
//{
//	mode_online_main_enable_need = true;
//}
//
///*inline*/ void Mode_Online_Main_Disable()
//{
//	mode_online_main_enable_need = false;
//}
//
///*********************************************************************/
//unsigned short mode_online_rdc_time_msec;
//unsigned int mode_online_rdc_time_ms;
//unsigned short mode_online_rdc_time_send_main_msec;
//unsigned short mode_online_rdc_time_send_status_sec;
//int mode_online_rdc_speed;
//long long mode_online_rdc_depth_x36000 = 0;
//bool mode_online_rdc_messaging = true;
//bool mode_online_rdc_imitation = false;
//
//bool Mode_Online_RDC_Reset()
//{
//	mode_online_rdc_time_ms = 0;
//	mode_online_rdc_depth_x36000 = 0;
//	mode_online_rdc_speed = 0;
//	mode_online_rdc_time_msec = RTC_Get_Millisecond();
//	mode_online_rdc_time_send_main_msec = 0;
//	mode_online_rdc_time_send_status_sec = 0;
//	mode_online_rdc_messaging = true;
//	mode_online_rdc_imitation = false;
//	return true;
//}
//
//inline unsigned int Mode_Online_RDC_Time_Ms_Get()
//{
//	return mode_online_rdc_time_ms;
//}
//
///*inline*/ void Mode_Online_RDC_Time_Reset()
//{
//	mode_online_rdc_time_msec = RTC_Get_Millisecond();
//	mode_online_rdc_time_ms = 0;
//}
//
///*inline*/ void Mode_Online_RDC_Imitation_Enable(int depth, int speed)
//{
//	mode_online_rdc_depth_x36000 = (long long)depth * 36000;
//	mode_online_rdc_speed = speed;
//	mode_online_rdc_imitation = true;
//}
//
///*inline*/ void Mode_Online_RDC_Imitation_Disable()
//{
//	mode_online_rdc_speed = 0;
//	mode_online_rdc_imitation = false;
//}
//
///*inline*/ void Mode_Online_RDC_Messaging_Enable()
//{
//	mode_online_rdc_messaging = true;
//}
//
///*inline*/ void Mode_Online_RDC_Messaging_Disable()
//{
//	mode_online_rdc_messaging = false;
//}
//
///*inline*/ int Mode_Online_RDC_Depth_Get()
//{
//	return (int)(mode_online_rdc_depth_x36000 / 36000);
//}
//
///*inline*/ void Mode_Online_RDC_Depth_Set(int depth)
//{
//	mode_online_rdc_depth_x36000 = (long long)depth * 36000;
//}
//
//inline int Mode_Online_RDC_Speed_Get()
//{
//	return mode_online_rdc_speed;
//}
//
//void Mode_Online_RDC_Init()
//{
//	Mode_Online_RDC_Reset();
//}
//
//void Mode_Online_RDC_Idle()
//{       
//	RTC_type rtc = RTC_Get();
//	unsigned short dmsec = ((unsigned short)((rtc.msec + 1000) - mode_online_rdc_time_msec) % 1000);
//	mode_online_rdc_time_ms += dmsec;
//	mode_online_rdc_time_msec = rtc.msec;
//	if(mode_online_rdc_imitation) mode_online_rdc_depth_x36000 += mode_online_rdc_speed * dmsec;
//	if(mode_online_rdc_messaging)
//	{
//		if(((unsigned short)((rtc.msec + 1000) - mode_online_rdc_time_send_main_msec) % 1000) >= MODE_ONLINE_RDC_SEND_MAIN_PERIOD_MS)
//		{
//			mode_online_rdc_time_send_main_msec = rtc.msec;
//			TRAP_RDC_SendMain(Mode_Online_RDC_Time_Ms_Get(), Mode_Online_RDC_Depth_Get(), Mode_Online_RDC_Speed_Get());
//		}
//	}
//	if(((unsigned char)((rtc.sec + 60) - mode_online_rdc_time_send_status_sec) % 60) >= MODE_ONLINE_RDC_SEND_STATUS_PERIOD_S)
//	{
//		mode_online_rdc_time_send_status_sec = rtc.sec;
//		TRAP_RDC_SendStatus(mode_online_rdc_messaging, mode_online_rdc_imitation);
//	}
//}
// 
///*********************************************************************/
//unsigned int mode_online_special_count = 0;
//unsigned int mode_online_special_time = 0;         
//
//bool Mode_Online_Special_Reset()
//{
//	mode_online_special_count = 0;
//	mode_online_special_time = RTC_Get_Millisecond();
//	return true;
//}
//
//void Mode_Online_Special_Init()
//{
//	Mode_Online_Special_Reset();
//}
//
//bool Mode_Online_Special_Busy()  // чруыє°ър
//{  
//	return false;	
//}
// 
//bool Mode_Online_Special_Idle()
//{  
//	if((!Mode_Online_Special_Busy()) && (!Mode_Online_Main_Enable_Get()))
//	{
//		return false;
//	}
//	unsigned short msec = RTC_Get_Millisecond();
//	if(((unsigned short)((msec + 1000) - mode_online_special_time) % 1000) >= MODE_ONLINE_SPECIAL_PERIOD_MS)
//	{
//		mode_online_special_time = msec;
//		unsigned int time = Mode_Online_RDC_Time_Ms_Get();
//		int depth = Mode_Online_RDC_Depth_Get();
//		int speed = Mode_Online_RDC_Depth_Get();
//		if(mode_online_special_count == 0)
//		{
//			mode_online_special_identification_type mode_online_special_identification;
//			mode_online_special_identification.command = MODE_ONLINE_SPECIAL_IDENTIFICATION_COMMAND;
//			mode_online_special_identification.number = FRAM_Main_Device_Number_Get();
//			mode_online_special_identification.version = VERSION;	
//			if(TRAP_VECTOR_SendVector(MODE_ONLINE_SPECIAL_IDENTIFICATION_COMMAND, time, depth, speed, 0, sizeof(mode_online_special_identification_type), (unsigned char *)(&mode_online_special_identification))) mode_online_special_count ++;
//		}
//		else
//		if(mode_online_special_count == 1)
//		{
//			mode_online_special_options_type mode_online_special_options;
//			mode_online_special_options.command = MODE_ONLINE_SPECIAL_OPTIONS_COMMAND;
//			if(TRAP_VECTOR_SendVector(MODE_ONLINE_SPECIAL_OPTIONS_COMMAND, time, depth, speed, 0, sizeof(mode_online_special_options_type), (unsigned char *)(&mode_online_special_options))) mode_online_special_count ++;
//		}
//		else
//		{
//			mode_online_special_measure_type mode_online_special_measure;
//			mode_online_special_measure.command = MODE_ONLINE_SPECIAL_MEASURE_COMMAND;
//			mode_online_special_measure.battery_voltage = Power_Battery_Voltage_Get();
//			mode_online_special_measure.line_voltage = Power_Line_Voltage_Get();	
//        		mode_online_special_measure.temperature_in = RTC_Get_Temperature();
//			mode_online_special_measure.ax = Sensors_Ax_Get();
//			mode_online_special_measure.ay = Sensors_Ay_Get();
//			mode_online_special_measure.az = Sensors_Az_Get();
//			if(TRAP_VECTOR_SendVector(MODE_ONLINE_SPECIAL_MEASURE_COMMAND, time, depth, speed, 0, sizeof(mode_online_special_measure_type), (unsigned char *)(&mode_online_special_measure))) mode_online_special_count ++;
//		}
//		return true;
//	}
//	return false;
//}
//
///*********************************************************************/
//unsigned char mode_online_control_buffer[MODE_ONLINE_CONTROL_BUFFER_SIZE];
//mode_online_control_type *mode_online_control = (mode_online_control_type *)mode_online_control_buffer;
//
//bool Mode_Online_Control_Reset() 
//{
//	mode_online_control->size = (unsigned short)offsetof(mode_online_control_type, pointer[0]);
//	mode_online_control->period_ms = 0;
//	mode_online_control->device_count = 0;
//	mode_online_control->command_count = 0; 
//	return true;
//}
//
//void Mode_Online_Control_Init() 
//{
//	Mode_Online_Control_Reset();	
//}
//
//bool Mode_Online_Control_Idle()
//{       
//	return true;
//}
////-----------------------------------------------
//bool Mode_Online_Control_Begin()
//{
//	Mode_Online_Control_Reset();
//	return true;
//}
//
//bool Mode_Online_Control_Cancel()
//{
//	Mode_Online_Control_Reset();
//	return true;
//}
//
//bool Mode_Online_Control_End()
//{
//	return true;
//}
//
////-----------------------------------------------
//
//inline mode_online_control_device_type *Mode_Online_Control_Device_Pointer_Get(unsigned char device)
//{
//	return (mode_online_control_device_type *)((unsigned int)mode_online_control + (unsigned int)mode_online_control->pointer[device]);
//}
//
//inline mode_online_control_command_type *Mode_Online_Control_Command_Pointer_Get(unsigned char device, unsigned char command)
//{
//	unsigned short index = mode_online_control->device_count + Mode_Online_Control_Device_Pointer_Get(device)->command_index + command;
//	if(mode_online_control->pointer[index]) return (mode_online_control_command_type *)((unsigned int)mode_online_control + (unsigned int)mode_online_control->pointer[index]);
//	else return 0;
//}
//
////-----------------------------------------------
//unsigned char Mode_Online_Control_Device_Index_Get() 
//{
//	if(mode_online_control->device_count == 0) return 0;
//	return mode_online_control->device_count - 1;
//}
//
//unsigned int Mode_Online_Control_Period_MS_Get()
//{
//	return mode_online_control->period_ms;
//}
//
//
//mode_online_control_status_type Mode_Online_Control_Period_MS_Set(unsigned int period_ms) 
//{
//	if(period_ms >= 1000 * 60 * 60 * 24) 
//	{
//		return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	}
//	mode_online_control->period_ms = period_ms;
//	return MODE_ONLINE_STATUS_OK;
//}
//
//mode_online_control_status_type Mode_Online_Control_Device_Add(unsigned int delay_ms, unsigned int period_min_ms, unsigned char command_count) 
//{
//	if(mode_online_control->size + sizeof(mode_online_control_device_type) + sizeof(unsigned short) * (command_count + 1) > sizeof(mode_online_control_buffer)) return MODE_ONLINE_STATUS_ERROR_STRUCT;
//	if(delay_ms >= 1000 * 60 * 60 * 24) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	if(period_min_ms >= 1000 * 60 * 60 * 24) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	unsigned short i;
//	// увеличиваем все указатели на sizeof(unsigned short) * (command_count + 1), если он не 0-ой
//	for(i = 0; i < mode_online_control->device_count + mode_online_control->command_count; i ++) if(mode_online_control->pointer[i]) mode_online_control->pointer[i] += sizeof(unsigned short) * (command_count + 1); 
//       	// сдвигаем всё вправо, часть на sizeof(unsigned short) * (command_count + 1), 
//	for(i = mode_online_control->size; i > (unsigned short)offsetof(mode_online_control_type, pointer[mode_online_control->device_count + mode_online_control->command_count]); i--) mode_online_control_buffer[i - 1 + sizeof(unsigned short) * (command_count + 1)] = mode_online_control_buffer[i - 1];
//	// и часть на sizeof(unsigned short)
//	for(i = (unsigned short)offsetof(mode_online_control_type, pointer[mode_online_control->device_count + mode_online_control->command_count]); i > (unsigned short)offsetof(mode_online_control_type, pointer[mode_online_control->device_count]); i--) mode_online_control_buffer[i - 1 + sizeof(unsigned short)] = mode_online_control_buffer[i - 1];
//	// заполняем указатель на прибор
//	mode_online_control->pointer[mode_online_control->device_count] = mode_online_control->size + sizeof(unsigned short) * (1 + command_count);
//	//  добавляем прибор 
//	mode_online_control_device_type *device = ((mode_online_control_device_type *)((unsigned int)mode_online_control + (unsigned int)mode_online_control->pointer[mode_online_control->device_count]));
//	device->delay_ms = delay_ms;
//	device->period_min_ms = period_min_ms;
//	device->command_count = command_count;
//	device->command_index = mode_online_control->command_count;
//	mode_online_control->device_count ++;
//	mode_online_control->size += sizeof(mode_online_control_device_type) + sizeof(unsigned short);
//	// заполняем указатель на команды
//	for(i = 0; i < command_count; i++) mode_online_control->pointer[mode_online_control->device_count + mode_online_control->command_count + i] = 0;
//	mode_online_control->command_count += command_count;
//	mode_online_control->size += sizeof(unsigned short) * command_count;
//	return MODE_ONLINE_STATUS_OK;
//}
//
//mode_online_control_status_type Mode_Online_Control_Device_Remove(unsigned char device) // не используется 
//{
//	return MODE_ONLINE_STATUS_NONE;
//}
//
//mode_online_control_status_type Mode_Online_Control_Device_Command_Add(unsigned char device_index, unsigned char command_index, 
//	unsigned char telemetry,
//	unsigned char mode,  
//	unsigned short offset_ms,
//	unsigned char tx_flags,
//	unsigned int tx_freq_hz,
//	unsigned short tx_size,
//	unsigned char rx_flags,
//	unsigned int rx_freq_hz,
//	unsigned int rx_timeout_mks, 
//	unsigned short rx_pause_mks,
//	unsigned short rx_size,
//	unsigned short *tx_data)
//{
//	// проверки
//	if(mode_online_control->size + sizeof(mode_online_control_command_type) + sizeof(unsigned short) * (tx_size) > sizeof(mode_online_control_buffer)) return MODE_ONLINE_STATUS_ERROR_STRUCT;
//	if(device_index >= mode_online_control->device_count) return MODE_ONLINE_STATUS_ERROR_INDEX;
//	if(command_index >= (Mode_Online_Control_Device_Pointer_Get(device_index))->command_count) return MODE_ONLINE_STATUS_ERROR_INDEX;
//        if(telemetry >= TELEMETRY_UNKNOWN) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	if(tx_freq_hz < 1000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	if(rx_freq_hz < 1000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	if(telemetry == TELEMETRY_MANCHESTER)
//	{
//		if(tx_freq_hz > 50000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//		if(rx_freq_hz > 50000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	}
//	if(telemetry == TELEMETRY_USART)
//	{
//		if(tx_freq_hz > 1000000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//		if(rx_freq_hz > 1000000) return MODE_ONLINE_STATUS_ERROR_PARAMETR;
//	}
//	//
//	unsigned short i;
//	// если команда уже была
//	if(Mode_Online_Control_Command_Pointer_Get(device_index, command_index)) Mode_Online_Control_Device_Command_Remove(device_index, command_index); // удаляем 		
//	// заполняем указатель на команду
//	mode_online_control->pointer[mode_online_control->device_count + (Mode_Online_Control_Device_Pointer_Get(device_index))->command_index + command_index] = mode_online_control->size;
//        //  добавляем команду
//	mode_online_control_command_type *command = ((mode_online_control_command_type *)((unsigned int)mode_online_control + (unsigned int)mode_online_control->pointer[mode_online_control->device_count + (Mode_Online_Control_Device_Pointer_Get(device_index))->command_index + command_index]));
//        command->telemetry = telemetry;
//	command->mode = mode;  
//	command->offset_ms = offset_ms;
//	command->tx_flags = tx_flags;
//	command->tx_freq_hz = tx_freq_hz;
//	command->tx_size = tx_size;
//	command->rx_flags = rx_flags;
//	command->rx_freq_hz = rx_freq_hz;
//	command->rx_timeout_mks = rx_timeout_mks; 
//	command->rx_pause_mks = rx_pause_mks;
//	command->rx_size = rx_size;
//	memmove((char *)command->tx_data, (char *)tx_data, tx_size * sizeof(unsigned short));
//	mode_online_control->size += sizeof(mode_online_control_command_type) + sizeof(unsigned short) * (tx_size);
//	return MODE_ONLINE_STATUS_OK;
//}
//
//mode_online_control_status_type  Mode_Online_Control_Device_Command_Remove(unsigned char device_index, unsigned char command_index) 
//{
//	if(device_index >= mode_online_control->device_count) return MODE_ONLINE_STATUS_ERROR_INDEX;
//	if(command_index >= (Mode_Online_Control_Device_Pointer_Get(device_index))->command_count) return MODE_ONLINE_STATUS_ERROR_INDEX;
//	unsigned int i;        
//	unsigned short index = mode_online_control->device_count + Mode_Online_Control_Device_Pointer_Get(device_index)->command_index + command_index;
//	if(mode_online_control->pointer[index] == 0) return MODE_ONLINE_STATUS_OK; // команда уже пуста
//       	// сдвигаем всё влево
//	unsigned short shift = (((mode_online_control_command_type *)((unsigned int)mode_online_control + (unsigned int)mode_online_control->pointer[mode_online_control->device_count + (Mode_Online_Control_Device_Pointer_Get(device_index))->command_index + command_index]))->tx_size) * sizeof(unsigned short) + sizeof(mode_online_control_command_type);
//	if(mode_online_control->size < shift) return MODE_ONLINE_STATUS_ERROR_STRUCT;
//	for(i = mode_online_control->pointer[index]; i < mode_online_control->size - shift; i ++) mode_online_control_buffer[i] = mode_online_control_buffer[i + shift];
//	// уменьшаем все указатели что правее на shift
//	for(i = 0; i < mode_online_control->device_count + mode_online_control->command_count; i ++) 
//	{
//		if(mode_online_control->pointer[i] > mode_online_control->pointer[index]) // если указатель больше того что удаляем
//		{
//			if(mode_online_control->pointer[i] > shift) mode_online_control->pointer[i] -= shift; 
//			else return MODE_ONLINE_STATUS_ERROR_STRUCT;
//		}	
//	}
//	mode_online_control->pointer[index] = 0;
//	// изменяем заголовок
//	mode_online_control->size -= shift;
//	return MODE_ONLINE_STATUS_OK;
//}
//
//
////-----------------------------------------------
//
//unsigned char Mode_Online_Control_Device_Count_Get()
//{
//	return mode_online_control->device_count;
//}
//
//unsigned char Mode_Online_Control_Device_Command_Count_Get(unsigned char device)
//{
//	return Mode_Online_Control_Device_Pointer_Get(device)->command_count;
//}
//
//unsigned int Mode_Online_Control_Device_Period_Min_MS_Get(unsigned char device)
//{
//	return Mode_Online_Control_Device_Pointer_Get(device)->period_min_ms;
//}
//
//unsigned int Mode_Online_Control_Device_Delay_MS_Get(unsigned char device)
//{
//	return Mode_Online_Control_Device_Pointer_Get(device)->delay_ms;
//}
//
////-----------------------------------------------
//
//bool Mode_Online_Control_Device_Command_Enable_Get(unsigned char device, unsigned char command)
//{
//	return (bool)Mode_Online_Control_Command_Pointer_Get(device, command);
//}
//
//unsigned char Mode_Online_Control_Device_Command_Telemetry_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->telemetry;
//}
//
//unsigned char Mode_Online_Control_Device_Command_Mode_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->mode;
//}
//
//unsigned short Mode_Online_Control_Device_Command_Offset_MS_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->offset_ms;
//}
//
//unsigned char Mode_Online_Control_Device_Command_TX_Flags_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->tx_flags;
//}
//
//unsigned int Mode_Online_Control_Device_Command_TX_Freq_HZ_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->tx_freq_hz;
//}
//
//unsigned short Mode_Online_Control_Device_Command_TX_Size_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->tx_size;
//}
//
//unsigned char Mode_Online_Control_Device_Command_RX_Flags_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->rx_flags;
//}
//
//unsigned int Mode_Online_Control_Device_Command_RX_Freq_HZ_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->rx_freq_hz;
//}
//
//unsigned int Mode_Online_Control_Device_Command_RX_Timeout_MKS_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->rx_timeout_mks;
//}
//
//unsigned short Mode_Online_Control_Device_Command_RX_Pause_MKS_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->rx_pause_mks;
//}
//
//unsigned short Mode_Online_Control_Device_Command_RX_Size_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->rx_size;
//}
//
//unsigned short *Mode_Online_Control_Device_Command_TX_Data_Get(unsigned char device, unsigned char command)
//{
//	return Mode_Online_Control_Command_Pointer_Get(device, command)->tx_data;
//}
//
///*********************************************************************/
//
//unsigned short *mode_online_measure_rx_buffer;
//unsigned short mode_online_measure_rx_buffer_size;
//unsigned int mode_online_measure_period_time_ms;
//unsigned int mode_online_measure_device_time_ms;
//unsigned int mode_online_measure_period = 0;
//unsigned char mode_online_measure_device = 0;
//unsigned char mode_online_measure_command = 0;
//unsigned char mode_online_measure_device_count = 1;
//unsigned char mode_online_measure_command_count = 1;
//unsigned char mode_online_measure_mode = 1;
//telemetry_type mode_online_measure_telemetry = TELEMETRY_UNKNOWN;
//
//bool mode_online_measure_period_wait = true;
//bool mode_online_measure_device_change = true;
//
//mode_online_measure_telemetry_status_type mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT;
//
//unsigned int mode_online_measure_request_time_ms = 0;
//int mode_online_measure_request_depth_ms = 0;
//int mode_online_measure_request_speed_ms = 0;
//unsigned char mode_online_measure_request_flags = 0;
//unsigned short mode_online_measure_request_result = 0;
//unsigned short mode_online_measure_request = 0;
//
//bool Mode_Online_Measure_Reset() 
//{
//	if(Mode_Online_Measure_Busy()) return false;
//	mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT;
//	mode_online_measure_rx_buffer = Telemetry_RX_Buffer();
//	mode_online_measure_rx_buffer_size = Telemetry_RX_Buffer_Size();
//	mode_online_measure_period = 0;
//	mode_online_measure_device = 0;
//	mode_online_measure_command = 0;
//	mode_online_measure_device_count = 1;
//	mode_online_measure_command_count = 1;
//	mode_online_measure_device_change = true;
//	mode_online_measure_period_wait = true;
//	RTC_type rtc = RTC_Get();
//	mode_online_measure_period_time_ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60;
//	mode_online_measure_device_time_ms = mode_online_measure_period_time_ms;
//	return true;
//}
//
//void Mode_Online_Measure_Init() 
//{
//	Mode_Online_Measure_Reset();
//}
//
//bool Mode_Online_Measure_Busy()
//{       
//	if(mode_online_measure_telemetry_status != MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT) return true;
//	else return false;
//}
//
//bool Mode_Online_Measure_Idle()
//{       
//	//if((!Mode_Online_Measure_Busy()) && (!Mode_Online_Main_Enable_Get())) return false;
//	//switch (mode_online_measure_telemetry_status)
//	//{
//	//	case MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT:
//	//		if((!USART1_TX_Stopped()) || (!USART1_RX_Stopped()))
//	//		{
// //             			USART1_TX_Stop();      
//	//			USART1_RX_Stop();      
//	//			USART1_Disable();
//	//		}
//	//		else
//	//		if((!Manchester_TX_Stopped()) || (!Manchester_RX_Stopped()))
//	//		{
// //             			Manchester_TX_Stop();      
// //              			Manchester_RX_Stop();      
//	//		}
//	//		else 
//	//		{
// //                      		RTC_type rtc = RTC_Get();
//	//			unsigned int ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60; 
//	//			if(mode_online_measure_period_wait)// всё отработало
//	//			{
//	//				unsigned int period = Mode_Online_Control_Period_MS_Get();
//	//				if((period) && ((((ms + 1000 * 60 * 60 * 24) - mode_online_measure_period_time_ms) % (1000 * 60 * 60 * 24)) >= period))
//	//				{
//	//					mode_online_measure_period_time_ms = ms;
//	//					mode_online_measure_period ++;
//	//					mode_online_measure_device = 0;
//	//					mode_online_measure_command = 0;
//	//					mode_online_measure_device_count = 1;
//	//					mode_online_measure_command_count = 1;
//	//					mode_online_measure_device_change = true;
//	//					mode_online_measure_period_wait = false;
//	//					mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_MAKE; 
//	//				} 
//	//			} else mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_MAKE; 
// //               	}	
//	//		break;
//	//	case MODE_ONLINE_MEASURE_TELEMETRY_STATUS_MAKE: 
//	//		mode_online_measure_device_count = Mode_Online_Control_Device_Count_Get();
//	//		if((!mode_online_measure_device_count) || (mode_online_measure_device_count <= mode_online_measure_device))
//	//		{
//	//			mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT;
//	//			mode_online_measure_period_wait = true;
//	//			break;
//	//		}
//	//		bool temp_device_work = true;  // проверка на задержку и мин.период
//	//		unsigned int temp_period = Mode_Online_Control_Period_MS_Get();
//	//		unsigned int temp_device_period_min = Mode_Online_Control_Device_Period_Min_MS_Get(mode_online_measure_device);
//	//		unsigned int temp_device_delay = Mode_Online_Control_Device_Delay_MS_Get(mode_online_measure_device);
//	//		if(temp_device_delay > mode_online_measure_period * temp_period) temp_device_work = false;
// //                       if(temp_device_work)
//	//		{
//	//			if((temp_device_period_min != 0) && (temp_period != 0))
//	//			{
//	//				if(temp_device_period_min % temp_period)
//	//				{
//	//		       	 	 	if((mode_online_measure_period % ((temp_device_period_min / temp_period) + 1)) != 0) temp_device_work = false;
//	//				}
//	//				else if((mode_online_measure_period % (temp_device_period_min / temp_period)) != 0) temp_device_work = false;
//	//			} 
//	//			else if(temp_device_period_min != 0) temp_device_work = false;
//	//		}
//	//		if(temp_device_work)
//	//		{
//	//			mode_online_measure_command_count = Mode_Online_Control_Device_Command_Count_Get(mode_online_measure_device);
// //               	        if((!mode_online_measure_command_count) || (mode_online_measure_command_count <= mode_online_measure_command)) temp_device_work = false;// наличие команд
//	//			while(!Mode_Online_Control_Device_Command_Enable_Get(mode_online_measure_device, mode_online_measure_command)) 
//	//			{
//	//				mode_online_measure_command ++;
//	//				if(mode_online_measure_command >= mode_online_measure_command_count)
//	//				{
//	//                       		 	temp_device_work = false;
//	//					break;
//	//				}
//	//			}
//	//		}
//	//		if(!temp_device_work) // если прибор не готов к работе (задержка от питания, пропуск периода)
//	//		{
//	//			mode_online_measure_command = 0;
// //      		         	mode_online_measure_device ++;   // переходим к опросу следующего прибора
//	//			mode_online_measure_device_change = true;
//	//			if(mode_online_measure_device >= mode_online_measure_device_count)
//	//			{
//	//				mode_online_measure_device = 0;
//	//				mode_online_measure_period_wait = true;
//	//			}
//	//			mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT;
//	//			break;
//	//		}
// //                       RTC_type rtc = RTC_Get();
//	//		unsigned int ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60; 
//	//		if(mode_online_measure_device_change)     // если новый прибор
//	//		{
//	//			mode_online_measure_device_time_ms = ms;
//	//			mode_online_measure_device_change = false;
//	//		}
//	//		// проверяем на смещение опроса от начала работы прибора
//	//		if((((ms + 1000 * 60 * 60 * 24) - mode_online_measure_device_time_ms) % (1000 * 60 * 60 *24)) < Mode_Online_Control_Device_Command_Offset_MS_Get(mode_online_measure_device, mode_online_measure_command))
//	//		{
//	//			// если команда не готова к работе - подождать
//	//			mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT;
//	//			break;
//	//		}
//	//		// собственно опрос прибора если все условия соблюдены			
//	//		unsigned short temp_command_tx_count = Mode_Online_Control_Device_Command_TX_Size_Get(mode_online_measure_device, mode_online_measure_command);
//	//		telemetry_tx_type *temp_command_tx_data = (telemetry_tx_type *)Mode_Online_Control_Device_Command_TX_Data_Get(mode_online_measure_device, mode_online_measure_command);
//	//		unsigned int temp_command_tx_freq = Mode_Online_Control_Device_Command_TX_Freq_HZ_Get(mode_online_measure_device, mode_online_measure_command);
//	//		unsigned short temp_command_rx_count = Mode_Online_Control_Device_Command_RX_Size_Get(mode_online_measure_device, mode_online_measure_command);
//	//		unsigned int temp_command_rx_freq = Mode_Online_Control_Device_Command_RX_Freq_HZ_Get(mode_online_measure_device, mode_online_measure_command);
//	//		mode_online_measure_telemetry = Mode_Online_Control_Device_Command_Telemetry_Get(mode_online_measure_device, mode_online_measure_command);
//	//		mode_online_measure_mode = Mode_Online_Control_Device_Command_Mode_Get(mode_online_measure_device, mode_online_measure_command);
// //      			mode_online_measure_request_flags = 0;
//	//		mode_online_measure_request_time_ms = Mode_Online_RDC_Time_Ms_Get();
//	//		mode_online_measure_request_depth_ms = Mode_Online_RDC_Depth_Get();
//	//		mode_online_measure_request_speed_ms = Mode_Online_RDC_Speed_Get();
//	//		mode_online_measure_request = temp_command_tx_data->request;
//	//		switch (mode_online_measure_telemetry)
//	//		{
//	//			case TELEMETRY_USART:
//	//				USART1_Enable(temp_command_tx_freq, temp_command_rx_freq); 
//	//				USART1_RX_Start(Mode_Online_Control_Device_Command_RX_Timeout_MKS_Get(mode_online_measure_device, mode_online_measure_command), 
//	//					Mode_Online_Control_Device_Command_RX_Pause_MKS_Get(mode_online_measure_device, mode_online_measure_command), 
// //                                     		temp_command_rx_count * sizeof(unsigned short) / sizeof(unsigned char), 
//	//					(unsigned char *)mode_online_measure_rx_buffer,
//	//					(mode_online_measure_rx_buffer_size));  // сначала RX, чтобы пока иниц.не закончился TX
//	//				USART1_TX_Start(temp_command_tx_count * sizeof(unsigned short) / sizeof(unsigned char), 
//	//					(unsigned char *)temp_command_tx_data);
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_TX; 
//	//				break;
//	//			case TELEMETRY_MANCHESTER:
//	//				Manchester_RX_Start(Mode_Online_Control_Device_Command_RX_Flags_Get(mode_online_measure_device, mode_online_measure_command), 
//	//					temp_command_rx_freq, 
//	//					Mode_Online_Control_Device_Command_RX_Timeout_MKS_Get(mode_online_measure_device, mode_online_measure_command), 
//	//					Mode_Online_Control_Device_Command_RX_Pause_MKS_Get(mode_online_measure_device, mode_online_measure_command), 
//	//					temp_command_rx_count, 
//	//					(unsigned short *)mode_online_measure_rx_buffer, 
//	//					(mode_online_measure_rx_buffer_size) / sizeof(unsigned short)); // сначала RX, чтобы пока иниц.не закончился TX
//	//				Manchester_TX_Start(Mode_Online_Control_Device_Command_TX_Flags_Get(mode_online_measure_device, mode_online_measure_command), 
//	//					temp_command_tx_freq, 
//	//					temp_command_tx_count, 
//	//					(unsigned short *)temp_command_tx_data);
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_TX; 
//	//				break;
//	//			default:
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_TX; 
//	//				break;
//	//		}
//	//		break;
//	//	case MODE_ONLINE_MEASURE_TELEMETRY_STATUS_TX:
//	//		switch (mode_online_measure_telemetry)
//	//		{
//	//			case TELEMETRY_USART:
//	//				if((USART1_TX_Error()) || (USART1_TX_Stopped())) mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
//	//				else if(USART1_TX_Ready()) mode_online_measure_request_flags &= ~(1 << TELEMETRY_FLAG_TX_ERROR_BIT);
//	//				else break;
//	//				USART1_TX_Stop();      
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_RX; 	
//	//				break;
//	//			case TELEMETRY_MANCHESTER:
//	//				if((Manchester_TX_Error()) || (Manchester_TX_Stopped())) mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
//	//				else if(Manchester_TX_Ready()) mode_online_measure_request_flags &= ~(1 << TELEMETRY_FLAG_TX_ERROR_BIT);
//	//				else break;
//	//				Manchester_TX_Stop();      
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_RX; 
//	//				break;
//	//			default:
//	//				mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_RX; 
//	//				break;
//	//		}
//	//		break;
//	//	case MODE_ONLINE_MEASURE_TELEMETRY_STATUS_RX:
//	//		switch (mode_online_measure_telemetry)
//	//		{
//	//			case TELEMETRY_USART:
//	//				if((USART1_RX_Error()) || (USART1_RX_Stopped())) mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
//	//				else if(USART1_RX_Ready()) mode_online_measure_request_flags &= ~(1 << TELEMETRY_FLAG_RX_ERROR_BIT);
//	//				else break;
//	//				USART1_RX_Stop();    
//	//				USART1_Disable();
//	//				mode_online_measure_request_result = USART1_RX_Get() * sizeof(unsigned char) / sizeof(unsigned short) ; 
// //               			mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_SEND; 	
//	//				break;
//	//			case TELEMETRY_MANCHESTER:
//	//				if((Manchester_RX_Error()) || (Manchester_RX_Stopped())) mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
//	//				else if(Manchester_RX_Ready()) mode_online_measure_request_flags &= ~(1 << TELEMETRY_FLAG_RX_ERROR_BIT);
//	//				else break;
//	//				Manchester_RX_Stop();    
//	//				mode_online_measure_request_result = Manchester_RX_Get(); 
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_SEND; 
//	//				break;
//	//			default:
//	//				mode_online_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
//	//				mode_online_measure_request_result = 0; 
//	//				mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_SEND; 
//	//				break;
//	//		}
//	//		break;		
//	//	case MODE_ONLINE_MEASURE_TELEMETRY_STATUS_SEND:
//	//		TRAP_VECTOR_SendVector(mode_online_measure_request, mode_online_measure_request_time_ms, mode_online_measure_request_depth_ms, mode_online_measure_request_speed_ms, mode_online_measure_request_flags, mode_online_measure_request_result * sizeof(unsigned short), (unsigned char *)mode_online_measure_rx_buffer);
//	//		// думаем что делать с тем что получили
//	//		bool temp_result = true;
//	//		if(mode_online_measure_request_flags & ((1 << TELEMETRY_FLAG_TX_ERROR_BIT) | (1 << TELEMETRY_FLAG_RX_ERROR_BIT))) temp_result = false;
// //       		// проверка на периодичность/отработанность
//	//		if(((temp_result) // команда успешно отработала
//	//		     && (mode_online_measure_mode & (1 << TELEMETRY_COMMAND_MODE_READY_STOP_SESSION_BIT))) // и больше не надо опрашивать в этой сессии при полож.результате
//	//		   || ((!temp_result) // команда неуспешно отработала
//	//		     && (mode_online_measure_mode & (1 << TELEMETRY_COMMAND_MODE_ERROR_STOP_SESSION_BIT)))) // и больше не надо опрашивать в этой сессии при отриц.результате
//	//		{       // удаляем команду
//	//                    	Mode_Online_Control_Device_Command_Remove(mode_online_measure_device, mode_online_measure_command);
//	//		}
//	//		// проверяем окончание работы с прибором на этом периоде
//	//		if(((!temp_result) && (mode_online_measure_mode & (1 << TELEMETRY_COMMAND_MODE_ERROR_STOP_DEVICE_BIT))) 
//	//		 || ((temp_result) && (mode_online_measure_mode & (1 << TELEMETRY_COMMAND_MODE_READY_STOP_DEVICE_BIT)))) 
//	//		{
//	//			mode_online_measure_command = mode_online_measure_command_count;
//	//		}	
//	//		else mode_online_measure_command ++; 
// //       		if(mode_online_measure_command >= mode_online_measure_command_count)
//	//		{
//	//			mode_online_measure_command = 0;
// //      		         	mode_online_measure_device ++; 
//	//			mode_online_measure_device_change = true;
//	//			if(mode_online_measure_device >= mode_online_measure_device_count) 
//	//			{	
//	//				mode_online_measure_device = 0;
//	//				mode_online_measure_period_wait = true;
//	//			}
//	//		}
// //                       mode_online_measure_telemetry_status = MODE_ONLINE_MEASURE_TELEMETRY_STATUS_WAIT; 
//	//		break;		
//	//}
//
//	return true;
//}
//
///*********************************************************************/
//
//void Mode_Online_Init()
//{
//	Mode_Online_Main_Init();
//	Mode_Online_RDC_Init();
//	Mode_Online_Special_Init();
//	Mode_Online_Control_Init();
//	Mode_Online_Measure_Init();
//}
//
//bool Mode_Online_Reset()
//{
//	bool result = true;
//	result &= Mode_Online_Main_Reset();
//	result &= Mode_Online_RDC_Reset();
//	result &= Mode_Online_Special_Reset();
//	result &= Mode_Online_Control_Reset();
//	result &= Mode_Online_Measure_Reset();
//	return result;
//}
//
//bool Mode_Online_Idle()
//{
//	Mode_Online_Main_Idle();
//	Mode_Online_RDC_Idle();
//	Mode_Online_Special_Idle();
//	Mode_Online_Measure_Idle();
//	Mode_Online_Control_Idle();
//	return true;
//}
//
//
