
// ***************
// * pin numbers *
// ***************

// do not assign encoder or switch pins to the same pin number on different ports
// example: pin 17 (PORTB1) and pin 1 (PORTA1) is not allowed, 
#define ENCODER_1 (1*16+4) //6 // must be PA6 and PA7 since TIM3 with channel 1 and channel 2 is used
#define ENCODER_2 (1*16+5) //7 // must be PA6 and PA7 since TIM3 with channel 1 and channel 2 is used
#define ENTER_SWITCH (3*16+14) // PD14
#define EXIT_SWITCH (3*16+15) // PD15

// adc pins used by the three potentiometer, refer to the datasheet for a list of possible options
// 
#define POT1 3 // PA3
#define POT2 (2*16+0) // PC0
#define POT3 (2*16+3) // PC3


// i2s interface
#define I2S_BCLK (0*16+1)
#define I2S_LRCLK (0*16+2)
#define I2S_DOUT (0*16+3)
#define I2S_DIN (0*16+4)

// i2c interface
#define I2C_SDA (0*16+1)
#define I2C_SCL (0*16+2)

//ssd1306 display interfaced using spi
#define SSD1306_SCK (0*16+0)
#define SSD1306_MOSI (0*16+0)
#define SSD1306_MISO (0*16+0)
#define SSD1306_CD (0*16+0)
#define SSD1306_RESET (0*16+0)



// ***************************************
// * additional device-specific settings *
// ***************************************

#define POT1_CHANNEL 3
#define POT2_CHANNEL 10
#define POT3_CHANNEL 13
