#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#include "hardware.h"
#include "time.h"
#include "comport.h"

bool run = true;

static ComPort com;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateCom()
{
	static byte i = 0;

	static ComPort::ReadBuffer rb;
	static byte buf[5000];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			
			com.Read(&rb, -1, 10);

			i++;

			break;

		case 1:

			if (!com.Update())
			{
				DrawWave(0, buf+4);
				DrawWave(1, buf+4+1000);
				i = 0;
			};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	com.Connect("COM2", 921600, 0);
	
	while(1)
	{
		static byte i = 0;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateHardware();			);
			CALL( UpdateCom();			);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL
	};
}
