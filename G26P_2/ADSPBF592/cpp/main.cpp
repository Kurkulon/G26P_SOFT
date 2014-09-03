#include "hardware.h"
#include "ComPort.h"

//static ComPort com1;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u32 rdata[10]; 
static u16 tdata[10];

static bool ready1 = false, ready2 = false;

static MRB mrb;
static MTB mtb;

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

	i = 2;

	while (1)
	{
//		*pPORTFIO_TOGGLE = 1<<2;

		//u32 t = GetRTT();

		//if (tm.Check(1000000))
		//{
		//	SendManCmd(data, 10);
		//};


		switch (i)
		{
			case 0:

				if (mrb.ready && mrb.OK)
				{
					mtb.data = tdata;
					mtb.len = 1;
					tdata[0] = 0xA990;
					i = (SendManData(&mtb)) ? 1 : 2;
				};

				break;

			case 1:

				if (mtb.ready)
				{
					i++;
				};

				break;

			case 2:

				mrb.data = rdata;
				mrb.maxLen = ArraySize(rdata);
				RcvManData(&mrb);
				i = 0;

				break;
		};

//		UpdateHardware();

		//if (!com1.Update())
		//{
		//	com1.Write(&wb);
		//};


//		*pPORTFIO_TOGGLE = 1<<2;

	};

	return 0;
}
