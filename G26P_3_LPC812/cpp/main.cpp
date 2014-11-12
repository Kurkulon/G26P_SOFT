#include "hardware.h"
#include "time.h"
#include "ComPort.h"
#include "crc16.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 fps = 0;

static ComPort com;
static ComPort::WriteBuffer wb;
static ComPort::ReadBuffer rb;

static byte data[256];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateCom()
{
	static byte i = 0;

	switch(i)
	{ 
		case 0:

			rb.data = data;
			rb.maxLen = sizeof(data);
			com.Read(&rb, -1, 800);
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved)
				{
					//GetCRC(rb.data, rb.len);
					//GetCRC16(rb.data, rb.len);
					wb.data = rb.data;
					wb.len = rb.len;
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

int main()
{
	byte i = 0;
	u32 j = 0;

	TM32	tm;

	InitHardware();

	com.Connect(0, 1562500, 0);
	
	while (1)
	{
		UpdateHardware();
		UpdateCom();

		HW::GPIO->SET0 = 1<<7;
		GetCRC16(data, sizeof(data));
		HW::GPIO->CLR0 = 1<<7;
	};
}
