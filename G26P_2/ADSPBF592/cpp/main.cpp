#include "hardware.h"
#include "ComPort.h"

//static ComPort com1;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 data[2048]; 

static bool ready1 = false, ready2 = false;

int main( void )
{
	static byte s = 0;

	static u32 pt = 0;
	static byte c = 0;
	static u16 i = 0;

	TM32 tm;

	InitHardware();

	//com1.Connect(0, 6250000, 0);

	//for (u16 i = 0; i < sizeof(data); i++) { data[i] = i; };
	//wb.data = data;
	//wb.len = sizeof(data);

	//com1.Write(&wb);
//	ReadSPORT(spd1, spd2, sizeof(spd1), sizeof(spd2), 19, &ready1, &ready2);

	//*pPORTFIO_SET = 1<<5;
	//*pPORTFIO_CLEAR = 1<<7;

	while (1)
	{
//		*pPORTFIO_TOGGLE = 1<<2;

		//u32 t = GetRTT();

		//if ((t-pt) >= 2500)
		//{
		//	ManDisable();

		//	c ^= 1;

		//	if (c)
		//	{
		//		ManOne();
		//	}
		//	else
		//	{
		//		ManZero();
		//	};

		//	pt = t;
		//};

		if (tm.Check(100000000))
		{
			SendManCmd(data, 100);
		};


		UpdateHardware();

		//if (!com1.Update())
		//{
		//	com1.Write(&wb);
		//};


//		*pPORTFIO_TOGGLE = 1<<2;

	};

	return 0;
}
