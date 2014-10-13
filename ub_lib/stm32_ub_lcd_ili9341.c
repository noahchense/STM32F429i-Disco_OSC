//--------------------------------------------------------------
// File     : stm32_ub_lcd_ili9341.c
// Datum    : 10.11.2013
// Version  : 1.2
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, SPI, LTDC, STM32_UB_SPI5, STM32_UB_SDRAM
// Funktion : Grafik-LCD Funktionen (Chip=ILI9341)
//            Der Zugriff erfolgt ueber den TFT-Controller
//            Auflösung : 320 x 240 Pixel
//            Farbmode = 16bit (5R6G5B = RGB565)
//          
// Hinweis  : Das Display benutzt die CPU-Pins :
//             PA3  = G2         PD3  = G7
//             PA4  = VSYNC      PD6  = B2
//             PA6  = G2         PD13 = WRX (CMD/DATA)
//             PA11 = R4         PF7  = SPI_CLK
//             PA12 = R5         PF8  = SPI_MISO
//             PB0  = R3         PF9  = SPI_MOSI
//             PB1  = R6         PF10 = DE (Enable)
//             PB8  = B6         PG6  = R7
//             PB9  = B7         PG7  = CLK
//             PB10 = G4         PG10 = G3
//             PB11 = G5         PG11 = B3
//             PC2  = SPI_CS     PG12 = B4
//             PC6  = HSYNC
//             PC7  = G6
//             PC10 = R2
//
// CPU-Interface : IM[3:0]='0110' => 4Wire SPI
// LCD-Settings  : RCM[1:0]='10', RIM[0]='0', DPI[2:0]='110'
//                 => DE-Mode, 18bit RGB-Interface
//
// SPI-Settings :  SPI-Mode = SPI_MODE_0_MSB, FRQ-Max = 6.6MHz
//                 SPI5 [CLK=PF7, MOSI=PF9, MISO=PF8]
//                 Chip-Select an PC2 
//
// Das externe SDRAM wird als Grafik-RAM benutzt
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_lcd_ili9341.h"


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_LCD9341_InitIO(void);
void P_LCD9341_InitLTCD(void);
void P_LCD9341_AFConfig(void);
void P_LCD9341_CS(BitAction wert);
void P_LCD9341_WRX(BitAction wert);
void P_LCD9341_InitChip(void);
void P_LCD9341_CMD(uint8_t wert);
void P_LCD9341_DATA(uint8_t wert);
void P_LCD9341_Delay(volatile uint32_t nCount);



//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
static uint16_t aktCursorX,aktCursorY;
static uint32_t aktCursorPos;



//--------------------------------------------------------------
// Init vom LCD-Display
// Return_wert :
//  -> ERROR   , wenn Display nicht gefunden wurde
//  -> SUCCESS , wenn Display OK
//--------------------------------------------------------------
ErrorStatus UB_LCD_Init(void)
{ 
  ErrorStatus ret_wert=ERROR;

  // init aller IO-Pins
  P_LCD9341_InitIO();
  // init vom SPI-BUS
  UB_SPI5_Init(SPI_MODE_0_MSB);
  // LCD-Controller initialisieren
  P_LCD9341_InitChip();
  // beim init auf Portrait-Mode
  LCD_DISPLAY_MODE=PORTRAIT;
  // init vom SDRAM
  UB_SDRAM_Init();
  // init vom LTDC
  P_LCD9341_InitLTCD();

  ret_wert=SUCCESS;
  aktCursorX=0;
  aktCursorY=0;
  aktCursorPos=0;

  LCD_CurrentFrameBuffer=LCD_FRAME_BUFFER;
  LCD_MenuFrameBuffer=LCD_FRAME_BUFFER + (2*LCD_FRAME_OFFSET);
  LCD_ADCFrameBuffer=LCD_FRAME_BUFFER + (3*LCD_FRAME_OFFSET);
  LCD_CurrentLayer = 0;
  LCD_CurrentOrientation=0;

  return(ret_wert);
}


//--------------------------------------------------------------
// stellt beide Layer auf "Fullscreen-Mode"
//--------------------------------------------------------------
void UB_LCD_LayerInit_Fullscreen(void)
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

  LTDC_Layer_InitStruct.LTDC_HorizontalStart = 30;
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_MAXX + 30 - 1);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = 4;
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_MAXY + 4 - 1);

  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
  
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_MAXX * 2) + 3);
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_MAXX * 2);
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_MAXY;

  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER;
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;

  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;

  LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);

  LTDC_ReloadConfig(LTDC_IMReload);

  LTDC_LayerCmd(LTDC_Layer1, ENABLE);
  LTDC_LayerCmd(LTDC_Layer2, ENABLE);

  LTDC_ReloadConfig(LTDC_IMReload);

  LTDC_DitherCmd(ENABLE);

  LTDC_Cmd(ENABLE);
}


//--------------------------------------------------------------
// Hintergrund-Layer aktivieren
//--------------------------------------------------------------
void UB_LCD_SetLayer_1(void)
{
  LCD_CurrentFrameBuffer = LCD_FRAME_BUFFER;
  LCD_CurrentLayer = 0;
}


//--------------------------------------------------------------
// Vordergrund-Layer aktivieren
//--------------------------------------------------------------
void UB_LCD_SetLayer_2(void)
{
  LCD_CurrentFrameBuffer = LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  LCD_CurrentLayer = 1;
}


//--------------------------------------------------------------
// Menu-Layer aktivieren
//--------------------------------------------------------------
void UB_LCD_SetLayer_Menu(void)
{
  LCD_CurrentFrameBuffer = LCD_MenuFrameBuffer;
  // CurrentLayer nicht verändern
}


//--------------------------------------------------------------
// ADC-Layer aktivieren
//--------------------------------------------------------------
void UB_LCD_SetLayer_ADC(void)
{
  LCD_CurrentFrameBuffer = LCD_ADCFrameBuffer;
  // CurrentLayer nicht verändern
}


//--------------------------------------------------------------
// alten Layer wieder aktivieren
//--------------------------------------------------------------
void UB_LCD_SetLayer_Back(void)
{
  if(LCD_CurrentLayer==0) {
    LCD_CurrentFrameBuffer = LCD_FRAME_BUFFER;
  }
  else {
    LCD_CurrentFrameBuffer = LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  }
}


//--------------------------------------------------------------
// Füllt den aktuellen Layer komplett mit einer Farbe
//--------------------------------------------------------------
void UB_LCD_FillLayer(uint16_t color)
{
  uint32_t index = 0;

  // Bildschirm loeschen
  for (index = 0x00; index < LCD_FRAME_OFFSET; index+=2) {
    *(volatile uint16_t*)(LCD_CurrentFrameBuffer + index) = color;
  }
}


//--------------------------------------------------------------
// setzt Transparenz Wert vom aktuellen Layer
// wert : [0...255] 0=durchsichtig ... 255=solid
//--------------------------------------------------------------
void UB_LCD_SetTransparency(uint8_t wert)
{
  if (LCD_CurrentLayer == 0) {
    LTDC_LayerAlpha(LTDC_Layer1, wert);
  }
  else {
    LTDC_LayerAlpha(LTDC_Layer2, wert);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}


//--------------------------------------------------------------
// setzt den Cursor auf x,y Position
//--------------------------------------------------------------
void UB_LCD_SetCursor2Draw(uint16_t xpos, uint16_t ypos)
{
  aktCursorX=xpos;
  aktCursorY=ypos;

  aktCursorPos=LCD_CurrentFrameBuffer+(2*((aktCursorY*LCD_MAXX)+aktCursorX));
}


//--------------------------------------------------------------
// zeichnet ein Pixel an aktueller XY-Position
// und incrementiert Cursor
//--------------------------------------------------------------
void UB_LCD_DrawPixel(uint16_t color)
{
  *(volatile uint16_t*)(aktCursorPos)=color;
  if(LCD_DISPLAY_MODE==PORTRAIT) {
    aktCursorX++;
    if(aktCursorX>=LCD_MAXX) {
      aktCursorX=0;
      aktCursorY++;
      if(aktCursorY>=LCD_MAXY) aktCursorY=0;
    }
  }
  else {
    aktCursorY++;
    if(aktCursorY>=LCD_MAXY) {
      aktCursorY=0;
      aktCursorX++;
      if(aktCursorX>=LCD_MAXX) aktCursorX=0;
    }
  }
  aktCursorPos=LCD_CurrentFrameBuffer+(2*((aktCursorY*LCD_MAXX)+aktCursorX));
}


//--------------------------------------------------------------
// Screen-Mode einstellen
// muss direkt nach dem Init gemacht werden
//
// Mode : [PORTRAIT=default, LANDSCAPE]
//--------------------------------------------------------------
void UB_LCD_SetMode(LCD_MODE_t mode)
{
  if(mode==PORTRAIT) {
    LCD_DISPLAY_MODE=PORTRAIT;
  }
  else {
    LCD_DISPLAY_MODE=LANDSCAPE;
  }
}


//--------------------------------------------------------------
// Screen-Rotation einstellen auf 0 Grad
//--------------------------------------------------------------
void UB_LCD_Rotate_0(void)
{
  P_LCD9341_CMD(LCD_MAC);
  P_LCD9341_DATA(0xC8);
  LCD_CurrentOrientation=0;
}


//--------------------------------------------------------------
// Screen-Rotation einstellen auf 180 Grad
//--------------------------------------------------------------
void UB_LCD_Rotate_180(void)
{
  P_LCD9341_CMD(LCD_MAC);
  P_LCD9341_DATA(0x08);
  LCD_CurrentOrientation=1;
}


//--------------------------------------------------------------
// kopiert kompletten Inhalt von Layer1 in Layer2
// (Hintergrund --> Vordergrund)
//--------------------------------------------------------------
void UB_LCD_Copy_Layer1_to_Layer2(void)
{
  uint32_t index;
  uint32_t quelle = LCD_FRAME_BUFFER;
  uint32_t ziel = LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;

  for (index = 0 ; index < LCD_FRAME_OFFSET;index+=2) {
    *(volatile uint16_t*)(ziel + index) = *(volatile uint16_t*)(quelle + index);
  }
}


//--------------------------------------------------------------
// kopiert kompletten Inhalt von Layer2 in Layer1
// (Vordergrund --> Hintergrund)
//--------------------------------------------------------------
void UB_LCD_Copy_Layer2_to_Layer1(void)
{
  uint32_t index;
  uint32_t quelle = LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  uint32_t ziel = LCD_FRAME_BUFFER;

  for (index = 0 ; index < LCD_FRAME_OFFSET;index+=2) {
    *(volatile uint16_t*)(ziel + index) = *(volatile uint16_t*)(quelle + index);
  }
}


//--------------------------------------------------------------
// wechselt den aktiven Layer zum zeichnen
// und zeigt den jeweils anderen Layer an
//--------------------------------------------------------------
void UB_LCD_Refresh(void)
{
  if(LCD_CurrentLayer==0) {
    UB_LCD_SetLayer_2();
    UB_LCD_SetTransparency(0);
  }
  else {
    UB_LCD_SetTransparency(255);
    UB_LCD_SetLayer_1();
  }
}


//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins fuer das Display
//--------------------------------------------------------------
void P_LCD9341_InitIO(void)
{	
  GPIO_InitTypeDef       GPIO_InitStructure;

  // WRX-Pin
  RCC_AHB1PeriphClockCmd(LCD_WRX_GPIO_CLK, ENABLE);
	  
  GPIO_InitStructure.GPIO_Pin = LCD_WRX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

  // ChipSelect-Pin
  RCC_AHB1PeriphClockCmd(LCD_CS_GPIO_CLK, ENABLE);
	  
  GPIO_InitStructure.GPIO_Pin = LCD_CS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStructure);	

  P_LCD9341_CS(Bit_SET);
}


//--------------------------------------------------------------
// interne Funktion
// Init vom LTCD fuer das Display
//--------------------------------------------------------------
void P_LCD9341_InitLTCD(void)
{
  LTDC_InitTypeDef       LTDC_InitStruct;

  // Clock enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE);
	  
  P_LCD9341_AFConfig();	  

  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

  LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;

  //---------------------------------------
  // PLLSAI einstellen (auf 6MHz)
  //---------------------------------------	  
  RCC_PLLSAIConfig(192, 7, 4);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);

  RCC_PLLSAICmd(ENABLE);
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET);

  //---------------------------------------
  // Timer konfig
  // HSync = 10       VSync = 2
  // HBP   = 20       VBP   = 2
  // HFP   = 10       VFP   = 4
  // 
  // Portrait  : W=240, H=320
  //---------------------------------------
  LTDC_InitStruct.LTDC_HorizontalSync = 9;       // HSync-1
  LTDC_InitStruct.LTDC_VerticalSync = 1;         // VSync-1
  LTDC_InitStruct.LTDC_AccumulatedHBP = 29;      // HSync+HBP-1
  LTDC_InitStruct.LTDC_AccumulatedVBP = 3;       // VSync+VBP-1
  LTDC_InitStruct.LTDC_AccumulatedActiveW = 269; // HSync+HBP+W-1
  LTDC_InitStruct.LTDC_AccumulatedActiveH = 323; // VSync+VBP+H-1
  LTDC_InitStruct.LTDC_TotalWidth = 279;         // HSync+HBP+W+HFP-1
  LTDC_InitStruct.LTDC_TotalHeigh = 327;         // VSync+VBP+H+VFP-1

  LTDC_Init(&LTDC_InitStruct);
}


//--------------------------------------------------------------
// interne Funktion
// ChipSelect-Pin schalten
// wert [Bit_SET, Bit_RESET]
//--------------------------------------------------------------
void P_LCD9341_CS(BitAction wert)
{
  if (wert == Bit_RESET) {
    LCD_CS_GPIO_PORT->BSRRH = LCD_CS_PIN;
  }
  else {
    LCD_CS_GPIO_PORT->BSRRL = LCD_CS_PIN;
  }
}


//--------------------------------------------------------------
// interne Funktion
// WRX-Pin schalten
// wert [Bit_SET, Bit_RESET]
//--------------------------------------------------------------
void P_LCD9341_WRX(BitAction wert)
{
  if (wert == Bit_RESET) {
    LCD_WRX_GPIO_PORT->BSRRH = LCD_WRX_PIN;
  }
  else {
    LCD_WRX_GPIO_PORT->BSRRL = LCD_WRX_PIN;
  }
}


//--------------------------------------------------------------
// interne Funktion
// initialisiert den ILI9341-Controller im Display
//--------------------------------------------------------------
void P_LCD9341_InitChip(void)
{
  P_LCD9341_CMD(0xCA);
  P_LCD9341_DATA(0xC3);
  P_LCD9341_DATA(0x08);
  P_LCD9341_DATA(0x50);
  P_LCD9341_CMD(LCD_POWERB);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0xC1);
  P_LCD9341_DATA(0x30);
  P_LCD9341_CMD(LCD_POWER_SEQ);
  P_LCD9341_DATA(0x64);
  P_LCD9341_DATA(0x03);
  P_LCD9341_DATA(0x12);
  P_LCD9341_DATA(0x81);
  P_LCD9341_CMD(LCD_DTCA);
  P_LCD9341_DATA(0x85);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x78);
  P_LCD9341_CMD(LCD_POWERA);
  P_LCD9341_DATA(0x39);
  P_LCD9341_DATA(0x2C);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x34);
  P_LCD9341_DATA(0x02);
  P_LCD9341_CMD(LCD_PRC);
  P_LCD9341_DATA(0x20);
  P_LCD9341_CMD(LCD_DTCB);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x00);
  P_LCD9341_CMD(LCD_FRC);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x1B);
  P_LCD9341_CMD(LCD_DFC);
  P_LCD9341_DATA(0x0A);
  P_LCD9341_DATA(0xA2);
  P_LCD9341_CMD(LCD_POWER1);
  P_LCD9341_DATA(0x10);
  P_LCD9341_CMD(LCD_POWER2);
  P_LCD9341_DATA(0x10);
  P_LCD9341_CMD(LCD_VCOM1);
  P_LCD9341_DATA(0x45);
  P_LCD9341_DATA(0x15);
  P_LCD9341_CMD(LCD_VCOM2);
  P_LCD9341_DATA(0x90);
  P_LCD9341_CMD(LCD_MAC);
  P_LCD9341_DATA(0xC8);
  P_LCD9341_CMD(LCD_3GAMMA_EN);
  P_LCD9341_DATA(0x00);
  P_LCD9341_CMD(LCD_RGB_INTERFACE);
  P_LCD9341_DATA(0xC2);
  P_LCD9341_CMD(LCD_DFC);
  P_LCD9341_DATA(0x0A);
  P_LCD9341_DATA(0xA7);
  P_LCD9341_DATA(0x27);
  P_LCD9341_DATA(0x04);

  P_LCD9341_CMD(LCD_COLUMN_ADDR);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0xEF);
  
  P_LCD9341_CMD(LCD_PAGE_ADDR);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x01);
  P_LCD9341_DATA(0x3F);
  P_LCD9341_CMD(LCD_INTERFACE);
  P_LCD9341_DATA(0x01);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x06);

  P_LCD9341_CMD(LCD_GRAM);
  P_LCD9341_Delay(LCD_INIT_PAUSE);

  P_LCD9341_CMD(LCD_GAMMA);
  P_LCD9341_DATA(0x01);

  P_LCD9341_CMD(LCD_PGAMMA);
  P_LCD9341_DATA(0x0F);
  P_LCD9341_DATA(0x29);
  P_LCD9341_DATA(0x24);
  P_LCD9341_DATA(0x0C);
  P_LCD9341_DATA(0x0E);
  P_LCD9341_DATA(0x09);
  P_LCD9341_DATA(0x4E);
  P_LCD9341_DATA(0x78);
  P_LCD9341_DATA(0x3C);
  P_LCD9341_DATA(0x09);
  P_LCD9341_DATA(0x13);
  P_LCD9341_DATA(0x05);
  P_LCD9341_DATA(0x17);
  P_LCD9341_DATA(0x11);
  P_LCD9341_DATA(0x00);
  P_LCD9341_CMD(LCD_NGAMMA);
  P_LCD9341_DATA(0x00);
  P_LCD9341_DATA(0x16);
  P_LCD9341_DATA(0x1B);
  P_LCD9341_DATA(0x04);
  P_LCD9341_DATA(0x11);
  P_LCD9341_DATA(0x07);
  P_LCD9341_DATA(0x31);
  P_LCD9341_DATA(0x33);
  P_LCD9341_DATA(0x42);
  P_LCD9341_DATA(0x05);
  P_LCD9341_DATA(0x0C);
  P_LCD9341_DATA(0x0A);
  P_LCD9341_DATA(0x28);
  P_LCD9341_DATA(0x2F);
  P_LCD9341_DATA(0x0F);

  P_LCD9341_CMD(LCD_SLEEP_OUT);
  P_LCD9341_Delay(LCD_INIT_PAUSE);
  P_LCD9341_CMD(LCD_DISPLAY_ON);
  
  P_LCD9341_CMD(LCD_GRAM);
}


//--------------------------------------------------------------
// interne Funktion
// Kommando per SPI an Display senden
//--------------------------------------------------------------
void P_LCD9341_CMD(uint8_t wert)
{
  // Kommando
  P_LCD9341_WRX(Bit_RESET);

  // ChipSelect auf Lo
  P_LCD9341_CS(Bit_RESET);

  // Wert senden
  UB_SPI5_SendByte(wert);


  // kleine Pause
  P_LCD9341_Delay(LCD_SPI_PAUSE);
 
  // ChipSelect auf Hi
  P_LCD9341_CS(Bit_SET);
}


//--------------------------------------------------------------
// interne Funktion
// Daten per SPI an Display senden
//--------------------------------------------------------------
void P_LCD9341_DATA(uint8_t wert)
{
  // Data
  P_LCD9341_WRX(Bit_SET);

  // ChipSelect auf Lo
  P_LCD9341_CS(Bit_RESET);

  // Wert senden
  UB_SPI5_SendByte(wert);
 
  // kleine Pause
  P_LCD9341_Delay(LCD_SPI_PAUSE);

  // ChipSelect auf Hi
  P_LCD9341_CS(Bit_SET);
}


//--------------------------------------------------------------
// interne Funktion
// alle AF-Funktionen
//--------------------------------------------------------------
void P_LCD9341_AFConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | \
                         RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | \
                         RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);



  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | \
                             GPIO_Pin_11 | GPIO_Pin_12;

  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, 0x09);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | \
                             GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;

  GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10;

  GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6;

  GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_LTDC);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;

  GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, 0x09);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, 0x09);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 | \
                             GPIO_Pin_11 | GPIO_Pin_12;

  GPIO_Init(GPIOG, &GPIO_InitStruct);

}


//--------------------------------------------------------------
// interne Funktion
// kleine Pause
//-------------------------------------------------------------- 
void P_LCD9341_Delay(volatile uint32_t nCount)
{
  volatile uint32_t index = 0;

  for(index = nCount; index != 0; index--);
}
