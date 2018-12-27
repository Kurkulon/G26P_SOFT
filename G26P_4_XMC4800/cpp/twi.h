#ifndef TWI_H__24_04_13__11_25
#define TWI_H__24_04_13__11_25

#include "types.h"
#include "core.h"


struct DSCTWI
{
	void*	wdata;
	void*	rdata;
	void*	wdata2;
	u16		wlen;
	u16		wlen2;
	u16		rlen;
	byte	adr;
	bool	ready;
};

struct TWI
{
//	u32		smask;
	DSCTWI* dsc;

	T_HW::USIC_CH_Type *hw;

	TWI() : hw(0), dsc(0) {}

	bool Init(byte num);

	bool Write(DSCTWI *d);
	bool Read(DSCTWI *d);
	bool Update();

	static __irq void Handler0();



};

#endif // TWI_H__24_04_13__11_25
