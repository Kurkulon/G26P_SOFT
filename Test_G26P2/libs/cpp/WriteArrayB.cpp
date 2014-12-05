#include "PointerCRC.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void PointerCRC::WriteArrayB(ConstDataPointer p, word count)
{
	for (; count > 0 ; count--)
	{
		CRC.w = tableCRC[CRC.b[0] ^ (*(b++) = *(p.b++))] ^ CRC.b[1];
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
