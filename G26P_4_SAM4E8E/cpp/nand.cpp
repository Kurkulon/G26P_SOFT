#include "nand.h"
#include "core.h"

static byte data[2112];

byte flashID[5];

byte * const FLC = (byte*)0x60400000;	
byte * const FLA = (byte*)0x60200000;	

volatile byte * const FLD = (byte*)0x60000000;	

static byte stateNAND = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool FlashReady()
{
	return (HW::PIOC->PDSR & (1UL<<31)) != 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline bool FlashBusy()
{
	return (HW::PIOC->PDSR & (1UL<<31)) == 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void BlockEraseAsync(u32 adr)
{
	*FLC = 0x60;
	*FLA = GD(&adr, u8, 0);
	*FLA = GD(&adr, u8, 1);
	*FLA = GD(&adr, u8, 2);
	*FLC = 0xD0;

	while(FlashReady()) ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void PageReadAsync(u32 adr)
{
	*FLC = 0x00;
	*FLA = GD(&adr, u8, 0);
	*FLA = GD(&adr, u8, 1) & 0xF;

	adr >>= 12;

	*FLA = GD(&adr, u8, 0);
	*FLA = GD(&adr, u8, 1);
	*FLA = GD(&adr, u8, 2) & 7;

	*FLC = 0x30;

	while(FlashReady()) ;

	while(FlashBusy()) ;

	using namespace HW;

	register u32 t;

	DMAC->EN = 1;

	t = DMAC->EBCISR;
	DMAC->CH[0].SADDR = (u32*)0x60000000;
	DMAC->CH[0].DADDR = data;
	DMAC->CH[0].DSCR = 0;
	DMAC->CH[0].CTRLA = sizeof(data);
	DMAC->CH[0].CTRLB = (1<<16)|(1<<20);
	DMAC->CH[0].CFG = 0;
	DMAC->CHER = 1;

	while (DMAC->CHSR & 1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void StartDMA()
{
	using namespace HW;

	register u32 t;

	DMAC->EN = 1;

	if (DMAC->CHSR & 1)
	{
		return;
	};

	t = DMAC->EBCISR;
	DMAC->CH[0].SADDR = (u32*)0x20000000;
	DMAC->CH[0].DADDR = (u32*)0x60000000;
	DMAC->CH[0].DSCR = 0;
	DMAC->CH[0].CTRLA = 2048;
	DMAC->CH[0].CTRLB = (1<<16)|(1<<20);
	DMAC->CH[0].CFG = 0;
	DMAC->CHER = 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateNAND()
{
	switch (stateNAND)
	{
		case 0:	// Idle

			break;

		case 1: // Read ID

			*FLC = 0x90;
			*FLA = 0;

			flashID[0] = *FLD;
			flashID[1] = *FLD;
			flashID[2] = *FLD;
			flashID[3] = *FLD;
			flashID[4] = *FLD;


	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



