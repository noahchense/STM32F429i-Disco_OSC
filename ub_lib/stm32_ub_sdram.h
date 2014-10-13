//--------------------------------------------------------------
// File     : stm32_ub_sdram.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_SDRAM_H
#define __STM32F4_UB_SDRAM_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_fmc.h"


//--------------------------------------------------------------
// defines
//--------------------------------------------------------------
#define SDRAM_START_ADR      ((uint32_t)0xD0000000)
#define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_16b
#define SDRAM_CLOCK_PERIOD    FMC_SDClock_Period_2
#define SDRAM_CAS_LATENCY     FMC_CAS_Latency_3
#define SDRAM_READBURST       FMC_Read_Burst_Disable
#define SDRAM_MAX_ADR         0x800000  // max=8MByte



//--------------------------------------------------------------
// defines
//--------------------------------------------------------------
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_SDRAM_Init(void);
void UB_SDRAM_Write8b(uint32_t adr, uint8_t wert);
uint8_t UB_SDRAM_Read8b(uint32_t adr);
void UB_SDRAM_Write16b(uint32_t adr, uint16_t wert);
uint16_t UB_SDRAM_Read16b(uint32_t adr);
void UB_SDRAM_Write32b(uint32_t adr, uint32_t wert);
uint32_t UB_SDRAM_Read32b(uint32_t adr);
void UB_SDRAM_WriteBuffer32b(uint32_t* ptrBuffer, uint32_t startAdr, uint32_t lenBuffer);
void UB_SDRAM_ReadBuffer32b(uint32_t* ptrBuffer, uint32_t startAdr, uint32_t lenBuffer);




//--------------------------------------------------------------
#endif // __STM32F4_UB_SDRAM_H
