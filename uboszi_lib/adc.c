//--------------------------------------------------------------
// File     : adc.c
// Datum    : 05.01.2014
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : ADC, DMA, TIM
// Funktion : ADC
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "adc.h"



//--------------------------------------------------------------
// Definition der benutzten ADC Pins (max=16)
// Reihenfolge wie bei ADC1d_NAME_t
//--------------------------------------------------------------
ADC1d_t ADC1d[] = {
  //NAME  ,PORT , PIN      , CLOCK              , Kanal
  {ADC_PA5,GPIOA,GPIO_Pin_5,RCC_AHB1Periph_GPIOA,ADC_Channel_5},   // ADC an PA5 = ADC12_IN5 // Noah 20141013
//  {ADC_PA3,GPIOA,GPIO_Pin_3,RCC_AHB1Periph_GPIOA,ADC_Channel_3},   // ADC an PA5 = ADC12_IN5
  {ADC_PA7,GPIOA,GPIO_Pin_7,RCC_AHB1Periph_GPIOA,ADC_Channel_7},   // ADC an PA7 = ADC12_IN7
};




//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_ADC_InitIO(void);
void P_ADC_InitDMA_DoubleBuffer(void);
void P_ADC_InitNVIC(void);
void P_ADC_InitADC(void);
void P_ADC_Start(void);
void P_ADC_InitTimer2(void);
void P_ADC_Clear(void);
void ADC_searchTrigger_A1(void);
void ADC_searchTrigger_B1(void);
void ADC_searchTrigger_A2(void);
void ADC_searchTrigger_B2(void);



//--------------------------------------------------------------
// init vom ADC1 und ADC2 im DMA Mode
// und starten der zyklischen Wandlung
//--------------------------------------------------------------
void ADC_Init_ALL(void)
{
  // init aller Variablen
  ADC_UB.status=ADC_VORLAUF;
  ADC_UB.trigger_pos=0;
  ADC_UB.trigger_quarter=0;
  ADC_UB.dma_status=0;

  P_ADC_Clear();
  P_ADC_InitTimer2();
  P_ADC_InitIO();
  P_ADC_InitDMA_DoubleBuffer();
  P_ADC_InitNVIC();
  P_ADC_InitADC();
  P_ADC_Start();
}


//--------------------------------------------------------------
// ändern der Frequenz vom Timer2
// (Timebase der Abtastrate)
//
// n : [0...16]
//--------------------------------------------------------------
void ADC_change_Frq(uint32_t n)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  uint32_t prescaler, period;

  // Timer anhalten
  TIM_Cmd(TIM2, DISABLE);

  switch(n) {
    case 0 : // 5s=>5Hz=5s
      prescaler=5375;period=3124;
    break;
    case 1 : // 2s=>12,5Hz=80ms
      prescaler=2687;period=2499;
    break;
    case 2 : // 1s=>25Hz=40ms
      prescaler=1343;period=2499;
    break;
    case 3 : // 500ms=>50Hz=20ms
      prescaler=671;period=2499;
    break;
    case 4 : // 200ms=>125Hz=8ms
      prescaler=335;period=1999;
    break;
    case 5 : // 100ms=>250Hz=4ms
      prescaler=167;period=1999;
    break;
    case 6 : // 50ms=>500Hz=2ms
      prescaler=83;period=1999;
    break;
    case 7 : // 20ms=>1,25kHz=800us
      prescaler=41;period=1599;
    break;
    case 8 : // 10ms=>2,5kHz400us
      prescaler=20;period=1599;
    break;
    case 9 : // 5ms=>5kHz=200us
      prescaler=20;period=799;
    break;
    case 10 : // 2ms=>12,5kHz=80us
      prescaler=20;period=319;
    break;
    case 11 : // 1ms=>25kHz=40us
      prescaler=20;period=159;
    break;
    case 12 : // 500us=>50kHz=20us
      prescaler=20;period=79;
    break;
    case 13 : // 200us=>125kHz=8us
      prescaler=20;period=31;
    break;
    case 14 : // 100us=>250kHz=4us
      prescaler=20;period=15;
    break;
    case 15 : // 50us=>500kHz=2us
      prescaler=20;period=7;
    break;
    case 16 : // 25us=>1MHz=1us
      prescaler=20;period=3;
    break;
    default :
      prescaler=OSZI_TIM2_PRESCALE;
      period=OSZI_TIM2_PERIODE;
  }

  // einstellen der neuen Werte
  TIM_TimeBaseStructure.TIM_Period =  period;
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Timer wieder starten, falls notwendig
  if(ADC_UB.status!=ADC_READY) {
    // Timer2 enable
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
  }
}



//--------------------------------------------------------------
// ändern vom Mode des DMA
// n != 1 => Double-Buffer-Mode
// n = 1  => Single-Buffer-Mode
//--------------------------------------------------------------
void ADC_change_Mode(uint32_t n)
{
  DMA_InitTypeDef       DMA_InitStructure;

  // Merker setzen
  ADC_UB.dma_status=1;

  // Timer analten
  TIM_Cmd(TIM2, DISABLE);

  // DMA-Disable
  DMA_Cmd(ADC1_DMA_STREAM, DISABLE);
  // warten bis DMA-Stream disable
  while(DMA_GetCmdStatus(ADC1_DMA_STREAM)==ENABLE);
  DMA_DeInit(ADC1_DMA_STREAM);

  // DMA-Config
  DMA_InitStructure.DMA_Channel = ADC1_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_CDR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC_DMA_Buffer_A;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = ADC1d_ANZ*ADC_ARRAY_LEN;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(ADC1_DMA_STREAM, &DMA_InitStructure);

  if(n!=1) {
    // Double-Buffer-Mode
    ADC1_DMA_STREAM->CR|=(uint32_t)DMA_SxCR_DBM;
    ADC1_DMA_STREAM->M1AR=(uint32_t)&ADC_DMA_Buffer_B;
  }
  else {
    // Normal Mode
    ADC1_DMA_STREAM->CR&=~(uint32_t)DMA_SxCR_DBM;
  }

  // Flags löschen
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);

  // DMA-enable
  DMA_Cmd(ADC1_DMA_STREAM, ENABLE);

  // warten bis DMA-Stream enable
  while(DMA_GetCmdStatus(ADC1_DMA_STREAM)==DISABLE);

  // NVIC neu initialisieren
  P_ADC_InitNVIC();

  // Timer wieder starten, falls notwendig
  if((ADC_UB.status!=ADC_READY) || (n==1)) {
    // Timer2 enable
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
  }

  // Merker zurücksetzen
  ADC_UB.dma_status=0;
}


//--------------------------------------------------------------
// interne Funktion
// löschen aller ADC-Arrays
//--------------------------------------------------------------
void P_ADC_Clear(void)
{
  uint32_t n;

  for(n=0;n<ADC_ARRAY_LEN;n++) {
    ADC_DMA_Buffer_A[n*2]=0x00;
    ADC_DMA_Buffer_A[(n*2)+1]=0x00;

    ADC_DMA_Buffer_B[n*2]=0x00;
    ADC_DMA_Buffer_B[(n*2)+1]=0x00;

    ADC_DMA_Buffer_C[n*2]=0x00;
    ADC_DMA_Buffer_C[(n*2)+1]=0x00;
  }
}


//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins
//--------------------------------------------------------------
void P_ADC_InitIO(void) {
  GPIO_InitTypeDef      GPIO_InitStructure;
  ADC1d_NAME_t adc_name;

  for(adc_name=0;adc_name<ADC1d_ANZ;adc_name++) {
    // Clock Enable
    RCC_AHB1PeriphClockCmd(ADC1d[adc_name].ADC_CLK, ENABLE);

    // Config des Pins als Analog-Eingang
    GPIO_InitStructure.GPIO_Pin = ADC1d[adc_name].ADC_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(ADC1d[adc_name].ADC_PORT, &GPIO_InitStructure);
  }
}


//--------------------------------------------------------------
// interne Funktion
// Init vom DMA (im Double-Buffer-Mode)
//--------------------------------------------------------------
void P_ADC_InitDMA_DoubleBuffer(void)
{
  DMA_InitTypeDef       DMA_InitStructure;

  // Clock Enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // DMA-Disable
  DMA_Cmd(ADC1_DMA_STREAM, DISABLE);
  // warten bis DMA-Stream disable
  while(DMA_GetCmdStatus(ADC1_DMA_STREAM)==ENABLE);
  DMA_DeInit(ADC1_DMA_STREAM);

  // DMA-Config
  DMA_InitStructure.DMA_Channel = ADC1_DMA_CHANNEL;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_CDR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC_DMA_Buffer_A;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = ADC1d_ANZ*ADC_ARRAY_LEN;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(ADC1_DMA_STREAM, &DMA_InitStructure);

  // Double-Buffer-Mode
  ADC1_DMA_STREAM->CR|=(uint32_t)DMA_SxCR_DBM;
  ADC1_DMA_STREAM->M1AR=(uint32_t)&ADC_DMA_Buffer_B;

  // Flags löschen
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);

  // DMA-enable
  DMA_Cmd(ADC1_DMA_STREAM, ENABLE);

  // warten bis DMA-Stream enable
  while(DMA_GetCmdStatus(ADC1_DMA_STREAM)==DISABLE);
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_ADC_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //---------------------------------------------
  // init vom DMA Interrupt
  // für TransferComplete Interrupt
  // und HalfTransferComplete Interrupt
  // DMA2, Stream0, Channel0
  //---------------------------------------------

  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC | DMA_IT_HT, ENABLE);

  // NVIC konfig
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// Init von ADC Nr.1+2 (im Dual regular simultaneous mode)
//
// @ 12bit + ADC_TwoSamplingDelay_5Cycles + 21MHz ADC-Clock :
//
// ADC_SampleTime_3Cycles  => Sample_Time =  3+12+5=20 => 952ns
// ADC_SampleTime_15Cycles => Sample_Time = 15+12+5=32 => 1.52us
// ADC_SampleTime_28Cycles => Sample_Time = 28+12+5=45 => 2.14us
//--------------------------------------------------------------
void P_ADC_InitADC(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef       ADC_InitStructure;

  // Clock Enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

  //-------------------------------------
  // ADC-Config (DualMode)
  //-------------------------------------

  ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
  ADC_CommonInitStructure.ADC_Prescaler = ADC1d_VORTEILER;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  //-------------------------------------
  // ADC1 (Master)
  //-------------------------------------

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; // nur ein Eintrag in der Regular-List -> disable
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; // Master wird getriggert
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC1, ADC1d[0].ADC_CH, 1, ADC_SampleTime_3Cycles);

  //-------------------------------------
  // ADC2 (Slave)
  //-------------------------------------

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; // nur ein Eintrag in der Regular-List -> disable
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // Slave darf nicht gertriggert werden
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC2, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC2, ADC1d[1].ADC_CH, 1, ADC_SampleTime_3Cycles);
}


//--------------------------------------------------------------
// interne Funktion
// Enable und start vom ADC und DMA
//--------------------------------------------------------------
void P_ADC_Start(void)
{
  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
  ADC_DMACmd(ADC1, ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);
  // Timer2 enable
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_Cmd(TIM2, ENABLE);
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_ADC_InitTimer2(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  //---------------------------------------------
  // init Timer2
  // Clock-Source for ADC-Conversion
  //---------------------------------------------


  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Timer2 init
  TIM_TimeBaseStructure.TIM_Period =  OSZI_TIM2_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = OSZI_TIM2_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
}


//--------------------------------------------------------------
// Interrupt (ISR-Funktion)
// wird bei DMA Interrupt aufgerufen
//   (Bei HalfTransferComplete und TransferCompleteInterrupt)
//
//--------------------------------------------------------------
void DMA2_Stream0_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0))
  {
    // HalfTransferInterruptComplete Interrupt von DMA2 ist aufgetreten
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);

    if(ADC_UB.dma_status==0) {
      if((ADC_UB.status==ADC_RUNNING) || (ADC_UB.status==ADC_PRE_TRIGGER)) {
        if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) == 0) {
          ADC_searchTrigger_A1();
        }
        else {
          ADC_searchTrigger_B1();
        }
      }
    }
  }
  else if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
  {
    // TransferInterruptComplete Interrupt von DMA2 ist aufgetreten
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);

    if(ADC_UB.dma_status==0) {
      TIM_Cmd(TIM2, DISABLE);

      if(ADC_UB.status!=ADC_VORLAUF) {
        if(ADC_UB.status==ADC_TRIGGER_OK) {
          ADC_UB.status=ADC_READY;
        }
        else {
    	  TIM_Cmd(TIM2, ENABLE);
          if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) != 0) {
            ADC_searchTrigger_A2();
          }
          else {
            ADC_searchTrigger_B2();
          }
        }
      }
      else {
        TIM_Cmd(TIM2, ENABLE);
        ADC_UB.status=ADC_RUNNING;
      }
    }
    UB_Led_Toggle(LED_GREEN);
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-1
//--------------------------------------------------------------
void ADC_searchTrigger_A1(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
  else {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=1;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-2
//--------------------------------------------------------------
void ADC_searchTrigger_A2(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
  else {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_A[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=2;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-3
//--------------------------------------------------------------
void ADC_searchTrigger_B1(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
  else {
    for(n=0;n<ADC_HALF_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=3;
          break;
        }
      }
    }
  }
}


//--------------------------------------------------------------
// interne Funktion
// sucht Trigger-Punkt in Quadrant-4
//--------------------------------------------------------------
void ADC_searchTrigger_B2(void)
{
  uint32_t n,ch;
  uint16_t wert,trigger;

  if(Menu.trigger.mode==1) return;

  if(Menu.trigger.source==0) {
    ch=0;
    trigger=Menu.trigger.value_ch1;
  }
  else {
    ch=1;
    trigger=Menu.trigger.value_ch2;
  }

  if(Menu.trigger.edge==0) {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert<trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert>=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
  else {
    for(n=ADC_HALF_ARRAY_LEN;n<ADC_ARRAY_LEN;n++) {
      wert=ADC_DMA_Buffer_B[(n*2)+ch];
      if(ADC_UB.status==ADC_RUNNING) {
        if(wert>trigger) {
          ADC_UB.status=ADC_PRE_TRIGGER;
        }
      }
      else {
        if(wert<=trigger) {
          ADC_UB.status=ADC_TRIGGER_OK;
          ADC_UB.trigger_pos=n;
          ADC_UB.trigger_quarter=4;
          break;
        }
      }
    }
  }
}
