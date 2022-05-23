#ifndef TRAP_DEF_H__28_01_2016__15_08
#define TRAP_DEF_H__28_01_2016__15_08

#include "rtc.h"
#include "emac.h"

#define TRAP_TX_DATA_BUFFER_SIZE	8400
/**************************************************************/
#define TRAP_PACKET_VERSION	0x2

#define TRAP_TRACE_DEVICE		'T'
#define TRAP_INFO_DEVICE		'I'
#define TRAP_CLOCK_DEVICE		'C'
#define TRAP_MEMORY_DEVICE		'O'
#define TRAP_BOOTLOADER_DEVICE	'L'
//#define TRAP_BATTERY_DEVICE		'B'
//#define TRAP_SENSORS_DEVICE		'S'
//#define TRAP_PROGRAMMING_DEVICE	'P'
//#define TRAP_VECTOR_DEVICE		'V'
//#define TRAP_ONLINE_DEVICE		'Y'
//#define TRAP_RDC_DEVICE			'G'
#define TRAP_DEVICES_MASK	((1u << (TRAP_TRACE_DEVICE - 'A')) | \
							(1u << (TRAP_INFO_DEVICE - 'A')) | \
							(1u << (TRAP_CLOCK_DEVICE - 'A')) | \
							(1u << (TRAP_MEMORY_DEVICE - 'A')) | \
							(1u << (TRAP_BOOTLOADER_DEVICE - 'A')))// | \
							//(1u << (TRAP_BATTERY_DEVICE - 'A')) | \
       //                     (1u << (TRAP_SENSORS_DEVICE - 'A')) | \
       //                     (1u << (TRAP_PROGRAMMING_DEVICE - 'A')) | \
       //                     (1u << (TRAP_VECTOR_DEVICE - 'A')) | \
       //                     (1u << (TRAP_ONLINE_DEVICE - 'A')) | \
							//(1u << (TRAP_RDC_DEVICE - 'A')))

#define TRAP_PACKET_ERROR_VERSION	0x7	// ошибка версии протокола
#define TRAP_PACKET_ERROR_CHECKSUM	0x6	// ошибка контрольной суммы
#define TRAP_PACKET_ERROR_UNKNOW	0x1	
#define TRAP_PACKET_ERROR_NONE		0x0	

#define TRAP_PACKET_NEED_ASK		0x1
#define TRAP_PACKET_NO_NEED_ASK		0x0
#define TRAP_PACKET_IS_ASK		0x1
#define TRAP_PACKET_NO_ASK		0x0

#define TRAPCMD(x,y) ((u16)((x<<8)|(y&0xFF)))

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapHdr
{
	u32		counter;
	u16		errors;
	byte	version;
	byte	status;
	byte	device;

	u16		cmd;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	Trap
{
	TrapHdr	hdr;
	byte	data[37];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	EthTrap
{
	EthUdp	eudp;
	Trap	trap;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapError
{
	TrapHdr	hdr;
	byte	error;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapAsk
{
	TrapHdr	hdr;
	u32		on_packet;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapIP
{
	TrapHdr	hdr;
	u32 ip;
	u16 port;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapInfo
{
	TrapHdr	hdr;
	u16 	version;	
	u16 	number;
	u16 	memory_mask;
	i64 	memory_size;
	u32 	devices_mask;
	byte 	device_type;
	byte 	device_telemetry;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapClock
{
	TrapHdr	hdr;
	RTC_type rtc;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapInfoSet
{
	TrapHdr	hdr;

	__packed union
	{
		u16 	number;
		byte 	type;
		byte 	telemetry;
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct TrapMemInfo
{
	TrapHdr	hdr;

	u16 mask;
	i64 size;
	i64 size_used;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct TrapMemStatus
{
	TrapHdr	hdr;

	u32		progress;
	byte	status;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SessionInfo
{
	u16			session;
	i64			size;		//если 0 то сессия немного порченная
	RTC_type	start_rtc;	//если 0 то сессия немного порченная
	RTC_type	stop_rtc;  
	i64			last_adress; 
	byte		flags;
//	u16			crc;
};	


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct TrapSession
{
	TrapHdr			hdr;

	SessionInfo		si;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct TrapVector
{
	TrapHdr			hdr;

	u16 session;
	u16 device;
	RTC_type rtc;
	byte flags;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapReadVector
{
	TrapHdr	hdr;
	u16 session;
	i64 last_adress;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapBattSetCoeffs
{
	TrapHdr	hdr;
	float coeff_k;
	float coeff_b;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapBattSetVolt
{
	TrapHdr	hdr;
	i16 setup_voltage;	//0.1V
	i16 min_voltage;	//0.1V
	i16 max_voltage;	//0.1V
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapSensCoeffs
{
	TrapHdr	hdr;
	float ax_coeff_k;
	float ax_coeff_b;
	float ay_coeff_k;
	float ay_coeff_b;
	float az_coeff_k;
	float az_coeff_b;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapProgBlock
{
	TrapHdr	hdr;
	u32 offset;
	u32 size;
	byte data[];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapOnlinePeriod
{
	TrapHdr	hdr;
	u32 period_ms;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapOnlineSetDevice
{
	TrapHdr	hdr;
	u32 delay_ms;
	u32 period_min_ms;
	byte command_count;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapOnlineSetCmd
{
	TrapHdr	hdr;
	byte command_index;
	byte telemetry;
	byte mode;  
	u16 offset_ms; 
	byte tx_flags;
	u32 tx_freq_hz;
	u16 tx_size;
	byte rx_flags;
	u32 rx_freq_hz;
	u32 rx_timeout_mks; 
	u16 rx_pause_mks;
	u16 rx_size;
	u16 tx_data[]; 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapOnlineAddCmd
{
	TrapHdr	hdr;

	byte device_index;
	byte command_index;
	byte telemetry;
	byte mode;  
	u16 offset_ms; 
	byte tx_flags;
	u32 tx_freq_hz;
	u16 tx_size;
	byte rx_flags;
	u32 rx_freq_hz;
	u32 rx_timeout_mks; 
	u16 rx_pause_mks;
	u16 rx_size;
	u16 tx_data[]; 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapOnlineRemoveCmd
{
	TrapHdr	hdr;

	byte device_index;
	byte command_index;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/************** Common header *****************************/

__packed struct TRAP_RX_PACKET_type
{
	u32 counter;
	u16 reserved;
	byte version;
	byte status;
    byte device;
};	

__packed struct	TRAP_TX_PACKET_type
{
	u32 counter;
	u16 errors;
	byte version;
	byte status;
    byte device;
};	

#define TRAP_RX_HEADERS_LEN (sizeof(TRAP_RX_PACKET_type))
#define TRAP_TX_HEADERS_LEN (sizeof(TRAP_TX_PACKET_type))

enum { TRAP_COMMAND_ASKNOWLEGE = TRAPCMD('A','A') };	// подтверждение

/***********************************************************/

__packed struct TRAP_command_type
{
	u16 command;
};	

/**************** TRACE *******************************/

enum { TRAP_TRACE_COMMAND_MAIN = TRAPCMD('T','M') };

/**************** CLOCK *******************************/

enum { TRAP_CLOCK_COMMAND_MAIN = TRAPCMD('T','T') };

__packed struct	TRAP_CLOCK_main_type
{
	RTC_type rtc;
} ;	

enum { TRAP_CLOCK_COMMAND_SET = ('S'<<8) + 'T' };

__packed struct	TRAP_CLOCK_set_type
{
	RTC_type rtc;
} ;	

enum { TRAP_CLOCK_COMMAND_GET = ('G'<<8) + 'T' };

/******* Info ******************************************************/
enum { TRAP_INFO_COMMAND_ERROR = ('E'<<8) + 'P' };	// Сообщение об ошибке в протоколе см. TRAP_PACKER_ERROR_xxxxx

__packed struct	TRAP_INFO_error_type
{
	byte error;
} ;	

enum { TRAP_INFO_CAPTURE_IP = ('C'<<8) + 'F' };	// Сообщение о захвате нового IP
enum { TRAP_INFO_LOST_IP = ('C'<<8) + 'L' };	// Сообщение о потере старого IP

__packed struct TRAP_INFO_ip_type
	{
		u32 ip;
		u16 port;
	} ;	

enum { TRAP_INFO_COMMAND_GET_INFO = ('G'<<8) + 'I' };	// запрос информации о составе станции и версии ПО
enum { TRAP_INFO_COMMAND_INFO = ('I'<<8) + 'G' };	// ответ на запрос информации о составе станции и версии ПО

__packed struct	TRAP_INFO_info_type
{
	u16 version;	
	u16 number;
	u16 memory_mask;
	i64 memory_size;
	u32 devices_mask;
	byte device_type;
	byte device_telemetry;
} ;	

enum { TRAP_INFO_COMMAND_SET_NUMBER = ('S'<<8) + 'N' };	

	__packed struct	TRAP_INFO_set_number_type
	{
		u16 number;
	} ;	

enum { TRAP_INFO_COMMAND_SET_TYPE = ('S'<<8) + 'C' };	

	__packed struct	TRAP_INFO_set_type_type
	{
		byte type;
	} ;	

enum { TRAP_INFO_COMMAND_SET_TELEMETRY = ('S'<<8) + 'T' };	

__packed struct	TRAP_INFO_set_telemetry_type
{
	byte telemetry;
} ;	

/**************** ПАМЯТЬ *******************************/

enum { TRAP_MEMORY_COMMAND_STATUS= ('T'<<8) + 'S' };
	__packed struct	TRAP_MEMORY_status_type
	{
		u32 progress;
		byte status;
	} ;	
enum { TRAP_MEMORY_COMMAND_GET_INFO = ('G'<<8) + 'I' };
enum { TRAP_MEMORY_COMMAND_INFO = ('T'<<8) + 'I' };
	__packed struct	TRAP_MEMORY_info_type
	{
		u16 mask;
		i64 size;
		i64 size_used;
	} ;	
enum { TRAP_MEMORY_COMMAND_READ_SESSION_START = ('E'<<8) + 'S' };
enum { TRAP_MEMORY_COMMAND_SESSION = ('T'<<8) + 'E' };
	__packed struct	TRAP_MEMORY_session_type
	{
		u16 session;
		i64 size; //если 0 то сессия немного порченная
		RTC_type start_rtc; //если 0 то сессия немного порченная
		RTC_type stop_rtc;  
		i64 last_adress; 
		byte flags;
	} ;	
enum { TRAP_MEMORY_COMMAND_READ_VECTOR_START = ('E'<<8) + 'V' };
	__packed struct	TRAP_MEMORY_start_read_vector_type
	{
		u16 session;
		i64 last_adress;
	} ;	

enum { TRAP_MEMORY_COMMAND_VECTOR = ('T'<<8) + 'V' };
	__packed struct	TRAP_MEMORY_vector_type
	{
		u16 session;
		u16 device;
		RTC_type rtc;
		byte flags;
	} ;	

enum { TRAP_MEMORY_COMMAND_STOP		= TRAPCMD('C','A') };
enum { TRAP_MEMORY_COMMAND_PAUSE	= TRAPCMD('P','A') };
enum { TRAP_MEMORY_COMMAND_RESUME	= TRAPCMD('R','E') };
enum { TRAP_MEMORY_COMMAND_ERASE	= TRAPCMD('E','R') };
enum { TRAP_MEMORY_COMMAND_UNERASE	= TRAPCMD('E','U') };

/*************** BOOTLOADER **************************************/
enum { TRAP_BOOTLOADER_COMMAND_START = ('B'<<8) + 'L' };	

/******* Battery ******************************************************/
enum { TRAP_BATTERY_COMMAND_MAIN = ('M'<<8) + 'M' };
enum { TRAP_BATTERY_COMMAND_GET_MAIN = ('G'<<8) + 'M' };
enum { TRAP_BATTERY_COMMAND_TAKE_MAIN = ('T'<<8) + 'M' };
	__packed struct	TRAP_BATTERY_main_type
	{
		i16 battery_voltage;	//0.1V
		i16 line_voltage;     //0.1V
		u32 status;
		byte battery_status;
		byte line_status;
	} ;	
enum { TRAP_BATTERY_COMMAND_STATUS = ('S'<<8) + 'S' };
enum { TRAP_BATTERY_COMMAND_GET_STATUS = ('G'<<8) + 'S' };
enum { TRAP_BATTERY_COMMAND_TAKE_STATUS = ('T'<<8) + 'S' };
	__packed struct	TRAP_BATTERY_status_type
	{
		i16 battery_setup_voltage;	//0.1V
		i16 battery_min_voltage;	//0.1V
		i16 battery_max_voltage;	//0.1V
		i16 line_setup_voltage;	//0.1V
		i16 line_min_voltage;	//0.1V
		i16 line_max_voltage;	//0.1V
		float battery_coeff_k;
		float battery_coeff_b;
		float line_coeff_k;
		float line_coeff_b;
	} ;	
enum { TRAP_BATTERY_COMMAND_SET_BATTERY_COEFFS = ('B'<<8) + 'C' };
enum { TRAP_BATTERY_COMMAND_SET_LINE_COEFFS = ('L'<<8) + 'C' };
	__packed struct	TRAP_BATTERY_set_coeffs_type
	{
		float coeff_k;
		float coeff_b;
	} ;	
enum { TRAP_BATTERY_COMMAND_SET_BATTERY_VOLTAGES = ('B'<<8) + 'V' };
enum { TRAP_BATTERY_COMMAND_SET_LINE_VOLTAGES = ('L'<<8) + 'V' };
	__packed struct	TRAP_BATTERY_set_voltages_type
	{
		i16 setup_voltage;	//0.1V
		i16 min_voltage;	//0.1V
		i16 max_voltage;	//0.1V
	} ;	
enum { TRAP_BATTERY_COMMAND_SWITCH_ON = ('O'<<8) + 'N' };
enum { TRAP_BATTERY_COMMAND_SWITCH_OFF = ('O'<<8) + 'F' };
enum { TRAP_BATTERY_COMMAND_MAIN_ENABLE = ('M'<<8) + 'E' };
enum { TRAP_BATTERY_COMMAND_MAIN_DISABLE = ('M'<<8) + 'D' };

/******* Sensors ******************************************************/
enum { TRAP_SENSORS_COMMAND_MAIN = ('D'<<8) + 'D' };
enum { TRAP_SENSORS_COMMAND_GET_MAIN = ('G'<<8) + 'D' };
enum { TRAP_SENSORS_COMMAND_TAKE_MAIN = ('T'<<8) + 'D' };
	__packed struct	TRAP_SENSORS_main_type
	{
		i16 temperature_in;  	//0.1gr
		i16 ax;               //0.01g
		i16 ay;               //0.01g
		i16 az;               //0.01g
	} ;	
enum { TRAP_SENSORS_COMMAND_GET_A_COEFFS = ('G'<<8) + 'C' };
enum { TRAP_SENSORS_COMMAND_TAKE_A_COEFFS = ('T'<<8) + 'C' };
enum { TRAP_SENSORS_COMMAND_SET_A_COEFFS = ('S'<<8) + 'C' };
	__packed struct	TRAP_SENSORS_a_coeffs_type
	{
		float ax_coeff_k;
		float ax_coeff_b;
		float ay_coeff_k;
		float ay_coeff_b;
		float az_coeff_k;
		float az_coeff_b;
	} ;	
/******* Sensors ******************************************************/
enum { TRAP_PROGRAMMING_COMMAND_GET_INFO = ('G'<<8) + 'D' };
enum { TRAP_PROGRAMMING_COMMAND_TAKE_INFO = ('T'<<8) + 'D' };
	__packed struct	TRAP_PROGRAMMING_info_type
	{
		byte version;
		u32 size;
		byte validation;
	} ;	
enum { TRAP_PROGRAMMING_COMMAND_WRITE_BEGIN = ('W'<<8) + 'B' };
enum { TRAP_PROGRAMMING_COMMAND_WRITE_END = ('W'<<8) + 'E' };
enum { TRAP_PROGRAMMING_COMMAND_WRITE_BLOCK = ('W'<<8) + 'W' };
enum { TRAP_PROGRAMMING_COMMAND_READ_BLOCK = ('R'<<8) + 'B' };
enum { TRAP_PROGRAMMING_COMMAND_TAKE_BLOCK = ('T'<<8) + 'B' };
	__packed struct	TRAP_PROGRAMMING_block_type
	{
		u32 offset;
		u32 size;
	} ;	
/******* Vector ******************************************************/
enum { TRAP_VECTOR_COMMAND_ENABLE = ('E'<<8) + 'T' };
enum { TRAP_VECTOR_COMMAND_DISABLE = ('D'<<8) + 'T' };
enum { TRAP_VECTOR_COMMAND_VECTOR = ('T'<<8) + 'V' };
	__packed struct	TRAP_VECTOR_vector_type
	{
		u16 command;
		u32 time_ms;
		int depth;
		int speed;
		byte flags;
		u16 size;
	} ;	
/******* Online ******************************************************/
enum { TRAP_ONLINE_COMMAND_SEND_STATUS = ('T'<<8) + 'S' };
	__packed struct	TRAP_ONLINE_status_type
	{
		byte status;
	} ;	

enum { TRAP_ONLINE_COMMAND_GET_PERIOD = ('G'<<8) + 'P' };
enum { TRAP_ONLINE_COMMAND_SET_PERIOD = ('S'<<8) + 'P' };
enum { TRAP_ONLINE_COMMAND_TAKE_PERIOD = ('T'<<8) + 'P' };

	__packed struct	TRAP_ONLINE_period_type
	{
		u32 period_ms;
	} ;	

enum { TRAP_ONLINE_COMMAND_BEGIN = ('B'<<8) + 'T' };
enum { TRAP_ONLINE_COMMAND_CANCEL = ('C'<<8) + 'T' };
enum { TRAP_ONLINE_COMMAND_END = ('E'<<8) + 'T' };
enum { TRAP_ONLINE_COMMAND_SET_DEVICE = ('S'<<8) + 'D' };

	__packed struct	TRAP_ONLINE_set_device_type
	{
		u32 delay_ms;
		u32 period_min_ms;
		byte command_count;
	} ;

enum { TRAP_ONLINE_COMMAND_SET_COMMAND = ('S'<<8) + 'C' };

	__packed struct	TRAP_ONLINE_set_command_type
	{
		byte command_index;
		byte telemetry;
		byte mode;  
		u16 offset_ms; 
		byte tx_flags;
		u32 tx_freq_hz;
		u16 tx_size;
		byte rx_flags;
		u32 rx_freq_hz;
		u32 rx_timeout_mks; 
		u16 rx_pause_mks;
		u16 rx_size;
		u16 tx_data[]; 
	} ;	

enum { TRAP_ONLINE_COMMAND_ADD_COMMAND = ('C'<<8) + 'O' };

	__packed struct	TRAP_ONLINE_add_command_type
	{
		byte device_index;
		byte command_index;
		byte telemetry;
		byte mode;  
		u16 offset_ms; 
		byte tx_flags;
		u32 tx_freq_hz;
		u16 tx_size;
		byte rx_flags;
		u32 rx_freq_hz;
		u32 rx_timeout_mks; 
		u16 rx_pause_mks;
		u16 rx_size;
		u16 tx_data[]; 
	} ;	

enum { TRAP_ONLINE_COMMAND_REMOVE_COMMAND = ('R'<<8) + 'C' };

	__packed struct	TRAP_ONLINE_remove_command_type
	{
		byte device_index;
		byte command_index;
	} ;	

enum { TRAP_ONLINE_COMMAND_GET_INDEX = ('G'<<8) + 'I' };
enum { TRAP_ONLINE_COMMAND_TAKE_INDEX = ('T'<<8) + 'I' };

	__packed struct	TRAP_ONLINE_index_type
	{
		byte device_index;
	} ;	

/******* Online ******************************************************/
enum { TRAP_RDC_COMMAND_SEND_MAIN = ('M'<<8) + 'M' };

	__packed struct	TRAP_RDC_main_type
	{
		u32 time_ms;
		int depth_sm;
		int speed_mh;
	} ;	

enum { TRAP_RDC_COMMAND_SEND_STATUS = ('M'<<8) + 'S' };

	__packed struct	TRAP_RDC_status_type
	{
		byte status;
	} ;	

enum { TRAP_RDC_COMMAND_IMITATION_ENABLE = ('I'<<8) + 'E' };
enum { TRAP_RDC_COMMAND_IMITATION_DISABLE = ('I'<<8) + 'D' };

	__packed struct	TRAP_RDC_imitation_type
	{
	 	int depth_sm;
		int speed_mh;
	} ;	

enum { TRAP_RDC_COMMAND_SET_DEPTH = ('S'<<8) + 'D' };
enum { TRAP_RDC_COMMAND_CHANGE_DEPTH = ('S'<<8) + 'C' };

	__packed struct	TRAP_RDC_depth_type
	{
	 	int depth_sm;
	} ;	

enum { TRAP_RDC_COMMAND_MESSAGING_ENABLE = ('M'<<8) + 'E' };
enum { TRAP_RDC_COMMAND_MESSAGING_DISABLE = ('M'<<8) + 'D' };
enum { TRAP_RDC_COMMAND_TIME_RESET = ('T'<<8) + 'R' };

#endif // TRAP_DEF_H__28_01_2016__15_08
