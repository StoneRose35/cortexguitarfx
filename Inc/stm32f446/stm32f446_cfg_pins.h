
// ***************
// * pin numbers *
// ***************

// do not assign encoder or switch pins to the same pin number on different ports
// example: pin 17 (PORTB1) and pin 1 (PORTA1) is not allowed, 
#define ENCODER_1 7
#define ENCODER_2 6
#define ENTER_SWITCH 2
#define EXIT_SWITCH 1

// adc pins used by the three potentiometer, refer to the datasheet for a list of possible options
// 
#define POT1 0 
#define POT2 1
#define POT3 2



// ***************************************
// * additional device-specific settings *
// ***************************************

#define POT1_CHANNEL 0
#define POT2_CHANNEL 1
#define POT3_CHANNEL 2
