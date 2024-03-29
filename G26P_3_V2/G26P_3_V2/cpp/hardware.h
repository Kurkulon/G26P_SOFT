#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "hw_conf.h"
#include <i2c.h>

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

extern void SetReqHV(u16 v);
extern void SetReqFireFreqM(u16 freq, u16 duty);
extern void SetReqFireFreqXY(u16 freq, u16 duty);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void SetReqFireCountXY(byte v)
{
	extern byte reqFireCountXY;
	if (v == 0) { v = 1; };
	reqFireCountXY = v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void SetReqFireCountM(byte v)
{
	extern byte reqFireCountM;
	if (v == 0) { v = 1; };
	reqFireCountM = v;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool IsSyncActive()
{
	extern bool syncActive;
	return syncActive;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//inline u32 GetSyncTime()
//{
//	extern u32 syncTime;
//	return syncTime;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#ifndef WIN32


#else


#endif

#endif // HARDWARE_H__23_12_2013__11_37
