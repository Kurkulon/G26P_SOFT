#ifndef	BYTEARRAYVAR_H__08_10_2008__13_27
#define	BYTEARRAYVAR_H__08_10_2008__13_27

#include "vars.h"

struct ByteArrayVar : public Var
{
	word SIZE;

	union
	{
		word	*dw;
		byte	*db;
		char	*str;
	};

	ByteArrayVar(word s, byte access, word i);

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;
	virtual void Save(PointerCRC &p) const;
	virtual void Load(PointerCRC &p);
	void	Clear(word j);

#ifdef _TEST_
	virtual bool SelfTest();
	static byte testsCount;
#endif

};

#endif	// BYTEARRAYVAR_H__08_10_2008__13_27
