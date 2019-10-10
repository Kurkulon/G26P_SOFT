#ifndef CORE_H__23_09_13__11_39
#define CORE_H__23_09_13__11_39

#define	CORETYPE_XMC4800
#define CPU_XMC48

#include "XMC4800.h"

typedef void(*ARM_IHP)() __irq;
extern ARM_IHP VectorTableInt[16]; //= (ARM_IHP*)0x1FFE8000;
extern ARM_IHP VectorTableExt[112];// = (ARM_IHP*)0x1FFE8040;

//#define VectorTableIntSize 16
//#define VectorTableExtSize 112

#endif // CORE_H__23_09_13__11_39
