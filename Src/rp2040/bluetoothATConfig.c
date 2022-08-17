/*
 * bluetoothATConfig.c
 *
 *  Created on: 04.11.2021
 *      Author: philipp
 */

#ifdef RP2040_FEATHER
#include <stdint.h>
#include "uart.h"
#include "neopixelDriver.h"
#include "stringFunctions.h"
#include "memoryAccess.h"

const char * ATCommandCheck = "AT";
const char * ATCommandSetNamePrefix = "AT+NAME";
const char * ATCommandSetBaudPrefix = "AT+BAUD";
const char * ATCommandSetPinPrefix = "AT+PIN";

uint8_t ATSendCommand(char * cmd,char * out)
{
	return 0;
}

uint8_t ATCheckEnabled()
{
	char response[32];
	char request[32];
	uint8_t c=0,creq=0;
	while (ATCommandCheck[c] != 0)
	{
		request[creq]=ATCommandSetNamePrefix[c];
		c++;
		creq++;
	}
	request[creq]=0;
	ATSendCommand(request,response);
	if (startsWith(response,"OK") > 0)
	{
		return 0;
	}
	return 1;
}

uint8_t ATSetName(char * name)
{
	uint8_t c=0,creq=0;
	char response[32];
	char request[32];
	while (ATCommandSetNamePrefix[c] != 0)
	{
		request[creq]=ATCommandSetNamePrefix[c];
		c++;
		creq++;
	}
	c=0;
	while(name[c] != 0)
	{
		request[creq]=name[c];
		c++;
		creq++;
	}
	request[creq] = 0;


	ATSendCommand(request,response);
	if (startsWith(response,"OKsetname") > 0)
	{
		return 0;
	}
	return 1;
}

uint8_t ATSetBaud(char * idx)
{
	return 0;
}

uint8_t ATSetPin(char * pin)
{
	uint8_t c=0,creq=0;
	char response[32];
	char request[32];
	while (ATCommandSetPinPrefix[c] != 0)
	{
		request[creq]=ATCommandSetPinPrefix[c];
		c++;
		creq++;
	}
	c=0;
	while(pin[c] != 0)
	{
		request[creq]=pin[c];
		c++;
		creq++;
	}
	request[creq] = 0;


	ATSendCommand(request,response);
	if (startsWith(response,"OK") > 0)
	{
		return 0;
	}
	return 1;
}

#endif
