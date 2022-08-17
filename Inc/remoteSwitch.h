#ifndef _REMOTE_SWITCH_H_
#define _REMOTE_SWITCH_H_
#include "stdint.h"


void sendRemoteSwitchCommand(uint32_t cmd);
void initRemoteSwitch();
void remoteSwitchOn();
void remoteSwitchOff();

#endif