#include "PointerCRC.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void PointerCRC::WriteArrayW(ConstDataPointer p, word count)
{
	for (; count > 0 ; count--)
	{
		CRC.w = tableCRC[CRC.b[0] ^ (*(b++) = *(p.b++))] ^ CRC.b[1];
		CRC.w = tableCRC[CRC.b[0] ^ (*(b++) = *(p.b++))] ^ CRC.b[1];
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
