#include "DateTimeVar.h"
#include "time.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _VARSDEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

DateTimeVar::DateTimeVar(byte acs, ChangeEvent preEvent, ChangeEvent postEvent)
:	Var(0, acs), preChangeEvent((preEvent != 0) ? preEvent : DefaultChangeEvent), 
	postChangeEvent((postEvent != 0) ? postEvent : DefaultChangeEvent)
{
//	if (_debug) printf("DateTimeVar::DateTimeVar 0x%X - OK!\n", address);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DateTimeVar::DefaultChangeEvent()
{
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DateTimeVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	time_bdc t;

	if (startAdr == address && count == SIZE)
	{
		GetTime(&t);

		*(data.b++) = t.hsecond;
		*(data.b++) = t.second;
		*(data.b++) = t.minute;
		*(data.b++) = t.hour;
		*(data.b++) = t.day;
		*(data.b++) = t.month;
		*data.w = ReverseWord(t.year);

//		if (_debug) printf("DateTimeVar::ReadEvent 0x%X, %u - OK!\n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("DateTimeVar::ReadEvent 0x%X, %u - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word DateTimeVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DateTimeVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	time_bdc t;

	if (startAdr == address && count == SIZE)
	{
		if (!IsAccessOk(acs)) return true;

		t.hsecond	= *(data.b++);
		t.second	= *(data.b++);
		t.minute	= *(data.b++);
		t.hour		= *(data.b++);
		t.day		= *(data.b++);
		t.month		= *(data.b++);
		t.year		= ReverseWord(*data.w);

		preChangeEvent();

		SetTime(t);

		postChangeEvent();

//		if (_debug) printf("DateTimeVar::WriteEvent 0x%X, %u - OK! \n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("DateTimeVar::WriteEvent 0x%X, %u - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

