#ifndef CORE_H__05_12_12__16_12
#define CORE_H__05_12_12__16_12

#define	CORETYPE_BF592

#define CLKIN 25000000
#define VRCTL_VALUE         0x0000

#define PLLDIV_CSEL			1	// 0 - CCLK = VCO / 1
								// 1 - CCLK = VCO / 2
								// 2 - CCLK = VCO / 4
								// 3 - CCLK = VCO / 8

#define PLLDIV_SSEL			2	// 1-15 - SCLK = VCO / X

#define PLLDIV_VALUE        ((PLLDIV_CSEL<<4)|PLLDIV_SSEL)

#define PLLCTL_MSEL			8

#define PLLCTL_VALUE        (PLLCTL_MSEL<<9)

#define PLLLOCKCNT_VALUE    0x0000
#define PLLSTAT_VALUE       0x0000
#define RSICLK_DIV          0x0001

#define VCO_VALUE (CLKIN*((PLLCTL_VALUE>>9)&63))
#define SCLK (VCO_VALUE / (PLLDIV_VALUE & 15))
#define CCLK (VCO_VALUE >> ((PLLDIV_VALUE >> 4) & 3))
//#define MCK CCLK

#define US2SCLK(x) ((u32)((x*(float)SCLK+500000)/1000000))
#define MS2SCLK(x) ((u32)((x*(float)SCLK+500)/1000))


#include "bf592.h"

#endif // CORE_H__05_12_12__16_12
