
#include "globalConfig.h"
// ***************
// * pin numbers *
// ***************

// A  B  C  D  E  F  G  H  
// 0  1  2  3  4  5  6  7 
#define ENCODER_1 (0*16+6) //PA6 since TIM3 with channel 1 and channel 2 is used
#define ENCODER_2 (0*16+7) //PA7 since TIM3 with channel 1 and channel 2 is used
#define ENTER_SWITCH (2*16+1) // PC1
#define EXIT_SWITCH (2*16+4) // PC4

// adc pins used by the three potentiometer, refer to the datasheet for a list of possible options
// 
#define POT1 (2*16+0) // PC0 
#define POT2 (0*16+3) // PA3
#define POT3 (1*16+1) // PB1



// i2s interface (internal)
#define I2S_BCLK (4*16+5) // PE5  SCK
#define I2S_LRCLK (4*16+4) // PE4 LRCLK
#define I2S_DOUT (4*16+3) // PE3 sai 1: transmitter
#define I2S_DIN (4*16+6) // PE6 sai 1: receiver
#define I2S_MCLK (4*16+2) // PE2 MCLK

// i2s interface (external)
#define I2S_EXT_BCLK (0*16+2) // PA2  SCK
#define I2S_EXT_LRCLK (6*16+9) // PG9 LRCLK
#define I2S_EXT_DOUT (0*16+0) // PA0 sai 2: transmitter
#define I2S_EXT_DIN (3*16+11) // PD11 sai 2: receiver
#define I2S_EXT_MCLK (0*16+1) // PA1 MCLK
//cs4270 reset line 
#define CS4270_RESET (2*16+11) //PC11
//pcm3060 reset line  (also pc11)
#define PCM3060_RESET (2*16+11) //PC11

#define DEBUG_OUTPUT_PIN (1*16+12) // PB12

// i2c interface

#define I2C_SDA_INTERNAL (1*16+11) // PB11
#define I2C_SCL_INTERNAL (7*16+4) // PH4
#define I2C_SDA_EXTERNAL (1*16+7) // PB7
#define I2C_SCL_EXTERNAL (1*16+6) // PB6
#define I2C_BLOCK_INTERNAL I2C2
#define I2C_BLOCK_EXTERNAL I2C1
#define I2C_ENR_INTERNAL RCC_APB1LENR_I2C2EN_Pos
#define I2C_ENR_EXTERNAL RCC_APB1LENR_I2C1EN_Pos

#define PCM3060_DEEMPHASIS (1*16 + 11) // PB11

#ifdef CS4270_CODEC
#define POWERSENSE_PIN (2*16 + 9) //PC9
#define CLIPPING_LED_INPUT (2*16 + 10) //PC10
#define CLIPPING_LED_OUTPUT (2*16 + 11) //PC11
#else
#define CLIPPING_LED_INPUT (2*16 + 10) //PC10
#define CLIPPING_LED_OUTPUT (2*16 + 9) //PC9
#endif

//ssd1306 display interfaced using spi
#define DISPLAY_SCK (6*16+11) // PG11
#define DISPLAY_MOSI (1*16+5) // PB5
//#define DISPLAY_MISO (1*16+4) // PB4
#define DISPLAY_CD (1*16+9) // PB9
#define DISPLAY_RESET (1*16+8) //PB8
#define DISPLAY_CS (6*16+10) //PG10

// avr programmer also interface over spi
#define AVRPROG_MISO (1*16+4) // PB4
#define AVRPROG_RESET (2*16 + 8) // PC8

// ***************************************
// * additional device-specific settings *
// ***************************************

#define POT1_CHANNEL 10
#define POT2_CHANNEL 15
#define POT3_CHANNEL 5
