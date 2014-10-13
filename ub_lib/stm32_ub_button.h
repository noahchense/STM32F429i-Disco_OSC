//--------------------------------------------------------------
// File     : stm32_ub_button.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_BUTTON_H
#define __STM32F4_UB_BUTTON_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"



//--------------------------------------------------------------
// Aktivieren, Deaktivieren der entprellten Funktionen mit Timer
// 1=aktiv, 0=inaktiv
//--------------------------------------------------------------
#define  BUTTON_USE_TIMER      1  // Funktionen per Timer


#if BUTTON_USE_TIMER==1
  #include "stm32f4xx_tim.h"
  #include "misc.h"
  #include <stdbool.h>
#endif


//--------------------------------------------------------------
// Liste aller Buttons
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum 
{
  BTN_USER = 0    // BTN1 auf dem STM32F429-Discovery
}BUTTON_NAME_t;

#define  BUTTON_ANZ   1 // Anzahl von Button_NAME_t


//--------------------------------------------------------------
// Status eines Buttons
//--------------------------------------------------------------
typedef enum {
  BTN_RELEASED = 0,  // Button losgelassen
  BTN_PRESSED        // Button gedrueckt
}BUTTON_STATUS_t;



//--------------------------------------------------------------
// Timer für Button => TIM7
// Grundfrequenz = 2*APB1 (APB1=45MHz) => TIM_CLK=90MHz
// TIM_Frq = TIM_CLK/(periode+1)/(vorteiler+1)
// TIM_Frq = 20Hz => 50ms (nicht kleiner als 1ms einstellen)
//--------------------------------------------------------------
#if BUTTON_USE_TIMER==1
  #define   UB_BUTTON_TIM              TIM7
  #define   UB_BUTTON_TIM_CLK          RCC_APB1Periph_TIM7
  #define   UB_BUTTON_TIM_PERIODE      4999
  #define   UB_BUTTON_TIM_PRESCALE     899
  #define   UB_BUTTON_TIM_IRQ          TIM7_IRQn
  #define   UB_BUTTON_TIM_ISR_HANDLER  TIM7_IRQHandler
#endif



//--------------------------------------------------------------
// Struktur eines Buttons
//--------------------------------------------------------------
typedef struct {
  BUTTON_NAME_t BUTTON_NAME;  // Name
  GPIO_TypeDef* BUTTON_PORT;  // Port
  const uint16_t BUTTON_PIN;  // Pin
  const uint32_t BUTTON_CLK;  // Clock
  GPIOPuPd_TypeDef BUTTON_R;  // Widerstand
  uint8_t BUTTON_AKT;         // Istwert
}BUTTON_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Button_Init(void);
BUTTON_STATUS_t UB_Button_Read(BUTTON_NAME_t btn_name);
#if BUTTON_USE_TIMER==1
  bool UB_Button_OnPressed(BUTTON_NAME_t btn_name);
  bool UB_Button_OnClick(BUTTON_NAME_t btn_name);
  bool UB_Button_OnRelease(BUTTON_NAME_t btn_name);
#endif

//--------------------------------------------------------------
#endif // __STM32F4_UB_BUTTON_H
