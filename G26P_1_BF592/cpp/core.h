#ifndef CORE_H__05_12_12__16_12
#define CORE_H__05_12_12__16_12

#define	CORETYPE_BF592

#define CLKIN 25000000
#define VRCTL_VALUE         0x0000
#define PLLCTL_VALUE        0x1000
#define PLLDIV_VALUE        0x0012
#define PLLLOCKCNT_VALUE    0x0000
#define PLLSTAT_VALUE       0x0000
#define RSICLK_DIV          0x0001

#define VCO_VALUE (CLKIN*((PLLCTL_VALUE>>9)&63))
#define SCLK (VCO_VALUE / (PLLDIV_VALUE & 15))
#define CCLK (VCO_VALUE >> ((PLLDIV_VALUE >> 4) & 3))
//#define MCK CCLK

#include "bf592.h"

#endif // CORE_H__05_12_12__16_12
