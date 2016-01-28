//#include "common.h"
#include "types.h"
#include "rtc.h"
#include "vector.h"
#include "fram.h"
#include "flash.h"

static u16 vector_current_session;
static i64 vector_current_session_adress = -1;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Vector_Reset()
{
	vector_current_session = FRAM_Memory_Current_Session_Get();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Vector_Init()
{
	Vector_Reset();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Vector_Idle()
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 Vector_Make(u16 session, u16 device, RTC_type rtc, byte flags, byte *in, byte *out, u16 size)
{
	((vector_type *)out)->control.id_session = session;
	((vector_type *)out)->control.id_device = device;
	((vector_type *)out)->control.rtc = rtc;
	((vector_type *)out)->control.adress_vector.hi = (byte)((i64)FRAM_Memory_Current_Vector_Adress_Get() >> 32);
	((vector_type *)out)->control.adress_vector.lo = (u32)FRAM_Memory_Current_Vector_Adress_Get();
	if(vector_current_session != ((vector_type *)out)->control.id_session) // новая сессия
	{
		vector_current_session_adress = -1;
		vector_current_session = ((vector_type *)out)->control.id_session;
	}
	else
	if(vector_current_session_adress == -1)
	{  	
	        vector_current_session_adress = FRAM_Memory_Current_Vector_Adress_Get();
	}
  	((vector_type *)out)->control.adress_session.hi = (byte)(vector_current_session_adress >> 32);
	((vector_type *)out)->control.adress_session.lo = (u32)vector_current_session_adress;
	((vector_type *)out)->control.flags = flags;

        u16 i;
	byte crc = 0;

	for(i = 0; i < size; i++)               // crc данных
	{
		out[i + sizeof(vector_type)] = in[i];
		crc += in[i];
	}                    

	for(i = sizeof(vector_header_type); i < sizeof(vector_type); i++) // crc control_header
	{
		crc += out[i];
	}                    

	((vector_type *)out)->header.magic = VECTOR_MAGIC;
	((vector_type *)out)->header.version = VECTOR_VERSION;
	((vector_type *)out)->header.type = VECTOR_TYPE_DATA;
	((vector_type *)out)->header.crc_header = 0;
	((vector_type *)out)->header.crc_data = crc;
	((vector_type *)out)->header.size = size + sizeof(vector_type);

	crc = 0;
	for(i = 0; i < sizeof(vector_header_type); i++)	//crc header
	{
		crc += out[i];
	}
	((vector_type *)out)->header.crc_header = crc;
	if(vector_current_session_adress != -1)          // новая сессия
	{
		i64 session_size = FRAM_Memory_Current_Adress_Get() - vector_current_session_adress;
		if(session_size < 0) session_size += FLASH_Full_Size_Get();
		if(session_size + ((vector_type *)out)->header.size >= FLASH_Full_Size_Get()) 
		{
			return 0;
		}
	}
	return ((vector_type *)out)->header.size;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Vector_Check_Header(byte *in, u16 *size)
{
	
	if(((vector_header_type *)in)->magic != VECTOR_MAGIC) return false;
	if(((vector_header_type *)in)->version > VECTOR_VERSION) return false;
	if(((vector_header_type *)in)->type != VECTOR_TYPE_DATA) return false;
	byte i;
	byte crc_header = ((vector_header_type *)in)->crc_header;
	((vector_header_type *)in)->crc_header = 0;
	byte crc = 0;
	for(i = 0; i < sizeof(vector_header_type); i++) crc += in[i];
	if(crc != crc_header) return false;
        *size = ((vector_header_type *)in)->size;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Vector_Check_Data(byte *in)
{
	u16 i;
	byte crc = 0;
	for(i = sizeof(vector_header_type); i < ((vector_header_type *)in)->size; i++) crc += in[i];
	if(crc != ((vector_header_type *)in)->crc_data) return false;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 Vector_Get_ID_Session(byte *in)
{
	return ((vector_type *)in)->control.id_session;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 Vector_Get_ID_Device(byte *in)
{
	return ((vector_type *)in)->control.id_device;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte Vector_Get_Flags(byte *in)
{
	return ((vector_type *)in)->control.flags;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Vector_Get_RTC(byte *in, RTC_type *rtc)
{
	rtc->time = ((vector_type *)in)->control.rtc.time;
	rtc->date = ((vector_type *)in)->control.rtc.date;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i64 Vector_Get_Adress_Vector(byte *in)
{
	i64 x = ((vector_type *)in)->control.adress_vector.lo;
	x += (((i64)((vector_type *)in)->control.adress_vector.hi) << 32);
	return x;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i64 Vector_Get_Adress_Session(byte *in)
{
	i64 x = ((vector_type *)in)->control.adress_session.lo;
	x += (((i64)((vector_type *)in)->control.adress_session.hi) << 32);
	return x;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 Vector_Get_Data(byte *in, byte **pdata)
{
	*pdata = (byte *)(in + sizeof(vector_type));
	return ((vector_type *)in)->header.size - (sizeof(vector_type));
}

/*****************************************************************************/




