#include "stdint.h"
#include "drivers/wm8731.h"
#include "drivers/i2c.h"

volatile uint16_t wm8731Volumes = (23 << (3 + 8)) | (23 << 3);
volatile uint8_t wm8731States = 3; // bit 0: left, bit 1: right, 0 is off, 1 is on

void wm8731_write(uint16_t data)
{
    uint8_t sendBfr[2];
    if (getTargetAddressExternal()!=WM8731_ADDRESS)
    {
        setTargetAddressExternal(WM8731_ADDRESS);
    }
    sendBfr[0] = (uint8_t)((data >> 8)&0xFF);
    sendBfr[1] = (uint8_t)(data&0xFF);
    I2CsendMultipleExternal(sendBfr,2);
    //masterTransmitInternal((uint8_t)((data >> 8)&0xFF),0);
    //masterTransmitInternal((uint8_t)(data&0xFF),1);
}


void setupWm8731(uint8_t sampledepth,uint8_t samplerate)
{
    uint16_t sampleRateConf;

    // reset, interface might have been enabled before
    const uint16_t registerDataReset = WM8731_R15;
    wm8731_write(registerDataReset);
    
    // R0, set volume to 0dB for both L and R channel, disable mute
    const uint16_t registerData0 = (WM8731_R0 | (23 << LIN_VOL) | (1 << LRIN_BOTH_LSB)); 
    wm8731_write(registerData0);

    //R4, enable dac and disable bypass
    const uint16_t registerData4 = WM8731_R4 | (1 << MUTE_MIC_LSB) | (1 << DACSEL_LSB);
    wm8731_write(registerData4);

    // R5, disable dac soft mute
    const uint16_t registerData5 = WM8731_R5;
    wm8731_write(registerData5);

    // R7: interface format: set sample depth and i2s format and slave mode
    const uint16_t registerData7 = WM8731_R7 | (2 << FORMAT_LSB) | (sampledepth << IWL_LSB) | (0 << MS_LSB);
    wm8731_write(registerData7);


    //R8: sample rate
    sampleRateConf = WM8731_R8;
    if (samplerate == SAMPLERATE_8KHZ)
    {
        sampleRateConf |= (3 << SR_LSB);
    }
    else if (samplerate == SAMPLERATE_32KHZ)
    {
        sampleRateConf |= (6 << SR_LSB);
    }
    else if (samplerate == SAMPLERATE_48KHZ)
    {
        sampleRateConf |= (0 << SR_LSB);
    }
    wm8731_write(sampleRateConf);

    //R6: enable line in, adc, dac, power on
    const uint16_t registerData6 = WM8731_R6 |
                    (1 << MICPD_LSB) | (1 << CLK_OUTPD_LSB) | (1 << OSCPD_LSB);
    wm8731_write(registerData6);

    //R9: activate interface
    const uint16_t registerData9 = WM8731_R9 | (1 << ACTIVE_LSB);
    wm8731_write(registerData9);
}

uint16_t wm8731GetOutputVolume(void)
{
    return wm8731Volumes;
}

void wm8731SetOutputVolume(uint8_t channels,uint8_t volume)
{
    wm8731Volumes =  (volume << 8) | (volume & 0xFF) ;
    uint16_t rData = (WM8731_R0 | (((~wm8731States) & 0x1) << LIN_MUTE_LSB ) | ((wm8731Volumes >> 3) & 0x1F)); 
    wm8731_write(rData);
    rData = (WM8731_R1 | (((~(wm8731States>> 1)) & 0x1) << RIN_MUTE_LSB ) | ((wm8731Volumes >> 3) & 0x1F)); 
    wm8731_write(rData);
}


uint8_t wm8731GetInputState(void)
{
    return wm8731States;
}

void wm8731SetInputState(uint8_t channel,uint8_t val)
{
    if (val == 0)
    {
        wm8731States &= ~(1 << channel);
    }
    else
    {
        wm8731States |= (1 << channel);
    }
    uint16_t rData = (WM8731_R0 | (((~wm8731States) & 0x1) << LIN_MUTE_LSB ) | ((wm8731Volumes >> 3) & 0x1F)); 
    wm8731_write(rData);
    rData = (WM8731_R1 | (((~(wm8731States>> 1)) & 0x1) << RIN_MUTE_LSB ) | ((wm8731Volumes >> 3) & 0x1F)); 
    wm8731_write(rData);

}