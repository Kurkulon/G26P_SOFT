#include "LogVar.h"
#include "AllocMem.h"

#ifdef _VARSDEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

LogVar::LogVar(word count) 

	: len(0), current(0), MAXLEN(count), SIZE(MAXLEN*((sizeof(LogData) + 1) / 2))
{ 
	data = (LogData*)AllocMemNV(sizeof(LogData) * MAXLEN);
//	cacheData = (LogCacheData*)AllocMemNV(sizeof(LogCacheData) * MAXLEN);

//	store = (STORE*)AllocMemNV(sizeof(STORE) * MAXLEN);
	storeStatus = (STORESTATUS*)AllocMemNV(sizeof(STORESTATUS));

//	Clear();

#ifdef _TEST_
if (testsCount < 1) { SelfTest(); testsCount += 1; };
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word LogVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool LogVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	i32 st = startAdr - address;

	if ((count&1) != 0 || (st&1) != 0) 
	{
		return false;
	};

	st >>= 1;
	st += current * (sizeof(LogData) / 4); 

	i32 c = count >>= 1;

	i32 c2;

	i32 size = SIZE/2;

	if (st >= size)
	{
		st -= size;
		c2 = 0;
	}
	else
	{
		c2 = c - (size - st);

		if (c2 > 0)
		{
			c -= c2;
		};
	};

	const u32 *p = dd + st;
	__packed u32 *d = data.d;

	for ( ; c > 0; c--)
	{
		*(d++) = *(p++);
	};

	for (p = dd; c2 > 0; c2--)
	{
		*(d++) = *(p++);
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::_Save() const
{
	_SaveStatus();

	for (word i = 0; i < MAXLEN; i++)
	{
		_SaveElem(i);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word LogVar::_Load()
{
	word crc = 0;

	for (word i = 0; i < MAXLEN; i++)
	{
		crc |= _LoadElem(i);
	};

	return crc|_LoadStatus();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::_SaveStatus() const
{	
	byte n = (storeStatus->n + 1) & 1;

	PointerCRC p(storeStatus->data[n]);
	p.CRC.w = 0xFFFF;

	p.WriteW(len);
	p.WriteW(current);
	p.WriteW(p.CRC.w);
	storeStatus->n = n;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word LogVar::_LoadStatus()
{
	byte n = storeStatus->n & 1;
	
	PointerCRC p(storeStatus->data[n]);

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;

		len = p.ReadW();
		current = p.ReadW();
		p.ReadW();

		if (p.CRC.w == 0) break;
	
		n = (n + 1) & 1;

		p = storeStatus->data[n];
	};

	if (p.CRC.w != 0)
	{
		Clear();
	};

	return p.CRC.w;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::_SaveElem(word elem) const
{
	LogData &ld = data[elem];

	ld.CRC = GetCRC16(&ld, sizeof(LogData)-2);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word LogVar::_LoadElem(word elem) 
{
	LogData &ld = data[elem];
//	LogCacheData &lcd = cacheData[elem];

	word CRC = GetCRC16(&ld, sizeof(LogData));

	if (CRC != 0)
	{
		ld.code = 0;
		ld.hsecond = 0;
		ld.second = 0;
		ld.minute = 0;
		ld.hour = 0;
		ld.day = 0;
		ld.month = 0;
		ld.year = 0;
		ld.newValue.D = 0;
		ld.oldValue.D = 0;

		//lcd.code = 0;
		//lcd.newValue.D = 0;
		//lcd.oldValue.D = 0;
		//lcd.timebdc.hsecond = 0;
		//lcd.timebdc.second = 0;
		//lcd.timebdc.minute = 0;
		//lcd.timebdc.hour = 0;
		//lcd.timebdc.day = 0;
		//lcd.timebdc.month = 0;
		//lcd.timebdc.year = 0;
		_SaveElem(elem);
	}
	else
	{
		//lcd.code = ReverseWord(ld.code);
		//lcd.newValue.D = ReverseDword(ld.newValue.D);
		//lcd.oldValue.D = ReverseDword(ld.oldValue.D);

		//lcd.timebdc.hsecond = ld.hsecond;
		//lcd.timebdc.second = ld.second;
		//lcd.timebdc.hour = ld.hour;
		//lcd.timebdc.minute= ld.minute;
		//lcd.timebdc.day = ld.day;
		//lcd.timebdc.month = ld.month;
		//lcd.timebdc.year = ReverseWord(ld.year) % 100;
	};

	return CRC;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::AddEvent(word c)
{
	time_bdc t;

	GetTime(&t);

	AddEvent(c, t);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::AddEventI(word c, i32 oldValue, i32 newValue)
{
	time_bdc t;

	GetTime(&t);

	AddEventI(c, oldValue, newValue, t);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::AddEventI(word c, i32 oldValue, i32 newValue, const time_bdc &t)
{
	current = (current == 0) ? MAXLEN - 1 : current - 1;
	len += (len < MAXLEN) ? 1 : 0;

	LogData &ld = data[current];
//	LogCacheData &lcd = cacheData[current];

	//lcd.code = c;
	//lcd.newValue.D = newValue;
	//lcd.oldValue.D = oldValue;
	//lcd.timebdc = t;
	//lcd.timebdc.year %= 100;

	ld.code = ReverseWord(c);
	ld.oldValue.D = ReverseDword(oldValue);
	ld.newValue.D = ReverseDword(newValue);
	ld.hsecond = t.hsecond;
	ld.second = t.second;
	ld.hour = t.hour;
	ld.minute = t.minute;
	ld.day = t.day;
	ld.month = t.month;
	ld.year = ReverseWord(t.year);

	_SaveElem(current);
	_SaveStatus();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void LogVar::Clear()
{
	len = 0;
	current = 0;

	for (word i = 0; i < MAXLEN; i++)
	{
		LogData &ld = data[i];
//		LogCacheData &lcd = cacheData[i];

		ld.code = 0;
		ld.hsecond = 0;
		ld.second = 0;
		ld.minute = 0;
		ld.hour = 0;
		ld.day = 0;
		ld.month = 0;
		ld.year = 0;
		ld.newValue.D = 0;
		ld.oldValue.D = 0;

		//lcd.code = 0;
		//lcd.newValue.D = 0;
		//lcd.oldValue.D = 0;
		//lcd.timebdc.hsecond = 0;
		//lcd.timebdc.second = 0;
		//lcd.timebdc.minute = 0;
		//lcd.timebdc.hour = 0;
		//lcd.timebdc.day = 0;
		//lcd.timebdc.month = 0;
		//lcd.timebdc.year = 0;
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const LogVar::LogData* LogVar::GetEvent(word num)
{
	if (num >= MAXLEN)
	{
		num = MAXLEN - 1;
	};

	word c = current + num;

	if (c >= MAXLEN)
	{
		c -= MAXLEN;
	};

	return data+c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _TEST_

byte LogVar::testsCount = 0;

bool LogVar::SelfTest()
{
	return true;
}
#endif
