//--------------------------------------------------------------
// File     : stm32_ub_spi5.c
// Datum    : 29.10.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, SPI
// Funktion : SPI-LoLevel-Funktionen (SPI-5) Full-Duplex-Mode
//
// Hinweis  : mögliche Pinbelegungen
//            SPI5 : SCK :[PF7] 
//                   MOSI:[PF9, PF11]
//                   MISO:[PF8]
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_spi5.h"


//--------------------------------------------------------------
// Definition von SPI5
//--------------------------------------------------------------
SPI5_DEV_t SPI5DEV = {
// PORT , PIN      , Clock              , Source 
  {GPIOF,GPIO_Pin_7,RCC_AHB1Periph_GPIOF,GPIO_PinSource7}, // SCK an PF7
  {GPIOF,GPIO_Pin_9,RCC_AHB1Periph_GPIOF,GPIO_PinSource9}, // MOSI an PF9
  {GPIOF,GPIO_Pin_8,RCC_AHB1Periph_GPIOF,GPIO_PinSource8}, // MISO an PF8
};



//--------------------------------------------------------------
// Init von SPI5 (im Full-Duplex-Mode) 
// Return_wert :
//  -> ERROR   , wenn SPI schon mit anderem Mode initialisiert
//  -> SUCCESS , wenn SPI init ok war
//--------------------------------------------------------------
ErrorStatus UB_SPI5_Init(SPI5_Mode_t mode)
{
  ErrorStatus ret_wert=ERROR;
  static uint8_t init_ok=0;
  static SPI5_Mode_t init_mode;
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    if(init_mode==mode) ret_wert=SUCCESS;
    return(ret_wert);
  }

  // SPI-Clock enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5, ENABLE);

  // Clock Enable der Pins
  RCC_AHB1PeriphClockCmd(SPI5DEV.SCK.CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI5DEV.MOSI.CLK, ENABLE);
  RCC_AHB1PeriphClockCmd(SPI5DEV.MISO.CLK, ENABLE);

  // SPI Alternative-Funktions mit den IO-Pins verbinden
  GPIO_PinAFConfig(SPI5DEV.SCK.PORT, SPI5DEV.SCK.SOURCE, GPIO_AF_SPI5);
  GPIO_PinAFConfig(SPI5DEV.MOSI.PORT, SPI5DEV.MOSI.SOURCE, GPIO_AF_SPI5);
  GPIO_PinAFConfig(SPI5DEV.MISO.PORT, SPI5DEV.MISO.SOURCE, GPIO_AF_SPI5);

  // SPI als Alternative-Funktion mit PullDown
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  // SCK-Pin
  GPIO_InitStructure.GPIO_Pin = SPI5DEV.SCK.PIN;
  GPIO_Init(SPI5DEV.SCK.PORT, &GPIO_InitStructure);
  // MOSI-Pin
  GPIO_InitStructure.GPIO_Pin = SPI5DEV.MOSI.PIN;
  GPIO_Init(SPI5DEV.MOSI.PORT, &GPIO_InitStructure);
  // MISO-Pin
  GPIO_InitStructure.GPIO_Pin = SPI5DEV.MISO.PIN;
  GPIO_Init(SPI5DEV.MISO.PORT, &GPIO_InitStructure);

  SPI_I2S_DeInit(SPI5);

  // SPI-Konfiguration
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  if((mode==SPI_MODE_0_MSB) || (mode==SPI_MODE_0_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else if((mode==SPI_MODE_1_MSB) || (mode==SPI_MODE_1_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  }else if((mode==SPI_MODE_2_MSB) || (mode==SPI_MODE_2_LSB)) {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }else {
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  } 
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI5_VORTEILER;

  if((mode==SPI_MODE_0_MSB) || (mode==SPI_MODE_1_MSB) ||
     (mode==SPI_MODE_2_MSB) || (mode==SPI_MODE_3_MSB)) {
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  }
  else {
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
  }

  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI5, &SPI_InitStructure); 

  // SPI enable
  SPI_Cmd(SPI5, ENABLE); 

  // init Mode speichern
  init_ok=1;
  init_mode=mode;
  ret_wert=SUCCESS;

  return(ret_wert);
}


//--------------------------------------------------------------
// sendet und empfängt ein Byte per SPI5 (im Full-Duplex-Mode)
// ChipSelect-Signal muss von rufender Funktion gemacht werden
//--------------------------------------------------------------
uint8_t UB_SPI5_SendByte(uint8_t wert)
{ 
  uint8_t ret_wert=0;

  // warte bis senden fertig
  while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_TXE) == RESET);

  // byte senden
  SPI_I2S_SendData(SPI5, wert);

  // warte bis empfang fertig
  while (SPI_I2S_GetFlagStatus(SPI5, SPI_I2S_FLAG_RXNE) == RESET); 

  // Daten einlesen
  ret_wert=SPI_I2S_ReceiveData(SPI5);

  return(ret_wert);
}
