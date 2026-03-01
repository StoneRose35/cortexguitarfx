#ifndef _UTIL_COMMANDS_H_
#define UTIL_COMMANDS_H_
#include "systemChoice.h"

#ifdef HARDWARE
typedef char* register_t ;
register_t address __asm__("sp");
#else
#include <stdio.h>
void* address;
#endif

void helpCommand(char * cmd,void* context);
void apiCommand(char * cmd,void* context);
void consoleCommand(char * cmd,void* context);
void setupBluetoothCommand(char * cmd,void* context);
void sysInfoCommand(char * cmd,void*context);
void setDateTimeCommand(char* cmd,void*context);

#endif