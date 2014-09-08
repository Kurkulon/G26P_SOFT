#include "ComPort.h"
#include "time.h"

ComPort com1;
ComPort combf;

ComPort::WriteBuffer wb;
ComPort::ReadBuffer rb;

byte buf[1024];

u32 fc = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	static byte i = 0;

	RTT_Init();

	for (u32 i = 0; i < sizeof(buf); i++)
	{
		buf[i] = 0x0F;
	};

	wb.data = buf;
	wb.len = sizeof(buf);
	
	com1.Connect(0, 6250000, 0);
	combf.Connect(3, 115200, 0);

//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

	while(1)
	{
		switch(i)
		{
			case 0:

				rb.data = buf;
				rb.maxLen = sizeof(buf);
				combf.Read(&rb, -1, MS2RT(2));
				i++;

				break;

			case 1:
	
				if (!combf.Update())
				{
					if (rb.recieved && rb.len > 0)
					{
						wb.data = rb.data;
						wb.len = rb.len;
						combf.Write(&wb);
						i++;
					}
					else
					{
						i = 0;
					};
				};

				break;

			case 2:

				if (!combf.Update())
				{
					i = 0;
				};

				break;
		};

		fps++;

		if (rtm.Check(32768))
		{ 
			fc = fps; fps = 0; 
			com1.TransmitByte(0);
		};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
