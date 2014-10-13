//--------------------------------------------------------------
// File     : stm32_ub_graphic2d.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_GRAPHIC2D_H
#define __STM32F4_UB_GRAPHIC2D_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32_ub_lcd_ili9341.h"
#include "stm32f4xx_dma2d.h"

typedef enum {
  LCD_DIR_HORIZONTAL =0,
  LCD_DIR_VERTICAL
}LCD_DIR_t;



//--------------------------------------------------------------
// Struktur von einem Image
//--------------------------------------------------------------
typedef struct UB_Image_t
{
  const uint16_t *table; // Tabelle mit den Daten
  uint16_t width;        // Breite des Bildes (in Pixel)
  uint16_t height;       // Hoehe des Bildes  (in Pixel)
}UB_Image;


//--------------------------------------------------------------
// Einbinden der benutzen Bilder
// fuer alle Bilder muss das entsprechende File
// auch in der CooCox-IDE hinzugefügt werden
//--------------------------------------------------------------
extern UB_Image GUI1;
extern UB_Image GUI2;


//--------------------------------------------------------------
// Struktur von einer Kopie-Koordinate
//--------------------------------------------------------------
typedef struct DMA2D_Koord_t {
  uint32_t source_xp; // Quelle X-Start
  uint32_t source_yp; // Quelle Y-Start
  uint32_t source_w;  // Quelle Breite
  uint32_t source_h;  // Quelle Höhe
  uint32_t dest_xp;   // Ziel X-Start
  uint32_t dest_yp;   // Ziel Y-Start
}DMA2D_Koord;


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Graphic2D_ClearSreenDMA(uint16_t c);
void UB_Graphic2D_DrawPixelNormal(uint16_t xp, uint16_t yp, uint16_t c);
void UB_Graphic2D_DrawLineNormal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void UB_Graphic2D_DrawStraightDMA(uint16_t xp, uint16_t yp, uint16_t l, LCD_DIR_t d, uint16_t c);
void UB_Graphic2D_DrawRectDMA(uint16_t xp, uint16_t yp, uint16_t w, uint16_t h, uint16_t c);
void UB_Graphic2D_DrawFullRectDMA(uint16_t xp, uint16_t yp, uint16_t w, uint16_t h, uint16_t c);
void UB_Graphic2D_DrawCircleNormal(uint16_t xp, uint16_t yp, uint16_t r, uint16_t c);
void UB_Graphic2D_DrawFullCircleDMA(uint16_t xp, uint16_t yp, uint16_t r, uint16_t c);
void UB_Graphic2D_CopyImgDMA(UB_Image *img, DMA2D_Koord koord);
void UB_Graphic2D_Copy1DMA(void);
void UB_Graphic2D_Copy2DMA(uint32_t tranzparenz);







//--------------------------------------------------------------
#endif // __STM32F4_UB_GRAPHIC2D_H
