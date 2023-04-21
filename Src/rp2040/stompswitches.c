/*
driver for a stomp switches interface which communicates it's status over i2c
*/
#include "i2c.h"
#include "stompswitches.h"


// bit 0: momentary
// bit 1: sticky pressed
// bit 2: sticky released
static volatile uint8_t switchesState[NR_STOMPSWITCHES];

// bitstuffen
static volatile 
union {
struct 
{
    uint8_t colorSwitch0 : 2;
    uint8_t colorSwitch1 : 2;
    uint8_t colorSwitch2 : 2;
};
 uint8_t rawColors;
}switchesColors;


uint8_t getStompSwitchState(uint8_t switchNr)
{
    return switchesState[switchNr];
}

void clearStompSwitchStickyPressed(uint8_t switchNr)
{
    switchesState[switchNr] &= ~(1 << 1);
}

void clearStompSwitchStickyReleased(uint8_t switchNr)
{
    switchesState[switchNr] &= ~(1 << 2);
}

void sendColors()
{
    if (getTargetAddress()!=STOMPSWITCHES_I2C_ADDRESS)
    {
        setTargetAddress(STOMPSWITCHES_I2C_ADDRESS);
    }
    masterTransmit(switchesColors.rawColors,1);
}

void setStompswitchColor(uint8_t switchNr,uint8_t clr)
{
    switch (switchNr)
    {
        case 0:
            switchesColors.colorSwitch0 = clr & 0x3;
            break;
        case 1:
            switchesColors.colorSwitch1 = clr & 0x3;
            break;
        case 2:
            switchesColors.colorSwitch2 = clr & 0x3;
            break;
    }
    sendColors();
}

void setStompswitchColorRaw(uint8_t data)
{
    switchesColors.rawColors = data;
    sendColors();
}

void requestSwitchesUpdate()
{
    uint8_t i2cData;
    i2cData = masterReceive(1);
    if (i2cData != 0xff)
    {
        for (uint8_t c=0;c<NR_STOMPSWITCHES;c++)
        {
            if (((switchesState[c] & SWITCH_STATE_MOMENTARY_MSK) == 0) && (i2cData & (1 << c)) == 0)
            {
                switchesState[c] |= (1 << 1); // set sticky pressed
            }
            else if (((switchesState[c] & SWITCH_STATE_MOMENTARY_MSK) != 0) && (i2cData & (1 << c)) != 0)
            {
                switchesState[c] |= (1 << 2); // set sticky released
            }
            // set momentary value
            switchesState[c] &= ~(1);
            switchesState[c] |= 0x1 ^ ((i2cData & (1 << c)) >> c);
        }
    }
}

void initStompSwitchesInterface()
{
    // assume that the i2c interface is initialized
    setTargetAddress(STOMPSWITCHES_I2C_ADDRESS);
    
}


