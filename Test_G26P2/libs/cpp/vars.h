#ifndef VARS_H__24_05_2006__13_42
#define VARS_H__24_05_2006__13_42

#include "types.h"
#include "PointerCRC.h"

//#define _TEST_

#define READONLY		0
#define READWRITE		255
#define AccessGroup1 	1
#define AccessGroup2 	2
#define AccessGroup3 	4
#define AccessGroup4 	8
#define AccessGroup5 	16
#define AccessGroup6 	32
#define AccessGroup7 	64
#define AccessGroup8 	128
#define AccessAll		255
#define AccessReadOnly	0

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32
inline word ReverseWord(word v) { return ((v&0x00FF)<<8 | (v&0xFF00)>>8); }
#else
inline word ReverseWord(word v) { __asm	{ rev16 v, v };	return v; }
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline dword ReverseDword(dword v) 
{ 
#ifdef WIN32
	v = (v&0x00FF00FF)<<8 | (v&0xFF00FF00)>>8;
	return (v&0x0000FFFF)<<16 | (v&0xFFFF0000)>>16;
#else
	return __rev(v);
#endif
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//inline float SwapFloat(FLOAT v) 

#ifdef WIN32
inline dword SwapDword(dword v) { return (v&0x00FF00FF)<<8 | (v&0xFF00FF00)>>8; }
#else
inline dword SwapDword(dword v) { __asm { rev16 v, v }; return v; }
#endif

#ifdef WIN32
inline float ReverseFloat(F32u v)
{ 
	v.d = (v.d&0x00FF00FF)<<8 | (v.d&0xFF00FF00)>>8;
	v.d = (v.d&0x0000FFFF)<<16 | (v.d&0xFFFF0000)>>16;
	return v.f;
}
#else
inline float ReverseFloat(F32u v)
{ 
	__asm	
	{ 
		rev v.d, v.d 
	};	
	return v.f; 
}
#endif

#ifdef __WATCOMC__

#pragma aux ReverseWord =	\
	"xchg ah, al"			\
	parm	[AX]			\
	value	[AX];

#pragma aux ReverseDword =	\
	"xchg ah, dl"			\
	"xchg dh, al"			\
	parm	[DX AX]			\
	value	[DX AX];

#pragma aux ReverseFloat =	\
	"xchg ah, dl"			\
	"xchg dh, al"			\
	parm	[DX AX]			\
	value	[DX AX];

#pragma aux SwapDword =		\
	"xchg ah, al"			\
	"xchg dh, dl"			\
	parm	[DX AX]			\
	value	[DX AX];

#pragma aux SwapFloat =		\
	"xchg ah, al"			\
	"xchg dh, dl"			\
	parm	[DX AX]			\
	value	[DX AX];

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct	Var
{
	byte	access;
	word	address;

//	static  word count;

	word	Address() const {return address;}
	void	SetAccessReadOnly() {access = READONLY;}
	void	SetAccessReadWrite() {access = READWRITE;}
	void	SetAccess(byte a) {access = a;}
	bool	IsAccessOk(byte a) { return (bool)(access & a); }
	virtual word Size() const = 0;
	virtual bool WriteEvent(byte acs, word startAdr, word count, DataPointer data);
	virtual bool ReadEvent(word startAdr, word count, DataPointer data) const;

	Var() : access(0), address(0) {  }
	Var(word adr, byte acs) : access(acs), address(adr)  {  }

#ifdef _TEST_
	virtual bool SelfTest();
#endif

};

struct Vars
{
	word curAddress;
	word curVar;
	word varsCount;
	word maxCount;

	Var** vars;


	Vars(Var** p, word count);

	void AddVar(Var &var);
	void CurrentAddress(word adr)	{curAddress = adr;}
};

#endif	/* VARS_H__24_05_2006__13_42 */

