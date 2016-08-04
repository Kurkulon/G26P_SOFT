#include <stdarg.h>
#include <stdio.h>
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
#include "xtrap.h"
#include "CRC16.h"

#pragma diag_suppress 2548,546,550,177

enum trap_status
{
	TRAP_WAIT = 0,
	TRAP_SEND_SESSION,
	TRAP_SEND_VECTOR,
	TRAP_PAUSE_VECTOR
};

static trap_status trapStatus = TRAP_WAIT;


static const bool __trace = true;


char TrapTxDataBuffer[TRAP_TX_DATA_BUFFER_SIZE];

u32 TrapRxCounter;
u32 TrapTxCounter;
u32 TrapRxLost;

static bool stop = false;
static bool start = false;
static bool pause = false;


/******************************************************/
static void TRAP_MakePacketHeaders(char *data, bool need_ask, bool is_ask, char device);
static void MakePacketHeaders(TrapHdr *p, bool need_ask, bool is_ask, char device);

/*********************** Info *************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void StartSendVector()
{
	trapStatus = TRAP_SEND_VECTOR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_INFO_SendError(u32 error)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapError &trap = (TrapError&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);

	trap.hdr.cmd = TRAP_INFO_COMMAND_ERROR;
	trap.error = error;

	buf->len = sizeof(EthUdp) + sizeof(trap);
	
	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_INFO_SendCaptureIP(u32 old_ip, u16 old_port)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapIP &trap = (TrapIP&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);
	trap.hdr.cmd = TRAP_INFO_CAPTURE_IP;
	trap.ip = old_ip;
	trap.port = old_port;

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_INFO_SendLostIP(u32 new_ip, u16 new_port)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapIP &trap = (TrapIP&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);
	trap.hdr.cmd = TRAP_INFO_LOST_IP;
	trap.ip = new_ip;
	trap.port = new_port;

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_INFO_SendInfo()
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapInfo &trap = (TrapInfo&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_INFO_DEVICE);

	trap.hdr.cmd = TRAP_INFO_COMMAND_INFO;

	trap.version = VERSION;
	trap.number = FRAM_Main_Device_Number_Get();
	trap.memory_mask = FLASH_Chip_Mask_Get();
	trap.memory_size = FLASH_Full_Size_Get();
	trap.devices_mask = TRAP_DEVICES_MASK;
	trap.device_type = FRAM_Main_Device_Type_Get();
	trap.device_telemetry = FRAM_Main_Device_Telemetry_Get();
	
	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

/******************** CLOCK ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_CLOCK_SendMain(/*const RTC_type &rtc*/)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapClock &trap = (TrapClock&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_CLOCK_DEVICE);

	trap.hdr.cmd = TRAP_CLOCK_COMMAND_MAIN;
	
	GetTime(&trap.rtc);	

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

//	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

/******************** TRACE ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_TRACE_SendData(const char *pData, u32 size)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

//	Trap &trap = (TrapClock&)buf->th;

	MakePacketHeaders(&buf->th, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_TRACE_DEVICE);

	buf->th.cmd = TRAP_TRACE_COMMAND_MAIN;

	if (size > sizeof(buf->data))
	{
		size = sizeof(buf->data);
	};

	COPY(pData, (char*)buf->data, size);

	buf->len = sizeof(EthUdp) + sizeof(TrapHdr) + size;

	SendTrap(buf);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_TRACE_PrintString(const char *data, ...)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

//	Trap &trap = (TrapClock&)buf->th;

	MakePacketHeaders(&buf->th, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_TRACE_DEVICE);

	buf->th.cmd = TRAP_TRACE_COMMAND_MAIN;

	va_list arglist;

    va_start(arglist, data);
    
	int i = vsnprintf((char*)buf->data, sizeof(buf->data) - 2, data, arglist);

//	u16 i = sizeof(buf->data) - 2;


	if (i < 0) i = 0;
	
	byte *dst = buf->data + i;

	//while((i > 0) && (*data != 0))
	//{
	//	*dst++ = *data++; i--;
	//};

	*dst++ = '\r';
	*dst++ = '\n';

	i += 2;

	buf->len = sizeof(EthUdp) + sizeof(TrapHdr) + i;

	SendTrap(buf);

	return true;
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

bool TRAP_MEMORY_SendInfo()
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapMemInfo &trap = (TrapMemInfo&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);

	trap.hdr.cmd = TRAP_MEMORY_COMMAND_INFO;

	trap.mask = FLASH_Chip_Mask_Get();
	trap.size = FLASH_Full_Size_Get();
	trap.size_used = FLASH_Used_Size_Get();

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_MEMORY_SendStatus(u32 progress, byte status)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapMemStatus &trap = (TrapMemStatus&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);

	trap.hdr.cmd = TRAP_MEMORY_COMMAND_STATUS;

	trap.progress = progress;
	trap.status = status;

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_MEMORY_SendSession(u16 session, i64 size, i64 last_adress, RTC_type start_rtc, RTC_type stop_rtc, byte flags)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapSession &trap = (TrapSession&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_MEMORY_DEVICE);

	trap.hdr.cmd = TRAP_MEMORY_COMMAND_SESSION;

	trap.si.session = session;
	trap.si.size = size;
	trap.si.last_adress = last_adress;
	trap.si.start_rtc = start_rtc;
	trap.si.stop_rtc = stop_rtc;
	trap.si.flags = flags;

	buf->len = sizeof(EthUdp) + sizeof(trap);

	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool TRAP_MEMORY_SendLastSession(const SessionInfo *si)
{
	return TRAP_MEMORY_SendSession(si->session, si->size, si->last_adress, si->start_rtc, si->stop_rtc, si->flags);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool TRAP_MEMORY_SendNullSession()
{
	SessionInfo si;

	si.size = 0;
	si.last_adress = -1;
	si.session = -1;

	return TRAP_MEMORY_SendSession(si.session, si.size, si.last_adress, si.start_rtc, si.stop_rtc, si.flags);
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

	if (__trace) { TRAP_TRACE_PrintString(__func__); };

	return true;
}

/******************** BATTERY ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_BATTERY_SendMainMessage(u16 cmd)
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_BATTERY_DEVICE);
	//TRAP_command_type c;
	//c.command = cmd;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_BATTERY_main_type m;
	//m.battery_voltage = Power_Battery_Voltage_Get();	
	//m.line_voltage = Power_Line_Voltage_Get();
	//m.status = Power_Status_Mask_Get();	
	//m.battery_status = Power_Battery_Status_Get();	
	//m.line_status = Power_Line_Status_Get();	
	//COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
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
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_BATTERY_DEVICE);
	//TRAP_command_type c;
	//c.command = cmd;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_BATTERY_status_type s;
	//FRAM_Power_Battery_Voltages_Get(&s.battery_setup_voltage, &s.battery_min_voltage, &s.battery_max_voltage);
	//FRAM_Power_Line_Voltages_Get(&s.line_setup_voltage, &s.line_min_voltage, &s.line_max_voltage);
	//FRAM_Power_Battery_Coeffs_Get(&s.battery_coeff_k, &s.battery_coeff_b);
	//FRAM_Power_Line_Coeffs_Get(&s.line_coeff_k, &s.line_coeff_b);
	//COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
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
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_SENSORS_DEVICE);
	//TRAP_command_type c;
	//c.command = cmd;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_SENSORS_main_type m;
	//m.temperature_in = Sensors_Temperature_In_Get();
	//m.ax = Sensors_Ax_Get();
	//m.ay = Sensors_Ay_Get();
	//m.az = Sensors_Az_Get();	
	//COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
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
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_SENSORS_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_SENSORS_COMMAND_TAKE_A_COEFFS;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_SENSORS_a_coeffs_type a;
	//FRAM_Sensors_Ax_Coeffs_Get(&a.ax_coeff_k, &a.ax_coeff_b);
	//FRAM_Sensors_Ay_Coeffs_Get(&a.ay_coeff_k, &a.ay_coeff_b);
	//FRAM_Sensors_Az_Coeffs_Get(&a.az_coeff_k, &a.az_coeff_b);
	//COPY((char *)(&a), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(a));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(a));
}

/******************** PROGRAMMING ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_PROGRAMMING_TakeInfo()
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_PROGRAMMING_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_PROGRAMMING_COMMAND_TAKE_INFO;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_PROGRAMMING_info_type i;
	//i.version = FRAM_Autonom_Version_Get();
	//i.size = FRAM_Autonom_Size_Get();
	//i.validation = FRAM_Autonom_Validation_Get();
	//COPY((char *)(&i), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(i));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(i));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_PROGRAMMING_ReadBlock(u32 offset, u32 size)
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_PROGRAMMING_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_PROGRAMMING_COMMAND_TAKE_BLOCK;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_PROGRAMMING_block_type b;
	//b.offset = offset;
	//b.size = size;
	//COPY((char *)(&b), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(b));
	//FRAM_Autonom_Read_Block(offset, size, (byte *)((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type)));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(b) +size);
}

/******************** VECTOR ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TRAP_VECTOR_SendVector(u16 command, u32 time_ms, int depth, int speed, byte flags, u16 size, byte *data) 
{
	// // для скорости хорошо бы просто указать где лежит вектор, да тогда при прикреплении эзернет заголовка попортится сам вектор, а он нужен в дальнейшем, потому тупо копируем его
	//if(TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_MEMORY_vector_type) + size > sizeof(TrapTxDataBuffer)) return false;
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_VECTOR_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_VECTOR_COMMAND_VECTOR;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_VECTOR_vector_type v;	
	//v.command = command;
	//v.time_ms = time_ms;
	//v.depth = depth;
	//v.speed = speed;
	//v.size = size;
	//v.flags = flags;
	//memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), (char *)(&v), sizeof(TRAP_VECTOR_vector_type));
	//memmove((char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_VECTOR_vector_type), (char *)data, size);
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(TRAP_VECTOR_vector_type) + size);
	return true;
}

/******************** ONLINE ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_TakePeriod()
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_ONLINE_COMMAND_TAKE_PERIOD;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_ONLINE_period_type p;
	//p.period_ms = Mode_Online_Control_Period_MS_Get();
	//COPY((char *)(&p), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(p));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(p));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_TakeIndex()
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_ONLINE_COMMAND_TAKE_INDEX;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_ONLINE_index_type i;
	//i.device_index = Mode_Online_Control_Device_Index_Get();
	//COPY((char *)(&i), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(i));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(i));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_ONLINE_SendStatus(byte status)
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_ONLINE_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_ONLINE_COMMAND_SEND_STATUS;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_ONLINE_status_type s;
	//s.status = status;
	//COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
}

/******************** RDC ******************************/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_RDC_SendMain(u32 time_ms, int depth_sm, int speed_mh)
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_RDC_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_RDC_COMMAND_SEND_MAIN;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_RDC_main_type m;
	//m.time_ms = time_ms;
	//m.depth_sm = depth_sm;
	//m.speed_mh = speed_mh;
	//COPY((char *)(&m), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(m));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(m));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_RDC_SendStatus(bool messaging, bool imitation)
{
	//TRAP_MakePacketHeaders((char *)TrapTxDataBuffer, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_NO_ASK, TRAP_RDC_DEVICE);
	//TRAP_command_type c;
	//c.command = TRAP_RDC_COMMAND_SEND_STATUS;
	//COPY((char *)(&c.command), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN, sizeof(TRAP_command_type));
	//TRAP_RDC_status_type s;
	//s.status = (messaging << 6) | (imitation << 7);
	//COPY((char *)(&s), (char *)(TrapTxDataBuffer) + TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type), sizeof(s));
	//EMAC_SendData((char *)TrapTxDataBuffer, TRAP_TX_HEADERS_LEN + sizeof(TRAP_command_type) + sizeof(s));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/******************** Common ******************************/


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool TRAP_SendAsknowlege(byte device, u32 on_packet)
{
	SmallTx* buf = GetSmallTxBuffer();

	if (buf == 0) return false;

	TrapAsk &trap = (TrapAsk&)buf->th;

	MakePacketHeaders(&trap.hdr, TRAP_PACKET_NO_NEED_ASK, TRAP_PACKET_IS_ASK, device);

	trap.hdr.cmd = TRAP_COMMAND_ASKNOWLEGE;
	trap.on_packet = on_packet;

	buf->len = sizeof(EthUdp) + sizeof(TrapAsk);
	
	SendTrap(buf);

	if (__trace) { TRAP_TRACE_PrintString("%s('%c', 0x%08X)", __func__, device, on_packet); };

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_HandleRxData(Trap *t, u32 size)
{
	if(size < TRAP_RX_HEADERS_LEN) { TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); return; }

//	TRAP_RX_PACKET_type *packet = (TRAP_RX_PACKET_type *)data;

	if((TrapRxCounter == 0) || (t->hdr.counter <= 1))
	{
		TrapRxLost = 0;
	}
	else
	{
		TrapRxLost += t->hdr.counter - TrapRxCounter - 1;	
	};

	TrapRxCounter = t->hdr.counter;

	bool need_ask = (((t->hdr.status)>>3)&0x1);

	bool is_ask = (((t->hdr.status)>>2)&0x1);

	byte version = t->hdr.version;

	if(version > TRAP_PACKET_VERSION)
	{
		TRAP_INFO_SendError(TRAP_PACKET_ERROR_VERSION);
		return;
	};

	if(is_ask == TRAP_PACKET_NO_ASK)
	{
//		TRAP_command_type *cmd = (TRAP_command_type *)(data + TRAP_RX_HEADERS_LEN);	

		if(size < TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type))
		{ 
			TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW); 
			return; 
		};

		switch (t->hdr.device)
		{
			case TRAP_INFO_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				TrapInfoSet &ts = (TrapInfoSet&)*t;

				switch (t->hdr.cmd)
				{
					case TRAP_INFO_COMMAND_GET_INFO:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);					
						TRAP_INFO_SendInfo();
						break;

					case TRAP_INFO_COMMAND_SET_NUMBER:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
						FRAM_Main_Device_Number_Set(ts.number);
						break;

					case TRAP_INFO_COMMAND_SET_TYPE:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
						FRAM_Main_Device_Type_Set(ts.type);
						break;

					case TRAP_INFO_COMMAND_SET_TELEMETRY:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_INFO_DEVICE, TrapRxCounter);
						FRAM_Main_Device_Telemetry_Set(ts.telemetry);
						break;

					default: 

						TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
						break;
				};

				break;

			case TRAP_CLOCK_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				switch (t->hdr.cmd)
				{
					case TRAP_CLOCK_COMMAND_GET:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_CLOCK_DEVICE, TrapRxCounter);					
						TRAP_CLOCK_SendMain(/*RTC_Get()*/);
						break;

					case TRAP_CLOCK_COMMAND_SET:

						TrapClock &tc = (TrapClock&)*t;

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_CLOCK_DEVICE, TrapRxCounter);	
						SetTime(tc.rtc);

						if (__trace) { TRAP_TRACE_PrintString(" TRAP_CLOCK_COMMAND_SET \r\n"); };

						break;

					default: 

						TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
						break;
				}
				break;

			case TRAP_MEMORY_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				switch (t->hdr.cmd)
				{
					case TRAP_MEMORY_COMMAND_GET_INFO:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
						TRAP_MEMORY_SendInfo();
						break;

					case TRAP_MEMORY_COMMAND_READ_SESSION_START:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);

						TRAP_MEMORY_SendLastSession(GetLastSessionInfo());
//						TRAP_MEMORY_SendNullSession();

//						Mode_Ethernet_Flash_Read_Session_Start();
						break;

					case TRAP_MEMORY_COMMAND_READ_VECTOR_START:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);

//						TrapReadVector &tr = (TrapReadVector&)*t;

						start = true;
						stop = false;

//						Mode_Ethernet_Flash_Read_Vector_Start(tr.session, tr.last_adress);

						break;

					case TRAP_MEMORY_COMMAND_STOP:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					

						stop = true;
						start = false;

//						Mode_Ethernet_Flash_Stop();
						break;

					case TRAP_MEMORY_COMMAND_PAUSE:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);	

						pause = true;
//						Mode_Ethernet_Flash_Pause();
						break;

					case TRAP_MEMORY_COMMAND_RESUME:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);

						pause = false;
//						Mode_Ethernet_Flash_Resume();
						break;

					case TRAP_MEMORY_COMMAND_ERASE:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
//						Mode_Ethernet_Flash_Erase();
						break;

					case TRAP_MEMORY_COMMAND_UNERASE:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_MEMORY_DEVICE, TrapRxCounter);					
//						Mode_Ethernet_Flash_UnErase();
						break;

					default: 

						TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
						break;
				};

				break;

			case TRAP_BOOTLOADER_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

				switch (t->hdr.cmd)
				{
					case TRAP_BOOTLOADER_COMMAND_START:

						if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BOOTLOADER_DEVICE, TrapRxCounter);
						BootLoader_Start_Delay();

						break;

					default: 

						TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);

						break;
				};

				break;

			//case TRAP_BATTERY_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	TrapBattSetCoeffs &tbsc = (TrapBattSetCoeffs&)*t;
			//	TrapBattSetVolt &tbsv = (TrapBattSetVolt&)*t;


			//	switch (t->hdr.cmd)
			//	{
			//		case TRAP_BATTERY_COMMAND_GET_MAIN:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			TRAP_BATTERY_TakeMain();
			//			break;

			//		case TRAP_BATTERY_COMMAND_GET_STATUS:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			TRAP_BATTERY_TakeStatus();
			//			break;

			//		case TRAP_BATTERY_COMMAND_SET_BATTERY_COEFFS:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);

			//			FRAM_Power_Battery_Coeffs_Set(tbsc.coeff_k, tbsc.coeff_b);

			//			break;
			//			
			//		case TRAP_BATTERY_COMMAND_SET_LINE_COEFFS:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			FRAM_Power_Line_Coeffs_Set(tbsc.coeff_k, tbsc.coeff_b);
			//			break;

			//		case TRAP_BATTERY_COMMAND_SET_BATTERY_VOLTAGES:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			FRAM_Power_Battery_Voltages_Set(tbsv.setup_voltage, tbsv.min_voltage, tbsv.max_voltage);
			//			break;

			//		case TRAP_BATTERY_COMMAND_SET_LINE_VOLTAGES:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			FRAM_Power_Line_Voltages_Set(tbsv.setup_voltage, tbsv.min_voltage, tbsv.max_voltage);
			//			break;

			//		case TRAP_BATTERY_COMMAND_SWITCH_ON:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			Power_Switch_Set(true);
			//			break;

			//		case TRAP_BATTERY_COMMAND_SWITCH_OFF:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			Power_Switch_Set(false);
			//			break;

			//		case TRAP_BATTERY_COMMAND_MAIN_ENABLE:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			Power_Transmission_Set(true);
			//			break;

			//		case TRAP_BATTERY_COMMAND_MAIN_DISABLE:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_BATTERY_DEVICE, TrapRxCounter);
			//			Power_Transmission_Set(false);
			//			break;

			//		default: 

			//			TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//			break;
			//	};

			//	break;

			//case TRAP_SENSORS_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	switch (t->hdr.cmd)
			//	{
			//		case TRAP_SENSORS_COMMAND_GET_MAIN:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);
			//			TRAP_SENSORS_TakeMain();
			//			break;

			//		case TRAP_SENSORS_COMMAND_GET_A_COEFFS:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);
			//			TRAP_SENSORS_Take_A_Coeffs();
			//			break;

			//		case TRAP_SENSORS_COMMAND_SET_A_COEFFS:

			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_SENSORS_DEVICE, TrapRxCounter);

			//			TrapSensCoeffs &tsc = (TrapSensCoeffs&)*t;

			//			FRAM_Sensors_Ax_Coeffs_Set(tsc.ax_coeff_k, tsc.ax_coeff_b);
			//			FRAM_Sensors_Ay_Coeffs_Set(tsc.ay_coeff_k, tsc.ay_coeff_b);
			//			FRAM_Sensors_Az_Coeffs_Set(tsc.az_coeff_k, tsc.az_coeff_b);
			//			break;

			//		default: 

			//			TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//			break;
			//	};

			//	break;

			//case TRAP_PROGRAMMING_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	TrapProgBlock &tpb = (TrapProgBlock&)*t;

			//	switch (t->hdr.cmd)
			//	{
			//	case TRAP_PROGRAMMING_COMMAND_GET_INFO:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
			//		TRAP_PROGRAMMING_TakeInfo();
			//		break;

			//	case TRAP_PROGRAMMING_COMMAND_WRITE_BEGIN:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
			//		FRAM_Autonom_Write_Begin();
			//		break;

			//	case TRAP_PROGRAMMING_COMMAND_WRITE_END:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
			//		FRAM_Autonom_Write_End();
			//		break;

			//	case TRAP_PROGRAMMING_COMMAND_WRITE_BLOCK:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
			//		FRAM_Autonom_Write_Block(tpb.offset, tpb.size, tpb.data);
			//		break;

			//	case TRAP_PROGRAMMING_COMMAND_READ_BLOCK:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_PROGRAMMING_DEVICE, TrapRxCounter);
			//		TRAP_PROGRAMMING_ReadBlock(tpb.offset, tpb.size);
			//		break;

			//	default: 

			//		TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//		break;
			//	}
			//	break;

			//case TRAP_VECTOR_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	switch (t->hdr.cmd)
			//	{
			//	case TRAP_VECTOR_COMMAND_ENABLE:
			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_VECTOR_DEVICE, TrapRxCounter);
			//		Mode_Online_Main_Enable();
			//		break;
			//	case TRAP_VECTOR_COMMAND_DISABLE:
			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_VECTOR_DEVICE, TrapRxCounter);
			//		Mode_Online_Main_Disable();
			//		break;
			//	default: 
			//		TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//		break;
			//	}
			//	break;

			//case TRAP_ONLINE_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	switch (t->hdr.cmd)
			//	{
			//	case TRAP_ONLINE_COMMAND_GET_PERIOD:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
			//		TRAP_ONLINE_TakePeriod();
			//		break;

			//	case TRAP_ONLINE_COMMAND_SET_PERIOD:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TrapOnlinePeriod &top = (TrapOnlinePeriod&)*t;

			//		TRAP_ONLINE_SendStatus(Mode_Online_Control_Period_MS_Set(top.period_ms));
			//		break;
			//	case TRAP_ONLINE_COMMAND_BEGIN:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
			//		Mode_Online_Control_Begin();
			//		break;
			//	case TRAP_ONLINE_COMMAND_CANCEL:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
			//		Mode_Online_Control_Cancel();
			//		break;

			//	case TRAP_ONLINE_COMMAND_END:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);
			//		Mode_Online_Control_End();
			//		break;

			//	case TRAP_ONLINE_COMMAND_SET_DEVICE:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TrapOnlineSetDevice &tosd = (TrapOnlineSetDevice&)*t;

			//		TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Add(tosd.delay_ms, tosd.period_min_ms, tosd.command_count));
			//		break;

			//	case TRAP_ONLINE_COMMAND_SET_COMMAND:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TrapOnlineSetCmd &tosc = (TrapOnlineSetCmd&)*t;

			//		TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Add(Mode_Online_Control_Device_Index_Get(), tosc.command_index, tosc.telemetry, tosc.mode, tosc.offset_ms, set_command->tx_flags, set_command->tx_freq_hz, set_command->tx_size, set_command->rx_flags, set_command->rx_freq_hz, set_command->rx_timeout_mks, set_command->rx_pause_mks, set_command->rx_size, set_command->tx_data));
			//		break;

			//	case TRAP_ONLINE_COMMAND_ADD_COMMAND:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TRAP_ONLINE_add_command_type *add_command = (TRAP_ONLINE_add_command_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
			//		TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Add(add_command->device_index, add_command->command_index, add_command->telemetry, add_command->mode, add_command->offset_ms, add_command->tx_flags, add_command->tx_freq_hz, add_command->tx_size, add_command->rx_flags, add_command->rx_freq_hz, add_command->rx_timeout_mks, add_command->rx_pause_mks, add_command->rx_size, add_command->tx_data));
			//		break;

			//	case TRAP_ONLINE_COMMAND_REMOVE_COMMAND:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TRAP_ONLINE_remove_command_type *remove_command = (TRAP_ONLINE_remove_command_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type));
			//		TRAP_ONLINE_SendStatus(Mode_Online_Control_Device_Command_Remove(remove_command->device_index, remove_command->command_index));
			//		break;

			//	case TRAP_ONLINE_COMMAND_GET_INDEX:

			//		if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_ONLINE_DEVICE, TrapRxCounter);

			//		TRAP_ONLINE_TakeIndex();
			//		break;

			//	default: 

			//		TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//		break;
			//	}
			//	break;

			//case TRAP_RDC_DEVICE: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			//	switch (t->hdr.cmd)
			//	{
			//		case TRAP_RDC_COMMAND_IMITATION_ENABLE:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Imitation_Enable(((TRAP_RDC_imitation_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm, ((TRAP_RDC_imitation_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->speed_mh);
			//			break;
			//		case TRAP_RDC_COMMAND_IMITATION_DISABLE:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Imitation_Disable();
			//			break;
			//		case TRAP_RDC_COMMAND_SET_DEPTH:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Depth_Set(((TRAP_RDC_depth_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm);
			//			break;
			//		case TRAP_RDC_COMMAND_CHANGE_DEPTH:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Depth_Set(Mode_Online_RDC_Depth_Get() + ((TRAP_RDC_depth_type *)(data + TRAP_RX_HEADERS_LEN + sizeof(TRAP_command_type)))->depth_sm);
			//			break;
			//		case TRAP_RDC_COMMAND_MESSAGING_ENABLE:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Messaging_Enable();
			//			break;
			//		case TRAP_RDC_COMMAND_MESSAGING_DISABLE:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Messaging_Disable();
			//			break;
			//		case TRAP_RDC_COMMAND_TIME_RESET:
			//			if(need_ask == TRAP_PACKET_NEED_ASK) TRAP_SendAsknowlege(TRAP_RDC_DEVICE, TrapRxCounter);
			//			Mode_Online_RDC_Time_Reset();
			//			break;
			//		default: 
			//			TRAP_INFO_SendError(TRAP_PACKET_ERROR_UNKNOW);
			//			break;
			//	}
			//	break;


			default: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

static void UpdateSendVector()
{
	static byte i = 0;
	static FLRB flrb;

	static HugeTx *t = 0;
	static VecData::Hdr h;

	TrapVector *trap = (TrapVector*)&t->th;

	switch (i)
	{
		case 0:

			if (start)
			{
				start = false;

				i++;
			};

			break;

		case 1:

			if (stop)
			{
				stop = false;
				i = 0;
			}
			else if (!pause)
			{
				t = GetHugeTxBuffer();

				if (t != 0)
				{
					flrb.data = trap->data;
					flrb.maxLen = sizeof(t->th) + sizeof(t->data) + sizeof(t->exdata) - sizeof(*trap) + sizeof(trap->data);
					flrb.vecStart = true;

					RequestFlashRead(&flrb);

					i++;
				};
			};

			break;

		case 2:

			if (flrb.ready)
			{
				if (flrb.len == 0)
				{
					i = 0;
				}
				else if (h.crc == 0)
				{
					trap->hdr.cmd = TRAP_MEMORY_COMMAND_VECTOR;
					trap->session = h.session;
					trap->device = 0xAA00; //h.device;
					trap->rtc = h.rtc;
					trap->flags = h.flags;

					t->len = sizeof(EthUdp) + sizeof(*trap) - sizeof(trap->data) + flrb.len;

					SendTrap(t);

					i = 1;
				}
				else
				{
					// найти следующий вектор
					__breakpoint(0);
				};

			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void TRAP_Init()
{
	TrapRxCounter = 0;
	TrapTxCounter = 0;
	TrapRxLost = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TRAP_Idle()
{
	UpdateSendVector();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

