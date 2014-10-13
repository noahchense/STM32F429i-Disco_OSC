//--------------------------------------------------------------
// File     : adc.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_ADC_H
#define __STM32F4_UB_ADC_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "menu.h"
#include "stm32_ub_led.h"


//--------------------------------------------------------------
// Liste aller ADC-Kanäle
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum {
  ADC_PA5 = 0,  // PA5
  ADC_PA3 = 1,  // PA3
  ADC_PA7 = 2   // PA7
}ADC1d_NAME_t;

#define  ADC1d_ANZ   2 // Anzahl von ADC1d_NAME_t (maximum = 16)


//--------------------------------------------------------------
// Defines vom ADC (nicht ändern)
//--------------------------------------------------------------
#define  ADC_ARRAY_LEN         300 // Anzahl der Messwerte (pro Kanal)
#define  ADC_HALF_ARRAY_LEN    ADC_ARRAY_LEN/2 



//--------------------------------------------------------------
// Globale ADC-Puffer
// Jeder Buffer ist so groß wie ein kompletter Oszi-Screen
// (300 Pixel x 2 Kanäle x 16bit)
//--------------------------------------------------------------
volatile uint16_t ADC_DMA_Buffer_A[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (A) Roh-Daten per DMA
volatile uint16_t ADC_DMA_Buffer_B[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (B) Roh-Daten per DMA
volatile uint16_t ADC_DMA_Buffer_C[ADC1d_ANZ*ADC_ARRAY_LEN];  //  (C) sortierte Daten


//--------------------------------------------------------------
// Adressen der ADCs
// (siehe Seite 66+427 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_BASE_ADR        ((uint32_t)0x40012000)
#define ADC_COM_OFFSET      ((uint32_t)0x00000300)


//--------------------------------------------------------------
// Adressen der Register
// (siehe Seite 427+428 vom Referenz Manual)
//--------------------------------------------------------------
#define ADC_REG_CDR_OFFSET         0x08

#define ADC1_CDR_ADDRESS    (ADC_BASE_ADR | ADC_COM_OFFSET | ADC_REG_CDR_OFFSET)




//--------------------------------------------------------------
// ADC-Clock
// Max-ADC-Frq = 36MHz
// Grundfrequenz = APB2 (APB2=84MHz @ Sysclock=168MHz)
// Mögliche Vorteiler = 2,4,6,8
//
// Max-Conversion-Time @ 21MHz : (with minimum SampleTime)
//   12bit Resolution : 3+12=15 TCycles => 714ns
//   10bit Resolution : 3+10=13 TCycles => 619ns
//    8bit Resolution : 3+8 =11 TCycles => 523ns
//    6bit Resolution : 3+6 =9  TCycles => 428ns
//--------------------------------------------------------------

//#define ADC1d_VORTEILER     ADC_Prescaler_Div2 // Frq = 42 MHz
#define ADC1d_VORTEILER     ADC_Prescaler_Div4   // Frq = 21 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div6 // Frq = 14 MHz
//#define ADC1d_VORTEILER     ADC_Prescaler_Div8 // Frq = 10.5 MHz


//--------------------------------------------------------------
// DMA Einstellung
// (siehe Seite 304+305 vom Referenz Manual)
// Moegliche DMAs fuer ADC1 :
//   DMA2_STREAM0_CHANNEL0
//   DMA2_STREAM4_CHANNEL0
//--------------------------------------------------------------


#define ADC1_DMA_STREAM            DMA2_Stream0
#define ADC1_DMA_CHANNEL           DMA_Channel_0



//--------------------------------------------------------------
// Struktur eines ADC Kanals
//--------------------------------------------------------------
typedef struct {
  ADC1d_NAME_t ADC_NAME;  // Name
  GPIO_TypeDef* ADC_PORT; // Port
  const uint16_t ADC_PIN; // Pin
  const uint32_t ADC_CLK; // Clock
  const uint8_t ADC_CH;   // ADC-Kanal
}ADC1d_t;



//--------------------------------------------------------------
// Timer-2
//
// Grundfreq = 2*APB1 (APB1=42MHz @ Sysclock=168MHz) => TIM_CLK=84MHz
//
// ADC-FRQ = 84MHz / (PRESCALE+1) / (PERIODE+1)
//
//--------------------------------------------------------------
#define OSZI_TIM2_PERIODE      299
#define OSZI_TIM2_PRESCALE     83


//--------------------------------------------------------------
typedef enum {
  ADC_VORLAUF =0,
  ADC_RUNNING,
  ADC_PRE_TRIGGER,
  ADC_TRIGGER_OK,
  ADC_READY
}ADC_Status_t;


//--------------------------------------------------------------
typedef struct {
  ADC_Status_t status;
  uint32_t trigger_pos;
  uint32_t trigger_quarter;
  uint32_t dma_status;
}ADC_t;
ADC_t ADC_UB;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void ADC_Init_ALL(void);
void ADC_change_Frq(uint32_t n);
void ADC_change_Mode(uint32_t n);



//--------------------------------------------------------------
#endif // __STM32F4_UB_ADC_H
