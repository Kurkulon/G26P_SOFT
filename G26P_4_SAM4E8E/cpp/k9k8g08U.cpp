#include "types.h"
#include "core.h"
//#include "common.h"
//#include "main.h"
#include "k9k8g08u.h"

byte * const FLC = (byte*)0x60400000;	
byte * const FLA = (byte*)0x60200000;	

volatile byte * const FLD = (byte*)0x60000000;	

static u32 chipSelect[8] = { 1<<13, 1<<16, 1<<23, 1<<15, 1<<14, 1<<24, 1<<25, 1<<26 };
static const u32 maskChipSelect = (0xF<<13)|(0xF<<23);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_COMMAND_LATCH_CYCLE(cmd) \
{                                    \
	*FLC = cmd;	\
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(col, row)	\
{                                   					\
	*FLA = col;											\
	*FLA = col >> 8;									\
	*FLA = row;											\
	*FLA = row >> 8;									\
	*FLA = row >> 16;									\
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_ADRESS_LATCH_COL_CYCLE(col) \
{                                    \
	*FLA = col;											\
	*FLA = col >> 8;									\
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_ADRESS_LATCH_ROW_CYCLE(row) \
{                                    \
	*FLA = row;											\
	*FLA = row >> 8;									\
	*FLA = row >> 16;									\
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_READ_CYCLE_PREPARE()/* \
	K9K8G08U_BASE_PIO_DATA->PIO_ODR = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_READ_CYCLE(data) \
{       				\
	K9K8G08U_BASE_PIO_RE->PIO_CODR = K9K8G08U_PIO_RE;	\
	data = ((K9K8G08U_BASE_PIO_DATA->PIO_PDSR >> K9K8G08U_PIO_DATA_OFFSET) & K9K8G08U_PIO_DATA_MASK); \
	K9K8G08U_BASE_PIO_RE->PIO_SODR = K9K8G08U_PIO_RE;	\
}       

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_WRITE_CYCLE_PREPARE()/* \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_WRITE_CYCLE(data) \
{       				\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((data & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~data) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
}       

////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashReady()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) != 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//inline bool FlashBusy()
//{
//	return (HW::PIOC->PDSR & (1UL<<31)) == 0;
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define K9K8G08U_BUSY() ((HW::PIOC->PDSR & (1UL<<31)) == 0)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Chip_Select(byte chip) 
{    
	HW::PIOA->SODR = maskChipSelect;

	if(chip < 8)                   
	{ 				
		HW::PIOA->CODR = chipSelect[chip];
	};
}                                                                              

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

k9k8g08u_status_operation_type k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WAIT;
k9k8g08u_memory_size_type k9k8g08u_memory_size;

u16 k9k8g08u_data_col;
u32 k9k8g08u_data_row;
byte k9k8g08u_data_chip;
u16 k9k8g08u_data_size;
u16 k9k8g08u_data_count;
byte *k9k8g08u_data;
u16 k9k8g08u_data_verify_errors;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

k9k8g08u_memory_size_type K9K8G08U_Size_Get()
{
	return k9k8g08u_memory_size;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Adress_Set(i64 a)
{
	while(a < 0) a += ((i64)1 << (k9k8g08u_memory_size.full)) * (((-a) >> (k9k8g08u_memory_size.full)) + 1);
	a %= ((i64)1 << (k9k8g08u_memory_size.full));
	k9k8g08u_data_chip = a / (1 << k9k8g08u_memory_size.chip);
	a %= (1 << k9k8g08u_memory_size.chip);
	k9k8g08u_data_row = a / (1 << k9k8g08u_memory_size.page);
	a %= (1 << k9k8g08u_memory_size.page);
	k9k8g08u_data_col = a;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Adress_Set_Next(i64 a)
{
	while(a < 0) a += ((i64)1 << (k9k8g08u_memory_size.full)) * (((-a) >> (k9k8g08u_memory_size.full)) + 1);
	a %= ((i64)1 << (k9k8g08u_memory_size.full));
	a += ((i64)1 << (k9k8g08u_memory_size.block)) - (a % ((i64)1 << k9k8g08u_memory_size.block));
	a %= ((i64)1 << (k9k8g08u_memory_size.full));
	k9k8g08u_data_chip = a / (1 << k9k8g08u_memory_size.chip);
	a %= (1 << k9k8g08u_memory_size.chip);
	k9k8g08u_data_row = a / (1 << k9k8g08u_memory_size.page);
	k9k8g08u_data_col = 0;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i64 K9K8G08U_Adress_Get()
{
	return (i64)((i64)k9k8g08u_data_chip * (i64)((i64)1 << k9k8g08u_memory_size.chip)) + ((i64)k9k8g08u_data_row * (i64)(1 << k9k8g08u_memory_size.page)) + (i64)k9k8g08u_data_col;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Status_Operation_Reset()
{
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WAIT;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

k9k8g08u_status_operation_type K9K8G08U_Status_Operation_Get()
{
	return k9k8g08u_status_operation;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Reset()
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	while(K9K8G08U_BUSY());
	K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_RESET);
	while(K9K8G08U_BUSY());
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Read_ID(k9k8g08u_id_type *id)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_ID);
	K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(0, 0);

	K9K8G08U_READ_CYCLE_PREPARE();

	byte *p = (byte*)id;

	for(byte i = 0; i < sizeof(k9k8g08u_id_type); i++) 
	{ 
		*p++ = *FLD;
	}

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Read_Data(i64 *adr, byte *data, u16 size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_Adress_Set(*adr);	
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Verify_Data(i64 *adr, byte *data, u16 size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_Adress_Set(*adr);	
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;
	k9k8g08u_data_verify_errors = 0;
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Write_Data(bool next, i64 *adr, byte *data, u16 size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	if(next)
	{
		K9K8G08U_Adress_Set_Next(*adr);
		*adr = K9K8G08U_Adress_Get();
	}
	else K9K8G08U_Adress_Set(*adr);
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;         
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START;
	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool K9K8G08U_Idle()
{
	if(K9K8G08U_BUSY()) return false;
        
	switch (k9k8g08u_status_operation)
	{
		case K9K8G08U_STATUS_OPERATION_WAIT: 

			break;

		case K9K8G08U_STATUS_OPERATION_READ_START:

			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_2);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_IDLE;

			break;

		case K9K8G08U_STATUS_OPERATION_READ_IDLE:	
//			nop();
			u16 x_r_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_READ_PACK_BYTES) x_r_i = K9K8G08U_READ_PACK_BYTES; else x_r_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_r_i + k9k8g08u_data_col) > (1 << k9k8g08u_memory_size.page)) x_r_i = (1 << k9k8g08u_memory_size.page) - k9k8g08u_data_col;
			u16 i_r_i = k9k8g08u_data_count;      // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			u16 e_r_i = i_r_i + x_r_i;
			K9K8G08U_READ_CYCLE_PREPARE();

			while(i_r_i < e_r_i)
			{ 
				k9k8g08u_data[i_r_i++] = *FLD;
			};

			k9k8g08u_data_count = i_r_i;
			k9k8g08u_data_col += x_r_i;
			k9k8g08u_data_col %= (1 << k9k8g08u_memory_size.page);
			
			if (k9k8g08u_data_col == 0) 
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row %= (1 << (k9k8g08u_memory_size.chip - k9k8g08u_memory_size.page));
				
				if (k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while(!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				};

				if (k9k8g08u_data_size > k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_START;
			};

			if(k9k8g08u_data_size <= k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_READY;

			break;
			// дочитываем до конца, чтобы адрес попал куда надо

		case K9K8G08U_STATUS_OPERATION_VERIFY_START:

			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
		    K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_2);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_IDLE;

			break; 

		case K9K8G08U_STATUS_OPERATION_VERIFY_IDLE:	

			__nop();
			u16 x_v_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_READ_PACK_BYTES) x_v_i = K9K8G08U_READ_PACK_BYTES; else x_v_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_v_i + k9k8g08u_data_col) > (1 << k9k8g08u_memory_size.page)) x_v_i = (1 << k9k8g08u_memory_size.page) - k9k8g08u_data_col;
			u16 i_v_i = k9k8g08u_data_count;       // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			u16 e_v_i = i_v_i + x_v_i;
			byte s_v_i;
			u16 q_v_i = k9k8g08u_data_verify_errors;
			
			K9K8G08U_READ_CYCLE_PREPARE();
			
			while(i_v_i < e_v_i)
			{ 
                s_v_i = *FLD;
				if(s_v_i != k9k8g08u_data[i_v_i]) q_v_i ++;
				i_v_i ++;
			};

			k9k8g08u_data_count = i_v_i;
			k9k8g08u_data_verify_errors = q_v_i;
			k9k8g08u_data_col += x_v_i;
			k9k8g08u_data_col %= (1 << k9k8g08u_memory_size.page);
			
			if(k9k8g08u_data_col == 0) 
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row %= (1 << (k9k8g08u_memory_size.chip - k9k8g08u_memory_size.page));
				
				if(k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while(!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				};
				
				if (k9k8g08u_data_size > k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_START;
			};
			
			if(k9k8g08u_data_size <= k9k8g08u_data_count)
			{
				if(k9k8g08u_data_verify_errors) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_ERROR;
				else k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_READY;
			};

			break;

		case K9K8G08U_STATUS_OPERATION_WRITE_START:
			
			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
			
			if((k9k8g08u_data_col == 0) && ((k9k8g08u_data_row % (1 << (k9k8g08u_memory_size.block - k9k8g08u_memory_size.page)) == 0)))	// новый блок
			{
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_BLOCK_ERASE_1);
				K9K8G08U_ADRESS_LATCH_ROW_CYCLE(k9k8g08u_data_row);
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_BLOCK_ERASE_2);
	            k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERASE_CHECK;
				break;
			};

		case K9K8G08U_STATUS_OPERATION_WRITE_START_ERASED:

			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_IDLE;
			
			//break; // не надо, т.к. можно обработать сразу IDLE (START_ERASED не занимает много времени)

		case K9K8G08U_STATUS_OPERATION_WRITE_IDLE:
			
			__nop();
			
			u16 x_w_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_WRITE_PACK_BYTES) x_w_i = K9K8G08U_WRITE_PACK_BYTES; else x_w_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_w_i + k9k8g08u_data_col) > (1 << k9k8g08u_memory_size.page)) x_w_i = (1 << k9k8g08u_memory_size.page) - k9k8g08u_data_col;
			u16 i_w_i = k9k8g08u_data_count;   // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			u16 e_w_i = i_w_i + x_w_i;
			
			K9K8G08U_WRITE_CYCLE_PREPARE();

			while(i_w_i < e_w_i)
			{ 
				*FLD = k9k8g08u_data[i_w_i++];
			};

			k9k8g08u_data_count = i_w_i;
			k9k8g08u_data_col += x_w_i;
			k9k8g08u_data_col %= (1 << k9k8g08u_memory_size.page);
			
			if(k9k8g08u_data_col == 0)  //end of page
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row %= (1 << (k9k8g08u_memory_size.chip - k9k8g08u_memory_size.page));
				
				if (k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while (!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				};

				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_2);
				k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_CHECK;
			};
			
			if (k9k8g08u_data_size <= k9k8g08u_data_count) 
			{
				k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_CHECK;
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_2);
			};
			
			break;

		case K9K8G08U_STATUS_OPERATION_WRITE_CHECK:
			
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_STATUS);
			
			K9K8G08U_READ_CYCLE_PREPARE();

			byte s_w_c = *FLD;

			if (s_w_c & 0x01)	// сбой
			{
                k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERROR;	
			}
			else
			{
		        if (k9k8g08u_data_size <= k9k8g08u_data_count)
				{
					k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_READY; 
				}
				else 
				{
					k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START;
				};
			};

			break;

		case K9K8G08U_STATUS_OPERATION_WRITE_ERASE_CHECK:

			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_STATUS);
			
			K9K8G08U_READ_CYCLE_PREPARE();

			byte s_e_c = *FLD;

			if (s_e_c & 0x01)	// сбой
			{
            	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERROR;	
			}
			else
			{
	        	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START_ERASED; 
			};

			break;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Init()
{
	k9k8g08u_memory_size.page = 0;
	k9k8g08u_memory_size.block = 0;
	k9k8g08u_memory_size.chip = 0;
	k9k8g08u_memory_size.full = 0;
	k9k8g08u_memory_size.mask = 0;

	for(byte chip = 0; chip < K9K8G08U_MAX_CHIP; chip ++)
	{
		K9K8G08U_Chip_Select(chip);
		K9K8G08U_Reset();
		k9k8g08u_id_type k9k8g08u_id;
		K9K8G08U_Read_ID(&k9k8g08u_id);
		
		if((k9k8g08u_id.marker == 0xEC) && (k9k8g08u_id.device == 0xD3))
		{
			k9k8g08u_memory_size.page = ((k9k8g08u_id.data[1] >> 0) & 0x03) + 10;
			k9k8g08u_memory_size.block = ((k9k8g08u_id.data[1] >> 4) & 0x03) + 16;
			k9k8g08u_memory_size.chip = (((k9k8g08u_id.data[2] >> 4) & 0x07) + 23) + (((k9k8g08u_id.data[2] >> 2) & 0x03) + 0);
			
			if(k9k8g08u_memory_size.full)
			{
				k9k8g08u_memory_size.full++; 
			}
			else
			{
				k9k8g08u_memory_size.full = k9k8g08u_memory_size.chip;
			};
			
			k9k8g08u_memory_size.mask |= (1 << chip);
		};
	};

	K9K8G08U_Chip_Select(0); ///пока один
		
}

