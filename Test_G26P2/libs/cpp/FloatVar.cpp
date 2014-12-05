#include "FloatVar.h"

#ifdef _VARSDEBUG_
//	const bool _debug = true;
#else
//	const bool _debug = false;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
FloatVar::FloatVar(float min, float max, byte acs, float v, User<float>::Event ev, word code) 
:	Var(0, acs), var(v, min, max, v, ev, code)
{
//	if (_debug) printf("FloatVar::FloatVar %u - OK!\n", address);

	if (max < min)	{ this->max = min; this->min = max;	}
	else			{ this->max = max; this->min = min;	};

#ifdef _TEST_
	if (testsCount < 1) { SelfTest(); testsCount += 1; };
#endif

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word FloatVar::Size() const
{
	return SIZE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FloatVar::WriteEvent(byte acs, word startAdr, word count, DataPointer data)
{
	if (startAdr == address && count == SIZE)
	{
		if (!IsAccessOk(acs)) return true;

		var.SetValue(ReverseFloat(*data.f));

//		if (_debug) printf("FloatVar::WriteEvent: 0x%X, %.2f - OK!\n", startAdr, var.v);

		return true;
	}
	else
	{
//		if (_debug) printf("FloatVar::WriteEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool FloatVar::ReadEvent(word startAdr, word count, DataPointer data) const
{
	if (startAdr == address && count == SIZE)
	{
		*data.f = ReverseFloat(var.v);

//		if (_debug) printf("FloatVar::ReadEvent: 0x%X, %i - OK!\n", startAdr, count);
		return true;
	}
	else
	{
//		if (_debug) printf("FloatVar::ReadEvent: 0x%X, %i - ERROR!\n", startAdr, count);
		return false;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _TEST_

byte FloatVar::testsCount = 0;

bool FloatVar::SelfTest()
{
	word CRC1, CRC2;

//	bool ret = true;

	byte store1[sizeof(FloatVar)];
	byte store2[sizeof(FloatVar)];

	FarPointerCRC p(store1);

	word p_address;
	float p_value, p_max, p_min;
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
		printf("ERROR in FloatVar::Save(FarPointer, DataCRC&) or FloatVar::Load(FarPointer, DataCRC&)\n");
		return false;
	};

	printf("FloatVar::SelfTest ... OK!!!\n");
	return true;
}
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
