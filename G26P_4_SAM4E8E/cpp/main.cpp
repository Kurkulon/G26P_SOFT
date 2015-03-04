#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	InitEMAC();

	u32 fps = 0;

	HW::GMAC->PEFRN = 0x5A5A;


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

		fps++;
//		__asm { WFE };
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
