//--------------------------------------------------------------
// File     : stm32_ub_font.c
// Datum    : 25.10.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de 
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : STM32_UB_LCD_ILI9341
// Funktion : Text-LCD Funktionen
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_font.h"
#include "stm32_ub_lcd_ili9341.h"



//--------------------------------------------------------------
// Zeichnet ein Ascii-Zeichen eines Fonts an x,y Position
// mit Vorder- und Hintergrundfarbe (Font = max 16 Pixel breite)
// -> Font muss mit &-Operator uebergeben werden
//--------------------------------------------------------------
void UB_Font_DrawChar(uint16_t x, uint16_t y, uint8_t ascii, UB_Font *font, uint16_t vg, uint16_t bg)
{
  uint16_t xn,yn,start_maske,maske;
  const uint16_t *wert;
  

  ascii -= 32;
  wert=&font->table[ascii * font->height];

  if(LCD_DISPLAY_MODE==PORTRAIT) {
    start_maske=0x80;
    if(font->width>8) start_maske=0x8000;
    
    for(yn = 0; yn < font->height; yn++) {
      maske=start_maske;
      // Cursor setzen
      UB_LCD_SetCursor2Draw(x,yn+y);
      for(xn = 0; xn < font->width; xn++) {
        if((wert[yn] & maske) == 0x00) {
          // Pixel in Hintergrundfarbe zeichnen
          UB_LCD_DrawPixel(bg);
        }
        else {
          // Pixel in Vordergrundfarbe zeichnen
          UB_LCD_DrawPixel(vg);
        }
        maske=(maske>>1);
      }
    }
  }
  else {
    start_maske=0x80;
    if(font->width>8) start_maske=0x8000;

    for(yn = 0; yn < font->height; yn++) {
      maske=start_maske;
      // Cursor setzen
      UB_LCD_SetCursor2Draw(x+font->height-yn,y);
      for(xn = 0; xn < font->width; xn++) {
        if((wert[yn] & maske) == 0x00) {
          // Pixel in Hintergrundfarbe zeichnen
          UB_LCD_DrawPixel(bg);
        }
        else {
          // Pixel in Vordergrundfarbe zeichnen
          UB_LCD_DrawPixel(vg);
        }
        maske=(maske>>1);
      }
    }
  }
}


//--------------------------------------------------------------
// Zeichnet einen String eines Fonts an x,y Position
// mit Vorder- und Hintergrundfarbe (Font = max 16 Pixel breite)
// -> Font muss mit &-Operator uebergeben werden
//--------------------------------------------------------------
void UB_Font_DrawString(uint16_t x, uint16_t y,char *ptr, UB_Font *font, uint16_t vg, uint16_t bg)
{
  uint16_t pos;

  if(LCD_DISPLAY_MODE==PORTRAIT) {
    pos=x;
    while (*ptr != 0) {
      UB_Font_DrawChar(pos,y,*ptr,font,vg,bg);
      pos+=font->width;
      ptr++;
    }
  }
  else {
    pos=y;
    while (*ptr != 0) {
      UB_Font_DrawChar(x,pos,*ptr,font,vg,bg);
      pos+=font->width;
      ptr++;
    }
  }
}


//--------------------------------------------------------------
// Zeichnet ein Ascii-Zeichen eines Fonts an x,y Position
// mit Vorder- und Hintergrundfarbe (Font = max 32 Pixel breite)
// -> Font muss mit &-Operator uebergeben werden
//--------------------------------------------------------------
void UB_Font_DrawChar32(uint16_t x, uint16_t y, uint8_t ascii, UB_Font32 *font, uint16_t vg, uint16_t bg)
{
  uint16_t xn,yn;
  uint32_t start_maske,maske;
  const uint32_t *wert;


  ascii -= 32;
  wert=&font->table[ascii * font->height];

  if(LCD_DISPLAY_MODE==PORTRAIT) {
    start_maske=0x80;
    if(font->width>8) start_maske=0x8000;
    if(font->width>16) start_maske=0x80000000;

    for(yn = 0; yn < font->height; yn++) {
      maske=start_maske;
      // Cursor setzen
      UB_LCD_SetCursor2Draw(x,yn+y);
      for(xn = 0; xn < font->width; xn++) {
        if((wert[yn] & maske) == 0x00) {
          // Pixel in Hintergrundfarbe zeichnen
          UB_LCD_DrawPixel(bg);
        }
        else {
          // Pixel in Vordergrundfarbe zeichnen
          UB_LCD_DrawPixel(vg);
        }
        maske=(maske>>1);
      }
    }
  }
  else {
    start_maske=0x80;
    if(font->width>8) start_maske=0x8000;
    if(font->width>16) start_maske=0x80000000;

    for(yn = 0; yn < font->height; yn++) {
      maske=start_maske;
      // Cursor setzen
      UB_LCD_SetCursor2Draw(x+font->height-yn,y);
      for(xn = 0; xn < font->width; xn++) {
        if((wert[yn] & maske) == 0x00) {
          // Pixel in Hintergrundfarbe zeichnen
          UB_LCD_DrawPixel(bg);
        }
        else {
          // Pixel in Vordergrundfarbe zeichnen
          UB_LCD_DrawPixel(vg);
        }
        maske=(maske>>1);
      }
    }
  }
}


//--------------------------------------------------------------
// Zeichnet einen String eines Fonts an x,y Position
// mit Vorder- und Hintergrundfarbe (Font = max 32 Pixel breite)
// -> Font muss mit &-Operator uebergeben werden
//--------------------------------------------------------------
void UB_Font_DrawString32(uint16_t x, uint16_t y,char *ptr, UB_Font32 *font, uint16_t vg, uint16_t bg)
{
  uint16_t pos;

  if(LCD_DISPLAY_MODE==PORTRAIT) {
    pos=x;
    while (*ptr != 0) {
      UB_Font_DrawChar32(pos,y,*ptr,font,vg,bg);
      pos+=font->width;
      ptr++;
    }
  }
  else {
    pos=y;
    while (*ptr != 0) {
      UB_Font_DrawChar32(x,pos,*ptr,font,vg,bg);
      pos+=font->width;
      ptr++;
    }
  }
}
