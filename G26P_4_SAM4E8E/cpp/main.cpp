#include "ComPort.h"
#include "time.h"
#include "CRC16.h"
#include "hardware.h"
#include "emac.h"
#include "xtrap.h"
#include "flash.h"
#include "vector.h"
#include "list.h"

u32 fps = 0;

extern byte Heap_Mem[10];

//byte data[1500];

u32 readsFlash = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateWriteFlash()
{
	static FLWB *fb = 0;
	static byte i = 0;
	static byte cnt = 1;
	static u32  w = 0;
	static bool ready;

	switch (i)
	{
		case 0:

			if ((fb = AllocFlashWriteBuffer()) != 0)
			{
				fb->ready = &ready;

				fb->hdrLen = 0;
				fb->dataLen = 2048;

				for (u32 n = 0; n < fb->dataLen; n++)
				{
					fb->data[n] = cnt;
				};

				cnt++;
				w++;

				RequestFlashWrite(fb);

				i++;
			};

			break;

		case 1:

			if (ready)
			{
				if (w >= 50000)
				{
					w = 0;
					NAND_NextSession();
				};

				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	InitEMAC();

	InitTraps();

	FLASH_Init();

	u32 f = 0;


	static RTM32 rtm;
	static RTM32 rtm2;


//	__breakpoint(0);

	while(1)
	{
		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateEMAC();		);
			CALL( UpdateTraps();	);
			CALL( NAND_Idle();		);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

		f++;

		if (rtm.Check(MS2RT(1000)))
		{
			fps = f;
			f = 0;
		};

//		__asm { WFE };
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
