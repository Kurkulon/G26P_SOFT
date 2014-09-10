#include "ComPort.h"
#include "time.h"
#include "CRC16.h"

ComPort com1;
ComPort combf;
ComPort comrcv;

//ComPort::WriteBuffer wb;
//ComPort::ReadBuffer rb;


u32 fc = 0;

static u32 bfCRCOK = 0;
static u32 bfCRCER = 0;

static u32 rcvCRCOK = 0;
static u32 rcvCRCER = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			combf.Read(&rb, -1, 1);
			i++;

			break;

		case 1:

			if (!combf.Update())
			{
				if (rb.recieved && rb.len > 0)
				{
					if (GetCRC16(rb.data, rb.len) == 0)	bfCRCOK++;	else	bfCRCER++;


					wb.data = rb.data;
					wb.len = rb.len;

					DataPointer p(wb.data);
					p.b += wb.len;
					*p.w = GetCRC16(wb.data, wb.len);
					wb.len += 2;

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
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateRecievers()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];
	static 	RTM32 rtm;
			
	DataPointer p(buf);

	switch(i)
	{
		case 0:
			
//			if (rtm.Check(MS2RT(1000)))
			{ 
				p.v = wb.data = buf;
				wb.len = 100;
				buf[0]++; buf[1]--; buf[2] += 11; 
				p.b += wb.len;
				*p.w = GetCRC16(wb.data, wb.len);
				wb.len += 2;
				comrcv.Write(&wb);
				i++;
			};

			break;

		case 1:

			if (!comrcv.Update())
			{
				rb.data = buf;
				rb.maxLen = sizeof(buf);
				comrcv.Read(&rb, MS2RT(100), 1);
				i++;
			};

			break;

		case 2:

			if (!comrcv.Update())
			{
				if (GetCRC16(rb.data, rb.len) == 0)
				{
					rcvCRCOK++;
				}
				else
				{
					rcvCRCER++;
				};

				i = 0;
			};

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateBlackFin()		);
		CALL( UpdateRecievers()		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


int main()
{
//	static byte i = 0;

	RTT_Init();


	com1.Connect(0, 6250000, 0);
	combf.Connect(3, 6250000, 0);
	comrcv.Connect(2, 6250000, 0);

//	com1.Write(&wb);

	u32 fps = 0;

	RTM32 rtm;

	while(1)
	{
		UpdateMisc();

		fps++;

		if (rtm.Check(32768))
		{ 
			fc = fps; fps = 0; 
			com1.TransmitByte(0);
		};
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
