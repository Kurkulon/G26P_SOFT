#ifndef MEMORY_AT91SAM7X256_ADC_H
#define MEMORY_AT91SAM7X256_ADC_H

#define ADC_PERIOD_MS		5

#define ADC_CHANNEL_BATTERY	1
#define ADC_CHANNEL_LINE	6
#define ADC_CHANNEL_AX		7
#define ADC_CHANNEL_AY		4 // ошибка в схеме
#define ADC_CHANNEL_AZ		5
#define ADC_CHANNELS_MASK 	((1 << ADC_CHANNEL_BATTERY) | (1 << ADC_CHANNEL_LINE) | (1 << ADC_CHANNEL_AX) | (1 << ADC_CHANNEL_AY) | (1 << ADC_CHANNEL_AZ))

typedef enum
{
	ADC_MEASURE_AVERAGING = 0,
	ADC_MEASURE_PEAK_DETECTION,
	ADC_MEASURE_MAX,
	ADC_MEASURE_MIN,
} adc_measure_type;

extern void ADC_Idle();
extern void ADC_Init();
extern bool ADC_Get(unsigned char channel, adc_measure_type type, unsigned short *value);

#endif
