
#include "pcm3060.h"
#include "gpio.h"
#include "stm32h750/stm32h750_cfg_pins.h"

/* use this when using the internal pcm3060 of the daisy board
void setupPCM3060()
{
    setAsOutput(PCM3060_DEEMPHASIS);
    setPin(PCM3060_DEEMPHASIS,0);
}
*/



#include "pcm3060.h"
#include "stdint.h"
#include "drivers/systick.h"
#include "drivers/i2c.h"
#include "globalConfig.h"


static uint8_t pcm3060Write(uint16_t data)
{
    uint8_t res=0;
    uint8_t sendBfr[2];
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }
    sendBfr[0] = (uint8_t)((data >> 8)&0xFF);
    sendBfr[1] = (uint8_t)(data&0xFF);
    res = I2CsendMultipleExternal(sendBfr,2);
    return res;
}

static uint8_t pcm3060Read(uint8_t reg)
{
    uint8_t buffer[1];
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }
    buffer[0]=reg;
    I2CsendMultipleExternal(buffer,1);
    I2CReceiveMultipleExternal(buffer,1);
    return buffer[0];
}

void pcm3060PowerDown()
{
    uint16_t regdata;
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }
    regdata = (PCM3060_R64 << 8) | (1 << PCM3060_R64_ADPSV) | (1 << PCM3060_R64_DAPSV); // power down
    pcm3060Write(regdata);
}

void setupPCM3060()
{
    // reset
    uint32_t port;
    uint32_t regbfr;
    GPIO_TypeDef * gpio;

    port = PCM3060_RESET >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((PCM3060_RESET & 0xF)<<1));
    regbfr |= (1 << ((PCM3060_RESET & 0xF)<<1));
    gpio->MODER=regbfr;
    gpio->OTYPER &= ~(1 << (PCM3060_RESET & 0xF));
    regbfr = gpio->OSPEEDR;
    regbfr |= (3 << ((PCM3060_RESET & 0xF)<<1));
    gpio->OSPEEDR = regbfr;
    gpio->PUPDR &= ~(3 << ((PCM3060_RESET & 0xF)<<1));


    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }

    // switch on master oscillator
    // reset low
    gpio->BSRR = (1 << ((PCM3060_RESET & 0xF)+16));
    volatile uint8_t i2c_error = 0;
    waitSysticks(1);
    // reset high
    gpio->BSRR = (1 << (PCM3060_RESET & 0xF));
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
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
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
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }
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
        I2CsendMultipleExternal(sendBfr,3);
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
    if (getTargetAddressExternal()!=PCM3060_I2C_ADDRESS)
    {
        setTargetAddressExternal(PCM3060_I2C_ADDRESS);
    }
    I2CsendMultipleExternal(buffer,1);
    I2CReceiveMultipleExternal(buffer,2);
    return *((uint16_t*)buffer);
}