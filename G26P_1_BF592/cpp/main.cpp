#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"

static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd1[512*2];
static u16 spd2[512*2];

static i16 ch1[512];
static i16 ch2[512];
static i16 ch3[512];
static i16 ch4[512];

static bool ready1 = false, ready2 = false;

static u32 CRCOK = 0;
static u32 CRCER = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, (u32)-1, 720);
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0)
				{
					if (GetCRC16(rb.data, rb.len) == 0)	CRCOK++;	else	CRCER++;

					wb.data = buf;
					wb.len = rb.len;

					//DataPointer p(wb.data);
					//p.b += wb.len;
					//*p.w = GetCRC16(wb.data, wb.len);
					//wb.len += 2;

					com.Write(&wb);
					i++;
				}
				else
				{
					i = 0;
				};
			};

			break;

		case 2:

			if (!com.Update())
			{
				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;

	static u32 pt = 0;

	InitHardware();

	com.Connect(6250000, 0);

	//for (u16 i = 0; i < sizeof(data); i++) { data[i] = i; };
	//wb.data = data;
	//wb.len = sizeof(data);

	//com1.Write(&wb);
	ReadSPORT(spd1, spd2, sizeof(spd1), sizeof(spd2), 19, &ready1, &ready2);

	//*pPORTFIO_SET = 1<<5;
	//*pPORTFIO_CLEAR = 1<<7;

	while (1)
	{
		WritePGA(0x2A61);
		//float v = ReadADC() * 3.3 / 4095;

		//u32 t = GetRTT();

		//if ((t-pt) >= 33333)
		//{
		//	*pPORTFIO_TOGGLE = 1<<5;
		//	*pPORTFIO_TOGGLE = 1<<7;
		//	pt = t;
		//};

		UpdateBlackFin();

		*pPORTFIO_TOGGLE = 1<<5;

//		UpdateHardware();

		//if (!com1.Update())
		//{
		//	com1.Write(&wb);
		//};

		if (ready1 && ready2)
		{
			for (u16 i = 0; i < 512; i++)
			{
				ch1[i] = spd1[i*2] - 0x7FFF;
				ch2[i] = spd1[i*2+1] - 0x7FFF;
				ch3[i] = spd2[i*2] - 0x7FFF;
				ch4[i] = spd2[i*2+1] - 0x7FFF;
			};

			*pPORTFIO_TOGGLE = 1<<7;
			ReadSPORT(spd1, spd2, sizeof(spd1), sizeof(spd2), 19, &ready1, &ready2);
			*pPORTFIO_TOGGLE = 1<<7;
		};

		*pPORTFIO_TOGGLE = 1<<5;

	};

	return 0;
}
