#ifndef	LOGVAR_H__15_07_2006__23_21
#define	LOGVAR_H__15_07_2006__23_21

#include "vars.h"
#include "time.h"

struct	LogVar : public Var
{
	struct LogData 
	{ 
		byte hsecond;
		byte second;
		byte minute;
		byte hour;
		byte day;
		byte month;
		word year;
		union {i32 D; float F;} oldValue; 
		union {i32 D; float F;} newValue; 
		word code; 
		word CRC;
	}; //size 10 reg

	//struct LogCacheData
	//{
	//	word code; 
	//	union {i32 D; float F;} oldValue; 
	//	union {i32 D; float F;} newValue; 
	//	time_bdc timebdc;
	//};

	struct STORESTATUS
	{
		byte n; byte data[2][10];
	};

//	enum	constants	{ MAXLEN = 64, SIZE = ( MAXLEN*sizeof(LogData) + 1) / 2 };

	STORESTATUS* storeStatus;

	word MAXLEN;
	word SIZE;

	word len;
	word current;

	union
	{
		LogData *data;
		u32		*dd;
	};

//	LogCacheData *cacheData;

	LogVar(word count);

	virtual word Size() const;
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;

	void	_Save() const;
	word	_Load();

	void	_SaveStatus() const;
	word	_LoadStatus();
	void	_SaveElem(word elem) const;
	word	_LoadElem(word elem);


	void AddEvent(word c);
	void AddEventI(word c, i32 oldValue, i32 newValue);
	void AddEventF(word c, float oldValue, float newValue) { AddEventI(c, *((i32*)&oldValue), *((i32*)&newValue)); }
	void AddEvent(word c, const time_bdc &t) { AddEventI(c, 0, 0, t); }
	void AddEventI(word c, i32 oldValue, i32 newValue, const time_bdc &t);
	void AddEventF(word c, float oldValue, float newValue, const time_bdc &t) {AddEventI(c, *((i32*)&oldValue), *((i32*)&newValue), t);}

	void Clear(); 

	const LogData* GetEvent(word num);


#ifdef _TEST_
	virtual bool SelfTest();
	static byte testsCount;
#endif

};


#endif // LOGVAR_H__15_07_2006__23_21

