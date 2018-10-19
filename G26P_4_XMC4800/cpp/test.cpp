#include "emac.h"
#include "core.h"
#include "xtrap.h"
#include "flash.h"
#include "time.h"
#include "ComPort.h"

#pragma diag_suppress 546,550,177

//#pragma O3
//#pragma Otime

byte buf[4000];

static ComPort com1;

u32 fps = 0;
u32 f = 0;

ComPort::WriteBuffer wb = { .transmited = false, .len = 10, .data = buf };

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;
	static TM32 tm;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateEMAC();	);
		CALL( UpdateTraps();	);
		CALL( if (tm.Check(1000)) {	HW::P5->BTGL(8); fps = f; f = 0; com1.Write(&wb); } else { com1.Update(); };	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( NAND_Idle();		);
		CALL( UpdateMisc();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
	using namespace HW;

	register u32 t __asm("r0");

	HW::GPDMA1->DMACFGREG = 1;

	if ((HW::GPDMA1->CHENREG & (1<<3)) == 0)
	{
		HW::P5->BTGL(9);

		HW::GPDMA1_CH3->CTLL = DINC(0)|SINC(0)|TT_FC(0)|DEST_MSIZE(1)|SRC_MSIZE(1);
		HW::GPDMA1_CH3->CTLH = BLOCK_TS(len);

	//	t = DMAC->EBCISR;

		HW::GPDMA1_CH3->SAR = (u32)src;
		HW::GPDMA1_CH3->DAR = (u32)dst;
		HW::GPDMA1_CH3->CFGL = 0;
		HW::GPDMA1_CH3->CFGH = PROTCTL(1);

		//HW::GPDMA1->CLEARBLOCK = 1<<3;
		//HW::GPDMA1->CLEARDSTTRAN = 1<<3;
		//HW::GPDMA1->CLEARERR = 1<<3;
		//HW::GPDMA1->CLEARSRCTRAN = 1<<3;
		//HW::GPDMA1->CLEARTFR = 1<<3;

		HW::GPDMA1->CHENREG = 0x101<<3;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	com1.Connect(1, 6250000, 0);

	InitTimer();

	InitEMAC();

	InitTraps();

	FLASH_Init();

	while(1)
	{
		f++;

		Update();
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

