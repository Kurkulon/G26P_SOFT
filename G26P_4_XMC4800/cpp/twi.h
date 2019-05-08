#ifndef TWI_H__24_04_13__11_25
#define TWI_H__24_04_13__11_25

#include "types.h"
#include "core.h"


struct DSCTWI
{
	DSCTWI*	next;
	void*	wdata;
	void*	rdata;
	void*	wdata2;
	u16		wlen;
	u16		wlen2;
	u16		rlen;
	byte	adr;
	bool	ready;
};

extern void Init_TWI();

extern bool Write_TWI(DSCTWI *d);
inline bool Read_TWI(DSCTWI *d) { return Write_TWI(d); }
extern bool Update_TWI();
extern bool AddRequest_TWI(DSCTWI *d);

#endif // TWI_H__24_04_13__11_25