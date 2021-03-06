;/*****************************************************************************
; * @file     startup_SAM4S.s
; * @brief    CMSIS Cortex-M4 Core Device Startup File
; *           for the Atmel SAM4S Device Series
; * @version  V1.10
; * @date     05. March 2013
; *
; * @note
; * Copyright (C) 2011-2013 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000200

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


VecTableIntSize	EQU		16*4	
VecTableExtSize	EQU		35*4	

				AREA	VTBL, NOINIT, READWRITE, ALIGN=7
                EXPORT  VectorTableInt
                EXPORT  VectorTableExt
VectorTableInt	SPACE	VecTableIntSize				
VectorTableExt	SPACE	VecTableExtSize				

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, CODE, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     SUPC_IRQHandler           ; 16: Supply Controller
                DCD     RSTC_IRQHandler           ; 17: Reset Controller
                DCD     RTC_IRQHandler            ; 18: Real Time Clock
                DCD     RTT_IRQHandler            ; 19: Real Time Timer
                DCD     WDT_IRQHandler            ; 20: Watchdog Timer
                DCD     PMC_IRQHandler            ; 21: Power Management Controller
                DCD     EEFC0_IRQHandler          ; 22: Enhanced Embedded Flash Controller 0
                DCD     EEFC1_IRQHandler          ; 23: Enhanced Embedded Flash Controller 1
                DCD     UART0_IRQHandler          ; 24: UART0
                DCD     UART1_IRQHandler          ; 25: UART1
                DCD     SMC_IRQHandler            ; 26: Static Memory Controller
                DCD     PIOA_IRQHandler           ; 27: Parallel I/O Controller A
                DCD     PIOB_IRQHandler           ; 28: Parallel I/O Controller B
                DCD     PIOC_IRQHandler           ; 29: Parallel I/O Controller C
                DCD     USART0_IRQHandler         ; 30: USART 0
                DCD     USART1_IRQHandler         ; 31: USART 1
                DCD     0                         ; 32: Reserved
                DCD     0                         ; 33: Reserved
                DCD     HSMCI_IRQHandler          ; 34: Multimedia Card Interface
                DCD     TWI0_IRQHandler           ; 35: Two Wire Interface 0
                DCD     TWI1_IRQHandler           ; 36: Two Wire Interface 1
                DCD     SPI_IRQHandler            ; 37: Serial Peripheral Interface
                DCD     SSC_IRQHandler            ; 38: Synchronous Serial Controller
                DCD     TC0_IRQHandler            ; 39: Timer/Counter 0
                DCD     TC1_IRQHandler            ; 40: Timer/Counter 1
                DCD     TC2_IRQHandler            ; 41: Timer/Counter 2
                DCD     TC3_IRQHandler            ; 42: Timer/Counter 3
                DCD     TC4_IRQHandler            ; 43: Timer/Counter 4
                DCD     TC5_IRQHandler            ; 44: Timer/Counter 5
                DCD     ADC_IRQHandler            ; 45: Analog-to-Digital Converter
                DCD     DACC_IRQHandler           ; 46: Digital-to-Analog Converter
                DCD     PWM_IRQHandler            ; 47: Pulse Width Modulation
                DCD     CRCCU_IRQHandler          ; 48: CRC Calculation Unit
                DCD     ACC_IRQHandler            ; 49: Analog Comparator 
                DCD     UDP_IRQHandler            ; 50: USB Device Port
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

 ;               AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
;                IMPORT  SystemInit
                IMPORT  __main
;                LDR     R0, =SystemInit
;                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

_sys_exit		PROC
				EXPORT	_sys_exit
				
				LDR		SP, [R0]
				LDR		R0, [R0, #4]
				BX		R0

                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  SUPC_IRQHandler           [WEAK]
                EXPORT  RSTC_IRQHandler           [WEAK]
                EXPORT  RTC_IRQHandler            [WEAK]
                EXPORT  RTT_IRQHandler            [WEAK]
                EXPORT  WDT_IRQHandler            [WEAK]
                EXPORT  PMC_IRQHandler            [WEAK]
                EXPORT  EEFC0_IRQHandler          [WEAK]
                EXPORT  EEFC1_IRQHandler          [WEAK]
                EXPORT  UART0_IRQHandler          [WEAK]
                EXPORT  UART1_IRQHandler          [WEAK]
                EXPORT  SMC_IRQHandler            [WEAK]
                EXPORT  PIOA_IRQHandler           [WEAK]
                EXPORT  PIOB_IRQHandler           [WEAK]
                EXPORT  PIOC_IRQHandler           [WEAK]
                EXPORT  USART0_IRQHandler         [WEAK]
                EXPORT  USART1_IRQHandler         [WEAK]
                EXPORT  HSMCI_IRQHandler          [WEAK]
                EXPORT  TWI0_IRQHandler           [WEAK]
                EXPORT  TWI1_IRQHandler           [WEAK]
                EXPORT  SPI_IRQHandler            [WEAK]
                EXPORT  SSC_IRQHandler            [WEAK]
                EXPORT  TC0_IRQHandler            [WEAK]
                EXPORT  TC1_IRQHandler            [WEAK]
                EXPORT  TC2_IRQHandler            [WEAK]
                EXPORT  TC3_IRQHandler            [WEAK]
                EXPORT  TC4_IRQHandler            [WEAK]
                EXPORT  TC5_IRQHandler            [WEAK]
                EXPORT  ADC_IRQHandler            [WEAK]
                EXPORT  DACC_IRQHandler           [WEAK]
                EXPORT  PWM_IRQHandler            [WEAK]
                EXPORT  CRCCU_IRQHandler          [WEAK]
                EXPORT  ACC_IRQHandler            [WEAK]
                EXPORT  UDP_IRQHandler            [WEAK]

SUPC_IRQHandler          
RSTC_IRQHandler
RTC_IRQHandler            
RTT_IRQHandler
WDT_IRQHandler
PMC_IRQHandler
EEFC0_IRQHandler
EEFC1_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SMC_IRQHandler
PIOA_IRQHandler
PIOB_IRQHandler
PIOC_IRQHandler
USART0_IRQHandler
USART1_IRQHandler
HSMCI_IRQHandler
TWI0_IRQHandler
TWI1_IRQHandler
SPI_IRQHandler
SSC_IRQHandler
TC0_IRQHandler
TC1_IRQHandler
TC2_IRQHandler
TC3_IRQHandler
TC4_IRQHandler
TC5_IRQHandler
ADC_IRQHandler
DACC_IRQHandler
PWM_IRQHandler
CRCCU_IRQHandler
ACC_IRQHandler          
UDP_IRQHandler
                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                
                ELSE
                
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
