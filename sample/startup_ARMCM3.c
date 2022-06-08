/******************************************************************************
 * @file     startup_ARMCM3.c
 * @brief    CMSIS-Core(M) Device Startup File for a Cortex-M3 Device
 * @version  V2.0.3
 * @date     31. March 2020
 ******************************************************************************/
/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined (ARMCM3)
  #include "ARMCM3.h"
#else
  #error device not specified!
#endif

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
            void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

void CSV_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void SWDT_Handler           (void) __attribute__ ((weak, alias("Default_Handler")));
void LVD_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void MFT_WG_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void INT0_7_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void INT8_15_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS0RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS0TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS1RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS1TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS2RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS2TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS3RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS3TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS4RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS4TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS5RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS5TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS6RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS6TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS7RX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void MFS7TX_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void PPG_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void MFT_FRT_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void MFT_IPC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void MFT_OPC_IRQHandler     (void) __attribute__ ((weak, alias("Default_Handler")));
void BT_IRQHandler          (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[240];
       const VECTOR_TABLE_Type __VECTOR_TABLE[240] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
  Reset_Handler,                            /*     Reset Handler */
  NMI_Handler,                              /* -14 NMI Handler */
  HardFault_Handler,                        /* -13 Hard Fault Handler */
  MemManage_Handler,                        /* -12 MPU Fault Handler */
  BusFault_Handler,                         /* -11 Bus Fault Handler */
  UsageFault_Handler,                       /* -10 Usage Fault Handler */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  SVC_Handler,                              /*  -5 SVC Handler */
  DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
  0,                                        /*     Reserved */
  PendSV_Handler,                           /*  -2 PendSV Handler */
  SysTick_Handler,                          /*  -1 SysTick Handler */

  /* Interrupts */
  CSV_Handler,               // 0: Clock Super Visor
  SWDT_Handler,              // 1: Software Watchdog Timer
  LVD_Handler,               // 2: Low Voltage Detector
  MFT_WG_IRQHandler,         // 3: Wave Form Generator / DTIF
  INT0_7_Handler,            // 4: External Interrupt Request ch.0 to ch.7
  INT8_15_Handler,           // 5: External Interrupt Request ch.8 to ch.15
  MFS0RX_IRQHandler,         // 6: MultiFunction Serial ch.0
  MFS0TX_IRQHandler,         // 7: MultiFunction Serial ch.0
  MFS1RX_IRQHandler,         // 8: MultiFunction Serial ch.1
  MFS1TX_IRQHandler,         // 9: MultiFunction Serial ch.1
  MFS2RX_IRQHandler,         // 10: MultiFunction Serial ch.2
  MFS2TX_IRQHandler,         // 11: MultiFunction Serial ch.2
  MFS3RX_IRQHandler,         // 12: MultiFunction Serial ch.3
  MFS3TX_IRQHandler,         // 13: MultiFunction Serial ch.3
  MFS4RX_IRQHandler,         // 14: MultiFunction Serial ch.4
  MFS4TX_IRQHandler,         // 15: MultiFunction Serial ch.4
  MFS5RX_IRQHandler,         // 16: MultiFunction Serial ch.5
  MFS5TX_IRQHandler,         // 17: MultiFunction Serial ch.5
  MFS6RX_IRQHandler,         // 18: MultiFunction Serial ch.6
  MFS6TX_IRQHandler,         // 19: MultiFunction Serial ch.6
  MFS7RX_IRQHandler,         // 20: MultiFunction Serial ch.7
  MFS7TX_IRQHandler,         // 21: MultiFunction Serial ch.7
  PPG_Handler,               // 22: PPG
  TIM_IRQHandler,            // 23: OSC / PLL / Realtime Clock
  ADC0_IRQHandler,           // 24: ADC0
  MFT_FRT_IRQHandler,        // 25: Free-run Timer
  MFT_IPC_IRQHandler,        // 26: Input Capture
  MFT_OPC_IRQHandler,        // 27: Output Compare
  BT_IRQHandler              // 28: Base Timer ch.0 to ch.7

  /* Other handlers are left out */
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
  SystemInit();                             /* CMSIS System Initialization */
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
  while(1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while(1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#endif

