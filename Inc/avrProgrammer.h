#ifndef _AVR_PROGRAMMER_H_
#define _AVR_PROGRAMMER_H_
#include "stdint.h"


#define AVR_PROG_CMD_ENABLE_PROG_B1 0xAC
#define AVR_PROG_CMD_ENABLE_PROG_B2 0x53
#define AVR_PROG_CMD_READ_PROGMEM_B1 0x20
#define AVR_PROG_CMD_READ_PROGMEM_B1_MSK 0x8
#define AVR_PROG_CMD_READ_PROGMEM_B2 0x0
#define AVR_PROG_CMD_READ_PROGMEM_B2_MSK 0x1F
#define AVR_PROG_CMD_LOAD_PROGMEM_B1 0x40
#define AVR_PROG_CMD_LOAD_PROGMEM_B1_MSK 0x8
#define AVR_PROG_CMD_WRITE_PROGMEM_B1 0x4C
#define AVR_PROG_CMD_WRITE_PROGMEM_B2 0x0
#define AVR_PROG_CMD_WRITE_PROGMEM_B2_MSK 0x1F
#define AVR_PROG_CMD_READ_SIG_B1 0x30
#define AVR_PROG_CMD_CHIP_ERASE_B1 0xAC
#define AVR_PROG_CMD_CHIP_ERASE_B2 0x80
#define AVR_PROG_CMD_POLL_RDY_B1 0xF0

#define AVR_FIRMWARE_VERSION_LOCATION 0x80

#define AVR_PROG_STATE_PROGRAMMING 1
#define AVR_PROG_STATE_IDLE 0

// initializes the prog reset pin as (preferably) open-drain output, set to high per default
// unwire the display cs pin
// set spi frequency to 100kHz
void initAvrProgrammer();


// check if a firmware identifier at a 256byte boundary matches an expected firmware number
// return 1 on match, 0 on no match
// and negative numbers on failure
int8_t matchAvrFirmwareVersion(uint32_t expectedFirmwareVersion);


// download the AVR Flash image into an array
// the array must be 8192 bytes or larger 
uint8_t downloadAvrFirmware(uint16_t * data);

// clear the AVR flash
uint8_t clearAvrFlash();

// upload a new avr Firmware onto the atmega88(p)
uint8_t uploadAvrFirmware(uint16_t * data,uint16_t size);

// read 2 bytes at a certain address, beware that addressing is 16-bit aligned
// when the program memory at address 0x1000 is 0xFE 0xAB 0x22 0x33
// readAvrProgramMemoryHalfword(0x1000) returns 0xABFE
// readAvrProgramMemoryHalfword(0x1001) returns 0x3322
uint16_t readAvrProgramMemoryHalfword(uint16_t address);

// enables the programming mode of the avr,
// the stomp switches and bicolor leds are disfunctional after this
// returns 0 if ok, 1 if failed
uint8_t enableAvrProgrammingMode();

// releases the avr from programming mode by pulling reset to 1
uint8_t disableAvrProgrammingMode();

// read the three signature bytes
// the expected result is 0x1e (Manufacturer Atmel), 0x93 (8kB Flash), 0x0A (Atmega88) or 0x0F (Atmega88p)
uint8_t readSignatureBytes(uint8_t * data);


uint8_t waitUntilReady();
#endif