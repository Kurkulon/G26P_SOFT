#ifndef CORE_H__23_09_13__11_39
#define CORE_H__23_09_13__11_39

#define	CORETYPE_XMC4800

#include "XMC4800.h"

typedef void(*ARM_IHP)() __irq;
extern ARM_IHP VectorTableInt[16];
extern ARM_IHP VectorTableExt[112];

#endif // CORE_H__23_09_13__11_39
