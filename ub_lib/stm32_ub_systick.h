//--------------------------------------------------------------
// File     : stm32_ub_systick.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SYSTICK_H
#define __STM32F4_UB_SYSTICK_H



//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"


//--------------------------------------------------------------
// Auflösung der Systick
// (entweder 1us oder 1000us als Auflösung einstellen)
//--------------------------------------------------------------
//#define  SYSTICK_RESOLUTION   1    // 1us Auflösung
#define  SYSTICK_RESOLUTION   1000   // 1ms Auflösung



uint32_t  GUI_Timer_ms;


//--------------------------------------------------------------
// Testpin-1 (PB2)
//--------------------------------------------------------------
#define  GPIO_TST_1_CLOCK   RCC_AHB1Periph_GPIOB
#define  GPIO_TST_1_PORT    GPIOB
#define  GPIO_TST_1_PIN     GPIO_Pin_2



//--------------------------------------------------------------
// Globale Pausen-Funktionen
//--------------------------------------------------------------
void UB_Systick_Init(void);
#if SYSTICK_RESOLUTION==1
  void UB_Systick_Pause_us(volatile uint32_t pause);
#endif
void UB_Systick_Pause_ms(volatile uint32_t pause);
void UB_Systick_Pause_s(volatile uint32_t pause);












//--------------------------------------------------------------
#endif // __STM32F4_UB_SYSTICK_H


