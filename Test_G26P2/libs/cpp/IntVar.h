#ifndef	INTVAR_H__15_07_2006__21_57
#define	INTVAR_H__15_07_2006__21_57


#include "vars.h"
#include "UserVar.h"

struct IntVar : public Var
{
	enum	constants { SIZE = 1 };

	typedef i16 T;

	User<T> var;

	IntVar(T v = 0) : Var(0, READONLY), var(v) {}
	IntVar(T min, T max, byte acs, T v, User<T>::Event ev, word code) : Var(0, acs), var(v, min, max, v, ev, code) {}

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;

	operator T() const { return var.v; }
	void operator=(T v){ var.v = v; }
};

struct WordVar : public Var
{
	enum	constants { SIZE = 1 };

	typedef u16 T;

	User<T> var;

	WordVar(T v = 0) : Var(0, READONLY), var(v) {}
	WordVar(T min, T max, byte acs, T v, User<T>::Event ev, word code) : Var(0, acs), var(v, min, max, v, ev, code) {}

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;

	operator T() const { return var.v; }
	void operator=(T v){ var.v = v; }
};

#endif	// INTVAR_H__15_07_2006__21_57
