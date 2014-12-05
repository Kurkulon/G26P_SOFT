#include "ByteArray.h"


#ifdef _VARSDEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif



ByteArrayVar::ByteArrayVar(word s, byte acs, word i)
:	Var(0, acs)
{
	SIZE = (s+1)/2;

	dw = new word[SIZE+2];

	if (dw == 0)
	{
		while(1){};
		//printf("Memory alloc error in ByteArrayVar\n\r");
		//exit(-1);
	};

	Clear(i);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//ByteArrayVar::~ByteArrayVar()
//{
//	if (dw != 0)
//	{
//		delete dw;
//	}
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word ByteArrayVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ByteArrayVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	if (IsAccessOk(acs))
	{
		for (word i = startAdr - address; count > 0; i++, count--)
		{
			dw[i] = *data.w++;
		};
	}

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ByteArrayVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	for (word i = startAdr - address; count > 0; i++, count--)
	{
		*data.w++ = dw[i];
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ByteArrayVar::Save(PointerCRC &p) const
{
//	if (_debug) printf("ByteArrayVar::Save 0x%X\n", address);

	p.WriteArrayW(dw, SIZE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ByteArrayVar::Load(PointerCRC &p)
{
//	if (_debug) printf("ByteArrayVar::Load 0x%X\n", address);

	p.ReadArrayW(dw, SIZE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ByteArrayVar::Clear(word j)
{
	for (word i = 0; i < SIZE; i++)
	{
		dw[i] = j; 
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

