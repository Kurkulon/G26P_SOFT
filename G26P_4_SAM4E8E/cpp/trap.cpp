#include <stdlib.h>

#include "common.h"
#include "trap.h"
#include "main.h"
#include "power.h"
#include "fram.h"
#include "flash.h"
#include "sensors.h"
#include "emac.h"
#include "mode_online.h"
#include "mode_ethernet.h"
#include "bootloader.h"

#include "trap_def.h"



#pragma diag_suppress 2548,546


char TrapTxDataBuffer[TRAP_TX_DATA_BUFFER_SIZE];

u32 TrapRxCounter;
u32 TrapTxCounter;
u32 TrapRxLost;

/******************************************************/
static void TRAP_MakePacketHeaders(char *data, bool need_ask, bool is_ask, char device);
static void MakePacketHeaders(TrapHdr *p, bool need_ask, bool is_ask, char device);

/*********************** Info *************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_INFO_SendError(u32 error)
{
	TrapError *trap = (TrapError*)TrapTxDataBuffer;
	
	MakePacketHeaders(&trap->hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);

	trap->cmd = TRAP_INFO_COMMAND_ERROR;
	trap->error = error;
	EMAC_SendData((char *)TrapTxDataBuffer, sizeof(TrapError));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_INFO_SendCaptureIP(u32 old_ip, u16 old_port)
{
	TrapIP *trap = (TrapIP*)TrapTxDataBuffer;

	MakePacketHeaders(&trap->hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);
	trap->cmd = TRAP_INFO_CAPTURE_IP;
	trap->ip = old_ip;
	trap->port = old_port;

	EMAC_SendData((char *)TrapTxDataBuffer, sizeof(TrapIP));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_INFO_SendLostIP(u32 new_ip, u16 new_port)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_INFO_LOST_IP;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_INFO_ip_type ip;	
	ip.ip = new_ip;
	ip.port = new_port;
	COPY((char *)(&ip.ip), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(TRAP_INFO_ip_type));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_INFO_ip_type));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_INFO_SendInfo()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_INFO_COMMAND_INFO;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_INFO_info_type info;	
	info.version = VERSION;
	info.number = FRAM_Main_Device_Number_Get();
	info.memory_mask = FLASH_Chip_Mask_Get();
	info.memory_size = FLASH_Full_Size_Get();
	info.devices_mask = TRAP_DEVICES_MASK;
	info.device_type = FRAM_Main_Device_Type_Get();
	info.device_telemetry = FRAM_Main_Device_Telemetry_Get();
	COPY((char *)(&info), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(TRAP_INFO_info_type));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_INFO_info_type));

}

/******************** CLOCK ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_CLOCK_SendMain(RTC_type rtc)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_CLOCK_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_CLOCK_COMMAND_MAIN;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_CLOCK_main_type m;
	m.rtc = rtc;	
	COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
}

/******************** TRACE ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_TRACE_SendData(char *pData, u32 size)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_TRACE_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_TRACE_COMMAND_MAIN;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	COPY(pData, (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), size);
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + size);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_TRACE_PrintString(char *data)
{
	byte i = 0;
	while((i < 0xFF) && (data[i] != '\0')) i++; 
	TRAP_TRACE_SendData(data, i);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_TRACE_PrintChar(char data)
{
	TRAP_TRACE_SendData(&data, sizeof(data));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_TRACE_PrintDec(int number)
{
	char buf[11];
	byte i;
	u32 n = abs(number);
	for(i = 0; i < 11; i++)	
	{
	        buf[10 - i] = n - 10 * (n / 10) + 0x30;
		n /= 10;
	}
    	i = 0;
	while ((buf[i]=='0') && (i < 10)) i++;
	if(number < 0) 
	{
		i--;
		buf[i] = '-';
	}
	TRAP_TRACE_SendData(&buf[i], 11 - i);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_TRACE_PrintHex(u32 number)
{
	char buf[10] = { '0' , 'x' };
	byte i;
	for(i = 0; i < 8; i++)	
	{
	        buf[7 - i + 2] = number - 16*(number/16);
		if(buf[7 - i + 2] >= 10) buf[7 - i + 2] += 0x41 - 10; else buf[7 - i + 2] += 0x30;
		number /= 16;
	}
	TRAP_TRACE_SendData(buf, sizeof(buf));
}

/******************** MEMORY ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_MEMORY_SendInfo()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_MEMORY_COMMAND_INFO;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_MEMORY_info_type info;	
	info.mask = FLASH_Chip_Mask_Get();
	info.size = FLASH_Full_Size_Get();
	info.size_used = FLASH_Used_Size_Get();
	COPY((char *)(&info.mask), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(TRAP_MEMORY_info_type));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_info_type));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_MEMORY_SendStatus(u32 progress, byte status)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_MEMORY_COMMAND_STATUS;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_MEMORY_status_type s;	
	s.progress = progress;
	s.status = status;
	COPY((char *)(&s.progress), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(TRAP_MEMORY_status_type));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_status_type));
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_MEMORY_SendSession(u16 session, i64 size, i64 last_adress, RTC_type start_rtc, RTC_type stop_rtc, byte flags)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_MEMORY_COMMAND_SESSION;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_MEMORY_session_type s;	
	s.session = session;
	s.size = size;
	s.last_adress = last_adress;
	s.start_rtc = start_rtc;
	s.stop_rtc = stop_rtc;
	s.flags = flags;
	COPY((char *)(&s.session), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(TRAP_MEMORY_session_type));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_session_type));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_MEMORY_SendVector(u16 session, u16 device, RTC_type rtc, byte *data, u16 size, byte flags) 
{
	 // для скорости хорошо бы просто указать где лежит вектор, да тогда при прикреплении эзернет заголовка попортится сам вектор, а он нужен в дальнейшем, потому тупо копируем его
	if(TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_vector_type) + size > sizeof(TrapTxDataBuffer)) return false;
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_MEMORY_COMMAND_VECTOR;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_MEMORY_vector_type v;	
	v.session = session;
	v.device = device;
	v.rtc = rtc;
	v.flags = flags;
	memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), (char *)(&v.session), sizeof(TRAP_MEMORY_vector_type));
	memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_vector_type), (char *)data, size);
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_vector_type) + size);
	return true;
}

/******************** BATTERY ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_SendMainMessage(u16 cmd)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_BATTERY_DEVICE);
	TRAP_command_type c;
	c.command = cmd;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_BATTERY_main_type m;
	m.battery_voltage = Power_Battery_Voltage_Get();	
	m.line_voltage = Power_Line_Voltage_Get();
	m.status = Power_Status_Mask_Get();	
	m.battery_status = Power_Battery_Status_Get();	
	m.line_status = Power_Line_Status_Get();	
	COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_SendMain()
{
	TRAP_BATTERY_SendMainMessage(TRAP_BATTERY_COMMAND_MAIN);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_TakeMain()
{
	TRAP_BATTERY_SendMainMessage(TRAP_BATTERY_COMMAND_TAKE_MAIN);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_SendStatusMessage(u16 cmd)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_BATTERY_DEVICE);
	TRAP_command_type c;
	c.command = cmd;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_BATTERY_status_type s;
	FRAM_Power_Battery_Voltages_Get(&s.battery_setup_voltage, &s.battery_min_voltage, &s.battery_max_voltage);
	FRAM_Power_Line_Voltages_Get(&s.line_setup_voltage, &s.line_min_voltage, &s.line_max_voltage);
	FRAM_Power_Battery_Coeffs_Get(&s.battery_coeff_k, &s.battery_coeff_b);
	FRAM_Power_Line_Coeffs_Get(&s.line_coeff_k, &s.line_coeff_b);
	COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_SendStatus()
{
	TRAP_BATTERY_SendStatusMessage(TRAP_BATTERY_COMMAND_STATUS);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_TakeStatus()
{
	TRAP_BATTERY_SendStatusMessage(TRAP_BATTERY_COMMAND_TAKE_STATUS);
}


/******************** SENSORS ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_SENSORS_SendMainMessage(u16 cmd)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_SENSORS_DEVICE);
	TRAP_command_type c;
	c.command = cmd;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_SENSORS_main_type m;
	m.temperature_in = Sensors_Temperature_In_Get();
	m.ax = Sensors_Ax_Get();
	m.ay = Sensors_Ay_Get();
	m.az = Sensors_Az_Get();	
	COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_SENSORS_SendMain()
{
	TRAP_SENSORS_SendMainMessage(TRAP_SENSORS_COMMAND_MAIN);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_SENSORS_TakeMain()
{
	TRAP_SENSORS_SendMainMessage(TRAP_SENSORS_COMMAND_TAKE_MAIN);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_SENSORS_Take_A_Coeffs()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_SENSORS_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_SENSORS_COMMAND_TAKE_A_COEFFS;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_SENSORS_a_coeffs_type a;
	FRAM_Sensors_Ax_Coeffs_Get(&a.ax_coeff_k, &a.ax_coeff_b);
	FRAM_Sensors_Ay_Coeffs_Get(&a.ay_coeff_k, &a.ay_coeff_b);
	FRAM_Sensors_Az_Coeffs_Get(&a.az_coeff_k, &a.az_coeff_b);
	COPY((char *)(&a), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(a));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(a));
}

/******************** PROGRAMMING ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_PROGRAMMING_TakeInfo()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_PROGRAMMING_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_PROGRAMMING_COMMAND_TAKE_INFO;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_PROGRAMMING_info_type i;
	i.version = FRAM_Autonom_Version_Get();
	i.size = FRAM_Autonom_Size_Get();
	i.validation = FRAM_Autonom_Validation_Get();
	COPY((char *)(&i), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(i));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(i));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_PROGRAMMING_ReadBlock(u32 offset, u32 size)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_PROGRAMMING_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_PROGRAMMING_COMMAND_TAKE_BLOCK;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_PROGRAMMING_block_type b;
	b.offset = offset;
	b.size = size;
	COPY((char *)(&b), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(b));
	FRAM_Autonom_Read_Block(offset, size, (byte *)((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type)));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(b) +size);
}

/******************** VECTOR ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_VECTOR_SendVector(u16 command, u32 time_ms, int depth, int speed, byte flags, u16 size, byte *data) 
{
	 // для скорости хорошо бы просто указать где лежит вектор, да тогда при прикреплении эзернет заголовка попортится сам вектор, а он нужен в дальнейшем, потому тупо копируем его
	if(TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_vector_type) + size > sizeof(TrapTxDataBuffer)) return false;
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_VECTOR_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_VECTOR_COMMAND_VECTOR;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_VECTOR_vector_type v;	
	v.command = command;
	v.time_ms = time_ms;
	v.depth = depth;
	v.speed = speed;
	v.size = size;
	v.flags = flags;
	memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), (char *)(&v), sizeof(TRAP_VECTOR_vector_type));
	memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_VECTOR_vector_type), (char *)data, size);
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_VECTOR_vector_type) + size);
	return true;
}

/******************** ONLINE ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_TakePeriod()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_ONLINE_COMMAND_TAKE_PERIOD;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_ONLINE_period_type p;
	p.period_ms = Mode_Online_Control_Period_MS_Get();
	COPY((char *)(&p), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(p));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(p));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_TakeIndex()
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_ONLINE_COMMAND_TAKE_INDEX;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_ONLINE_index_type i;
	i.device_index = Mode_Online_Control_Device_Index_Get();
	COPY((char *)(&i), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(i));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(i));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_SendStatus(byte status)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_ONLINE_COMMAND_SEND_STATUS;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_ONLINE_status_type s;
	s.status = status;
	COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
}

/******************** RDC ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_RDC_SendMain(u32 time_ms, int depth_sm, int speed_mh)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_RDC_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_RDC_COMMAND_SEND_MAIN;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_RDC_main_type m;
	m.time_ms = time_ms;
	m.depth_sm = depth_sm;
	m.speed_mh = speed_mh;
	COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_RDC_SendStatus(bool messaging, bool imitation)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_RDC_DEVICE);
	TRAP_command_type c;
	c.command = TRAP_RDC_COMMAND_SEND_STATUS;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	TRAP_RDC_status_type s;
	s.status = (messaging << 6) | (imitation << 7);
	COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/******************** Common ******************************/


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_SendAsknowlege(byte device, u32 on_packet)
{
	TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_IS_ASK, device);
	TRAP_command_type c;
	c.command = TRAP_COMMAND_ASKNOWLEGE;
	COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	COPY((char *)(&on_packet), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(u32));
	EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(u32));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_HandleRxData(char *data, u32 size)
{
	if(size < TRAP_RX_HEADERS_LEN) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); return; }
	TRAP_RX_PACKET_type *packet = (TRAP_RX_PACKET_type *)data;
	if((TrapRxCounter == 0)||(packet->counter<=1)) TrapRxLost = 0;
	else TrapRxLost += (int)((int)(packet->counter) - (int)TrapRxCounter - 1);	
	TrapRxCounter = packet->counter;
	bool need_ask = (((packet->status)>>3)&0x1);
	bool is_ask = (((packet->status)>>2)&0x1);
	byte version = packet->version;
	if(version > TRAP_PACKET_VERSION)
	{
		TRAP_INFO_SendError(TRAP_PACKET_ERROR_VERSION);
		return;
	}
	if(is_ask == TRAP_PACKET_NO_ASK)
	{
		TRAP_command_type *cmd = (TRAP_command_type *)(data + TRAP_RX_HEADERS_LEN);	
		switch (packet->device)
		{
			/*******************************************************************************/
		case TRAP_INFO_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_INFO_COMMAND_GET_INFO:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);					
				TRAP_INFO_SendInfo();
				break;
			case TRAP_INFO_COMMAND_SET_NUMBER:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
				FRAM_Main_Device_Number_Set(((TRAP_INFO_set_number_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->number);
				break;
			case TRAP_INFO_COMMAND_SET_TYPE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
				FRAM_Main_Device_Type_Set(((TRAP_INFO_set_type_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->type);
				break;
			case TRAP_INFO_COMMAND_SET_TELEMETRY:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
				FRAM_Main_Device_Telemetry_Set(((TRAP_INFO_set_telemetry_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->telemetry);
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_CLOCK_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_CLOCK_COMMAND_GET:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_CLOCK_DEVICE, TrapRxCounter);					
				TRAP_CLOCK_SendMain(RTC_Get());
				break;
			case TRAP_CLOCK_COMMAND_SET:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_CLOCK_DEVICE, TrapRxCounter);	
				RTC_Set(((TRAP_CLOCK_set_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->rtc);
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_MEMORY_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_MEMORY_COMMAND_GET_INFO:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				TRAP_MEMORY_SendInfo();
				break;
			case TRAP_MEMORY_COMMAND_READ_SESSION_START:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Read_Session_Start();
				break;
			case TRAP_MEMORY_COMMAND_READ_VECTOR_START:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Read_Vector_Start(((TRAP_MEMORY_start_read_vector_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->session, ((TRAP_MEMORY_start_read_vector_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->last_adress);
				break;
			case TRAP_MEMORY_COMMAND_STOP:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Stop();
				break;
			case TRAP_MEMORY_COMMAND_PAUSE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Pause();
				break;
			case TRAP_MEMORY_COMMAND_RESUME:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Resume();
				break;
			case TRAP_MEMORY_COMMAND_ERASE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_Erase();
				break;
			case TRAP_MEMORY_COMMAND_UNERASE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
				Mode_Ethernet_Flash_UnErase();
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_BOOTLOADER_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_BOOTLOADER_COMMAND_START:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BOOTLOADER_DEVICE, TrapRxCounter);
				BootLoader_Start_Delay();
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_BATTERY_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_BATTERY_COMMAND_GET_MAIN:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				TRAP_BATTERY_TakeMain();
				break;
			case TRAP_BATTERY_COMMAND_GET_STATUS:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				TRAP_BATTERY_TakeStatus();
				break;
			case TRAP_BATTERY_COMMAND_SET_BATTERY_COEFFS:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				FRAM_Power_Battery_Coeffs_Set((float *)(&((TRAP_BATTERY_set_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->coeff_k), (float *)(&((TRAP_BATTERY_set_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->coeff_b));
				break;
			case TRAP_BATTERY_COMMAND_SET_LINE_COEFFS:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				FRAM_Power_Line_Coeffs_Set((float *)(&((TRAP_BATTERY_set_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->coeff_k), (float *)(&((TRAP_BATTERY_set_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->coeff_b));
				break;
			case TRAP_BATTERY_COMMAND_SET_BATTERY_VOLTAGES:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				FRAM_Power_Battery_Voltages_Set(((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->setup_voltage, ((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->min_voltage, ((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->max_voltage);
				break;
			case TRAP_BATTERY_COMMAND_SET_LINE_VOLTAGES:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				FRAM_Power_Line_Voltages_Set(((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->setup_voltage, ((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->min_voltage, ((TRAP_BATTERY_set_voltages_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->max_voltage);
				break;
			case TRAP_BATTERY_COMMAND_SWITCH_ON:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				Power_Switch_Set(true);
				break;
			case TRAP_BATTERY_COMMAND_SWITCH_OFF:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				Power_Switch_Set(false);
				break;
			case TRAP_BATTERY_COMMAND_MAIN_ENABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				Power_Transmission_Set(true);
				break;
			case TRAP_BATTERY_COMMAND_MAIN_DISABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
				Power_Transmission_Set(false);
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_SENSORS_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_SENSORS_COMMAND_GET_MAIN:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);
				TRAP_SENSORS_TakeMain();
				break;
			case TRAP_SENSORS_COMMAND_GET_A_COEFFS:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);
				TRAP_SENSORS_Take_A_Coeffs();
				break;
			case TRAP_SENSORS_COMMAND_SET_A_COEFFS:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);
				FRAM_Sensors_Ax_Coeffs_Set((float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->ax_coeff_k), (float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->ax_coeff_b));
				FRAM_Sensors_Ay_Coeffs_Set((float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->ay_coeff_k), (float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->ay_coeff_b));
				FRAM_Sensors_Az_Coeffs_Set((float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->az_coeff_k), (float *)(&((TRAP_SENSORS_a_coeffs_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->az_coeff_b));
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_PROGRAMMING_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_PROGRAMMING_COMMAND_GET_INFO:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
				TRAP_PROGRAMMING_TakeInfo();
				break;
			case TRAP_PROGRAMMING_COMMAND_WRITE_BEGIN:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
				FRAM_Autonom_Write_Begin();
				break;
			case TRAP_PROGRAMMING_COMMAND_WRITE_END:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
				FRAM_Autonom_Write_End();
				break;
			case TRAP_PROGRAMMING_COMMAND_WRITE_BLOCK:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
				FRAM_Autonom_Write_Block(((TRAP_PROGRAMMING_block_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->offset, ((TRAP_PROGRAMMING_block_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->size, ((byte *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_PROGRAMMING_block_type))));
				break;
			case TRAP_PROGRAMMING_COMMAND_READ_BLOCK:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
				TRAP_PROGRAMMING_ReadBlock(((TRAP_PROGRAMMING_block_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->offset, ((TRAP_PROGRAMMING_block_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->size);
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_VECTOR_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_VECTOR_COMMAND_ENABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_VECTOR_DEVICE, TrapRxCounter);
				Mode_Online_Main_Enable();
				break;
			case TRAP_VECTOR_COMMAND_DISABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_VECTOR_DEVICE, TrapRxCounter);
				Mode_Online_Main_Disable();
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_ONLINE_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }

			switch (cmd->command)
			{
			case TRAP_ONLINE_COMMAND_GET_PERIOD:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_TakePeriod();
				break;
			case TRAP_ONLINE_COMMAND_SET_PERIOD:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_SendStatus(Mode_Online_Control_Period_MS_Set(((TRAP_ONLINE_period_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->period_ms));
				break;
			case TRAP_ONLINE_COMMAND_BEGIN:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				Mode_Online_Control_Begin();
				break;
			case TRAP_ONLINE_COMMAND_CANCEL:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				Mode_Online_Control_Cancel();
				break;
			case TRAP_ONLINE_COMMAND_END:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				Mode_Online_Control_End();
				break;
			case TRAP_ONLINE_COMMAND_SET_DEVICE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_set_device_type *set_device = (TRAP_ONLINE_set_device_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
				TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Add(set_device->delay_ms, set_device->period_min_ms, set_device->command_count));
				break;
			case TRAP_ONLINE_COMMAND_SET_COMMAND:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_set_command_type *set_command = (TRAP_ONLINE_set_command_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
				TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Add(Mode_Online_Control_Device_Index_Get(), set_command->command_index, set_command->telemetry, set_command->mode, set_command->offset_ms, set_command->tx_flags, set_command->tx_freq_hz, set_command->tx_size, set_command->rx_flags, set_command->rx_freq_hz, set_command->rx_timeout_mks, set_command->rx_pause_mks, set_command->rx_size, set_command->tx_data));
				break;
			case TRAP_ONLINE_COMMAND_ADD_COMMAND:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_add_command_type *add_command = (TRAP_ONLINE_add_command_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
				TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Add(add_command->device_index, add_command->command_index, add_command->telemetry, add_command->mode, add_command->offset_ms, add_command->tx_flags, add_command->tx_freq_hz, add_command->tx_size, add_command->rx_flags, add_command->rx_freq_hz, add_command->rx_timeout_mks, add_command->rx_pause_mks, add_command->rx_size, add_command->tx_data));
				break;
			case TRAP_ONLINE_COMMAND_REMOVE_COMMAND:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_remove_command_type *remove_command = (TRAP_ONLINE_remove_command_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
				TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Remove(remove_command->device_index, remove_command->command_index));
				break;
			case TRAP_ONLINE_COMMAND_GET_INDEX:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
				TRAP_ONLINE_TakeIndex();
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		case TRAP_RDC_DEVICE:
			if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); break; }
			switch (cmd->command)
			{
			case TRAP_RDC_COMMAND_IMITATION_ENABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Imitation_Enable(((TRAP_RDC_imitation_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm, ((TRAP_RDC_imitation_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->speed_mh);
				break;
			case TRAP_RDC_COMMAND_IMITATION_DISABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Imitation_Disable();
				break;
			case TRAP_RDC_COMMAND_SET_DEPTH:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Depth_Set(((TRAP_RDC_depth_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm);
				break;
			case TRAP_RDC_COMMAND_CHANGE_DEPTH:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Depth_Set(Mode_Online_RDC_Depth_Get() + ((TRAP_RDC_depth_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm);
				break;
			case TRAP_RDC_COMMAND_MESSAGING_ENABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Messaging_Enable();
				break;
			case TRAP_RDC_COMMAND_MESSAGING_DISABLE:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Messaging_Disable();
				break;
			case TRAP_RDC_COMMAND_TIME_RESET:
				if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
				Mode_Online_RDC_Time_Reset();
				break;
			default: 
				TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
				break;
			}
			break;
			/*******************************************************************************/
		default:
			TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			break;

		}
	}
	else 
	{
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void EMAC_HandleRxError()
{
	TRAP_INFO_SendError(TRAP_PACKET_ERROR_CHECKSUM);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void MakePacketHeaders(TrapHdr *p, bool need_ask, bool is_ask, char device)
{
//	TRAP_TX_PACKET_type p;
	p->counter = TrapTxCounter++;
	p->errors = (u16)TrapRxLost;
	p->version = TRAP_PACKET_VERSION;
	p->status = ((is_ask&0x1)<<2) | ((need_ask&0x1)<<3);
	p->device = device;
//        COPY((char *)(&p.counter), (char *)data, sizeof(TRAP_TX_PACKET_type));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void TRAP_MakePacketHeaders(char *data, bool need_ask, bool is_ask, char device)
{
	TRAP_TX_PACKET_type p;
	p.counter = TrapTxCounter++;
	p.errors = (u16)TrapRxLost;
	p.version = TRAP_PACKET_VERSION;
	p.status = ((is_ask&0x1)<<2) | ((need_ask&0x1)<<3);
	p.device = device;
        COPY((char *)(&p.counter), (char *)data, sizeof(TRAP_TX_PACKET_type));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_Init()
{
	TrapRxCounter = 0;
	TrapTxCounter = 0;
	TrapRxLost = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_Idle()
{
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

