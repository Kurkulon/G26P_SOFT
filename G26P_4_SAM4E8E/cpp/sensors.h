#ifndef MEMORY_AT91SAM7X256_SENSORS_H
#define MEMORY_AT91SAM7X256_SENSORS_H

#define SENSORS_MESURE_PERIOD_MS	200

extern void Sensors_Idle();
extern void Sensors_Init();

extern short Sensors_Temperature_In_Get();
extern short Sensors_Ax_Get();
extern short Sensors_Ay_Get();
extern short Sensors_Az_Get();

#endif
