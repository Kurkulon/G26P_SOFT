#include "IntVar.h"

#ifdef _VARSDEBUG_
//	const bool _debug = true;
#else
//	const bool _debug = false;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word IntVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool IntVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	if (startAdr == address && count == SIZE)
	{
		if (!IsAccessOk(acs)) return true;

		var.SetValue(ReverseWord(*data.w));

//		if (_debug) printf("IntVar::WriteEvent: 0x%X, %i - OK!\n", startAdr, count);

		return true;
	}
	else
	{
//		if (_debug) printf("IntVar::WriteEvent: 0x%X, %i - ERROR!\n", startAdr, count);

		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool IntVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	if (startAdr == address && count == SIZE)
	{
		*data.w = ReverseWord(var.v);
//		if (_debug) printf("IntVar::ReadEvent: 0x%X, %i - OK!\n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("IntVar::ReadEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word WordVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool WordVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	if (startAdr == address && count == SIZE)
	{
		if (!IsAccessOk(acs)) return true;

		var.SetValue(ReverseWord(*data.w));

//		if (_debug) printf("IntVar::WriteEvent: 0x%X, %i - OK!\n", startAdr, count);

		return true;
	}
	else
	{
//		if (_debug) printf("IntVar::WriteEvent: 0x%X, %i - ERROR!\n", startAdr, count);

		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool WordVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	if (startAdr == address && count == SIZE)
	{
		*data.w = ReverseWord(var.v);
//		if (_debug) printf("IntVar::ReadEvent: 0x%X, %i - OK!\n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("IntVar::ReadEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
