#include <types.h>
#include <core.h>
#include <SEGGER_RTT.h>
#include <list.h>

#include "hardware.h"
#include "hw_conf.h"
#include "hw_rtm.h"
#include "hw_nand.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <class T> List< PtrItem<T> > PtrItem<T>::_freeList;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//#define NAND_SAMSUNG
#define NAND_MICRON

#define NAND_CHIP_BITS		3
#define NAND_MAX_CHIP		(1<<NAND_CHIP_BITS)
#define NAND_CHIP_MASK		(NAND_MAX_CHIP-1)

#define LIST_ITEMS_NUM			128
#define FLASH_WRITE_BUFFER_NUM	8
#define FLASH_READ_BUFFER_NUM	8

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const bool verifyWritePage = false; // �������� ��������� ��������, ���� ������ �������� � ��������� � �������
static const bool verifySpare = true;	// �������� ��������� ��������, ���� ������ �������� � ��������� � �������

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static const bool forceEraseWrite = true;

static u32 chipSelect[NAND_MAX_CHIP] = { FCS0, FCS1, FCS2, FCS3, FCS4, FCS5, FCS6, FCS7 };

#define maskChipSelect (FCS0|FCS1|FCS2|FCS3|FCS4|FCS5|FCS6|FCS7)

static const char* chipRefDes[NAND_MAX_CHIP] = { "DD7 ", "DD8 ", "DD9 ", "DD10", "DD11", "DD12", "DD13", "DD14" };

#else

static const bool forceEraseWrite = true;

#endif

#ifdef CPU_SAME53	

	#define NAND_DIR_IN() { PIO_NAND_DATA->DIRCLR = 0xFF; }
	#define NAND_DIR_OUT() { PIO_NAND_DATA->DIRSET = 0xFF; }

#elif defined(CPU_XMC48)

	volatile byte * const FLC = (byte*)0x60000008;	
	volatile byte * const FLA = (byte*)0x60000010;	
	volatile byte * const FLD = (byte*)0x60000000;	

	#define NAND_DIR_IN() {}
	#define NAND_DIR_OUT() {}

	#define NAND_WAITRDC	NS2EBUCLK(60)	
	#define NAND_WAITWRC	NS2EBUCLK(45)	

#elif defined(WIN32)

	#define NAND_DIR_IN() {}
	#define NAND_DIR_OUT() {}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <hw_nand_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <flash_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
