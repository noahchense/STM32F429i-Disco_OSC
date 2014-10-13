//--------------------------------------------------------------
// File     : stm32_ub_spi5.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SPI5_H
#define __STM32F4_UB_SPI5_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"


//--------------------------------------------------------------
// SPI-Mode (Full-Duplex)
//--------------------------------------------------------------
typedef enum {
  SPI_MODE_0_MSB = 0,  // CPOL=0, CPHA=0 (MSB-First)
  SPI_MODE_1_MSB,      // CPOL=0, CPHA=1 (MSB-First)
  SPI_MODE_2_MSB,      // CPOL=1, CPHA=0 (MSB-First)
  SPI_MODE_3_MSB,      // CPOL=1, CPHA=1 (MSB-First)
  SPI_MODE_0_LSB,      // CPOL=0, CPHA=0 (LSB-First)
  SPI_MODE_1_LSB,      // CPOL=0, CPHA=1 (LSB-First)
  SPI_MODE_2_LSB,      // CPOL=1, CPHA=0 (LSB-First)
  SPI_MODE_3_LSB       // CPOL=1, CPHA=1 (LSB-First) 
}SPI5_Mode_t;


//--------------------------------------------------------------
// SPI-Clock
// Grundfrequenz (SPI5)= APB2 (APB2=90MHz)
// Mögliche Vorteiler = 2,4,8,16,32,64,128,256
//--------------------------------------------------------------

//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_2   // Frq = 45 MHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_4   // Frq = 22,5 MHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_8   // Frq = 11,25 MHz
#define SPI5_VORTEILER     SPI_BaudRatePrescaler_16  // Frq = 5.625 MHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_32  // Frq = 2.812 MHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_64  // Frq = 1.406 MHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_128 // Frq = 703.2 kHz
//#define SPI5_VORTEILER     SPI_BaudRatePrescaler_256 // Frq = 351.5 kHz




//--------------------------------------------------------------
// Struktur eines SPI-Pins
//--------------------------------------------------------------
typedef struct {
  GPIO_TypeDef* PORT;     // Port
  const uint16_t PIN;     // Pin
  const uint32_t CLK;     // Clock
  const uint8_t SOURCE;   // Source
}SPI5_PIN_t; 

//--------------------------------------------------------------
// Struktur vom SPI-Device (im Full-Duplex-Mode)
//--------------------------------------------------------------
typedef struct {
  SPI5_PIN_t  SCK;        // SCK-Pin
  SPI5_PIN_t  MOSI;       // MOSI-Pin
  SPI5_PIN_t  MISO;       // MISO-Pin
}SPI5_DEV_t;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_SPI5_Init(SPI5_Mode_t mode);
uint8_t UB_SPI5_SendByte(uint8_t wert);




//--------------------------------------------------------------
#endif // __STM32F4_UB_SPI5_H
