//--------------------------------------------------------------
// File     : stm32_ub_oszi.c
// Datum    : 24.03.2014
// Version  : 1.6
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : keine
// Funktion : Oszilloskop
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "oszi.h"


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
uint32_t p_oszi_hw_init(void);
void p_oszi_sw_init(void);
void p_oszi_clear_all(void);
void p_oszi_draw_background(void);
void p_oszi_draw_scale(void);
void p_oszi_draw_line_h(uint16_t xp, uint16_t c, uint16_t m);
void p_oszi_draw_line_v(uint16_t yp, uint16_t c, uint16_t m);
void p_oszi_sort_adc(void);
void p_oszi_fill_fft(void);
void p_oszi_draw_adc(void);
int16_t oszi_adc2pixel(uint16_t adc, uint32_t faktor);
void p_oszi_send_data(void);
void p_oszi_send_uart(char *ptr);
void p_oszi_send_screen(void);


//--------------------------------------------------------------
// Header fuer BMP-Transfer
// (fix als einen kompletten Screen (320x240) im Landscape-Mode)
//--------------------------------------------------------------
uint8_t BMP_HEADER[BMP_HEADER_LEN]={
0x42,0x4D,0x36,0x84,0x03,0x00,0x00,0x00,0x00,0x00, // ID=BM, Filsize=(240x320x3+54)
0x36,0x00,0x00,0x00,0x28,0x00,0x00,0x00,           // Offset=54d, Headerlen=40d
0x40,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x00, // W=320d, H=240d (landscape)
0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x84,0x03,0x00, // 24bpp, unkomprimiert, Data=(240x320x3)
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,           // nc
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};          // nc



//--------------------------------------------------------------
// init vom Oszi
//--------------------------------------------------------------
void oszi_init(void)
{
  uint32_t check;

  //---------------------------------------------
  // Hardware init
  //--------------------------------------------- 
  check=p_oszi_hw_init();
  p_oszi_send_uart("OSZI 4 STM32F429 [UB]");
  if(check==1) {
    // Touch init error
    UB_LCD_FillLayer(BACKGROUND_COL);
    UB_Font_DrawString(10,10,"Touch ERR",&Arial_7x10,FONT_COL,BACKGROUND_COL);
    while(1);
  }
  else if(check==2) {
    // Fehler in den Defines
    UB_LCD_FillLayer(BACKGROUND_COL);
    UB_Font_DrawString(10,10,"Wrong ADC Array-LEN",&Arial_7x10,FONT_COL,BACKGROUND_COL);
    while(1);
  }


  //---------------------------------------------
  // FFT init
  //---------------------------------------------
  fft_init();

  //---------------------------------------------
  // Software init
  //---------------------------------------------
  p_oszi_sw_init();
  ADC_change_Frq(Menu.timebase.value);
}


//--------------------------------------------------------------
// start vom Oszi (Endlosloop)
//--------------------------------------------------------------
void oszi_start(void)
{
  MENU_Status_t status;

  p_oszi_draw_background();
  UB_Graphic2D_Copy2DMA(Menu.akt_transparenz);  

  while(1) {
    //---------------------------------------------
    // warten bis GUI-Timer abgelaufen ist
    //---------------------------------------------
    if(GUI_Timer_ms==0) {
      GUI_Timer_ms=GUI_INTERVALL_MS;
      //--------------------------------------
      // User-Button einlesen (fuer RUN/STOP)
      //--------------------------------------
      if(UB_Button_OnClick(BTN_USER)==true) {
        status=MENU_NO_CHANGE;
        if(Menu.trigger.mode==2) { // "single"
          if(Menu.trigger.single==4) {
            Menu.trigger.single=5;  // von "Ready" auf "Stop"
            status=MENU_CHANGE_NORMAL;
          }
        }
        else { // "normal" oder "auto"
          if(Menu.trigger.single==0) {
            Menu.trigger.single=1; // von "Run" auf "Stop"
            status=MENU_CHANGE_NORMAL;
          }
          else if(Menu.trigger.single==1) {
            Menu.trigger.single=2; // von "Stop" auf "Weiter"
            status=MENU_CHANGE_NORMAL;
          }
        }
      }
      else {
        //--------------------------------------
        // Test ob Touch betaetigt
        //--------------------------------------
        status=menu_check_touch();
      }
      if(status!=MENU_NO_CHANGE) {
        p_oszi_draw_background();
        if(status==MENU_CHANGE_FRQ) ADC_change_Frq(Menu.timebase.value);
        if(status==MENU_CHANGE_MODE) {
          ADC_change_Mode(Menu.trigger.mode);
          if(Menu.trigger.mode!=2) {
            Menu.trigger.single=0;
          }
          else {
            Menu.trigger.single=3;
          }
          p_oszi_draw_background(); // nochmal zeichnen, zum update
          ADC_UB.status=ADC_VORLAUF;
          TIM_Cmd(TIM2, ENABLE);
        }
        if(status==MENU_SEND_DATA) {
          p_oszi_draw_background(); // nochmal zeichnen, zum update
          p_oszi_draw_adc();
          // gesendet wird am Ende
        }
      }

      if(Menu.trigger.mode==1) {
        //--------------------------------------
        // Trigger-Mode = "AUTO"
        // Screnn immer neu zeichnen
        //--------------------------------------
        if(Menu.trigger.single==0) {
          if ((ADC1_DMA_STREAM->CR & DMA_SxCR_CT) == 0) {
            ADC_UB.trigger_pos=SCALE_X_MITTE;
            ADC_UB.trigger_quarter=2;
          }
          else {
            ADC_UB.trigger_pos=SCALE_X_MITTE;
            ADC_UB.trigger_quarter=4;
          }
          p_oszi_sort_adc();
          p_oszi_fill_fft();
          if(Menu.fft.mode!=0) fft_calc();
          p_oszi_draw_adc();
          ADC_UB.status=ADC_VORLAUF;
          UB_Led_Toggle(LED_RED);
        }
        else if(Menu.trigger.single==1) {
          // Button "STOP" wurde gedrückt
          // Timer analten
          TIM_Cmd(TIM2, DISABLE);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
        else if(Menu.trigger.single==2) {
          // Button "START" wurde gedrückt
          Menu.trigger.single=0;
          ADC_UB.status=ADC_VORLAUF;
          TIM_Cmd(TIM2, ENABLE);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }
      else if(Menu.trigger.mode==0) {
        //--------------------------------------
        // Trigger-Mode = "NORMAL"
        // Screnn nur zeichnen, nach Triggerevent
        //--------------------------------------
        if(Menu.trigger.single==0) {
          if(ADC_UB.status==ADC_READY) {
            UB_Led_Toggle(LED_RED);
            p_oszi_sort_adc();
            p_oszi_fill_fft();
            if(Menu.fft.mode!=0) fft_calc();
            p_oszi_draw_adc();
            ADC_UB.status=ADC_VORLAUF;
            TIM_Cmd(TIM2, ENABLE);
          }
          else {
            // oder wenn Menu verändert wurde
            if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
          }
        }
        else if(Menu.trigger.single==1) {
          // Button "STOP" wurde gedrückt
          // Timer analten
          TIM_Cmd(TIM2, DISABLE);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
        else if(Menu.trigger.single==2) {
          // Button "START" wurde gedrückt
          Menu.trigger.single=0;
          ADC_UB.status=ADC_VORLAUF;
          TIM_Cmd(TIM2, ENABLE);
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }
      else {
        //--------------------------------------
        // Trigger-Mode = "SINGLE"
        // Screnn nur einmal zeichnen, nach Triggerevent
        //--------------------------------------
        if(Menu.trigger.single==3) {
          // warten auf Trigger-Event
          if(ADC_UB.status==ADC_READY) {
            Menu.trigger.single=4;
            UB_Led_Toggle(LED_RED);
            p_oszi_sort_adc();
            p_oszi_fill_fft();
            if(Menu.fft.mode!=0) fft_calc();
            p_oszi_draw_adc();
          }
          else {
            // oder wenn Menu verändert wurde
            if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
          }
        }
        else if(Menu.trigger.single==5) {
          // Button "Reset" wurde gedrückt
          Menu.trigger.single=3;
          p_oszi_draw_adc();
          ADC_UB.status=ADC_VORLAUF;
          TIM_Cmd(TIM2, ENABLE);
        }
        else {
          // oder wenn Menu verändert wurde
          if(status!=MENU_NO_CHANGE) p_oszi_draw_adc();
        }
      }

      if(GUI.gui_xpos==GUI_XPOS_OFF) {
        // ohne GUI => ohne Transparenz zeichnen
        UB_Graphic2D_Copy1DMA();
      }
      else {
        // mit GUI => mit Transparenz zeichnen
        UB_Graphic2D_Copy2DMA(Menu.akt_transparenz);
      }

      // Refreh vom LCD
      UB_LCD_Refresh();

      // event. Daten senden
      if(Menu.send.data!=0) {
        p_oszi_send_data();
        Menu.send.data=0;
      }
    }
  }
}


//--------------------------------------------------------------
// init der Hardware
//--------------------------------------------------------------
uint32_t p_oszi_hw_init(void)
{
  uint32_t ret_wert=0;

  // init vom Touch
  if(UB_Touch_Init()!=SUCCESS) {
    ret_wert=1; // Touch error
  }

  // Check der Defines
  if(ADC_ARRAY_LEN!=SCALE_W) {
    ret_wert=2; // define error
  }

  // init vom Systick
  UB_Systick_Init();

  // init der LEDs
  UB_Led_Init();

  // init vom Button
  UB_Button_Init();

  // init der UART
  UB_Uart_Init();

  // init vom LCD (und SD-RAM)
  UB_LCD_Init();
  UB_LCD_LayerInit_Fullscreen();
  UB_LCD_SetMode(LANDSCAPE);

  // alle Puffer löschen
  p_oszi_clear_all();

  // init vom ADC
  ADC_Init_ALL();

  return(ret_wert);
}


//--------------------------------------------------------------
// init der Software
//--------------------------------------------------------------
void p_oszi_sw_init(void)
{
  //--------------------------------------
  // Default Einstellungen
  //--------------------------------------
  Menu.akt_transparenz=100;
  Menu.akt_setting=SETTING_TRIGGER;

  Menu.ch1.faktor=1;      // 2v/div
  Menu.ch1.visible=0;     // visible=true
  Menu.ch1.position=25;

  Menu.ch2.faktor=2;      // 1v/div
  Menu.ch2.visible=0;     // visible=true
  Menu.ch2.position=-75;

  Menu.timebase.value=9;  // 5ms/div

  Menu.trigger.source=0;  // trigger=CH1
  Menu.trigger.edge=0;    // hi-flanke
  Menu.trigger.mode=1;    // auto
  Menu.trigger.single=0;
  Menu.trigger.value_ch1=1024;
  Menu.trigger.value_ch2=2048;

  Menu.cursor.mode=0;     // cursor Off
  Menu.cursor.p1=2048;
  Menu.cursor.p2=3072;
  Menu.cursor.t1=1700;
  Menu.cursor.t2=2300;
  Menu.cursor.f1=1000;

  Menu.send.mode=0; // nur CH1
  Menu.send.screen=SETTING_TRIGGER;
  Menu.send.data=0;

  Menu.fft.mode=1; // FFT=CH1

  GUI.gui_xpos=GUI_XPOS_OFF;  // GUI ausgeblendet
  GUI.akt_menu=MM_NONE;
  GUI.old_menu=MM_CH1;
  GUI.akt_button=GUI_BTN_NONE;
  GUI.old_button=GUI_BTN_NONE;
}


//--------------------------------------------------------------
// löscht alle Speicher
//--------------------------------------------------------------
void p_oszi_clear_all(void)
{
  UB_LCD_SetLayer_2();
  UB_LCD_SetTransparency(255);
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_Copy_Layer2_to_Layer1();
  UB_LCD_SetLayer_Menu();
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_SetLayer_ADC();
  UB_LCD_FillLayer(BACKGROUND_COL);
  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// zeichnet den Hintergrund vom Oszi
// (Skala, Cursor, Menüs usw)
// Zieladresse im SD-RAM = [MENU]
//--------------------------------------------------------------
void p_oszi_draw_background(void)
{
  UB_LCD_SetLayer_Menu();
  UB_LCD_FillLayer(BACKGROUND_COL);

  // GUI zuerst zeichnen
  menu_draw_all();
  // dann Skala und Cursor zeichnen
  p_oszi_draw_scale();

  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// zeichnet die Skala und die Cursor vom Oszi
//--------------------------------------------------------------
void p_oszi_draw_scale(void)
{
  uint32_t n,m;
  uint16_t xs,ys;
  int16_t signed_int;

  xs=SCALE_START_X;
  ys=SCALE_START_Y;

  //---------------------------------------------
  // Raster aus einzelnen Punkten
  //---------------------------------------------
  for(m=0;m<=SCALE_H;m+=SCALE_SPACE) {
    for(n=0;n<=SCALE_W;n+=SCALE_SPACE) {
      UB_Graphic2D_DrawPixelNormal(m+xs,n+ys,SCALE_COL);
    }
  }

  //---------------------------------------------
  // X-Achse (Horizontale Mittel-Linie)
  //---------------------------------------------
  signed_int=SCALE_Y_MITTE+xs;
  p_oszi_draw_line_h(signed_int,SCALE_COL,0);

  //---------------------------------------------
  // Y-Achse (Vertikale Mittel-Linie)
  //---------------------------------------------
  signed_int=SCALE_X_MITTE+ys;
  p_oszi_draw_line_v(signed_int,SCALE_COL,0);
 
  //---------------------------------------------
  // Umrandung
  //---------------------------------------------
  // unterste linie
  UB_Graphic2D_DrawStraightDMA(xs-1,ys-1,SCALE_W+2,LCD_DIR_HORIZONTAL,SCALE_COL);
  // oberste linie
  UB_Graphic2D_DrawStraightDMA(xs+SCALE_H+1,ys-1,SCALE_W+2,LCD_DIR_HORIZONTAL,SCALE_COL);
  // linke linie
  UB_Graphic2D_DrawStraightDMA(xs-1,ys-1,SCALE_H+2,LCD_DIR_VERTICAL,SCALE_COL);
  // rechte linie
  UB_Graphic2D_DrawStraightDMA(xs-1,ys+SCALE_W+1,SCALE_H+2,LCD_DIR_VERTICAL,SCALE_COL);


  //---------------------------------------------
  // Trigger-Linie (immer Sichtbar)
  //---------------------------------------------
  if(Menu.trigger.source==0) {
    signed_int=oszi_adc2pixel(Menu.trigger.value_ch1, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,ADC_CH1_COL,1);
    UB_Font_DrawString(signed_int-3,0,"T",&Arial_7x10,ADC_CH1_COL,BACKGROUND_COL);
  }
  else if(Menu.trigger.source==1) {
    signed_int=oszi_adc2pixel(Menu.trigger.value_ch2, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,ADC_CH2_COL,1);
    UB_Font_DrawString(signed_int-3,0,"T",&Arial_7x10,ADC_CH2_COL,BACKGROUND_COL);
  }

  //---------------------------------------------
  // Cursor-Linien (nur falls aktiviert)
  //---------------------------------------------
  if(Menu.cursor.mode==1) {
    //------------------------------- 
    // Cursor (CH1)
    //-------------------------------
    signed_int=oszi_adc2pixel(Menu.cursor.p1, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=oszi_adc2pixel(Menu.cursor.p2, Menu.ch1.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==2) {
    //-------------------------------
    // Cursor (CH2)
    //-------------------------------
    signed_int=oszi_adc2pixel(Menu.cursor.p1, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=oszi_adc2pixel(Menu.cursor.p2, Menu.ch2.faktor);
    signed_int+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
    if(signed_int<SCALE_START_X) signed_int=SCALE_START_X;
    if(signed_int>SCALE_MX_PIXEL) signed_int=SCALE_MX_PIXEL;

    p_oszi_draw_line_h(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(signed_int-3,312,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==3) {
    //-------------------------------
    // Cursor (TIME)
    //-------------------------------
    signed_int=Menu.cursor.t1*FAKTOR_T;
    signed_int+=SCALE_START_Y;
    if(signed_int<SCALE_START_Y) signed_int=SCALE_START_Y;
    if(signed_int>SCALE_MY_PIXEL) signed_int=SCALE_MY_PIXEL;

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);

    signed_int=Menu.cursor.t2*FAKTOR_T;
    signed_int+=SCALE_START_Y;
    if(signed_int<SCALE_START_Y) signed_int=SCALE_START_Y;
    if(signed_int>SCALE_MY_PIXEL) signed_int=SCALE_MY_PIXEL;

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"B",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
  else if(Menu.cursor.mode==4) {
    //-------------------------------
    // Cursor (FFT)
    //-------------------------------
    signed_int=Menu.cursor.f1*FAKTOR_F;
    signed_int+=FFT_START_Y+1;
    if(signed_int<FFT_START_Y) signed_int=FFT_START_Y;
    if(signed_int>(FFT_START_Y+FFT_VISIBLE_LENGTH)) signed_int=(FFT_START_Y+FFT_VISIBLE_LENGTH);

    p_oszi_draw_line_v(signed_int,CURSOR_COL,2);
    UB_Font_DrawString(215,signed_int-3,"A",&Arial_7x10,CURSOR_COL,BACKGROUND_COL);
  }
}


//--------------------------------------------------------------
// zeichnet eine horizontale Line auf das Oszi-Gitter
// an "xp", mit Farbe "c" und Mode "m"
//--------------------------------------------------------------
void p_oszi_draw_line_h(uint16_t xp, uint16_t c, uint16_t m)
{
  uint32_t n,t;

  if(m==0) {
    // Linie : "X----X----X----X----X----X"
    for(n=0;n<=SCALE_W;n+=5) {
      UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,c);
    } 
  }
  else if(m==1) {
    // Linie : "X-X-X-X-X-X-X-X-X"
    for(n=0;n<=SCALE_W;n+=2) {
      UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,c);
    }
  }
  else if(m==2) {
    // Linie : "XX---XX---XX---XX---XX"
    t=0;
    for(n=0;n<=SCALE_W;n++) {
      if(t<2) UB_Graphic2D_DrawPixelNormal(xp,n+SCALE_START_Y,c);
      t++;
      if(t>4) t=0;
    }
  }
}


//--------------------------------------------------------------
// zeichnet eine vertikale Line auf das Oszi-Gitter
// an "yp", mit Farbe "c" und Mode "m"
//--------------------------------------------------------------
void p_oszi_draw_line_v(uint16_t yp, uint16_t c, uint16_t m)
{
  uint32_t n,t;

  if(m==0) {
    // Linie : "X----X----X----X----X----X"
    for(n=0;n<=SCALE_H;n+=5) {
      UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,c);
    } 
  }
  else if(m==1) {
    // Linie : "X-X-X-X-X-X-X-X-X"
    for(n=0;n<=SCALE_H;n+=2) {
      UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,c);
    }
  }
  else if(m==2) {
    // Linie : "XX---XX---XX---XX---XX"
    t=0;
    for(n=0;n<=SCALE_H;n++) {
      if(t<2) UB_Graphic2D_DrawPixelNormal(n+SCALE_START_X,yp,c);
      t++;
      if(t>4) t=0;
    }
  }
}


//--------------------------------------------------------------
// sortiert die Daten der ADC-Kanäle von Buffer_A und Buffer_B
// in den Buffer_C um
// die Daten werden so sortiert, das das Trigger-Event in der Mitte
// vom Datenbreich liegt (das ist später die Mitte vom Screen)
//
// Der Trigger-Punkt kann in einem der 4 Quadranten
// vom Daten-Puffer liegen
// Quadrant-1 = erste hälfte vom Buffer-A
// Quadrant-2 = zweite hälfte vom Buffer-A
// Quadrant-3 = erste hälfte vom Buffer-B
// Quadrant-4 = zweite hälfte vom Buffer-B
//--------------------------------------------------------------
void p_oszi_sort_adc(void)
{
  uint32_t n=0;
  uint32_t start=0,anz1=0,anz2=0;
  uint16_t wert;

  if(ADC_UB.trigger_quarter==1) {
    //-------------------------------
    // Trigger-Punkt liegt in Q1
    //-------------------------------
    anz1=(SCALE_X_MITTE-ADC_UB.trigger_pos);
    start=SCALE_W-anz1;

    //-------------------------------
    // linker Teil kopieren
    //-------------------------------
    for(n=0;n<anz1;n++) {
      wert=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=wert;
      wert=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=wert;
    }
    //-------------------------------
    // rechter Teil kopieren
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      wert=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=wert;
      wert=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=wert;
    }
  }
  else if(ADC_UB.trigger_quarter==2) {
    //-------------------------------
    // Trigger-Punkt liegt in Q2
    //-------------------------------
    anz1=SCALE_W-((ADC_UB.trigger_pos-SCALE_X_MITTE));
    start=SCALE_W-anz1;

    //-------------------------------
    // linker Teil kopieren
    //-------------------------------
    for(n=0;n<anz1;n++) {
      wert=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=wert;
      wert=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=wert;
    }
    //-------------------------------
    // rechter Teil kopieren
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      wert=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=wert;
      wert=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=wert;
    }
  }
  else if(ADC_UB.trigger_quarter==3) {
    //-------------------------------
    // Trigger-Punkt liegt in Q3
    //-------------------------------
    anz1=(SCALE_X_MITTE-ADC_UB.trigger_pos);
    start=SCALE_W-anz1;

    //-------------------------------
    // linker Teil kopieren
    //-------------------------------
    for(n=0;n<anz1;n++) {
      wert=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=wert;
      wert=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=wert;
    }
    //-------------------------------
    // rechter Teil kopieren
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      wert=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=wert;
      wert=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=wert;
    }
  }
  else if(ADC_UB.trigger_quarter==4) {
    //-------------------------------
    // Trigger-Punkt liegt in Q4
    //-------------------------------
    anz1=SCALE_W-((ADC_UB.trigger_pos-SCALE_X_MITTE));
    start=SCALE_W-anz1;

    //-------------------------------
    // linker Teil kopieren
    //-------------------------------
    for(n=0;n<anz1;n++) {
      wert=ADC_DMA_Buffer_B[(start+n)*2];
      ADC_DMA_Buffer_C[n*2]=wert;
      wert=ADC_DMA_Buffer_B[((start+n)*2)+1];
      ADC_DMA_Buffer_C[(n*2)+1]=wert;
    }
    //-------------------------------
    // rechter Teil kopieren
    //-------------------------------
    anz2=SCALE_W-anz1;
    start=0;
    for(n=0;n<anz2;n++) {
      wert=ADC_DMA_Buffer_A[(start+n)*2];
      ADC_DMA_Buffer_C[(n+anz1)*2]=wert;
      wert=ADC_DMA_Buffer_A[((start+n)*2)+1];
      ADC_DMA_Buffer_C[((n+anz1)*2)+1]=wert;
    }
  }
}


//--------------------------------------------------------------
// fuellt den FFT-Input-Puffer
// mit den Sample-Daten von CH1 oder CH2
// (rest mit 0 auffuellen)
//--------------------------------------------------------------
void p_oszi_fill_fft(void)
{
  uint32_t n,m;

  if(Menu.fft.mode==1) {
    m=0;
    for(n=0;n<FFT_LENGTH;n++) {
      if(m<SCALE_W) {
        FFT_DATA_IN[n]=(float32_t)((ADC_DMA_Buffer_C[(m*2)]-2048.0)/1000.0);
      }
      else {
        FFT_DATA_IN[n]=0.0;
      }
      m++;
    }
  }
  else if(Menu.fft.mode==2) {
    m=0;
    for(n=0;n<FFT_LENGTH;n++) {
      if(m<SCALE_W) {
        FFT_DATA_IN[n]=(float32_t)((ADC_DMA_Buffer_C[(m*2)+1]-2048.0)/1000.0);
      }
      else {
        FFT_DATA_IN[n]=0.0;
      }
      m++;
    }
  }
}


//--------------------------------------------------------------
// zeichnet die Daten der zwei ADC-Kanäle (und die FFT)
//--------------------------------------------------------------
void p_oszi_draw_adc(void)
{
  uint32_t n=0;
  int16_t ch1_wert1,ch1_wert2;
  int16_t ch2_wert1,ch2_wert2;
  int16_t fft_wert1,fft_wert2;

  p_oszi_draw_background();
  UB_LCD_SetLayer_Menu();

  // startwerte
  ch1_wert1=oszi_adc2pixel(ADC_DMA_Buffer_C[0], Menu.ch1.faktor);
  ch1_wert1+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
  if(ch1_wert1<SCALE_START_X) ch1_wert1=SCALE_START_X;
  if(ch1_wert1>SCALE_MX_PIXEL) ch1_wert1=SCALE_MX_PIXEL;

  ch2_wert1=oszi_adc2pixel(ADC_DMA_Buffer_C[1], Menu.ch2.faktor);
  ch2_wert1+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
  if(ch2_wert1<SCALE_START_X) ch2_wert1=SCALE_START_X;
  if(ch2_wert1>SCALE_MX_PIXEL) ch2_wert1=SCALE_MX_PIXEL;

  fft_wert1=FFT_UINT_DATA[0];
  fft_wert1+=FFT_START_X;
  if(fft_wert1<SCALE_START_X) fft_wert1=SCALE_START_X;
  if(fft_wert1>SCALE_MX_PIXEL) fft_wert1=SCALE_MX_PIXEL;

  // komplette Kurve
  for(n=1;n<SCALE_W;n++) {
    if(Menu.ch1.visible==0) {
      ch1_wert2=oszi_adc2pixel(ADC_DMA_Buffer_C[n*2], Menu.ch1.faktor);
      ch1_wert2+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch1.position;
      if(ch1_wert2<SCALE_START_X) ch1_wert2=SCALE_START_X;
      if(ch1_wert2>SCALE_MX_PIXEL) ch1_wert2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(ch1_wert1,SCALE_START_Y+n,ch1_wert2,SCALE_START_Y+n+1,ADC_CH1_COL);
      ch1_wert1=ch1_wert2;
    }

    if(Menu.ch2.visible==0) {
      ch2_wert2=oszi_adc2pixel(ADC_DMA_Buffer_C[(n*2)+1], Menu.ch2.faktor);
      ch2_wert2+=SCALE_Y_MITTE+SCALE_START_X+Menu.ch2.position;
      if(ch2_wert2<SCALE_START_X) ch2_wert2=SCALE_START_X;
      if(ch2_wert2>SCALE_MX_PIXEL) ch2_wert2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(ch2_wert1,SCALE_START_Y+n,ch2_wert2,SCALE_START_Y+n+1,ADC_CH2_COL);
      ch2_wert1=ch2_wert2;
    }
  }

  // nur die linke hälfte der FFT zeichnen
  // (die rechte ist das Spiegelbild)
  if(Menu.fft.mode!=0) {
    for(n=1;n<FFT_VISIBLE_LENGTH;n++) {
      fft_wert2=FFT_UINT_DATA[n];
      fft_wert2+=FFT_START_X;
      if(fft_wert2<SCALE_START_X) fft_wert2=SCALE_START_X;
      if(fft_wert2>SCALE_MX_PIXEL) fft_wert2=SCALE_MX_PIXEL;
      UB_Graphic2D_DrawLineNormal(fft_wert1,FFT_START_Y+n,fft_wert2,FFT_START_Y+n+1,FFT_COL);
      fft_wert1=fft_wert2;
    }
  }

  UB_LCD_SetLayer_Back();
}


//--------------------------------------------------------------
// Zum umrechnen von adc-Wert in Pixel-Position
//--------------------------------------------------------------
int16_t oszi_adc2pixel(uint16_t adc, uint32_t faktor)
{
  int16_t ret_wert=0;

  switch(faktor) {
    case 0 : ret_wert=adc*FAKTOR_5V;break;
    case 1 : ret_wert=adc*FAKTOR_2V;break;
    case 2 : ret_wert=adc*FAKTOR_1V;break;
    case 3 : ret_wert=adc*FAKTOR_0V5;break;
    case 4 : ret_wert=adc*FAKTOR_0V2;break;
    case 5 : ret_wert=adc*FAKTOR_0V1;break;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Daten per UART senden
//--------------------------------------------------------------
void p_oszi_send_data(void)
{
  uint32_t n;
  uint16_t wert1,wert2;
  char buf[10];
  extern const SM_Item_t UM_01[];
  extern const SM_Item_t UM_02[];

  //--------------------------------
  // send Screen as Bitmap
  //--------------------------------
  if(Menu.send.mode==6) {
    p_oszi_send_screen();
    return;
  }

  //--------------------------------
  // send settings
  //--------------------------------
  p_oszi_send_uart("SETTINGS:");
  if((Menu.send.mode==0) || (Menu.send.mode==1) || (Menu.send.mode==4) || (Menu.send.mode==5)) {
    sprintf(buf,"CH1=%s/div",UM_01[Menu.ch1.faktor].stxt);
    p_oszi_send_uart(buf);
  }
  if((Menu.send.mode==2) || (Menu.send.mode==3) || (Menu.send.mode==4) || (Menu.send.mode==5)) {
    sprintf(buf,"CH2=%s/div",UM_01[Menu.ch2.faktor].stxt);
    p_oszi_send_uart(buf);
  }
  sprintf(buf,"Time=%s/div",UM_02[Menu.timebase.value].stxt);
  p_oszi_send_uart(buf);
  p_oszi_send_uart("1div=25");

  sprintf(buf,"count=%d",SCALE_W);
  p_oszi_send_uart(buf);

  //--------------------------------
  // send data
  //--------------------------------
  p_oszi_send_uart("DATA:");
  if((Menu.send.mode==0) || (Menu.send.mode==1)) {
    p_oszi_send_uart("CH1");
    for(n=0;n<SCALE_W;n++) {
      wert1=ADC_DMA_Buffer_C[n*2];
      sprintf(buf,"%d",wert1);
      p_oszi_send_uart(buf);
    }
  }
  else if((Menu.send.mode==2) || (Menu.send.mode==3)) {
    p_oszi_send_uart("CH2");
    for(n=0;n<SCALE_W;n++) {
      wert2=ADC_DMA_Buffer_C[(n*2)+1];
      sprintf(buf,"%d",wert2);
      p_oszi_send_uart(buf);
    }
  }
  else if((Menu.send.mode==4) || (Menu.send.mode==5)) {
    p_oszi_send_uart("CH1,CH2");
    for(n=0;n<SCALE_W;n++) {
      wert1=ADC_DMA_Buffer_C[n*2];
      wert2=ADC_DMA_Buffer_C[(n*2)+1];
      sprintf(buf,"%d,%d",wert1,wert2);
      p_oszi_send_uart(buf);
    }
  }
  //--------------------------------
  // send fft
  //--------------------------------
  if((Menu.send.mode==1) || (Menu.send.mode==3) || (Menu.send.mode==5)) {
    if(Menu.fft.mode==1) {
      p_oszi_send_uart("FFT:");
      p_oszi_send_uart("CH1");
      sprintf(buf,"count=%d",FFT_VISIBLE_LENGTH);
      p_oszi_send_uart(buf);
      for(n=0;n<FFT_VISIBLE_LENGTH;n++) {
        wert2=FFT_UINT_DATA[n];
        sprintf(buf,"%d",wert2);
        p_oszi_send_uart(buf);
      }
    }
    else if(Menu.fft.mode==2) {
      p_oszi_send_uart("FFT:");
      p_oszi_send_uart("CH2");
      sprintf(buf,"count=%d",FFT_VISIBLE_LENGTH);
      p_oszi_send_uart(buf);
      for(n=0;n<FFT_VISIBLE_LENGTH;n++) {
        wert2=FFT_UINT_DATA[n];
        sprintf(buf,"%d",wert2);
        p_oszi_send_uart(buf);
      }
    }
  }
  p_oszi_send_uart("END.");
}

//--------------------------------------------------------------
// string per UART senden
//--------------------------------------------------------------
void p_oszi_send_uart(char *ptr)
{
  UB_Uart_SendString(COM1,ptr,CRLF);
}


//--------------------------------------------------------------
// Screen als Bitmap (*.bmp) per UART senden
// dauert bei 115200 Baud ca. 20 sekunden
//--------------------------------------------------------------
void p_oszi_send_screen(void)
{
  uint32_t n,adr;
  uint16_t x,y,color;
  uint8_t r,g,b;

  // BMP-Header senden
  for(n=0;n<BMP_HEADER_LEN;n++) {
    UB_Uart_SendByte(COM1,BMP_HEADER[n]);
  }

  // den richigen Buffer zum senden raussuchen
  if(LCD_CurrentLayer==1) {
    adr=LCD_FRAME_BUFFER;
  }
  else {
    adr=LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  }

  // alle Farb-Daten senden
  for(x=0;x<LCD_MAXX;x++) {
    for(y=0;y<LCD_MAXY;y++) {
      n=y*(LCD_MAXX*2)+(x*2);
      color=*(volatile uint16_t*)(adr+n);
      r=((color&0xF800)>>8);  // 5bit rot
      g=((color&0x07E0)>>3);  // 6bit gruen
      b=((color&0x001F)<<3);  // 5bit blau
      UB_Uart_SendByte(COM1,b);
      UB_Uart_SendByte(COM1,g);
      UB_Uart_SendByte(COM1,r);
    }
  }
}
