#include "drivers/wm8731.h"
#include "drivers/i2c.h"
#include "drivers/debugLed.h"

uint8_t wm8731_write(uint16_t data)
{
    uint8_t retcodes=0;
    retcodes += masterTransmit((uint8_t)((data >> 8)&0xFF),0);
    retcodes += masterTransmit((uint8_t)(data&0xFF),1);
    return retcodes;
}

void setupWm8731(uint8_t sampledepth,uint8_t samplerate)
{
    uint16_t registerData;
    uint8_t rcs=0;

    // R15, reset device
    registerData = WM8731_R15;
    rcs += wm8731_write(registerData);

    // R0, set volume to 0dB for both L and R channel, disable mute
    registerData = WM8731_R0 | (23 << LIN_VOL) | (1 << LRIN_BOTH_LSB);
    rcs += wm8731_write(registerData);

    //R4, enable dac and disable bypass
    registerData = WM8731_R4 | (1 << MUTE_MIC_LSB) | (1 << DACSEL_LSB) | (0 << BYPASS_LSB) | (0 << SIDETONE_LSB);
    rcs += wm8731_write(registerData);

    // R5, disable dac soft mute
    registerData = WM8731_R5;
    rcs += wm8731_write(registerData);


    //R8: sample rate
    registerData = WM8731_R8;
    if (samplerate == SAMPLERATE_8KHZ)
    {
        registerData |= (3 << SR_LSB);
    }
    else if (samplerate == SAMPLERATE_32KHZ)
    {
        registerData |= (6 << SR_LSB);
    }
    else if (samplerate == SAMPLERATE_48KHZ)
    {
        registerData |= (0 << SR_LSB);
    }
    rcs += wm8731_write(registerData);

    // R7: interface format: set sample depth and i2s format and master mode
    registerData = WM8731_R7 | (2 << FORMAT_LSB) | (1 << MS_LSB) | (sampledepth << IWL_LSB); 
    rcs += wm8731_write(registerData);

    //R6: enable line in, adc, dac, output oscillator, power on
    registerData = WM8731_R6 |
                    (1 << MICPD_LSB) | (1 << CLK_OUTPD_LSB);
    rcs += wm8731_write(registerData);

    //R9: activate interface
    registerData = WM8731_R9 | (1 << ACTIVE_LSB);
    rcs += wm8731_write(registerData);

    if (rcs > 0)
    {
        DebugLedOn();
    }
}
