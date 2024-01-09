//#include "types.h"
//#include "core.h"
//#include "time.h"
//#include "COM_DEF.h"
//#include "CRC16_8005.h"
//#include "list.h"
//#include "PointerCRC.h"

#include "hardware.h"
#include "SEGGER_RTT.h"
//#include "hw_conf.h"
//#include "hw_rtm.h"


static S_I2C i2c(I2C_USIC_NUM, PIN_SCL, PIN_SDA, MCK);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//List<DSCI2C>	i2c_ReqList;
//DSCI2C*			i2c_dsc = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_Update()
{
	return i2c.Update();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_AddRequest(DSCI2C *d)
{
	return i2c.AddRequest(d);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void I2C_Init()
{
	i2c.Connect(I2C_BAUDRATE);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++