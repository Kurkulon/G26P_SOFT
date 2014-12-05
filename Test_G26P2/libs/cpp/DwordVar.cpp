#include "DwordVar.h"

#ifdef _VARSDEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

DwordVar::DwordVar(dword min, dword max, byte acs, dword v, User<dword>::Event ev, word code) 
:	Var(0, acs), var(v, min, max, v, ev, code)
{

	if (max < min)	{ this->max = min; this->min = max;	}
	else			{ this->max = max; this->min = min;	};

#ifdef _TEST_
	if (testsCount < 1) { SelfTest(); testsCount += 1; };
#endif

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DwordVar::Load(PointerCRC &p)
{
//	if (_debug) printf("DwordVar::Load 0x%X\n", address);
	var.v = p.ReadD();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DwordVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	if (startAdr == address && count == SIZE)
	{
		*data.d = SwapDword(var.v);

//		if (_debug) printf("DwordVar::ReadEvent: 0x%X, %i - OK!\n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("DwordVar::ReadEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DwordVar::Save(PointerCRC &p) const
{
//	if (_debug) printf("DwordVar::Save 0x%X\n", address);
	p.WriteD(var.v);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _TEST_

byte DwordVar::testsCount = 0;

bool DwordVar::SelfTest()
{
	word CRC1, CRC2;

//	bool ret = true;

	byte store1[sizeof(DwordVar)];
	byte store2[sizeof(DwordVar)];

	FarPointerCRC p(store1);

	word p_address;
	dword p_value, p_max, p_min;
	byte p_access;

	for (word i = 0; i < sizeof(store1); i++)
	{
		store1[i] = store2[i] = 0;
	};

	p.CRC.w = 0xFFFF;

	Randomize();
	dw[0] = Rand();
	dw[1] = Rand();

	p_value = value;
	p_address = address;
	p_max = max;
	p_min = min;
	p_access = access;

	Save(p);
	value += 1;
	CRC1 = p.CRC.w;

	p = store1;
	p.CRC.w = 0xFFFF;
	Load(p);

	if (p.CRC.w != CRC1 || value != p_value || address != p_address || p_max != max || p_min != min || p_access != access)
	{
		printf("ERROR in DwordVar::Save(FarPointer, DataCRC&) or DwordVar::Load(FarPointer, DataCRC&)\n");
		return false;
	};

	printf("DwordVar::SelfTest ... OK!!!\n");
	return true;
}
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word DwordVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DwordVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	if (startAdr == address && count == SIZE)
	{
		if (!IsAccessOk(acs)) return true;

		var.SetValue(ReverseDword(*data.d));

//		if (_debug) printf("DwordVar::WriteEvent: 0x%X, %lu - OK!\n", startAdr, var.v);

		return true;
	}
	else
	{
//		if (_debug) printf("DwordVar::WriteEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

