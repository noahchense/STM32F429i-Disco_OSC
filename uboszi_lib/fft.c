//--------------------------------------------------------------
// File     : stm32_ub_fft.c
// Datum    : 11.03.2014
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : ARM_DSP-Module
// Funktion : FFT
//
// Hineise :
// "Config/Compile/Options"  : "FPU soft" oder "FPU hard"
// "Config/Compile/Defines"  : "ARM_MATH_CM4" , "__FPU_PRESENT=1"
// "Config/Link/MiscControl" : "-lm; -lgcc; -lc; " hinzufügen
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "fft.h"


//--------------------------------------------------------------
// Globale Variabeln
//--------------------------------------------------------------
static float32_t FFT_CMPLX_DATA[FFT_CMPLX_LENGTH];
static float32_t FFT_MAG_DATA[FFT_LENGTH];

arm_cfft_radix4_instance_f32 S_CFFT;
arm_rfft_instance_f32 S;



//--------------------------------------------------------------
// init vom FFT-Modul
// ret_wert : 0 = Fehler beim init
//            1 = init ok
//--------------------------------------------------------------
uint32_t fft_init(void)
{
  uint32_t ret_wert=0;
  arm_status status;
  uint32_t doBitReverse = 1;
  uint32_t ifftFlag = 0;

  status = ARM_MATH_SUCCESS;
  // FFT init
  status = arm_rfft_init_f32(&S, &S_CFFT, FFT_LENGTH, ifftFlag, doBitReverse);
  if(status!=ARM_MATH_SUCCESS) return(0);

  ret_wert=1;

  return(ret_wert);
}


//--------------------------------------------------------------
// berechnet die FFT
// rechnet die Daten dann noch in Pixelwerte um
//--------------------------------------------------------------
void fft_calc(void)
{
  float32_t maxValue;
  uint32_t n;

  // FFT berechnen
  arm_rfft_f32(&S, FFT_DATA_IN, FFT_CMPLX_DATA);
  arm_cmplx_mag_f32(FFT_CMPLX_DATA, FFT_MAG_DATA, FFT_LENGTH);

  // Maximum manuell suchen
  // die ersten beiden Einträge überspringen
  maxValue=0.1;
  for(n=2;n<FFT_VISIBLE_LENGTH;n++) {
    if(FFT_MAG_DATA[n]>maxValue) maxValue=FFT_MAG_DATA[n];
  }

  // alle Werte auf das Maximum normieren
  // die ersten beiden Einträge auf 0 setzen
  FFT_UINT_DATA[0]=0;
  FFT_UINT_DATA[1]=0;
  for(n=2;n<FFT_VISIBLE_LENGTH;n++) {
    FFT_UINT_DATA[n]=(uint16_t)(FFT_UINT_MAXWERT*FFT_MAG_DATA[n]/maxValue);
  }
}



