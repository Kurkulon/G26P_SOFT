#ifndef	FLOATVAR_H__15_07_2006__21_57
#define	FLOATVAR_H__15_07_2006__21_57

struct FloatVar;

#include "vars.h"
#include "UserVar.h"

struct FloatVar : public Var
{
	enum	constants { SIZE = 2 };

	User<float> var;

	word	eventCode;
	float	max;
	float	min;

	FloatVar() : Var(0, READONLY) {}
	FloatVar(float min, float max, byte acs, float v, User<float>::Event ev, word code);

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;

	operator float() const { return var.v; }
	void operator=(float v){ var.v = v; }

#ifdef _TEST_
	virtual bool SelfTest();
	static byte testsCount;
#endif

};

#endif	// FLOATVAR_H__15_07_2006__21_57
