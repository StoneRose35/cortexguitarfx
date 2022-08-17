#include "wm8731.h"
#include "i2c.h"

void wm8731_write(uint16_t data)
{
    masterTransmit((uint8_t)((data >> 8)&0xFF),0);
    masterTransmit((uint8_t)(data&0xFF),1);
}

void setupWm8731(uint8_t sampledepth,uint8_t samplerate)
{
    uint16_t registerData;
    
    // R0, set volume to 0dB for both L and R channel, disable mute
    registerData = WM8731_R0 & (23 << LIN_VOL) & (1 << LRIN_BOTH_LSB);
    wm8731_write(registerData);

    //R4, enable dac and disable bypass
    registerData = WM8731_R4 & (1 << MUTE_MIC_LSB) & (1 << DACSEL_LSB);
    wm8731_write(registerData);

    // R5, disable dac soft mute
    registerData = WM8731_R5;
    wm8731_write(registerData);

    // R7: interface format: set sample depth and i2s format and master mode
    registerData = WM8731_R7 & (2 << FORMAT_LSB) & (sampledepth << IWL_LSB) & (1 << MS_LSB);
    wm8731_write(registerData);


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
    wm8731_write(registerData);

    //R6: enable line in, adc, dac, output oscillator, power on
    registerData = WM8731_R6 &
                    (1 << MICPD_LSB) & (1 << CLK_OUTPD_LSB);
    wm8731_write(registerData);

    //R9: activate interface
    registerData = WM8731_R9 & (1 << ACTIVE_LSB);
    wm8731_write(registerData);
    
    
}
