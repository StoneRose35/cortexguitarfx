/*
 * fakeBluetoothATConfig.c
 *
 *  Created on: 04.11.2021
 *      Author: philipp
 */

#ifndef STM32

#include <stdint.h>
#include "bluetoothATConfig.h"

uint8_t ATCheckEnabled()
{
	return 0;
}

uint8_t ATSendCommand(char * cmd,char * response)
{
	return 0;
}

uint8_t ATSetName(char * name)
{
	return 0;
}

uint8_t ATSetBaud(char * index)
{
	return 0;
}

uint8_t ATSetPin(char * pin){
	return 0;
}

#endif
