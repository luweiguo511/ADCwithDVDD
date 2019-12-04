/**************************************************************************//**
 * @file
 * @brief Use the ADC to take nonblocking single-ended measurements
 * @version 0.0.1
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2018 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs Software License Agreement. See
 * "http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt"
 * for details. Before using this software for any purpose, you must agree to the
 * terms of that agreement.
 *
 ******************************************************************************/

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_adc.h"
#include "em_emu.h"
#include "bsp.h"
#define adcFreq   16000000

volatile uint32_t sample;
volatile uint32_t millivolts;

/**************************************************************************//**
 * @brief  Initialize ADC function
 *****************************************************************************/
void initADC (void)
{
  // Enable ADC0 clock
  CMU_ClockEnable(cmuClock_ADC0, true);

  // Declare init structs
  ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
  ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

  // Modify init structs and initialize
  init.prescale = ADC_PrescaleCalc(adcFreq, 0); // Init to max ADC clock for Series 1
  init.timebase = ADC_TimebaseCalc(0);

  initSingle.diff       = false;        // single ended
  initSingle.reference  = adcRef5V;    // internal 2.5V reference
  initSingle.resolution = adcRes12Bit;  // 12-bit resolution
  initSingle.acqTime    = adcAcqTime4;  // set acquisition time to meet minimum requirements

  // Select ADC input. See README for corresponding EXP header pin.
  initSingle.posSel = adcPosSelAPORT4XCH11;

  ADC_Init(ADC0, &init);
  ADC_InitSingle(ADC0, &initSingle);

  // Enable ADC Single Conversion Complete interrupt
  ADC_IntEnable(ADC0, ADC_IEN_SINGLE);

  // Enable ADC interrupts
  NVIC_ClearPendingIRQ(ADC0_IRQn);
  NVIC_EnableIRQ(ADC0_IRQn);
}

/**************************************************************************//**
 * @brief  ADC Handler
 *****************************************************************************/
void ADC0_IRQHandler(void)
{
  // Get ADC result
  sample = ADC_DataSingleGet(ADC0);

  // Calculate input voltage in mV
  millivolts = (sample * 5000) / 4096;

  // Start next ADC conversion
  ADC_Start(ADC0, adcStartSingle);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_STK_DEFAULT;

  /* Chip errata */
  CHIP_Init();

  EMU_DCDCInit(&dcdcInit);
  CHIP_Init();
  EMU->PWRCTRL |= EMU_PWRCTRL_ANASW;
  initADC();

  // Start first conversion
  ADC_Start(ADC0, adcStartSingle);

  // Infinite loop
  while(1);
}
