#ifndef	DATETIMEVAR_H__15_07_2006__21_57
#define	DATETIMEVAR_H__15_07_2006__21_57

#include "vars.h"

struct DateTimeVar : public Var
{
	typedef 	bool (*ChangeEvent)();

	enum	constants { SIZE = 4 };

	ChangeEvent preChangeEvent;
	ChangeEvent postChangeEvent;

	DateTimeVar(byte access, ChangeEvent preEvent, ChangeEvent postEvent);

	virtual word Size() const;

	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;
	static bool DefaultChangeEvent();
};


#endif	// DATETIMEVAR_H__15_07_2006__21_57
