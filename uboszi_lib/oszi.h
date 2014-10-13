//--------------------------------------------------------------
// File     : oszi.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_OSZI_H
#define __STM32F4_UB_OSZI_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32_ub_led.h"
#include "stm32_ub_lcd_ili9341.h"
#include "stm32_ub_graphic2d.h"
#include "stm32_ub_font.h"
#include "stm32_ub_touch_stmpe811.h"
#include "stm32_ub_systick.h"
#include "stm32_ub_uart.h"
#include "stm32_ub_button.h"
#include "adc.h"
#include "menu.h"
#include "fft.h"
#include <stdio.h>




//--------------------------------------------------------------
// Definitionen der Oszi-Scala
// (nicht abändern !!)
//--------------------------------------------------------------
#define  SCALE_START_X      13   // dont change
#define  SCALE_START_Y      9    // dont change
#define  FFT_START_Y        31   // dont change
#define  FFT_START_X        20   // dont change

#define  SCALE_SPACE        25   // dont change
#define  SCALE_ANZ_Y        9    // dont change
#define  SCALE_ANZ_X        13   // dont change

#define  SCALE_W            SCALE_SPACE*(SCALE_ANZ_X-1)
#define  SCALE_H            SCALE_SPACE*(SCALE_ANZ_Y-1)

#define  SCALE_MX_PIXEL     SCALE_START_X+SCALE_H
#define  SCALE_MY_PIXEL     SCALE_START_Y+SCALE_W

#define  SCALE_X_MITTE      SCALE_W/2
#define  SCALE_Y_MITTE      SCALE_H/2




//--------------------------------------------------------------
// Defines zum Umrechnen : ADC-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_5V          SCALE_SPACE/6825
#define  FAKTOR_2V          SCALE_SPACE/2730
#define  FAKTOR_1V          SCALE_SPACE/1365
#define  FAKTOR_0V5         SCALE_SPACE/682.5
#define  FAKTOR_0V2         SCALE_SPACE/273
#define  FAKTOR_0V1         SCALE_SPACE/136.5


//--------------------------------------------------------------
// Defines zum Umrechnen : Zeit-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_T           SCALE_W/4095



//--------------------------------------------------------------
// Defines zum Umrechnen : FFT-Wert -> Pixel-Wert
//--------------------------------------------------------------
#define  FAKTOR_F           FFT_VISIBLE_LENGTH/4095


//--------------------------------------------------------------
// Aktuallisierungs-Intervall der GUI
//--------------------------------------------------------------
#define  GUI_INTERVALL_MS     50   // interall der GUI (in ms)


//--------------------------------------------------------------
// color defines
//--------------------------------------------------------------
#define  BACKGROUND_COL     RGB_COL_BLACK  // Farbe vom Hintergrund
#define  FONT_COL           RGB_COL_BLUE   // (debug Schriftfarbe)
#define  ADC_CH1_COL        RGB_COL_CYAN   // Farbe von Kanal-1
#define  ADC_CH2_COL        RGB_COL_YELLOW // Faebe von Kanal-2
#define  SCALE_COL          RGB_COL_GREY   // Farbe vom Oszi-Gitter
#define  CURSOR_COL         RGB_COL_GREEN  // Farbe vom Cursor
#define  FFT_COL            RGB_COL_MAGENTA  // Farbe der FFT


//--------------------------------------------------------------
#define  BMP_HEADER_LEN   54 // Header = 54 Bytes (dont change)


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void oszi_init(void);
void oszi_start(void);



//--------------------------------------------------------------
#endif // __STM32F4_UB_OSZI_H
