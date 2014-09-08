#include "hardware.h"
#include "ComPort.h"

static ComPort com1;
static ComPort::WriteBuffer wb;
static ComPort::ReadBuffer rb;

//static byte data[256*48];

static u32 rdata[10]; 
static u16 tdata[2100];

static bool ready1 = false, ready2 = false;

static MRB mrb;
static MTB mtb;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;

	static u32 pt = 0;
	static byte c = 0;
	static u16 i = 0;

	TM32 tm;

	InitHardware();

	com1.Connect(115200, 0);

	//for (u16 i = 0; i < sizeof(data); i++) { data[i] = i; };
	//wb.data = data;
	//wb.len = sizeof(data);

	//com1.Write(&wb);
//	ReadSPORT(spd1, spd2, sizeof(spd1), sizeof(spd2), 19, &ready1, &ready2);

	//*pPORTFIO_SET = 1<<5;
	//*pPORTFIO_CLEAR = 1<<7;

	i = 2;

	while (1)
	{
//		*pPORTFIO_TOGGLE = 1<<2;

		//u32 t = GetRTT();

		switch (i)
		{
			case 0:

				if (tm.Check(10000000))
				{
					wb.data = tdata;
					wb.len = 5;
					tdata[0]++; tdata[1]++; tdata[2] = 0x5555; 
					com1.Write(&wb);
					i++;
				};

				break;

			case 1:

				if (!com1.Update())
				{
					rb.data = rdata;
					rb.maxLen = sizeof(rdata);
					com1.Read(&rb, (u32)-1, 100000);
					i++;
				};

				break;

			case 2:

				if (!com1.Update())
				{
					i = 0;
				};

				break;
		};

//		UpdateHardware();

		//if (!com1.Update())
		//{
		//	com1.Write(&wb);
		//};


//		*pPORTFIO_TOGGLE = 1<<2;

	};

	return 0;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//bool Request(MRB *mrb, MTB *mtb)
//{
//	if (!mrb->OK || mrb->data == 0 || mrb->len == 0) return false;
//
//	bool result = false;
//
//	u32 *p = (u32*)mrb->data;
//
//	u16 func = (u16)(p[0]>>1);
//	u16 t;
//
//	tdata[0] = func;
//	mtb->data = tdata;
//	mtb->len = 1;
//
//	switch (func)
//	{
//		case 0xA900:
//
//			tdata[0] = 0xA900;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			mtb->data = tdata;
//			mtb->len = 3;
//			result = true;
//
//			break;
//
//		case 0xA910:
//
//			tdata[0] = 0xA910;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			tdata[6] = 1;
//			tdata[7] = 1;
//			tdata[8] = 1;
//			tdata[9] = 1;
//			tdata[10] = 1;
//			mtb->data = tdata;
//			mtb->len = 11;
//			result = true;
//
//			break;
//
//		case 0xA920:
//
//			tdata[0] = 0xA920;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			tdata[6] = 1;
//			tdata[7] = 1;
//			tdata[8] = 1;
//			tdata[9] = 1;
//			tdata[10] = 1;
//			mtb->data = tdata;
//			mtb->len = 11;
//			result = true;
//
//			break;
//
//		case 0xA930:
//		case 0xA931:
//		case 0xA932:
//		case 0xA933:
//		case 0xA934:
//		case 0xA935:
//		case 0xA936:
//		case 0xA937:
//		case 0xA938:
//		case 0xA939:
//
//			tdata[0] = func;
//			tdata[1] = 1;
//			tdata[2] = 1;
//			tdata[3] = 1;
//			tdata[4] = 1;
//			tdata[5] = 1;
//			mtb->data = tdata;
//			mtb->len = 6;
//
//			result = true;
//
//			if (mrb->len == 1)
//			{
//				mtb->len += 2048;
//			}
//			else if (mrb->len == 3)
//			{
//				mtb->len += (u16)(p[2]>>1);
//			};
//
//			break;
//
//		case 0xA990:
//
//			t = (u16)(p[1]>>1);
//
//			if (mrb->len >= 3 && (t < 3))
//			{
//				result = true;
//
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				if (t == 2)
//				{
//					t = (u16)(p[2]>>1);
//
//					if (t < 4)
//					{
//						SetTrmBoudRate(t);
//						result = true;
//					}
//					else
//					{
//						result = false;
//					};
//				};
//			};
//
//			break;
//
//		case 0xA9A0:
//
//			if (mrb->len >= 3)
//			{
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				result = true;
//			};
//
//			break;
//
//		case 0xA9F0:
//
//			if (mrb->len >= 1)
//			{
//				tdata[0] = func;
//				mtb->data = tdata;
//				mtb->len = 1;
//
//				result = true;
//			};
//
//			break;
//
//		default:
//
//			result = false;
//
//			break;
//	};
//
//	return result;
//
//
//};
