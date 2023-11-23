#ifndef _DISPLAY_COMMANDS_H
#define _DISPLAY_COMMANDS_H

#include <stdint.h>

void initDisplayCommand(char * cmd,void* context);

void setBacklightCommand(char * cmd,void* context);

void setCursorCommand(char * cmd,void* context);

void writeStringCommand(char * cmd,void * context);

#endif