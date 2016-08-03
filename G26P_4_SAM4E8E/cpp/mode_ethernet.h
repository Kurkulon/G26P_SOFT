#ifndef MEMORY_AT91SAM7X256_MODE_ETHERNET_H
#define MEMORY_AT91SAM7X256_MODE_ETHERNET_H


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define MODE_ETHERNET_CLOCK_SEND_MAIN_PERIOD_MS		500	// если больше секунды, менять алгоритм
//#define MODE_ETHERNET_FLASH_SEND_STATUS_PERIOD_MS	250	// если больше секунды, менять алгоритм
//#define MODE_ETHERNET_POWER_SEND_MAIN_PERIOD_MS		250	// если больше секунды, менять алгоритм
//#define MODE_ETHERNET_POWER_SEND_STATUS_PERIOD_MS	500	// если больше секунды, менять алгоритм
//#define MODE_ETHERNET_SENSORS_SEND_MAIN_PERIOD_MS	250	// если больше секунды, менять алгоритм

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//typedef enum __attribute__ ((packed)) 
//{
//	MODE_ETHERNET_FLASH_STATE_NONE = 0,
//	MODE_ETHERNET_FLASH_READ_SESSION_STATE_WAIT,
//	MODE_ETHERNET_FLASH_READ_SESSION_STATE_IDLE,
//	MODE_ETHERNET_FLASH_READ_SESSION_STATE_READY,
//	MODE_ETHERNET_FLASH_READ_SESSION_STATE_ERROR,
//	MODE_ETHERNET_FLASH_READ_VECTOR_STATE_WAIT,
//	MODE_ETHERNET_FLASH_READ_VECTOR_STATE_IDLE,
//	MODE_ETHERNET_FLASH_READ_VECTOR_STATE_READY,
//	MODE_ETHERNET_FLASH_READ_VECTOR_STATE_ERROR,
//} mode_ethernet_flash_state_type;
//
//
//typedef enum __attribute__ ((packed)) 
//{
//	MODE_ETHERNET_FLASH_COMMAND_NONE = 0,
//	MODE_ETHERNET_FLASH_COMMAND_STOP,
//	MODE_ETHERNET_FLASH_COMMAND_PAUSE,
//	MODE_ETHERNET_FLASH_COMMAND_RESUME,
//	MODE_ETHERNET_FLASH_COMMAND_ERASE,
//	MODE_ETHERNET_FLASH_COMMAND_UNERASE,
//	MODE_ETHERNET_FLASH_COMMAND_READ_SESSION,
//	MODE_ETHERNET_FLASH_COMMAND_READ_VECTOR,
//} mode_ethernet_flash_command_type;
//
//typedef enum __attribute__ ((packed)) 
//{
//	MODE_ETHERNET_FLASH_STATUS_NONE = 0,
//	MODE_ETHERNET_FLASH_STATUS_BUSY,	 // занят неизвестной для компьютера операцией (например записью в память)
//	MODE_ETHERNET_FLASH_STATUS_STOP,
//	MODE_ETHERNET_FLASH_STATUS_PAUSE,
//	MODE_ETHERNET_FLASH_STATUS_RESUME,
//	MODE_ETHERNET_FLASH_STATUS_ERASE,
//	MODE_ETHERNET_FLASH_STATUS_UNERASE,
//	MODE_ETHERNET_FLASH_STATUS_READ_SESSION_START,
//	MODE_ETHERNET_FLASH_STATUS_READ_SESSION_IDLE,
//	MODE_ETHERNET_FLASH_STATUS_READ_SESSION_FIND, // поиск вектора по несвязанной области
//	MODE_ETHERNET_FLASH_STATUS_READ_SESSION_ERROR,
//	MODE_ETHERNET_FLASH_STATUS_READ_SESSION_READY,
//	MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_START,
//	MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_IDLE,
//	MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_FIND, // поиск вектора по несвязанной области
//	MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_ERROR,
//	MODE_ETHERNET_FLASH_STATUS_READ_VECTOR_READY,
//} mode_ethernet_flash_status_type;
//
//
///*****************************************************************************/
//
//extern bool Mode_Ethernet_Idle();
//extern void Mode_Ethernet_Init();
//extern bool Mode_Ethernet_Reset();
//
//extern bool Mode_Ethernet_Flash_Read_Session_Start();
//extern bool Mode_Ethernet_Flash_Read_Vector_Start(unsigned short session, long long last_adress);
//extern bool Mode_Ethernet_Flash_Stop();
//extern bool Mode_Ethernet_Flash_Pause();
//extern bool Mode_Ethernet_Flash_Resume();
//extern bool Mode_Ethernet_Flash_Erase();
//extern bool Mode_Ethernet_Flash_UnErase();




#endif
