
#include "drivers/pcm3060.h"
#include "stdint.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "drivers/systick.h"
#include "drivers/i2c.h"
#include "globalConfig.h"


static uint8_t pcm3060Write(uint16_t data)
{
    uint8_t res=0;
    uint8_t sendBfr[2];
    sendBfr[0] = (uint8_t)((data >> 8)&0xFF);
    sendBfr[1] = (uint8_t)(data&0xFF);
    res = I2CsendMultiple(sendBfr,2,PCM3060_I2C_ADDRESS);
    return res;
}

static uint8_t pcm3060Read(uint8_t reg)
{
    uint8_t buffer[1];

    buffer[0]=reg;
    I2CsendMultiple(buffer,1,PCM3060_I2C_ADDRESS);
    I2CReceiveMultiple(buffer,1,PCM3060_I2C_ADDRESS);
    return buffer[0];
}

void pcm3060PowerDown()
{
    uint16_t regdata;
    if (getTargetAddress()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddress(PCM3060_I2C_ADDRESS);
    }
    regdata = (PCM3060_R64 << 8) | (1 << PCM3060_R64_ADPSV) | (1 << PCM3060_R64_DAPSV); // power down
    pcm3060Write(regdata);
}

void setupPCM3060()
{
    // reset


    if (getTargetAddress()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddress(PCM3060_I2C_ADDRESS);
    }
    *GPIO_OE |= (1 << AUDIO_CODEC_RESET);
    *AUDIO_CODEC_RESET_PIN_CNTR = 5;

    // switch on master oscillator
    // reset low
    *(GPIO_OUT + 2) = (1 << AUDIO_CODEC_RESET);
    #ifndef TRIGGER_UI_BY_CORE_0
    volatile uint8_t i2c_error = 0;
    waitSysticks(1);
    // reset high
    *(GPIO_OUT + 1) = (1 << AUDIO_CODEC_RESET);
    waitSysticks(1);


    // disable powersave
        i2c_error += pcm3060Write((PCM3060_R64 << 8 )
                |(0 << PCM3060_R64_ADPSV)
                |(0 << PCM3060_R64_DAPSV)
            );

    // master mode for adc, systemclock is 256*fs
    i2c_error += pcm3060Write((PCM3060_R72 << 8) | (4 << PCM3060_R72_MS));

    // select clock 1 for dac
    i2c_error += pcm3060Write((PCM3060_R67 << 8) | (1 << PCM3060_R67_CSEL2)); 

    // disable power save, disable single ended mode, clock 1 for dac
    i2c_error += pcm3060Write((PCM3060_R64 << 8 )
                |(0 << PCM3060_R64_ADPSV)
                |(0 << PCM3060_R64_DAPSV)
                |(1 << PCM3060_R64_MRST)
                |(1 << PCM3060_R64_SRST)
                |(1 << PCM3060_R64_SE)
            );

    #endif

}

/*
    channel is either LEFT(1) RIGHT(0) or BOTH (2)
    val: 1 means unmuted (on) and 0 means muted (off)
*/
void pcm3060SetInputState(uint8_t channel,uint8_t val)
{
    uint8_t regContent;
    regContent = pcm3060Read(PCM3060_R73);
    if (val)
    {
        if (channel==PCM3060_CHANNEL_BOTH)
        {
            regContent &= ~(3 << (PCM3060_R73_MUT));
        }
        else
        {
            regContent &= ~(1 << (PCM3060_R73_MUT+channel));
        }
    }
    else
    {
        if (channel==PCM3060_CHANNEL_BOTH)
        {
            regContent |= (3 << (PCM3060_R73_MUT));
        }
        else
        {
            regContent |= (1 << (PCM3060_R73_MUT+channel));
        }
    }
    pcm3060Write((PCM3060_R73 << 8) | regContent);
}

/*
    channel is either LEFT (1), RIGHT (0) or BOTH 2
    bit 0: channel Left, bit 1: channel right
*/
uint8_t pcm3060GetInputState()
{
    uint8_t regContent;
    if (getTargetAddress()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddress(PCM3060_I2C_ADDRESS);
    }
    regContent = pcm3060Read(PCM3060_R73);
    regContent &= 0x3;
    regContent = ((regContent & 0x2) >> 1) | (regContent &0x1 << 1);
    return regContent;
}

void pcm3060SetOutputVolume(uint8_t channel,uint8_t volume)
{
    uint16_t regData;
    uint8_t sendBfr[3];

    if (channel ==PCM3060_CHANNEL_LEFT || channel == PCM3060_CHANNEL_BOTH)
    {
        regData = (PCM3060_R65 << 8);
    }
    else
    {
        regData = (PCM3060_R66 << 8);
    }
    if (channel != PCM3060_CHANNEL_BOTH)
    {
        regData |= volume;
        pcm3060Write(regData);
    }
    else
    {
        sendBfr[0]=PCM3060_R65;
        sendBfr[1]=volume;
        sendBfr[2]=volume;
        I2CsendMultiple(sendBfr,3,PCM3060_I2C_ADDRESS);
    }
}


/*
returns the output volume for both channels
LEFT is in the MSB, RIGHT in the LSB
*/
uint16_t pcm3060GetOutputVolume()
{
    uint8_t buffer[2];
    buffer[0] = PCM3060_R65;
    I2CsendMultiple(buffer,1,PCM3060_I2C_ADDRESS);
    I2CReceiveMultiple(buffer,2,PCM3060_I2C_ADDRESS);
    return *((uint16_t*)buffer);
}