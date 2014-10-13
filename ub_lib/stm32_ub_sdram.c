//--------------------------------------------------------------
// File     : stm32_ub_sdram.c
// Datum    : 24.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, FMC
// Funktion : Externes SDRAM [8MByte]
//            Typ = IS42S16400J [64Mbit, 1M x 16bit x 4Bank, 7ns]
//            Der Zugriff erfolgt ueber den FMC-Controller
//
// Hinweis  : Das SDRAM benutzt die CPU-Pins :
//             PB5  = SDCKE1 (CKE)      PF0  = A0
//             PB6  = SDNE1  (/CS)      PF1  = A1
//             PC0  = SDNWE  (/WE)      PF2  = A2
//             PD0  = D2                PF3  = A3
//             PD1  = D3                PF4  = A4
//             PD8  = D13               PF5  = A5
//             PD9  = D14               PF11 = SDNRAS (/RAS)
//             PD10 = D15               PF12 = A6
//             PD14 = D0                PF13 = A7
//             PD15 = D1                PF14 = A8
//             PE0  = NBL0   (LDQM)     PF15 = A9
//             PE1  = NBL1   (UDQM)     PG0  = A10
//             PE7  = D4                PG1  = A11
//             PE8  = D5                PG4  = BA0    (BANK A0)
//             PE9  = D6                PG5  = BA1    (BANK A1)
//             PE10 = D7                PG8  = SDCLK  (CLK)
//             PE11 = D8                PG15 = SDNCAS (/CAS)
//             PE12 = D9
//             PE13 = D10
//             PE14 = D11
//             PE15 = D12
//         
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_sdram.h"


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_SDRAM_InitIO(void);
void P_SDRAM_InitFMC(void);
void P_SDRAM_InitSequence(void);
static void P_SDRAM_delay(volatile uint32_t nCount);



//--------------------------------------------------------------
// Init vom externen SDRAM
// Return_wert :
//  -> ERROR   , wenn SDRAM nicht gefunden wurde
//  -> SUCCESS , wenn SDRAM gefunden wurde
//--------------------------------------------------------------
ErrorStatus UB_SDRAM_Init(void)
{
  ErrorStatus ret_wert=ERROR;
  uint16_t oldwert,istwert;
  static uint8_t init_ok=0;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) { 
    if(init_ok==1) {
      return(SUCCESS);
    }
    else {
      return(ERROR);
    }
  }

  // IO-Lines initialisieren
  P_SDRAM_InitIO();
  // FMC initialisieren
  P_SDRAM_InitFMC();

  //-----------------------------------------
  // check ob SDRAM vorhanden ist
  // schreib-/lese-Test auf Adr 0x00
  //-----------------------------------------
  oldwert=UB_SDRAM_Read16b(0x00);
  UB_SDRAM_Write16b(0x00, 0x5A3C);
  istwert=UB_SDRAM_Read16b(0x00);
  UB_SDRAM_Write16b(0x00, oldwert);
  if(istwert==0x5A3C) ret_wert=SUCCESS; // RAM vorhanden

  // init Mode speichern
  if(ret_wert==SUCCESS) {
    init_ok=1;
  }
  else {
    init_ok=2;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// schreibt einen 8bit Wert ins externe SDRAM
// in eine Adresse
// wert : [0 bis 255]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 1 Byte abstand zueinander haben
//--------------------------------------------------------------
void UB_SDRAM_Write8b(uint32_t adr, uint8_t wert)
{
  *(uint8_t*) (SDRAM_START_ADR + adr) = wert;
}


//--------------------------------------------------------------
// lieﬂt einen 8bit Wert aus dem externen SDRAM
// von einer Adresse
// ret_wert : [0 bis 255]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 1 Byte abstand zueinander haben
//-------------------------------------------------------------- 
uint8_t UB_SDRAM_Read8b(uint32_t adr)
{
  uint8_t ret_wert=0;

  ret_wert = *(__IO uint8_t*)(SDRAM_START_ADR + adr);

  return(ret_wert);
}


//--------------------------------------------------------------
// schreibt einen 16bit Wert ins externe SDRAM
// in eine Adresse
// wert : [0 bis 65535]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 2 Bytes abstand zueinander haben
//--------------------------------------------------------------
void UB_SDRAM_Write16b(uint32_t adr, uint16_t wert)
{
  *(uint16_t*) (SDRAM_START_ADR + adr) = wert;
}


//--------------------------------------------------------------
// lieﬂt einen 16bit Wert aus dem externen SDRAM
// von einer Adresse
// ret_wert : [0 bis 65535]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 2 Bytes abstand zueinander haben
//-------------------------------------------------------------- 
uint16_t UB_SDRAM_Read16b(uint32_t adr)
{
  uint16_t ret_wert=0;

  ret_wert = *(__IO uint16_t*)(SDRAM_START_ADR + adr);

  return(ret_wert);
}


//--------------------------------------------------------------
// schreibt einen 32bit Wert ins externe SDRAM
// in eine Adresse
// wert : [0 bis 0xFFFFFFFF]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 4 Bytes abstand zueinander haben
//--------------------------------------------------------------
void UB_SDRAM_Write32b(uint32_t adr, uint32_t wert)
{
  *(uint32_t*) (SDRAM_START_ADR + adr) = wert;
}


//--------------------------------------------------------------
// lieﬂt einen 32bit Wert aus dem externen SDRAM
// von einer Adresse
// ret_wert : [0 bis 0xFFFFFFFF]
// adr  : [0 bis SDRAM_MAX_ADR]
// Adressen muessen 4 Bytes abstand zueinander haben
//-------------------------------------------------------------- 
uint32_t UB_SDRAM_Read32b(uint32_t adr)
{
  uint32_t ret_wert=0;

  ret_wert = *(__IO uint32_t*)(SDRAM_START_ADR + adr);

  return(ret_wert);
}


//--------------------------------------------------------------
// schreibt einen Block von 32bit Werten ins externe SDRAM
// ab einer Adresse
// ptrBuffer : Pointer auf den Start vom Block der geschrieben wird
// startAdr  : Start Adresse vom Ram ab der geschrieben wird
// lenBuffer : anzahl der Daten die geschrieben werden sollen
//--------------------------------------------------------------
void UB_SDRAM_WriteBuffer32b(uint32_t* ptrBuffer, uint32_t startAdr, uint32_t lenBuffer)
{
  volatile uint32_t write_pointer = (uint32_t)startAdr;

  FMC_SDRAMWriteProtectionConfig(FMC_Bank2_SDRAM, DISABLE);

  // warten bis Controller bereit
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  // alle Daten schreiben
  for (; lenBuffer != 0; lenBuffer--) {
    // Transfer data to the memory
    *(uint32_t *) (SDRAM_START_ADR + write_pointer) = *ptrBuffer++;
    write_pointer += 4;
  }
}


//--------------------------------------------------------------
// lieﬂt einen Block von 32bit Werten aus dem externen SDRAM
// ab einer Adresse
// ptrBuffer : Pointer auf den Start vom Block der gelesen wird
// startAdr  : Start Adresse vom Ram ab der gelesen wird
// uwBufferSize  : [0 bis SRAM_MAX_ADR]
//--------------------------------------------------------------
void UB_SDRAM_ReadBuffer32b(uint32_t* ptrBuffer, uint32_t startAdr, uint32_t lenBuffer)
{
  volatile uint32_t write_pointer = (uint32_t)startAdr;


  // warten bis Controller bereit
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  // alle Daten lesen
  for(; lenBuffer != 0x00; lenBuffer--) {
    *ptrBuffer++ = *(volatile uint32_t *)(SDRAM_START_ADR + write_pointer );
    write_pointer += 4;
  }
}



//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins fuer das SRAM
//--------------------------------------------------------------
void P_SDRAM_InitIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
                         RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

   
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

   
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6 , GPIO_AF_FMC);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5  | GPIO_Pin_6;      

  GPIO_Init(GPIOB, &GPIO_InitStructure);  

   
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource0 , GPIO_AF_FMC);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;      

  GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
   
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1  | GPIO_Pin_8 |
                                GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
                                GPIO_Pin_15;

  GPIO_Init(GPIOD, &GPIO_InitStructure);

  
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 |
                                GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOE, &GPIO_InitStructure);

   
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1 | GPIO_Pin_2 | 
                                GPIO_Pin_3  | GPIO_Pin_4 | GPIO_Pin_5 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;      

  GPIO_Init(GPIOF, &GPIO_InitStructure);

   
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);
  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 |
                                GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_15;

  GPIO_Init(GPIOG, &GPIO_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// Init vom FMC fuer das SDRAM
//--------------------------------------------------------------
void P_SDRAM_InitFMC(void)
{
  FMC_SDRAMInitTypeDef  FMC_SDRAMInitStructure;
  FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure;

  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

  //---------------------------------------------------------
  // FMC auf 180MHz/2 = 90MHz einstellen
  // 90MHz = 11,11 ns
  // Alle Timings laut Datasheet und Speedgrade -7 (=7ns)
  //---------------------------------------------------------
  FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay = 2;    // tMRD=2CLK
  FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 7; // tXSR min=70ns
  FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime = 4;      // tRAS min=42ns
  FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay = 7;        // tRC  min=63ns
  FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime = 2;    // tWR =2CLK 
  FMC_SDRAMTimingInitStructure.FMC_RPDelay = 2;              // tRP  min=15ns
  FMC_SDRAMTimingInitStructure.FMC_RCDDelay = 2;             // tRCD min=15ns

  FMC_SDRAMInitStructure.FMC_Bank = FMC_Bank2_SDRAM;
  FMC_SDRAMInitStructure.FMC_ColumnBitsNumber = FMC_ColumnBits_Number_8b;
  FMC_SDRAMInitStructure.FMC_RowBitsNumber = FMC_RowBits_Number_12b;
  FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth = SDRAM_MEMORY_WIDTH;
  FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;
  FMC_SDRAMInitStructure.FMC_CASLatency = SDRAM_CAS_LATENCY;
  FMC_SDRAMInitStructure.FMC_WriteProtection = FMC_Write_Protection_Disable;
  FMC_SDRAMInitStructure.FMC_SDClockPeriod = SDRAM_CLOCK_PERIOD;
  FMC_SDRAMInitStructure.FMC_ReadBurst = SDRAM_READBURST;
  FMC_SDRAMInitStructure.FMC_ReadPipeDelay = FMC_ReadPipe_Delay_1;
  FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct = &FMC_SDRAMTimingInitStructure;

  FMC_SDRAMInit(&FMC_SDRAMInitStructure);

  P_SDRAM_InitSequence();
}


//--------------------------------------------------------------
// interne Funktion
// Initsequenz fuer das SDRAM
//--------------------------------------------------------------
void P_SDRAM_InitSequence(void)
{
  FMC_SDRAMCommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;

  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_CLK_Enabled;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  P_SDRAM_delay(10);


  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_PALL;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_AutoRefresh;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 4;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);


  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                   SDRAM_MODEREG_CAS_LATENCY_3           |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_LoadMode;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank2;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;
  
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);

  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  //-----------------------------------------------
  // FMC_CLK = 90MHz
  // Refresh_Rate = 7.81us
  // Counter = (FMC_CLK * Refresh_Rate) - 20
  //-----------------------------------------------
  FMC_SetRefreshCount(683);
 
  while(FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET);
}


//--------------------------------------------------------------
// interne Funktion
// kleine Pause
//--------------------------------------------------------------
static void P_SDRAM_delay(volatile uint32_t nCount)
{
  volatile uint32_t index = 0;
  for(index = (100000 * nCount); index != 0; index--);
}

