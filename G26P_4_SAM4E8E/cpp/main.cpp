#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"
#include "xtrap.h"

u32 fps = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	InitEMAC();

	InitTraps();

	u32 f = 0;

	static RTM32 rtm;

	while(1)
	{
//		HW::PIOA->CODR = 1<<13;
		
//		PageReadAsync(0);

		//*FLC = 0x90;
		//*FLA = 0;

		//flashID[0] = *FLD;
		//flashID[1] = *FLD;
		//flashID[2] = *FLD;
		//flashID[3] = *FLD;
		//flashID[4] = *FLD;

		//HW::PIOA->SODR = 1<<13;

		UpdateEMAC();

		UpdateTraps();

		f++;

		if (rtm.Check(MS2RT(1000)))
		{
			fps = f;
			f = 0;
		};

//		__asm { WFE };
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
