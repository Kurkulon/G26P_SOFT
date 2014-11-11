#include "hardware.h"
#include "time.h"
#include "ComPort.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 fps = 0;

static ComPort com;
static ComPort::WriteBuffer wb;
static ComPort::ReadBuffer rb;

static byte data[256];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	byte i = 0;
	u32 j = 0;

	TM32	tm;

	InitHardware();

	com.Connect(0, 1562500, 2);
	
	while (1)
	{
		UpdateHardware();

		if (!com.Update())
		{
			if (tm.Check(1000))
			{
				rb.data = data;
				rb.maxLen = sizeof(data);
				com.Read(&rb, -1, 800);
				i = 0;
			};
		};

		if (HW::USART0->STAT & 4)
		{
			HW::GPIO->SET0 = 1;
			HW::USART0->TXDATA = i++;
		};
	};
}
