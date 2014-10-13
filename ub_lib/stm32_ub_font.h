//--------------------------------------------------------------
// File     : stm32_ub_Font.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_FONT_H
#define __STM32F4_UB_FONT_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"



//--------------------------------------------------------------
// Struktur von einem Font (max 16 Pixel Breite)
//-------------------------------------------------------------- 
typedef struct UB_Font_t {
  const uint16_t *table; // Tabelle mit den Daten
  uint16_t width;        // Breite eines Zeichens (in Pixel)
  uint16_t height;       // Hoehe eines Zeichens  (in Pixel)
}UB_Font;


//--------------------------------------------------------------
// Struktur von einem Font (max 32 Pixel Breite)
//--------------------------------------------------------------
typedef struct UB_Font32_t {
  const uint32_t *table; // Tabelle mit den Daten
  uint16_t width;        // Breite eines Zeichens (in Pixel)
  uint16_t height;       // Hoehe eines Zeichens  (in Pixel)
}UB_Font32;



//--------------------------------------------------------------
// Aktivierung der verschiedenen Fonts
// (unbenutzte einfach auskommentieren)
// fuer alle benutzten Fonts muss das entsprechende C-File
// in der CooCox-IDE hinzugefügt werden
//--------------------------------------------------------------
extern UB_Font Arial_7x10;
extern UB_Font Arial_8x13;
extern UB_Font Arial_10x15;
extern UB_Font Arial_11x18;
extern UB_Font Arial_13x19;
extern UB_Font Arial_14x22;
extern UB_Font Arial_16x25;
extern UB_Font32 Arial_18x27;



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_Font_DrawChar(uint16_t x, uint16_t y, uint8_t ascii, UB_Font *font, uint16_t vg, uint16_t bg);
void UB_Font_DrawString(uint16_t x, uint16_t y,char *ptr, UB_Font *font, uint16_t vg, uint16_t bg);
void UB_Font_DrawChar32(uint16_t x, uint16_t y, uint8_t ascii, UB_Font32 *font, uint16_t vg, uint16_t bg);
void UB_Font_DrawString32(uint16_t x, uint16_t y,char *ptr, UB_Font32 *font, uint16_t vg, uint16_t bg);


//--------------------------------------------------------------
#endif // __STM32F4_UB_FONT_H
