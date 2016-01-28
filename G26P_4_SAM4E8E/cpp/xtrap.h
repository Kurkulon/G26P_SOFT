#ifndef XTRAP_H__12_03_20015__11_14
#define XTRAP_H__12_03_20015__11_14

#include "types.h"
#include "emac.h"

extern void RequestTrap(EthUdp *h, u32 stat);
extern void InitTraps();
extern void UpdateTraps();

#endif // TRAP_H__12_03_20015__11_14
