#ifndef MEMORY_AT91SAM7X256_VECTOR_H
#define MEMORY_AT91SAM7X256_VECTOR_H

#include "rtc.h"

#define VECTOR_MAGIC		0xAA
#define VECTOR_VERSION		2
#define VECTOR_TYPE_DATA	1

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct __attribute__ ((packed))   
{
	byte magic;
	byte version;		// версия структуры 
	byte type;
	byte crc_header;	// контрольная сумма header_type, исключая crc
	byte crc_data;		// контрольная сумма данных
	u16 size;		// длина пакета в байтах включая header и control
} vector_header_type;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct __attribute__ ((packed))   
{
	byte hi;
 	u32 lo;	//адрес предыдущего вектора
} vector_adress_type;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct __attribute__ ((packed))   
{
	u16 id_session;
	u16 id_device;
	RTC_type rtc;		
	vector_adress_type adress_vector;	// адрес предыдущего вектора
	vector_adress_type adress_session;	// адрес начала сессии
	byte flags;	// erorr  т.п	
} vector_control_type;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct __attribute__ ((packed))   
{
	vector_header_type header;
	vector_control_type control;
} vector_type;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void Vector_Init();
extern void Vector_Reset();
extern void Vector_Idle();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern u16 Vector_Make(u16 session, u16 device, RTC_type rtc, byte flags, byte *in, byte *out, u16 size);
extern bool Vector_Check_Header(byte *in, u16 *size);
extern bool Vector_Check_Data(byte *in);

extern u16 Vector_Get_ID_Session(byte *in);
extern u16 Vector_Get_ID_Device(byte *in);
extern byte Vector_Get_Flags(byte *in);
extern void Vector_Get_RTC(byte *in, RTC_type *rtc);
extern i64 Vector_Get_Adress_Vector(byte *in);
extern i64 Vector_Get_Adress_Session(byte *in);
extern u16 Vector_Get_Data(byte *in, byte **pdata);


#endif
