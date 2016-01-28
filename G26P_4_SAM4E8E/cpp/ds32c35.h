#ifndef MEMORY_AT91SAM7X256_DS32C35_H
#define MEMORY_AT91SAM7X256_DS32C35_H


#define DS32C35_RTC_DEVICE	0x68
#define DS32C35_FRAM_DEVICE	0x50

#define DS32C35_FRAM_SIZE	0x2000

/*****************************************************************************/
extern void DS32C35_Init();
extern void DS32C35_Idle();
extern bool DS32C35_RTC_Get_Second(unsigned char *sec);
extern bool DS32C35_RTC_Set_Second(unsigned char sec);
extern bool DS32C35_RTC_Get_Minute(unsigned char *min);
extern bool DS32C35_RTC_Set_Minute(unsigned char min);
extern bool DS32C35_RTC_Get_Hour(unsigned char *hour);
extern bool DS32C35_RTC_Set_Hour(unsigned char hour);
extern bool DS32C35_RTC_Get_Day(unsigned char *day);
extern bool DS32C35_RTC_Set_Day(unsigned char day);
extern bool DS32C35_RTC_Get_Month(unsigned char *mon);
extern bool DS32C35_RTC_Set_Month(unsigned char mon);
extern bool DS32C35_RTC_Get_Year(unsigned char *year);
extern bool DS32C35_RTC_Set_Year(unsigned char year);
extern bool DS32C35_RTC_Start_Temperature_Conversion();
extern bool DS32C35_RTC_Get_Temperature(signed char *t);
extern bool DS32C35_RTC_Get_Busy(bool *b);
extern bool DS32C35_FRAM_Write(unsigned short adr, unsigned char *data, unsigned short size);
extern bool DS32C35_FRAM_Read(unsigned short adr, unsigned char *data, unsigned short size);
/*****************************************************************************/

#endif
