#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
#include "core.h"

#ifdef WIN32
#include <windows.h>
#endif

extern void InitHardware();
extern void UpdateHardware();
extern void FireXX();
extern void FireYY();
extern u16 GetCRC(const void *data, u32 len);


#ifndef WIN32


#else


#endif

#endif // HARDWARE_H__23_12_2013__11_37
