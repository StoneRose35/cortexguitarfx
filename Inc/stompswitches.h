#ifndef _STOMPSWITCHES_H_
#define _STOMPSWITCHES_H_

#define NR_STOMPSWITCHES 3
#define STOMPSWITCHES_I2C_ADDRESS 23
#define SWITCH_STATE_MOMENTARY_MSK 1

uint8_t getStompSwitchState(uint8_t switchNr);
void clearStompSwitchStickyPressed(uint8_t switchNr);
void clearStompSwitchStickyReleased(uint8_t switchNr);

void initStompSwitchesInterface();
void setStompswitchColor(uint8_t switchNr,uint8_t clr);
void requestSwitchesUpdate();

 #endif