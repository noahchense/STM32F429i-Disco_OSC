//--------------------------------------------------------------
// File     : stm32_ub_button.c
// Datum    : 24.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, (TIM, MISC)
// Funktion : Button Funktionen
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_button.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
#if BUTTON_USE_TIMER==1
  void P_Button_InitTIM(void);
  void P_Button_InitNVIC(void);
#endif




//--------------------------------------------------------------
// Definition aller Buttons
// Reihenfolge wie bei BUTTON_NAME_t
//
// Widerstand : [GPIO_PuPd_NOPULL,GPIO_PuPd_UP,GPIO_PuPd_DOWN]
//--------------------------------------------------------------
BUTTON_t BUTTON[] = {
  // Name    ,PORT , PIN       , CLOCK              ,Widerstand      , Status
  {BTN_USER  ,GPIOA,GPIO_Pin_0 ,RCC_AHB1Periph_GPIOA,GPIO_PuPd_NOPULL, Bit_RESET},  // PA0=User-Button auf dem Discovery-Board
};


//--------------------------------------------------------------
// Init aller Buttons
//--------------------------------------------------------------
void UB_Button_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  BUTTON_NAME_t btn_name;
  
  for(btn_name=0;btn_name<BUTTON_ANZ;btn_name++) {
    // Clock Enable
    RCC_AHB1PeriphClockCmd(BUTTON[btn_name].BUTTON_CLK, ENABLE);
  
    // Config als Digital-Eingang
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = BUTTON[btn_name].BUTTON_R;
    GPIO_InitStructure.GPIO_Pin = BUTTON[btn_name].BUTTON_PIN;
    GPIO_Init(BUTTON[btn_name].BUTTON_PORT, &GPIO_InitStructure);
  }

#if BUTTON_USE_TIMER==1
  // Init vom Timer
  P_Button_InitTIM();
  // Init vom NVIC
  P_Button_InitNVIC();
#endif
}


//--------------------------------------------------------------
// Status von einem Button auslesen (nicht entprellt)
// Return Wert :
//  -> wenn Button losgelassen = BTN_RELEASED
//  -> wenn Button gedrueckt   = BTN_PRESSED
//--------------------------------------------------------------
BUTTON_STATUS_t UB_Button_Read(BUTTON_NAME_t btn_name)
{
  uint8_t wert;

  wert=GPIO_ReadInputDataBit(BUTTON[btn_name].BUTTON_PORT, BUTTON[btn_name].BUTTON_PIN);
  if(wert==Bit_RESET) {
    return(BTN_RELEASED);
  }
  else {
    return(BTN_PRESSED);
  }
} 


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnPressed Auswertung (entprellt)
// ret_wert, ist solange true wie der Button betätigt ist
//--------------------------------------------------------------
bool UB_Button_OnPressed(BUTTON_NAME_t btn_name)
{
  uint8_t wert;

  wert=BUTTON[btn_name].BUTTON_AKT;

  if(wert==Bit_RESET) {
    return(false);
  }
  else {
    return(true);
  }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnClick Auswertung (entprellt)
// ret_wert, ist nur einmal true wenn der Button betätigt wurde
//--------------------------------------------------------------
bool UB_Button_OnClick(BUTTON_NAME_t btn_name)
{
  uint8_t wert,old;
  static uint8_t old_wert[BUTTON_ANZ];

  wert=BUTTON[btn_name].BUTTON_AKT;
  old=old_wert[btn_name];
  old_wert[btn_name]=wert;

  if(wert==Bit_RESET) {
    return(false);
  }
  else if(old!=Bit_RESET) {
    return(false);
  }
  else {
    return(true);
  }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// Button OnRelease Auswertung (entprellt)
// ret_wert, ist nur einmal true wenn der Button losgelassen wurde
//--------------------------------------------------------------
bool UB_Button_OnRelease(BUTTON_NAME_t btn_name)
{
  uint8_t wert,old;
  static uint8_t old_wert[BUTTON_ANZ];

  wert=BUTTON[btn_name].BUTTON_AKT;
  old=old_wert[btn_name];
  old_wert[btn_name]=wert;

  if(wert!=Bit_RESET) {
    return(false);
  }
  else if(old==Bit_RESET) {
    return(false);
  }
  else {
    return(true);
  }
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_Button_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  // Clock enable
  RCC_APB1PeriphClockCmd(UB_BUTTON_TIM_CLK, ENABLE);

  // Timer init
  TIM_TimeBaseStructure.TIM_Period =  UB_BUTTON_TIM_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = UB_BUTTON_TIM_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(UB_BUTTON_TIM, &TIM_TimeBaseStructure);

  // Timer enable
  TIM_ARRPreloadConfig(UB_BUTTON_TIM, ENABLE);
  TIM_Cmd(UB_BUTTON_TIM, ENABLE);
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_Button_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //---------------------------------------------
  // init vom Timer Interrupt
  //---------------------------------------------
  TIM_ITConfig(UB_BUTTON_TIM,TIM_IT_Update,ENABLE);

  // NVIC konfig
  NVIC_InitStructure.NVIC_IRQChannel = UB_BUTTON_TIM_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
#endif


#if BUTTON_USE_TIMER==1
//--------------------------------------------------------------
// ISR vom Timer
//--------------------------------------------------------------
void UB_BUTTON_TIM_ISR_HANDLER(void)
{
  BUTTON_NAME_t btn_name;
  uint8_t wert;

  // es gibt hier nur eine Interrupt Quelle
  TIM_ClearITPendingBit(UB_BUTTON_TIM, TIM_IT_Update);

  // alle Buttons einlesen
  for(btn_name=0;btn_name<BUTTON_ANZ;btn_name++) {
    wert=GPIO_ReadInputDataBit(BUTTON[btn_name].BUTTON_PORT, BUTTON[btn_name].BUTTON_PIN);
    BUTTON[btn_name].BUTTON_AKT=wert;
  }
}
#endif
