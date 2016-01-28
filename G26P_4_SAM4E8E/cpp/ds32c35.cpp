#include "common.h"
#include "main.h"
#include "ds32c35.h"

#define nop2()	{__nop();__nop();}
#define nop4()	{nop2();nop2();}
#define nop8()	{nop4();nop4();}
#define nop16()	{nop8();nop8();}
#define nop32()	{nop16();nop16();}

bool DS32C35_TWI_Write(unsigned char adr, unsigned char *cmd, unsigned char cmd_size, unsigned char *data, unsigned short data_size)
{
	bool ok = true;
	//adr <<= 1;
	//adr &= ~1;
	//AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA9;	
	//AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//nop32();
	//AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//nop4()
	//unsigned short i;
	//unsigned char j;
	//for(i = 0; i < cmd_size + data_size + 1; i++)
	//{
	//	unsigned char mask = 0x80;
	//	for(j = 0; j <8; j++)
	//	{
	//		if(i > cmd_size)
	//		{
	//			if((*data)&mask) AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9; else  AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//			mask >>= 1;
	//		}
	//		else
	//		if(i > 0)
	//		{
	//			if((*cmd)&mask) AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9; else  AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//			mask >>= 1;
	//		}
	//		else
	//		{
	//			if((adr)&mask) AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9; else  AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//			mask >>= 1;
	//		}
	//		nop16();
	//		AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//		nop16();
	//		AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//	}	
	//	if(i > cmd_size) data ++;
	//	else if(i > 0) cmd ++;
	//	AT91C_BASE_PIOA->PIO_ODR = AT91C_PIO_PA9;
	//	nop32();
	//	AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//	nop16();
	//	if(AT91C_BASE_PIOA->PIO_PDSR & AT91C_PIO_PA9) ok = false;
	//	AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//	AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA9;
	//	if(!ok) break;
	//}
	//nop32();
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//nop32();
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9;
	return ok;
}

bool DS32C35_TWI_Read(unsigned char adr, unsigned char *data, unsigned short size)
{
	bool ok = true;
	//adr <<= 1;
	//adr |= 1;
	//AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA9; 
	//AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//nop32();
	//AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//nop4()
	//unsigned short i;
	//unsigned char j;
	//for(i = 0; i < size + 1; i++)
	//{
	//	if(i) AT91C_BASE_PIOA->PIO_ODR = AT91C_PIO_PA9;
	//	for(j = 0; j <8; j++)
	//	{
	//		if(!i)
	//		{
	//			if((adr)&0x80) AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9; else  AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;
	//			(adr) <<= 1;
	//		}
	//		nop16();
	//		nop8();
	//		AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//		nop16();
	//		if(i)
	//		{
	//			*data <<= 1;
	//			if(AT91C_BASE_PIOA->PIO_PDSR & AT91C_PIO_PA9) *data |= 1;
	//		}
	//		AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//	}	
	//	if(i) data ++;
	//	if(i) 
	//	{
	//		AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA9;	 
	//		AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA9; 
	//	}
	//	else AT91C_BASE_PIOA->PIO_ODR = AT91C_PIO_PA9;
	//	nop32();
	//	AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//	nop16();
	//	if(!i) if(AT91C_BASE_PIOA->PIO_PDSR & AT91C_PIO_PA9) ok = false;
	//	AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA8;
	//	if(!ok) break;
	//}
	//nop32();
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//nop32();
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9;
	return ok;
}

void DS32C35_Idle()
{
	
}

bool DS32C35_RTC_Get_Second(unsigned char *sec)
{
	unsigned char cmd[1] = {0x00};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*sec = (data[0]&0xF) + ((data[0]>>4)&0x7) * 10;
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Second(unsigned char sec)
{
	unsigned char cmd[1] = {0x00};
	unsigned char data[1];
	data[0] = (((sec / 10) & 0x7) << 4) + ((sec % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Get_Minute(unsigned char *min)
{
	unsigned char cmd[1] = {0x01};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*min = (data[0]&0xF) + ((data[0]>>4)&0x7) * 10;
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Minute(unsigned char min)
{
	unsigned char cmd[1] = {0x01};
	unsigned char data[1];
	data[0] = (((min / 10) & 0x7) << 4) + ((min % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Get_Hour(unsigned char *hour)
{
	unsigned char cmd[1] = {0x02};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*hour = (data[0]&0xF) + ((data[0]>>4)&0x3) * 10;
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Hour(unsigned char hour)
{
	unsigned char cmd[1] = {0x02};
	unsigned char data[1];
	data[0] = (((hour / 10) & 0x3) << 4) + ((hour % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Get_Day(unsigned char *day)
{
	unsigned char cmd[1] = {0x04};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*day = (data[0]&0xF) + ((data[0]>>4)&0x3) * 10;
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Day(unsigned char day)
{
	unsigned char cmd[1] = {0x04};
	unsigned char data[1];
	data[0] = (((day / 10) & 0x3) << 4) + ((day % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Get_Month(unsigned char *mon)
{
	unsigned char cmd[1] = {0x05};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*mon = (data[0]&0xF) + ((data[0]>>4)&0x1) * 10;
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Month(unsigned char mon)
{
	unsigned char cmd[1] = {0x05};
	unsigned char data[1];
	data[0] = (((mon / 10) & 0x1) << 4) + ((mon % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Get_Year(unsigned char *year)
{
	unsigned char cmd[1] = {0x06};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*year = (data[0]&0xF) + ((data[0]>>4)&0xF) * 10; 
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Set_Year(unsigned char year)
{
	unsigned char cmd[1] = {0x06};
	unsigned char data[1];
	data[0] = (((year / 10) & 0xF) << 4) + ((year % 10) & 0xF);
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1))
	{
		return true;
	}
	return false;
}

bool DS32C35_RTC_Start_Temperature_Conversion()
{
	unsigned char cmd[1] = {0x0E};
	unsigned char data[1] = {0x20};
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1)) return true;
	return false;
}

bool DS32C35_RTC_Get_Temperature(signed char *t)
{
	unsigned char cmd[1] = {0x11};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*t = (signed char)data[0]; 
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Get_Busy(bool *b)
{
	unsigned char cmd[1] = {0x0F};
	unsigned char data[1];
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_RTC_DEVICE, data, 1))
		{
	        	*b = ((data[0]>>2)&0x1); 
			return true;
		}
	}
	return false;
}

bool DS32C35_RTC_Disable_32_KHZ()
{
	unsigned char cmd[1] = {0x0F};
	unsigned char data[1] = {0};
	if(DS32C35_TWI_Write(DS32C35_RTC_DEVICE, cmd, 1, data, 1)) return true;
	return false;
}

bool DS32C35_FRAM_Write(unsigned short adr, unsigned char *data, unsigned short size)
{
	unsigned char cmd[2] = {(adr>>8)&0xFF, (adr)&0xFF};
	if(DS32C35_TWI_Write(DS32C35_FRAM_DEVICE, cmd, 2, data, size))
	{
		return true;
	}	
	return false;
}

bool DS32C35_FRAM_Read(unsigned short adr, unsigned char *data, unsigned short size)
{
	unsigned char cmd[2] = {(adr>>8)&0xFF, (adr)&0xFF};
	if(DS32C35_TWI_Write(DS32C35_FRAM_DEVICE, cmd, 2, data, 0))
	{
		if(DS32C35_TWI_Read(DS32C35_FRAM_DEVICE, data, size))
		{
			return true;
		}	
	}
	return false;
}

void DS32C35_Init()
{
	//AT91C_BASE_PIOA->PIO_PER = AT91C_PIO_PA8;	//SCL
 //       AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA8;
	//AT91C_BASE_PIOA->PIO_OWER = AT91C_PIO_PA8;
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA8;
	//AT91C_BASE_PIOA->PIO_PER = AT91C_PIO_PA9;       //SDA
 //       AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA9;
	//AT91C_BASE_PIOA->PIO_OWER = AT91C_PIO_PA9;
	//AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA9;
	//AT91C_BASE_PIOA->PIO_PPUER = AT91C_PIO_PA9;
	DS32C35_RTC_Disable_32_KHZ();
}
