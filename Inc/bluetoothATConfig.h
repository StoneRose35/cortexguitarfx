/*
 * bluetootlATConfig.h
 *
 *  Created on: 04.11.2021
 *      Author: philipp
 */


#ifndef BLUETOOTHATCONFIG_H_
#define BLUETOOTHATCONFIG_H_

#include <stdint.h>

uint8_t ATCheckEnabled();

uint8_t ATSendCommand(char * ,char * );

uint8_t ATSetName(char *);

uint8_t ATSetBaud(char*);

uint8_t ATSetPin(char * pin);

#endif /* BLUETOOTHATCONFIG_H_ */


