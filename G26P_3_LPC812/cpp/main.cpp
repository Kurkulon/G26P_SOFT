#include "hardware.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 fps = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	byte i = 0;
	u32 j = 0;
	u32 c = 0;

	InitHardware();
	
	HW::SYSCON->SYSAHBCLKCTRL |= HW::CLK::MRT_M;

	HW::MRT->Channel[0].CTRL = 2;
	HW::MRT->Channel[0].INTVAL = (1UL<<31)|(1 * 25000000); 

	while (1)
	{
		HW::CRC->SUM = j++;

		UpdateHardware();

		if (HW::MRT->Channel[0].STAT & 1)
		{
			HW::MRT->Channel[0].STAT = 1;
			HW::MRT->Channel[0].INTVAL = (1UL<<31)|(1 * 25000000);
		
			if ((i&1) == 0) FireXX(); else FireYY();

			i++;

			fps = c; c = 0;
		};

		c++;

	};
}
