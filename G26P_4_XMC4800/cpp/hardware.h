#ifndef HARDWARE_H__15_05_2009__14_35
#define HARDWARE_H__15_05_2009__14_35

#include "types.h"
#include "core.h"
#include "time.h"

extern void InitHardware();
extern void UpdateHardware();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u16 CheckParity(u16 x)
{
	u16 y = x ^ (x >> 1);

	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	
	return (y ^ (y >> 8))^1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MRB
{
	volatile bool	ready;
	volatile bool	OK;
	volatile u16	len;
	u16				maxLen;
	u16				*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MTB
{
	volatile bool	ready;
	u16				len;
	const u16		*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool RcvManData(MRB *mrb);
extern bool SendManData(MTB *mtb);
extern void SetTrmBoudRate(byte i);
extern void ManRcvUpdate();
extern void ManRcvStop();

extern bool SendMLT3(MTB *mtb);
extern void SetClock(const RTC &t);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct DSCSPI
{
	DSCSPI*			next;
	void*			wdata;
	void*			rdata;
	u32				adr;
	u16				alen;
	u16				wlen;
	u16				rlen;
	volatile bool	ready;
	byte			csnum;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool SPI_AddRequest(DSCSPI *d);
extern bool SPI_Update();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#endif // HARDWARE_H__15_05_2009__14_35
