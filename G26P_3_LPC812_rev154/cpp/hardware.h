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
extern void WaitFireSync(byte t);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u16 GetCurHV()
{
	extern u16 curHV;
	return curHV;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void SetReqHV(u16 v)
{
	extern u16 reqHV;
	reqHV = v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void SetReqFireCount(byte v)
{
	extern byte reqFireCount;
	reqFireCount = v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool IsSyncActive()
{
	extern bool syncActive;
	return syncActive;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u32 GetSyncTime()
{
	extern u32 syncTime;
	return syncTime;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifndef WIN32


#else


#endif

#endif // HARDWARE_H__23_12_2013__11_37
