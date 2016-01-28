#include "common.h"
#include "main.h"
#include "fram.h"
#include "flash.h"
//#include "manchester.h"
//#include "usart1.h"
#include "rtc.h"
#include "power.h"
#include "mode_autonom.h"
#include "vector.h"
#include "telemetry.h"
#include "sensors.h"

/*********************************************************************/
bool Mode_Autonom_Special_Busy();
bool Mode_Autonom_Measure_Busy();
bool Mode_Autonom_Special_Reset();
bool Mode_Autonom_Measure_Reset();	

unsigned char mode_autonom_main_time_sec;
unsigned char mode_autonom_main_timeout_sec;
bool mode_autonom_main_enable;    
bool mode_autonom_main_session_enable;    
unsigned char mode_autonom_main_session;  

bool Mode_Autonom_Main_Reset()
{
	mode_autonom_main_timeout_sec = MODE_AUTONOM_MAIN_TIMEOUT_S + 1;
	mode_autonom_main_time_sec = RTC_Get().sec;
	mode_autonom_main_enable = false;
	mode_autonom_main_session_enable = false;
	mode_autonom_main_session = 0;
	return true;
}

void Mode_Autonom_Main_Init()
{
	Mode_Autonom_Main_Reset();
}

bool Mode_Autonom_Main_Idle() // здесь будет вся работа в временами старта/стопа сессий, сделать сессию до включения, рабочие сессии и после выключения
{       
	//if(mode_autonom_main_timeout_sec)
	//{
	//	RTC_type rtc = RTC_Get();
	//	if(rtc.sec != mode_autonom_main_time_sec)
	//	{
	//		mode_autonom_main_timeout_sec --;
	//		mode_autonom_main_time_sec = rtc.sec;
	//		if(!mode_autonom_main_timeout_sec)    // общий старт автономки
	//		{
	//			EMAC_Power_Off();
	//		}
	//	}	
	//}
	//if(!mode_autonom_main_timeout_sec) 	// далее парсинг времен сессий
	//{
	//	if((!Mode_Autonom_Special_Busy()) && (!Mode_Autonom_Measure_Busy()))
	//	{
	//		bool new_session = false;
	//		if(!mode_autonom_main_enable)
	//		{
	//			mode_autonom_main_enable = true; // включаю
	//			new_session = true;
	//		}
	//		if(mode_autonom_main_enable)
	//		{
	//			if(!mode_autonom_main_session_enable)	// если сессия не включена
	//			{				
	//				unsigned char n;	
	//				for(n = 0; n < FRAM_Autonom_Session_Count_Get(); n ++) // от 0, вдруг сессии перемешаны
	//				{
	//					if((RTC_Check(FRAM_Autonom_Session_Start_Get(n))) && (!RTC_Check(FRAM_Autonom_Session_Stop_Get(n)))) // включить
	//					{
	//	       		                  	mode_autonom_main_session_enable = true; // включаю
	//						mode_autonom_main_session = n; // включаю
	//	                			Power_Switch_Set(true);	// врубаю питание
	//						new_session = true;
	//						break;
	//					}
	//				}
 //      				}
	//			else	// включена сессия
	//			{
	//				if((!RTC_Check(FRAM_Autonom_Session_Start_Get(mode_autonom_main_session))) || (RTC_Check(FRAM_Autonom_Session_Stop_Get(mode_autonom_main_session)))) // выключить
	//				{
	//					mode_autonom_main_session = 0;
	//                       		  	mode_autonom_main_session_enable = false; // выключаю
	//					mode_autonom_main_enable = false; // и это выключаю, само перевключится
	//                			Power_Switch_Set(false);
	//				}
	//			}
	//		
	//		}
	//		if(new_session) 
	//		{
 //      		         	FRAM_Memory_Current_Session_Set(FRAM_Memory_Current_Session_Get() + 1); // добавляю сессию
 //      			 	FLASH_Vectors_Saved_Reset();
	//		 	FLASH_Vectors_Errors_Reset();
	//			Mode_Autonom_Special_Reset();	// обнуляю все по МП
	//			Mode_Autonom_Measure_Reset();	// обнуляю по приборам
	//		}
	//	}		
	//}

}

 
inline bool Mode_Autonom_Main_Enable_Get()
{
	return mode_autonom_main_enable;
}

inline bool Mode_Autonom_Main_Session_Enable_Get()
{
	return mode_autonom_main_session_enable;
}

inline unsigned char Mode_Autonom_Main_Session_Get()
{
	return mode_autonom_main_session;
}

/*********************************************************************/
unsigned int mode_autonom_special_count = 0;
unsigned int mode_autonom_special_time = 0;         

bool Mode_Autonom_Special_Reset()
{
	mode_autonom_special_count = 0;
	mode_autonom_special_time = (unsigned int)RTC_Get().msec + (unsigned int)RTC_Get().sec * 1000 + (unsigned int)RTC_Get().min * 60 * 1000;
	return true;
}

void Mode_Autonom_Special_Init()
{
	Mode_Autonom_Special_Reset();
}

bool Mode_Autonom_Special_Busy()  // заглушка
{  
	return false;	
}
 
bool Mode_Autonom_Special_Idle()
{  
	if((!Mode_Autonom_Special_Busy()) && (!Mode_Autonom_Main_Enable_Get()))
	{
		return false;
	}
	unsigned int period = FRAM_Autonom_Special_Period_MS_Get();
	if(!period) return false;
	if(FLASH_Busy()) return false;
	unsigned int msec = (unsigned int)RTC_Get().msec + (unsigned int)RTC_Get().sec * 1000 + (unsigned int)RTC_Get().min * 60 * 1000 + (unsigned int)RTC_Get().hour * 60 * 60 * 1000;
	if(((msec + (24 * 60 * 60 * 1000) - mode_autonom_special_time) % (24 * 60 * 60 * 1000)) >= period)
	{
		unsigned char flag = 0;
		if(!Mode_Autonom_Main_Session_Enable_Get()) flag = (1 << TELEMETRY_FLAG_DEVICES_DISABLE_BIT);
		mode_autonom_special_time = msec;
		if(mode_autonom_special_count == 0)
		{
			mode_autonom_special_identification_type mode_autonom_special_identification;
			mode_autonom_special_identification.command = MODE_AUTONOM_SPECIAL_IDENTIFICATION_COMMAND;
			mode_autonom_special_identification.number = FRAM_Main_Device_Number_Get();
			mode_autonom_special_identification.version = VERSION;	
			if(FLASH_Write_Vector(FRAM_Memory_Current_Session_Get(), MODE_AUTONOM_SPECIAL_IDENTIFICATION_COMMAND, RTC_Get(), flag, (unsigned char *)(&mode_autonom_special_identification), sizeof(mode_autonom_special_identification_type), FLASH_SAVE_REPEAT_NORMAL)) mode_autonom_special_count ++;
		}
		else
		if(mode_autonom_special_count == 1)
		{
			mode_autonom_special_options_type mode_autonom_special_options;
			mode_autonom_special_options.command = MODE_AUTONOM_SPECIAL_OPTIONS_COMMAND;
			if(FLASH_Write_Vector(FRAM_Memory_Current_Session_Get(), MODE_AUTONOM_SPECIAL_OPTIONS_COMMAND, RTC_Get(), flag, (unsigned char *)(&mode_autonom_special_options), sizeof(mode_autonom_special_options_type), FLASH_SAVE_REPEAT_NORMAL)) mode_autonom_special_count ++;
		}
		else
		{
			mode_autonom_special_measure_type mode_autonom_special_measure;
			mode_autonom_special_measure.command = MODE_AUTONOM_SPECIAL_MEASURE_COMMAND;
			mode_autonom_special_measure.battery_voltage = Power_Battery_Voltage_Get();
			mode_autonom_special_measure.line_voltage = Power_Line_Voltage_Get();	
        		mode_autonom_special_measure.temperature_in = RTC_Get_Temperature();
			mode_autonom_special_measure.ax = Sensors_Ax_Get();
			mode_autonom_special_measure.ay = Sensors_Ay_Get();
			mode_autonom_special_measure.az = Sensors_Az_Get();
			if(FLASH_Write_Vector(FRAM_Memory_Current_Session_Get(), MODE_AUTONOM_SPECIAL_MEASURE_COMMAND, RTC_Get(), flag, (unsigned char *)(&mode_autonom_special_measure), sizeof(mode_autonom_special_measure_type), FLASH_SAVE_REPEAT_NORMAL))  mode_autonom_special_count ++;
		}
		return true;
	}
	return false;
}

/*********************************************************************/
unsigned short *mode_autonom_measure_rx_buffer;
unsigned short mode_autonom_measure_rx_buffer_size;

unsigned int mode_autonom_measure_period_time_ms;
unsigned int mode_autonom_measure_device_time_ms;
unsigned int mode_autonom_measure_period = 0;
unsigned char mode_autonom_measure_session = 0;
unsigned char mode_autonom_measure_device = 0;
unsigned char mode_autonom_measure_command = 0;
unsigned char mode_autonom_measure_device_count = 1;
unsigned char mode_autonom_measure_command_count = 1;
bool mode_autonom_measure_period_wait = true;
bool mode_autonom_measure_device_change = true;

telemetry_type mode_autonom_measure_telemetry = TELEMETRY_UNKNOWN;
mode_autonom_measure_telemetry_status_type mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT;

RTC_type mode_autonom_measure_request_rtc;
unsigned char mode_autonom_measure_request_flags = 0;
unsigned short mode_autonom_measure_request_result = 0;
unsigned short mode_autonom_measure_request = 0;

bool Mode_Autonom_Measure_Reset() 
{
	if(Mode_Autonom_Measure_Busy()) return false;
	mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT;
	mode_autonom_measure_rx_buffer = Telemetry_RX_Buffer();
	mode_autonom_measure_rx_buffer_size = Telemetry_RX_Buffer_Size();
	FRAM_Autonom_Result_Reset();
	mode_autonom_measure_period = 0;
	mode_autonom_measure_session = 0;
	mode_autonom_measure_device = 0;
	mode_autonom_measure_command = 0;
	mode_autonom_measure_device_count = 1;
	mode_autonom_measure_command_count = 1;
	mode_autonom_measure_device_change = true;
	mode_autonom_measure_period_wait = true;
	RTC_type rtc = RTC_Get();
	mode_autonom_measure_period_time_ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60;
	mode_autonom_measure_device_time_ms = mode_autonom_measure_period_time_ms;
	return true;
}

void Mode_Autonom_Measure_Init() 
{
	Mode_Autonom_Measure_Reset();
}

bool Mode_Autonom_Measure_Busy()
{       
	if(mode_autonom_measure_telemetry_status != MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT) return true;
	else return false;
}

bool Mode_Autonom_Measure_Idle()
{       
	//if((!Mode_Autonom_Measure_Busy()) && ((!Mode_Autonom_Main_Enable_Get()) || (!Mode_Autonom_Main_Session_Enable_Get()))) return false;
	//switch (mode_autonom_measure_telemetry_status)
	//{
	//	case MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT:
	//		if((!USART1_TX_Stopped()) || (!USART1_RX_Stopped()))
	//		{
 //             			USART1_TX_Stop();      
	//			USART1_RX_Stop();      
	//			USART1_Disable();
	//		}
	//		else
	//		if((!Manchester_TX_Stopped()) || (!Manchester_RX_Stopped()))
	//		{
 //             			Manchester_TX_Stop();      
 //              			Manchester_RX_Stop();      
	//		}
	//		else 
	//		{
 //                      		RTC_type rtc = RTC_Get();
	//			unsigned int ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60; 
	//			if(mode_autonom_measure_period_wait)// всё отработало
	//			{
	//				if((((ms + 1000 * 60 * 60 * 24) - mode_autonom_measure_period_time_ms) % (1000 * 60 * 60 * 24)) >= FRAM_Autonom_Session_Period_MS_Get(Mode_Autonom_Main_Session_Get()))   
	//				{
	//					mode_autonom_measure_period_time_ms = ms;
	//					mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_MAKE; 
	//					mode_autonom_measure_period ++;
	//					mode_autonom_measure_device = 0;
	//					mode_autonom_measure_command = 0;
	//					mode_autonom_measure_device_count = 1;
	//					mode_autonom_measure_command_count = 1;
	//					mode_autonom_measure_device_change = true;
	//					mode_autonom_measure_period_wait = false;
	//				} 
	//			} else mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_MAKE; 
 //               	}	
	//		break;
	//	case MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_MAKE:
	//	        nop();
	//		mode_autonom_measure_session = Mode_Autonom_Main_Session_Get();
	//		mode_autonom_measure_device_count = FRAM_Autonom_Session_Device_Count_Get(mode_autonom_measure_session);         // все проверки живут во FRAM
 //     			mode_autonom_measure_command_count = FRAM_Autonom_Session_Device_Command_Count_Get(mode_autonom_measure_session, mode_autonom_measure_device); // все проверки живут во FRAM
	//		bool temp_device_work = true;  // проверка на задержку и мин.период
	//		unsigned int temp_session_period = FRAM_Autonom_Session_Period_MS_Get(Mode_Autonom_Main_Session_Get());
	//		unsigned int temp_device_period_min = FRAM_Autonom_Session_Device_Period_Min_MS_Get(mode_autonom_measure_session, mode_autonom_measure_device);
	//		if(FRAM_Autonom_Session_Device_Delay_MS_Get(mode_autonom_measure_session, mode_autonom_measure_device) > mode_autonom_measure_period * temp_session_period) temp_device_work = false;
	//		if((temp_device_period_min != 0) && (temp_session_period != 0))
	//		{
	//			if(temp_device_period_min % temp_session_period)
	//			{
	//	        	 	if((mode_autonom_measure_period % ((temp_device_period_min / temp_session_period) + 1)) != 0) temp_device_work = false;
	//			}
	//			else if((mode_autonom_measure_period % (temp_device_period_min / temp_session_period)) != 0) temp_device_work = false;
	//		} 
	//		else if(temp_device_period_min != 0) temp_device_work = false;
	//		if(!temp_device_work) // если прибор не готов к работе (задержка от питания, пропуск периода)
	//		{
	//			mode_autonom_measure_command = 0;
 //      		         	mode_autonom_measure_device ++;   // переходим к опросу следующего прибора
	//			mode_autonom_measure_device_change = true;
	//			if(mode_autonom_measure_device >= mode_autonom_measure_device_count)
	//			{
	//				mode_autonom_measure_device = 0;
	//				mode_autonom_measure_period_wait = true;
	//			}
	//			mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT;
	//			break;
	//		}
 //                       RTC_type rtc = RTC_Get();
	//		unsigned int ms = rtc.msec + (unsigned int)rtc.sec * 1000 + (unsigned int)rtc.min * 1000 * 60 + (unsigned int)rtc.hour * 1000 * 60 * 60; 
	//		if(mode_autonom_measure_device_change)     // если новый прибор
	//		{
	//			mode_autonom_measure_device_time_ms = ms;
	//			mode_autonom_measure_device_change = false;
	//		}
	//		// проверка на периодичность/отработанность
	//		if((FRAM_Autonom_Result_Ready_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) // команда успешно отработала
	//		     && (FRAM_Autonom_Session_Device_Command_Mode_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) & (1 << TELEMETRY_COMMAND_MODE_READY_STOP_SESSION_BIT))) // и больше не надо опрашивать в этой сессии при полож.результате
	//		   || (FRAM_Autonom_Result_Error_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) // команда неуспешно отработала
	//		     && (FRAM_Autonom_Session_Device_Command_Mode_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) & (1 << TELEMETRY_COMMAND_MODE_ERROR_STOP_SESSION_BIT)))) // и больше не надо опрашивать в этой сессии при отриц.результате
	//		{       // следующая команда
	//			mode_autonom_measure_command ++; 
	//			if(mode_autonom_measure_command >= mode_autonom_measure_command_count)
	//			{
	//				mode_autonom_measure_command = 0;
 //      		       		  	mode_autonom_measure_device ++; 
	//				mode_autonom_measure_device_change = true;
	//				if(mode_autonom_measure_device >= mode_autonom_measure_device_count) 
	//				{	
	//					mode_autonom_measure_device = 0;
	//					mode_autonom_measure_period_wait = true;
	//				}
	//			}
	//			mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT;
	//			break;
	//		}
	//		// проверяем на смещение опроса от начала работы прибора
	//		if((((ms + 1000 * 60 * 60 * 24) - mode_autonom_measure_device_time_ms) % (1000 * 60 * 60 *24)) < FRAM_Autonom_Session_Device_Command_Offset_MS_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command))
	//		{
	//			// если команда не готова к работе - подождать
	//			mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT;
	//			break;
	//		}
	//		// собственно опрос прибора если все условия соблюдены			
	//		unsigned short temp_command_tx_count = FRAM_Autonom_Session_Device_Command_TX_Size_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
	//		telemetry_tx_type *temp_command_tx_data = (telemetry_tx_type *)FRAM_Autonom_Session_Device_Command_TX_Data_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
	//		unsigned int temp_command_tx_freq = FRAM_Autonom_Session_Device_Command_TX_Freq_HZ_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
	//		unsigned short temp_command_rx_count = FRAM_Autonom_Session_Device_Command_RX_Size_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
	//		unsigned int temp_command_rx_freq = FRAM_Autonom_Session_Device_Command_RX_Freq_HZ_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
	//		mode_autonom_measure_telemetry = FRAM_Autonom_Session_Device_Command_Telemetry_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command);
 //      			mode_autonom_measure_request_flags = 0;
	//		mode_autonom_measure_request_rtc = RTC_Get();
	//		mode_autonom_measure_request = temp_command_tx_data->request;

	//		switch (mode_autonom_measure_telemetry)
	//		{
	//			case TELEMETRY_USART:
	//				USART1_Enable(temp_command_tx_freq, temp_command_rx_freq); 
	//				USART1_RX_Start(FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
	//					FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
 //                                     		temp_command_rx_count * sizeof(unsigned short) / sizeof(unsigned char), 
	//					(unsigned char *)mode_autonom_measure_rx_buffer,
	//					(mode_autonom_measure_rx_buffer_size));  // сначала RX, чтобы пока иниц.не закончился TX
	//				USART1_TX_Start(temp_command_tx_count * sizeof(unsigned short) / sizeof(unsigned char), 
	//					(unsigned char *)temp_command_tx_data);
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_TX; 
	//				break;
	//			case TELEMETRY_MANCHESTER:
	//				Manchester_RX_Start(FRAM_Autonom_Session_Device_Command_RX_Flags_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
	//					temp_command_rx_freq, 
	//					FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
	//					FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
	//					temp_command_rx_count, 
	//					(unsigned short *)mode_autonom_measure_rx_buffer, 
	//					(mode_autonom_measure_rx_buffer_size) / sizeof(unsigned short)); // сначала RX, чтобы пока иниц.не закончился TX
	//				Manchester_TX_Start(FRAM_Autonom_Session_Device_Command_TX_Flags_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command), 
	//					temp_command_tx_freq, 
	//					temp_command_tx_count, 
	//					(unsigned short *)temp_command_tx_data);
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_TX; 
	//				break;
	//			default:
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_TX; 
	//				break;
	//		}
	//		break;
	//	case MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_TX:
	//		switch (mode_autonom_measure_telemetry)
	//		{
	//			case TELEMETRY_USART:
	//				if((USART1_TX_Error()) || (USART1_TX_Stopped())) mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
	//				else if(USART1_TX_Ready()) mode_autonom_measure_request_flags &= ~(1 << TELEMETRY_FLAG_TX_ERROR_BIT);
	//				else break;
	//				USART1_TX_Stop();      
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_RX; 	
	//				break;
	//			case TELEMETRY_MANCHESTER:
	//				if((Manchester_TX_Error()) || (Manchester_TX_Stopped())) mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
	//				else if(Manchester_TX_Ready()) mode_autonom_measure_request_flags &= ~(1 << TELEMETRY_FLAG_TX_ERROR_BIT);
	//				else break;
	//				Manchester_TX_Stop();      
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_RX; 
	//				break;
	//			default:
	//				mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_TX_ERROR_BIT);
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_RX; 
	//				break;
	//		}
	//		break;
	//	case MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_RX:
	//		switch (mode_autonom_measure_telemetry)
	//		{
	//			case TELEMETRY_USART:
	//				if((USART1_RX_Error()) || (USART1_RX_Stopped())) mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
	//				else if(USART1_RX_Ready()) mode_autonom_measure_request_flags &= ~(1 << TELEMETRY_FLAG_RX_ERROR_BIT);
	//				else break;
	//				USART1_RX_Stop();    
	//				USART1_Disable();
	//				mode_autonom_measure_request_result =	USART1_RX_Get() * sizeof(unsigned char) / sizeof(unsigned short) ; 
 //               			mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_SAVE; 	
	//				break;
	//			case TELEMETRY_MANCHESTER:
	//				if((Manchester_RX_Error()) || (Manchester_RX_Stopped())) mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
	//				else if(Manchester_RX_Ready()) mode_autonom_measure_request_flags &= ~(1 << TELEMETRY_FLAG_RX_ERROR_BIT);
	//				else break;
	//				Manchester_RX_Stop();    
	//				mode_autonom_measure_request_result =	Manchester_RX_Get(); 
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_SAVE; 
	//				break;
	//			default:
	//				mode_autonom_measure_request_flags |= (1 << TELEMETRY_FLAG_RX_ERROR_BIT);
	//				mode_autonom_measure_request_result =	0; 
	//				mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_SAVE; 
	//				break;
	//		}
	//		break;		
	//	case MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_SAVE:
	//		if(FLASH_Busy()) break;
	//		FLASH_Write_Vector(FRAM_Memory_Current_Session_Get(), mode_autonom_measure_request, mode_autonom_measure_request_rtc, mode_autonom_measure_request_flags, (unsigned char *)mode_autonom_measure_rx_buffer, mode_autonom_measure_request_result * sizeof(unsigned short), FLASH_SAVE_REPEAT_NORMAL);
	//		// думаем что делать с тем что получили
	//		bool temp_result = true;
	//		if(mode_autonom_measure_request_flags & ((1 << TELEMETRY_FLAG_TX_ERROR_BIT) | (1 << TELEMETRY_FLAG_RX_ERROR_BIT))) temp_result = false;
	//		FRAM_Autonom_Result_Set(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command, temp_result);
	//		if(((!temp_result) && (FRAM_Autonom_Session_Device_Command_Mode_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) & (1 << TELEMETRY_COMMAND_MODE_ERROR_STOP_DEVICE_BIT))) 
	//		 || ((temp_result) && (FRAM_Autonom_Session_Device_Command_Mode_Get(mode_autonom_measure_session, mode_autonom_measure_device, mode_autonom_measure_command) & (1 << TELEMETRY_COMMAND_MODE_READY_STOP_DEVICE_BIT)))) 
	//		{
	//		 	// заканчиваем работу с прибором на этом периоде
	//			mode_autonom_measure_command = mode_autonom_measure_command_count;
	//		}	
	//		else mode_autonom_measure_command ++; 
	//		if(mode_autonom_measure_command >= mode_autonom_measure_command_count)
	//		{
	//			mode_autonom_measure_command = 0;
 //      		         	mode_autonom_measure_device ++; 
	//			mode_autonom_measure_device_change = true;
	//			if(mode_autonom_measure_device >= mode_autonom_measure_device_count) 
	//			{	
	//				mode_autonom_measure_device = 0;
	//				mode_autonom_measure_period_wait = true;
	//			}
	//		}
 //                       mode_autonom_measure_telemetry_status = MODE_AUTONOM_MEASURE_TELEMETRY_STATUS_WAIT; 
	//		break;		
	//}
}

/*********************************************************************/
unsigned short mode_autonom_led_time_ms;

bool Mode_Autonom_Led_Reset()
{
	mode_autonom_led_time_ms = RTC_Get_Millisecond();
	return true;
}

void Mode_Autonom_Led_Init()
{
	//AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB30;	// светодиодик
 //       AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB30;
	//AT91C_BASE_PIOB->PIO_OWER = AT91C_PIO_PB30;
	//AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB30;
	//Mode_Autonom_Led_Reset();
}

bool Mode_Autonom_Led_Idle()
{       
	//if(!Mode_Autonom_Main_Enable_Get()) return false;
	//unsigned short msec = RTC_Get_Millisecond();
	//if((((msec + 1000) - mode_autonom_led_time_ms) % 1000) < POWER_CONTROL_PERIOD_MS)  return; // раз в 10 мс, чаще не надо
	//mode_autonom_led_time_ms = msec;
	//msec = RTC_Get().msec;
	//unsigned char sec = RTC_Get().sec;
	//// светоидиот
	//if((RTC_Get().year < 2014) || (FRAM_Autonom_Validation_Get() != FRAM_AUTONOM_VALIDATION_OK))
	//{ 	// сбиты часы или память       ____x_______xxxx____x_________xxxx____x_________xxxx____x_________xxxx
	// 	if(((sec % MODE_AUTONOM_LED_FAIL_OPTIONS_PERIOD_S) == 0) && (msec < MODE_AUTONOM_LED_FAIL_OPTIONS_DUTY1_MS)) AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB30; 
	//	else if(((sec % MODE_AUTONOM_LED_FAIL_OPTIONS_PERIOD_S) == 1) && (msec < MODE_AUTONOM_LED_FAIL_OPTIONS_DUTY2_MS)) AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB30; 
	//	else AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB30;
	//}
	//else
	//if((Power_Battery_Status_Get() == POWER_BATTERY_STATUS_ON) && (Power_Line_Status_Get() == POWER_LINE_STATUS_OFF))
	//{	// режим ожидания   ____x____________________x______________________x___
	// 	if(((sec % MODE_AUTONOM_LED_WAIT_PERIOD_S) == 0) && (msec < MODE_AUTONOM_LED_WAIT_DUTY_MS)) AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB30; 
	//	else AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB30;
	//}
	//else
	//if((Power_Battery_Status_Get() == POWER_BATTERY_STATUS_ON) && (Power_Line_Status_Get() == POWER_LINE_STATUS_ON))
	//{	// режим рабочий   xxxxxxx_xxxxxxx_xxxxxxx_xxxxxxx_xxxxxxx_xxxxxxx_
	// 	if(((sec % MODE_AUTONOM_LED_WORK_PERIOD_S) == 0) && (msec < MODE_AUTONOM_LED_WORK_DUTY_MS)) AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB30; 
	//	else AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB30;
	//}
	//else
	//{ 	// что-то не так	xxxx____xxxx____xxxx____xxxx____xxxx____xxxx____xxxx____xxxx____xxxx____	
	// 	if(((sec % MODE_AUTONOM_LED_FAIL_POWER_PERIOD_S) == 0) && (msec < MODE_AUTONOM_LED_FAIL_POWER_DUTY_MS)) AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB30; 
	//	else AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB30;
	//}
}

/*********************************************************************/

void Mode_Autonom_Init()
{
	Mode_Autonom_Main_Init();
	Mode_Autonom_Special_Init();
	Mode_Autonom_Measure_Init();
	Mode_Autonom_Led_Init();
}

bool Mode_Autonom_Reset()
{
	bool reset = true;
	reset &= Mode_Autonom_Main_Reset();
	reset &= Mode_Autonom_Special_Reset();
	reset &= Mode_Autonom_Measure_Reset();
	reset &= Mode_Autonom_Led_Reset();
	return reset;
}

bool Mode_Autonom_Idle()
{
	Mode_Autonom_Main_Idle();
	Mode_Autonom_Special_Idle();
	Mode_Autonom_Measure_Idle();
	Mode_Autonom_Led_Idle();
	return true;
}


