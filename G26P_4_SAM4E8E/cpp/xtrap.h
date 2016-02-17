#ifndef XTRAP_H__12_03_20015__11_14
#define XTRAP_H__12_03_20015__11_14

#include "types.h"
#include "emac.h"
#include "trap_def.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SmallTx : public EthUdpBuf
{
	TrapHdr th;		// 9
	u16 cmd;		// 2
	byte data[75];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct HugeTx : public SmallTx
{
	byte exdata[1408];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void RequestTrap(EthUdp *h, u32 stat);
extern void InitTraps();
extern void UpdateTraps();

extern SmallTx*	GetSmallTxBuffer();
extern HugeTx*	GetHugeTxBuffer();
extern void SendTrap(SmallTx *p);

#endif // TRAP_H__12_03_20015__11_14