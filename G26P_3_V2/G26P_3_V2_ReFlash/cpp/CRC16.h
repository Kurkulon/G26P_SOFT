#ifndef CRC16_H__14_02_2008__09_01
#define CRC16_H__14_02_2008__09_01

#include "types.h"

extern const u16 tableCRC[256];

extern word GetCRC16(const void *data, u32 len);
extern word GetCRC16(const void *data, u32 len, word init, word xorOut);

#endif // CRC16_H__14_02_2008__09_01
