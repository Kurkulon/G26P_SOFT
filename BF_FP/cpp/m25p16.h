/*******************************************************************************/
/*                                                                             */
/*   (C) Copyright 2008 - Analog Devices, Inc.  All rights reserved.           */
/*                                                                             */
/*    FILE:     at25df021.h                                                       */
/*																			   */
/*    PURPOSE:  This header file defines items specific to the M25P16 flash.   */
/*                                                                             */
/*******************************************************************************/

#ifndef __M25P16_H__
#define __M25P16_H__

#include "types.h"

/* application definitions */
#define COMMON_SPI_SETTINGS (SPE|MSTR|CPOL|CPHA)  /* settings to the SPI_CTL */
#define TIMOD01 (0x01)                  /* sets the SPI to work with core instructions */

#define BAUD_RATE_DIVISOR 	2
#define PE4 0x0010

/* interface function prototypes */
ERROR_CODE at25df021_Open(void);						/* open the flash device */
ERROR_CODE at25df021_Close(void);						/* close the flas device */

extern ERROR_CODE at25df021_Read(byte *data, u32 stAdr, u32 count );
extern ERROR_CODE at25df021_Write(byte *data, u32 stAdr, u32 count, bool verify);

ERROR_CODE at25df021_Control(unsigned int uiCmd,		/* send a command to device */
						  COMMAND_STRUCT *pCmdStruct);

extern ERROR_CODE GetCodes(int *pnManCode, int *pnDevCode);
extern ERROR_CODE GetSectorStartEnd( unsigned long *ulStartOff, unsigned long *ulEndOff, int nSector );
extern ERROR_CODE GetSectorNumber( unsigned long ulAddr, int *pnSector );
extern ERROR_CODE EraseFlash();
extern ERROR_CODE EraseBlock(int nBlock);
extern ERROR_CODE ResetFlash();
extern u32 GetNumSectors();
extern u32 GetSectorSize();



#endif	/* __M25P16_H__ */
