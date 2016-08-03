//#include "common.h"
//#include "main.h"
//#include "fram.h"
//#include "flash.h"
//#include "trap.h"
//#include "rtc.h"
//#include "power.h"
//#include "mode_ethernet.h"
//#include "vector.h"
//
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//unsigned short mode_ethernet_clock_main_period;
//
//void Mode_Ethernet_Clock_Idle()
//{
//	unsigned short ms = RTC_Get_Millisecond();
//	unsigned short temp = (ms + 1000 - mode_ethernet_clock_main_period) % 1000;
//	if(temp >= MODE_ETHERNET_CLOCK_SEND_MAIN_PERIOD_MS)
//	{	
//		mode_ethernet_clock_main_period = ms;
//	       	TRAP_CLOCK_SendMain(/*RTC_Get()*/);
//	}
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//short mode_ethernet_power_main_period;
//short mode_ethernet_power_status_period;
//
//void Mode_Ethernet_Power_Idle()
//{
//	if(!Power_Transmission_Get()) return;
//	unsigned short ms = RTC_Get_Millisecond();
//	unsigned short temp;
//	temp = (ms + 1000 - mode_ethernet_power_main_period) % 1000;
//	if(temp >= MODE_ETHERNET_POWER_SEND_MAIN_PERIOD_MS)
//	{	
//		mode_ethernet_power_main_period = ms;
//		TRAP_BATTERY_SendMain();
//	}
//	temp = (ms + 1000 - mode_ethernet_power_status_period) % 1000;
//	if(temp >= MODE_ETHERNET_POWER_SEND_STATUS_PERIOD_MS)
//	{	
//		mode_ethernet_power_status_period = ms;
//		TRAP_BATTERY_SendStatus();
//	}
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//short mode_ethernet_sensors_main_period;
//
//void Mode_Ethernet_Sensors_Idle()
//{
//	unsigned short ms = RTC_Get_Millisecond();
//	unsigned short temp;
//	temp = (ms + 1000 - mode_ethernet_sensors_main_period) % 1000;
//	if(temp >= MODE_ETHERNET_SENSORS_SEND_MAIN_PERIOD_MS)
//	{	
//		mode_ethernet_sensors_main_period = ms;
//		TRAP_SENSORS_SendMain();
//	}
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//short 		mode_ethernet_flash_status_period;
//
//bool 			mode_ethernet_flash_read_vector_ready = false;
//unsigned short 		mode_ethernet_flash_read_vector_size = 0;
//unsigned char 		*mode_ethernet_flash_read_vector;
//
//long long 		mode_ethernet_flash_read_vector_adress;
//long long 		mode_ethernet_flash_read_vector_last_adress;
//long long 		mode_ethernet_flash_read_vector_first_adress;
//RTC_type 		mode_ethernet_flash_read_vector_last_rtc;
//unsigned short 		mode_ethernet_flash_read_vector_last_size = 0;
//
//unsigned short 		mode_ethernet_flash_read_vector_session_need;
//long long 		mode_ethernet_flash_read_vector_adress_need;
//
//mode_ethernet_flash_state_type 	mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//
//unsigned int 			mode_ethernet_flash_progress = 0;
//bool 				mode_ethernet_flash_pause = false;
//mode_ethernet_flash_status_type mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_NONE;
//mode_ethernet_flash_status_type mode_ethernet_flash_status_pre = MODE_ETHERNET_FLASH_STATUS_NONE;
//
//mode_ethernet_flash_command_type mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Read_Session_Start()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_READ_SESSION;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Read_Vector_Start(unsigned short session, long long last_adress)
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_READ_VECTOR;
//	mode_ethernet_flash_read_vector_session_need = session;
//	mode_ethernet_flash_read_vector_adress_need = last_adress;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Stop()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_STOP;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Pause()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_PAUSE;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Resume()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_RESUME;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Erase()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_ERASE;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_UnErase()
//{
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_UNERASE;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Flash_Idle()
//{
//	// отправка статусов
//	unsigned short ms = RTC_Get_Millisecond();
//	unsigned short temp = (ms + 1000 - mode_ethernet_flash_status_period) % 1000;
//	if((mode_ethernet_flash_status_pre != mode_ethernet_flash_status) || (temp >= MODE_ETHERNET_FLASH_SEND_STATUS_PERIOD_MS))
//	{	
//		mode_ethernet_flash_status_period = ms;;
//		mode_ethernet_flash_status_pre = mode_ethernet_flash_status;
//		TRAP_MEMORY_SendStatus(mode_ethernet_flash_progress, mode_ethernet_flash_status);
//	}
//
//	// if по причине смены mode_ethernet_flash_state, потому и не switch
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_SESSION_STATE_IDLE)
//	{
//		if(mode_ethernet_flash_read_vector_ready)
//		{
//			mode_ethernet_flash_read_vector_ready = false;
//
//			long long handle_size_pre = FRAM_Memory_Current_Adress_Get() - mode_ethernet_flash_read_vector_adress;
//			if(handle_size_pre < 0) handle_size_pre += FLASH_Full_Size_Get();
//			long long need_size = FLASH_Full_Size_Get();
//			if(FRAM_Memory_Start_Adress_Get() != -1) // full
//			{
//				need_size = FRAM_Memory_Current_Adress_Get() - FRAM_Memory_Start_Adress_Get();
//				if(need_size < 0) need_size += FLASH_Full_Size_Get();
//				if(need_size > FLASH_Full_Size_Get()) need_size -= FLASH_Full_Size_Get();
//			} 
//			if(need_size != 0)
//			{
//				if(mode_ethernet_flash_read_vector_size != 0)
//				{
//					if(Vector_Get_Adress_Session(mode_ethernet_flash_read_vector) == -1)
//					{
//						long long session_size = mode_ethernet_flash_read_vector_last_adress - mode_ethernet_flash_read_vector_adress + mode_ethernet_flash_read_vector_last_size;
//						if(session_size < 0) session_size += FLASH_Full_Size_Get();
//						if(session_size > FLASH_Full_Size_Get()) session_size -= FLASH_Full_Size_Get();
//						if(mode_ethernet_flash_read_vector_adress == Vector_Get_Adress_Vector(mode_ethernet_flash_read_vector))
//						{
//							mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_ERROR;
//							mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_ERROR;
//						}
//						else
//						{
//							mode_ethernet_flash_read_vector_adress = Vector_Get_Adress_Vector(mode_ethernet_flash_read_vector);
//							RTC_type start_rtc;
//							Vector_Get_RTC(mode_ethernet_flash_read_vector, &start_rtc);
//							TRAP_MEMORY_SendSession(Vector_Get_ID_Session(mode_ethernet_flash_read_vector), session_size, mode_ethernet_flash_read_vector_last_adress , start_rtc, mode_ethernet_flash_read_vector_last_rtc, Vector_Get_Flags(mode_ethernet_flash_read_vector));
//							mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_IDLE;
//							mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT;
//						}
//					}
//					else
//					{
//						mode_ethernet_flash_read_vector_last_adress = mode_ethernet_flash_read_vector_adress;
//						mode_ethernet_flash_read_vector_last_size = mode_ethernet_flash_read_vector_size;
//						if(mode_ethernet_flash_read_vector_adress == Vector_Get_Adress_Session(mode_ethernet_flash_read_vector))
//						{
//							mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_ERROR;
//							mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_ERROR;
//						}
//						else
//						{
//							mode_ethernet_flash_read_vector_adress = Vector_Get_Adress_Session(mode_ethernet_flash_read_vector);
//							RTC_type start_rtc;
//							start_rtc.time = 0;
//							start_rtc.date = 0;
//							Vector_Get_RTC(mode_ethernet_flash_read_vector, &mode_ethernet_flash_read_vector_last_rtc);
//							TRAP_MEMORY_SendSession(Vector_Get_ID_Session(mode_ethernet_flash_read_vector), 0, mode_ethernet_flash_read_vector_last_adress , start_rtc, mode_ethernet_flash_read_vector_last_rtc, Vector_Get_Flags(mode_ethernet_flash_read_vector));
//							mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_IDLE;
//							mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT;
//						}
//					}
//				} 
//				else 
//				{
//					mode_ethernet_flash_read_vector_adress--;
//
//					if(mode_ethernet_flash_read_vector_adress < 0) mode_ethernet_flash_read_vector_adress += FLASH_Full_Size_Get();
//					mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_FIND;
//					mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT;
//				}
//			}
//
//			long long handle_size = FRAM_Memory_Current_Adress_Get() - mode_ethernet_flash_read_vector_adress;
//			if(handle_size < 0) handle_size += FLASH_Full_Size_Get();
//			if((handle_size < handle_size_pre) || (handle_size > need_size))
//			{
//				mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_READY;
//				mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_READY;
//			} 
//			else 
//			{
//				if((need_size / 0x100) > 0) mode_ethernet_flash_progress = (handle_size * 0x1000000) / (need_size / 0x100);
//			}
//		}
//	}
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_SESSION_STATE_READY)
//	{
//		mode_ethernet_flash_progress = 0xFFFFFFFF;
//		mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_READY;
//		mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//	}
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_SESSION_STATE_ERROR)
//	{
//		mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_ERROR;
//		mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//	}
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_VECTOR_STATE_IDLE)
//	{
//		if(mode_ethernet_flash_read_vector_ready)
//		{
//			mode_ethernet_flash_read_vector_ready = false;
//			long long handle_size = mode_ethernet_flash_read_vector_adress_need - mode_ethernet_flash_read_vector_adress;
//			if(handle_size < 0) handle_size += FLASH_Full_Size_Get();                                                   
//			long long need_max_size = FLASH_Full_Size_Get();
//			if(FRAM_Memory_Start_Adress_Get() != -1) // full
//			{
//				need_max_size = FRAM_Memory_Current_Adress_Get() - FRAM_Memory_Start_Adress_Get();
//				if(need_max_size < 0) need_max_size += FLASH_Full_Size_Get();
//			} 
//			
//			if(need_max_size > 0)
//			{
//				if(mode_ethernet_flash_read_vector_size != 0)
//				{
//					mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_IDLE;
//					mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT;
//					if((mode_ethernet_flash_read_vector_adress == mode_ethernet_flash_read_vector_adress_need) && (Vector_Get_ID_Session(mode_ethernet_flash_read_vector) != mode_ethernet_flash_read_vector_session_need))
//					{
//						mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_ERROR;
//						mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_ERROR;
//					}
//					else
//						if(Vector_Get_ID_Session(mode_ethernet_flash_read_vector) == mode_ethernet_flash_read_vector_session_need)	
//						{
//							unsigned char *data_vector;
//
//							unsigned short size_vector = Vector_Get_Data(mode_ethernet_flash_read_vector, &data_vector);
//
//							Vector_Get_RTC(mode_ethernet_flash_read_vector, &mode_ethernet_flash_read_vector_last_rtc);
//							TRAP_MEMORY_SendVector(Vector_Get_ID_Session(mode_ethernet_flash_read_vector), 
//								Vector_Get_ID_Device(mode_ethernet_flash_read_vector),
//								mode_ethernet_flash_read_vector_last_rtc,
//								data_vector,
//								size_vector, 
//								Vector_Get_Flags(mode_ethernet_flash_read_vector));
//							if(Vector_Get_Adress_Session(mode_ethernet_flash_read_vector) != -1)  // последний вектор сесси
//							{
//								mode_ethernet_flash_read_vector_first_adress = Vector_Get_Adress_Session(mode_ethernet_flash_read_vector);
//							}
//							mode_ethernet_flash_read_vector_adress = Vector_Get_Adress_Vector(mode_ethernet_flash_read_vector);
//						}
//						else
//						{
//							mode_ethernet_flash_read_vector_adress --;
//							if(mode_ethernet_flash_read_vector_adress < 0) mode_ethernet_flash_read_vector_adress += FLASH_Full_Size_Get();
//							mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_FIND;
//							mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT;
//						}
//				} 
//				else 
//				{
//					mode_ethernet_flash_read_vector_adress --;
//					if(mode_ethernet_flash_read_vector_adress < 0) mode_ethernet_flash_read_vector_adress += FLASH_Full_Size_Get();
//					mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_FIND;
//					mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT;
//				}
//			}
//			long long need_size = FLASH_Full_Size_Get();;
//			if(mode_ethernet_flash_read_vector_first_adress != -1)
//			{
//				need_size = mode_ethernet_flash_read_vector_adress_need - mode_ethernet_flash_read_vector_first_adress;
//				if(need_size < 0) need_size += FLASH_Full_Size_Get();
//			}
//			if(need_max_size < need_size) need_size = need_max_size;
//			if(handle_size >= need_size)
//			{
//				mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_READY;
//				mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_READY;
//			} 
//			else 
//			{
//				mode_ethernet_flash_progress = (handle_size * 0x1000000) / (need_size / 0x100);
//			}
//		}
//	}
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_VECTOR_STATE_READY)
//	{
//		mode_ethernet_flash_progress = 0xFFFFFFFF;
//		mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_READY;
//		mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//	}
//
//	if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_VECTOR_STATE_ERROR)
//	{
//		mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_ERROR;
//		mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//	}
//
//	if((mode_ethernet_flash_state == MODE_ETHERNET_FLASH_STATE_NONE) || 
//		(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT) ||
//		(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT))
//	{
//		switch (mode_ethernet_flash_command)
//		{
//		case MODE_ETHERNET_FLASH_COMMAND_READ_SESSION:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_progress = 0;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_START;
//			mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT;
//			mode_ethernet_flash_read_vector_adress = FRAM_Memory_Current_Vector_Adress_Get();
//			mode_ethernet_flash_read_vector_last_adress = mode_ethernet_flash_read_vector_adress;
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_READ_VECTOR:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_progress = 0;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_START;
//			mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT;
//			mode_ethernet_flash_read_vector_adress = mode_ethernet_flash_read_vector_adress_need;
//			mode_ethernet_flash_read_vector_first_adress = -1;
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_STOP:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_progress = 0;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_STOP;
//			mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_PAUSE:
//			mode_ethernet_flash_pause = true;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_PAUSE;
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_RESUME:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_RESUME;
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_ERASE:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_progress = 0;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_ERASE;
//			mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//			FLASH_Erase_Full();
//			break;
//		case MODE_ETHERNET_FLASH_COMMAND_UNERASE:
//			mode_ethernet_flash_pause = false;
//			mode_ethernet_flash_command = MODE_ETHERNET_FLASH_COMMAND_NONE;
//			mode_ethernet_flash_progress = 0;
//			mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_UNERASE;
//			mode_ethernet_flash_state = MODE_ETHERNET_FLASH_STATE_NONE;
//			FLASH_UnErase_Full();
//			break;
//		}
//		if(mode_ethernet_flash_pause) mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_PAUSE;
//		else
//			if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT)
//			{
//				if(FLASH_Read_Vector(mode_ethernet_flash_read_vector_adress, &mode_ethernet_flash_read_vector_size, &mode_ethernet_flash_read_vector_ready, &mode_ethernet_flash_read_vector))
//				{
//					mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_IDLE;
//				}
//				else
//				{
//					mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_SESSION_ERROR;
//					mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_SESSION_STATE_ERROR;
//				}
//			}
//			else
//				if(mode_ethernet_flash_state == MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT)
//				{
//					if(FLASH_Read_Vector(mode_ethernet_flash_read_vector_adress, &mode_ethernet_flash_read_vector_size, &mode_ethernet_flash_read_vector_ready, &mode_ethernet_flash_read_vector))
//					{
//						mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_IDLE;
//					}
//					else
//					{
//						mode_ethernet_flash_status = MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_ERROR;
//						mode_ethernet_flash_state = MODE_ETHERNET_FLASH_READ_VECTOR_STATE_ERROR;
//					}
//				}
//	}
//	if((mode_ethernet_flash_progress == 0) || (mode_ethernet_flash_progress == 0xFFFFFFFF)) return false;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Idle()
//{
//	if(!Mode_Ethernet_Flash_Idle()) // в момент считывания памяти не грузим траффик
//	{	
//		Mode_Ethernet_Clock_Idle();
//		Mode_Ethernet_Power_Idle();
//		Mode_Ethernet_Sensors_Idle();
//		return false;
//	}
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//void Mode_Ethernet_Init()
//{
//
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//bool Mode_Ethernet_Reset()
//{
//	if((mode_ethernet_flash_state != MODE_ETHERNET_FLASH_STATE_NONE) &&
//	   (mode_ethernet_flash_state != MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT) &&
//	   (mode_ethernet_flash_state != MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT)) return false;
//	if(mode_ethernet_flash_command != MODE_ETHERNET_FLASH_COMMAND_NONE) return false;
//	return true;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
