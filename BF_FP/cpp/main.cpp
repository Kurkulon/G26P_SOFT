/*******************************************************************************
*
*   (C) Copyright 2004 - Analog Devices, Inc.  All rights reserved.
*
*    FILE:     main.c
*
*    CHANGES:  1.00.0  - initial release
*              1.00.1  - modified to support the at25df021 flash library
*			   1.00.2  - need to set ADI_DEV_1D_BUFFER.pNext to NULL
*						 before all reads and writes
*			   2.00.1  - not allocating enough memory for pSectorInfo
*			   2.00.2  - Compare was failing because one of the adi_dev_Control
*			    		 calls in ReadData was passing in NULL instead of the
*						 device handle
*			   2.10.0  - now using non-SS/DD model to access flash
*
*    PURPOSE:  VisualDSP++ "Flash Programmer" flash interface application for use
*              with hardware containing the Numonyx M25P16 flash device.
*
*******************************************************************************/


#include <stdlib.h>					// malloc includes
#include <drivers\flash\util.h>
#include <drivers\flash\Errors.h>
#include "m25p16.h"

#include "types.h"

static char 	*pFlashDesc =		"Atmel AT25DF021";
static char 	*pDeviceCompany	=	"Atmel Corporation";

#define FLASH_START_ADDR	0x00000000
#define BUFFER_SIZE			0x4000

byte buffer[BUFFER_SIZE];

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

//Flash Programmer commands
typedef enum
{
	FLASH_NO_COMMAND,		// 0
	FLASH_GET_CODES,		// 1
	FLASH_RESET,			// 2
	FLASH_WRITE,			// 3
	FLASH_FILL,				// 4
	FLASH_ERASE_ALL,		// 5
	FLASH_ERASE_SECT,		// 6
	FLASH_READ,				// 7
	FLASH_GET_SECTNUM,		// 8
	FLASH_GET_SECSTARTEND,	// 9
}enProgCmds;

//----- g l o b a l s -----//

char 			*AFP_Title ;							// EzKit info
char 			*AFP_Description;						// Device Description
char			*AFP_DeviceCompany;						// Device Company
char 			*AFP_DrvVersion		= "1.01.0";			// Driver Version
char			*AFP_BuildDate		= __DATE__;			// Driver Build Date
enProgCmds 		AFP_Command 		= FLASH_NO_COMMAND;	// command sent down from the GUI
int 			AFP_ManCode 		= -1;				// 0x20 = Numonyx
int 			AFP_DevCode 		= -1;				// 0x15 = M25P16
unsigned long 	AFP_Offset 			= 0x0;				// offset into flash
int 			*AFP_Buffer;							// buffer used to read and write flash
long 			AFP_Size 			= BUFFER_SIZE;		// buffer size
long 			AFP_Count 			= -1;				// count of locations to be read or written
long 			AFP_Stride 			= -1;				// stride used when reading or writing
int				AFP_SectorSize		= -1;
int 			AFP_NumSectors 		= -1;				// number of sectors in the flash device
int 			AFP_Sector 			= -1;				// sector number
int 			AFP_Error 			= NO_ERR;			// contains last error encountered
bool 			AFP_Verify 			= FALSE;			// verify writes or not
unsigned long 	AFP_StartOff 		= 0x0;				// sector start offset
unsigned long 	AFP_EndOff 			= 0x0;				// sector end offset
int				AFP_FlashWidth		= 0x8;				// width of the flash device
//int 			*AFP_SectorInfo;

bool bExit = FALSE; 								//exit flag

static char *pEzKitTitle = "ADSP-BF592 GEOFT";


//----- c o n s t a n t   d e f i n i t i o n s -----//

// structure for flash sector information

typedef struct _SECTORLOCATION
{
 	unsigned long ulStartOff;
	unsigned long ulEndOff;
}SECTORLOCATION;


//----- f u n c t i o n   p r o t o t y p e s -----//
ERROR_CODE AllocateAFPBuffer(void);
ERROR_CODE GetSectorMap(SECTORLOCATION *pSectInfo);
ERROR_CODE GetFlashInfo(void);
ERROR_CODE ProcessCommand(void);
ERROR_CODE FillData( unsigned long ulStart, long lCount, long lStride, int *pnData );
ERROR_CODE ReadData( unsigned long ulStart, long lCount, long lStride, int *pnData );
ERROR_CODE WriteData( unsigned long ulStart, long lCount, long lStride, int *pnData );
void FreeAFPBuffer(void);
void InitPLL_SDRAM(void);
extern ERROR_CODE SetupForFlash(void);


//------------- m a i n ( ) ----------------//

int main(void)
{
//	SECTORLOCATION *pSectorInfo;
    ERROR_CODE Result;							// result

    /* open flash driver */
	AFP_Error = at25df021_Open();

	if (AFP_Error != NO_ERR)
		return FALSE;

	// get flash manufacturer & device codes, title & desc
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = GetFlashInfo();
	}

	// get the number of sectors for this device
	if( AFP_Error == NO_ERR )
	{
		AFP_NumSectors = 4;//GetNumSectors();
		AFP_SectorSize = GetSectorSize();
	}

	//if( AFP_Error == NO_ERR )
	//{
	//	// malloc enough space to hold our start and end offsets
	//	pSectorInfo = (SECTORLOCATION *)malloc(AFP_NumSectors * sizeof(SECTORLOCATION));
	//}

	// allocate AFP_Buffer
	if( AFP_Error == NO_ERR )
	{
		AFP_Error = AllocateAFPBuffer();
	}

	// get sector map
	//if( AFP_Error == NO_ERR )
	//{
	//	AFP_Error = GetSectorMap(pSectorInfo);
	//}

	// point AFP_SectorInfo to our sector info structure
	if( AFP_Error == NO_ERR )
	{
//		AFP_SectorInfo = (int*)pSectorInfo;
	}

	// command processing loop
	while ( !bExit )
	{
		// the plug-in will set a breakpoint at "AFP_BreakReady" so it knows
		// when we are ready for a new command because the DSP will halt
		//
		// the jump is used so that the label will be part of the debug
		// information in the driver image otherwise it may be left out
 		// since the label is not referenced anywhere
		asm("AFP_BreakReady:");
       		asm("nop;");
			if ( FALSE )
				asm("jump AFP_BreakReady;");

		// Make a call to the ProcessCommand
		   AFP_Error = ProcessCommand();
	}

	// Clear the AFP_Buffer
	FreeAFPBuffer();

	//if( pSectorInfo )
	//{
	//	free(pSectorInfo);
	//	pSectorInfo = NULL;
	//}

	// Close the Device
	AFP_Error = at25df021_Close();

	if (AFP_Error != NO_ERR)
		return FALSE;

	return TRUE;
}


//----------- P r o c e s s   C o m m a n d  ( ) ----------//
//
//  PURPOSE
//  	Process each command sent by the GUI based on the value in
//  	the AFP_Command.
//
// 	RETURN VALUE
//  	ERROR_CODE - value if any error occurs during Opcode scan
//  	NO_ERR     - otherwise
//
// 	CHANGES
//  	9-28-2005 Created

ERROR_CODE ProcessCommand()
{

	ERROR_CODE ErrorCode = 	NO_ERR; 		// return error code

	COMMAND_STRUCT CmdStruct;

//	AFP_Command = FLASH_ERASE_ALL;

	// switch on the command and fill command structure.
	switch ( AFP_Command )
	{

		// erase all
		case FLASH_ERASE_ALL:
			ErrorCode = EraseFlash();
			break;

		// erase sector
		case FLASH_ERASE_SECT:
			ErrorCode = EraseBlock(AFP_Sector);
			break;

		// fill
		case FLASH_FILL:
			ErrorCode = FillData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
			break;

		// get manufacturer and device codes
		case FLASH_GET_CODES:
			ErrorCode = GetCodes(&AFP_ManCode, &AFP_DevCode);
			break;

		// get sector number based on address
		case FLASH_GET_SECTNUM:
			ErrorCode = GetSectorNumber(AFP_Offset, &AFP_Sector);
			break;

		// get sector number start and end offset
		case FLASH_GET_SECSTARTEND:
			ErrorCode = GetSectorStartEnd(&AFP_StartOff, &AFP_EndOff,  AFP_Sector);
			break;

		// read
		case FLASH_READ:
			ErrorCode = ReadData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
			break;

		// reset
		case FLASH_RESET:
			ErrorCode = ResetFlash();
			break;

		// write
		case FLASH_WRITE:
			ErrorCode = WriteData( AFP_Offset, AFP_Count, AFP_Stride, AFP_Buffer );
			break;

		// no command or unknown command do nothing
		case FLASH_NO_COMMAND:
		default:
			// set our error
			ErrorCode = UNKNOWN_COMMAND;
			break;
	}

	// clear the command
	AFP_Command = FLASH_NO_COMMAND;

	return(ErrorCode);
}


//----------- A l l o c a t e A F P B u f f e r  ( ) ----------//
//
//  PURPOSE
//  	Allocate memory for the AFP_Buffer
//
// 	RETURN VALUE
//  	ERROR_CODE - value if any error occurs
//  	NO_ERR     - otherwise
//
// 	CHANGES
//  	9-28-2005 Created

ERROR_CODE AllocateAFPBuffer()
{

	ERROR_CODE ErrorCode = NO_ERR;	//return error code

	// by making AFP_Buffer as big as possible the plug-in can send and
	// receive more data at a time making the data transfer quicker
	//
	// by allocating it on the heap the compiler does not create an
	// initialized array therefore making the driver image smaller
	// and faster to load
	//
	// The linker description file (LDF) could be modified so that
	// the heap is larger, therefore allowing the BUFFER_SIZE to increase.

	// the data type of the data being sent from the flash programmer GUI
	// is in bytes but we store the data as integers to make data
	// manipulation easier when actually programming the data.  This is why
	// BUFFER_SIZE bytes are being allocated rather than BUFFER_SIZE * sizeof(int).
	
//	AFP_Buffer = (int *)malloc(BUFFER_SIZE);

	AFP_Buffer = (int *)buffer;

	// AFP_Buffer will be NULL if we could not allocate storage for the
	// buffer
	if ( AFP_Buffer == NULL )
	{
		// tell GUI that our buffer was not initialized
		ErrorCode = BUFFER_IS_NULL;
	}

	return(ErrorCode);
}


//----------- F r e e A F P B u f f e r  ( ) ----------//
//
//  PURPOSE
//  	Free the AFP_Buffer
//
// 	RETURN VALUE
//  	ERROR_CODE - value if any error occurs
//  	NO_ERR     - otherwise
//
// 	CHANGES
//  	9-28-2005 Created

void FreeAFPBuffer()
{
	// free the buffer if we were able to allocate one
//	if ( AFP_Buffer )
//		free( AFP_Buffer );

}


//----------- G e t F l a s h I n f o  ( ) ----------//
//
//  PURPOSE
//  	Get the manufacturer code and device code
//
// 	RETURN VALUE
//  	ERROR_CODE - value if any error occurs
//  	NO_ERR     - otherwise
//
// 	CHANGES
//  	9-28-2005 Created

ERROR_CODE GetFlashInfo()
{

	ERROR_CODE ErrorCode = NO_ERR;		//return error code

	//setup code so that flash programmer can just read memory instead of call GetCodes().

	ErrorCode = GetCodes(&AFP_ManCode, &AFP_DevCode);

	if(!ErrorCode)
	{
		AFP_Title = pEzKitTitle;
		AFP_Description = pFlashDesc;
		AFP_DeviceCompany = pDeviceCompany;
	}

	return(ErrorCode);
}


//----------- F i l l D a t a  ( ) ----------//
//
//  PURPOSE
//  	Fill flash device with a value.
//
// 	INPUTS
//	 	unsigned long ulStart - address in flash to start the writes at
// 		long lCount - number of elements to write, in this case bytes
// 		long lStride - number of locations to skip between writes
// 		int *pnData - pointer to data buffer
//
// 	RETURN VALUE
// 		ERROR_CODE - value if any error occurs during fill
// 		NO_ERR     - otherwise
//
// 	CHANGES
//  	9-28-2005 Created

ERROR_CODE FillData( unsigned long ulStart, long lCount, long lStride, int* pnData )
{
	long i = 0;							// loop counter
	ERROR_CODE ErrorCode = NO_ERR;		// tells whether we had an error while filling
	bool bVerifyError = FALSE;			// lets us know if there was a verify error
	unsigned long ulNewOffset = 0;		// used if we have an odd address
	bool bByteSwapMid = FALSE;			// if writing to odd address the middle needs byte swapping
	unsigned long ulStartAddr;   		// current address to fill
	unsigned long ulSector = 0;			// sector number to verify address
	int nCompare = 0;					// value that we use to verify flash

	ulStartAddr = FLASH_START_ADDR + ulStart;
	COMMAND_STRUCT	CmdStruct;	//structure for GetSectStartEnd

	// if we have an odd offset we need to write a byte
	// to the first location and the last
	//if(ulStartAddr%4 != 0)
	//{
	//	// read the offset - 1 and OR in our value
	//	ulNewOffset = ulStartAddr - 1;
	//	ErrorCode = at25df021_Read( (unsigned short*)&nCompare, ulNewOffset, 0x1 );

	//	nCompare &= 0x00FF;
	//	nCompare |= ( (pnData[0] << 8) & 0xFF00 );

	//	// unlock the flash, do the write, and wait for completion
	//	ErrorCode = at25df021_Write( (unsigned short*)&nCompare, ulNewOffset, 0x1 );

	//	// move to the last offset
	//	ulNewOffset = ( (ulStartAddr - 1) + (lCount * ( lStride  ) ) );

	//	// read the value and OR in our value
	//	ErrorCode = at25df021_Read( (unsigned short*)&nCompare, ulNewOffset, 0x1 );

	//	nCompare &= 0xFF00;
	//	nCompare |= ( ( pnData[0] >> 8) & 0x00FF );

	//	// unlock the flash, do the write, and wait for completion
	//	ErrorCode = at25df021_Write( (unsigned short*)&nCompare, ulNewOffset, 0x1 );

	//	// increment the offset and count
	//	ulStartAddr = ( (ulStartAddr - 1) + ( lStride  ) );
	//	lCount--;

	//	bByteSwapMid = TRUE;
	//}

	//if( bByteSwapMid == TRUE )
	//{
	//	int nTemp = (char)pnData[0];
	//	pnData[0] &= 0xFF00;
	//	pnData[0] >>= 8;
	//	pnData[0] |= (nTemp << 8);
	//}

	//// verify writes if the user wants to
	//if( AFP_Verify == TRUE )
	//{
	//	// fill the value
	//	for (i = 0; ( ( i < lCount ) && ( ErrorCode == NO_ERR ) ); i++, ulStartAddr += ( lStride ) )
	//	{

	//		// check to see that the address is within a valid sector
	//		CmdStruct.SGetSectNum.ulOffset = ulStartAddr;
	//		CmdStruct.SGetSectNum.pSectorNum = &ulSector;
	//		ErrorCode = at25df021_Control( CNTRL_GET_SECTNUM, &CmdStruct  );

	//		if( NO_ERR == ErrorCode )
	//		{
	//			// unlock the flash, do the write, and wait for completion
	//			ErrorCode = at25df021_Write( (unsigned short*)&pnData[0], ulStartAddr, 0x1 );
	//			ErrorCode = at25df021_Read( (unsigned short*)&nCompare, ulStartAddr, 0x1 );

	//			if( nCompare != ( pnData[0] & 0x0000FFFF ) )
	//			{
	//				bVerifyError = TRUE;
	//				break;
	//			}
	//		}
	//		else
	//		{
	//			return ErrorCode;
	//		}

	//	}

	//	// return appropriate error code if there was a verification error
	//	if( bVerifyError == TRUE )
	//		return VERIFY_WRITE;
	//}
	//// user did not want to verify writes
	//else
	//{
	//	// fill the value
	//	for (i = 0; ( ( i < lCount ) && ( ErrorCode == NO_ERR ) ); i++, ulStartAddr += ( lStride ))
	//	{

	//		// check to see that the address is within a valid sector
	//		CmdStruct.SGetSectNum.ulOffset = ulStartAddr;
	//		CmdStruct.SGetSectNum.pSectorNum = &ulSector;
	//		ErrorCode = at25df021_Control( CNTRL_GET_SECTNUM, &CmdStruct  );

	//		if( NO_ERR == ErrorCode )
	//		{
	//			// unlock the flash, do the write, and wait for completion
	//			ErrorCode = at25df021_Write( (unsigned short*)&pnData[0], ulStartAddr, 0x1 );
	//		}
	//		else
	//		{
	//			return ErrorCode;
	//		}
	//	}
	//}

	// return the appropriate error code
	return ErrorCode;
}

//----------- W r i t e D a t a  ( ) ----------//
//
//  PURPOSE
//  	Write a buffer to flash device.
//
// 	INPUTS
// 		unsigned long ulStart - address in flash to start the writes at
//		long lCount - number of elements to write, in this case bytes
//		long lStride - number of locations to skip between writes
//		int *pnData - pointer to data buffer
//
//  RETURN VALUE
//  	ERROR_CODE - value if any error occurs during writing
//  	NO_ERR     - otherwise
//
//  CHANGES
//  	9-28-2005 Created

ERROR_CODE WriteData( unsigned long ulStart, long lCount, long lStride, int *pnData )
{
	// if the user wants to verify then do it

	return at25df021_Write((byte*)pnData, ulStart, lCount, AFP_Verify);
}

//----------- R e a d D a t a  ( ) ----------//
//
//  PURPOSE
//  	Read a buffer from flash device.
//
// 	INPUTS
//		unsigned long ulStart - address in flash to start the reads at
//		long lCount - number of elements to read, in this case bytes
//		long lStride - number of locations to skip between reads
//		int *pnData - pointer to data buffer to fill
//
//	RETURN VALUE
//  	ERROR_CODE - value if any error occurs during reading
//  	NO_ERR     - otherwise
//
//  CHANGES
//  	9-28-2005 Created

ERROR_CODE ReadData( unsigned long ulStart, long lCount, long lStride, int *pnData )
{
	return at25df021_Read((byte*)pnData, ulStart, lCount);
}

