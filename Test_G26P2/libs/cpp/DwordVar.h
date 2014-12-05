#ifndef	DWORDVAR_H__25_09_2006__17_02
#define	DWORDVAR_H__25_09_2006__17_02

struct DwordVar;

#include "vars.h"
#include "UserVar.h"

struct DwordVar : public Var
{
	enum	constants { SIZE = 2 };

	User<dword> var;

	word	eventCode;
	dword	max;
	dword	min;

	DwordVar() : Var(0, READONLY) {}
	DwordVar(dword min, dword max, byte access, dword v, User<dword>::Event ev, word code);

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;
	virtual void Save(PointerCRC &p) const;
	virtual void Load(PointerCRC &p);

	operator dword() const { return var.v; }
	void operator=(dword v){ var.v = v; }

#ifdef _TEST_
	virtual bool SelfTest();
	static byte testsCount;
#endif

};

#endif	//DWORDVAR_H__25_09_2006__17_02
