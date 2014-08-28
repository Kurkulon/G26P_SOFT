#include "ComPort.h"
#include "time.h"

ComPort com1;

ComPort::WriteBuffer wb;

byte buf[1024];

u32 fc = 0;

	 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	RTT_Init();

	for (u32 i = 0; i < sizeof(buf); i++)
	{
		buf[i] = 0x0F;
	};

	wb.data = buf;
	wb.len = sizeof(buf);
	
	com1.Connect(0, 6250000, 0);

//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

	while(1)
	{
		//if (!com1.Update())
		//{
		//	wb.data = buf;
		//	wb.len = sizeof(buf);
		//	com1.Write(&wb);
		//};

		fps++;

		if (rtm.Check(32768))
		{ 
			fc = fps; fps = 0; 
			com1.TransmitByte(0);
		};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
