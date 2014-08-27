#include "ComPort.h"

ComPort com1;

ComPort::WriteBuffer wb;

byte buf[1024];
	 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	for (u32 i = 0; i < sizeof(buf); i++)
	{
		buf[i] = 0x0F;
	};

	wb.data = buf;
	wb.len = sizeof(buf);
	
	com1.Connect(0, 6250000, 0);

	com1.Write(&wb);

	while(1)
	{
		if (!com1.Update())
		{
			wb.data = buf;
			wb.len = sizeof(buf);
			com1.Write(&wb);
		};

	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
