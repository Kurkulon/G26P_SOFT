#include "types.h"
#include "CRC16.h"

#pragma O3
#pragma Otime

word GetCRC16(const void *data, i32 len)
{
//	DataCRC CRC = { 0xFFFF };
	u16 CRC = 0xFFFF;

	const byte *s = (const byte*)data;

	for ( ; len > 0; len--)
	{
//		CRC.w = tableCRC[CRC.b[0] ^ *(s++)] ^ CRC.b[1];
		CRC = tableCRC[(byte)CRC ^ *(s++)] ^ (CRC>>8);
	};

	return CRC;
}
