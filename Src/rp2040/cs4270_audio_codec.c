#include "stdint.h"
#include "globalConfig.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "drivers/systick.h"
#include "drivers/i2c.h"
#include "drivers/cs4270_audio_codec.h"


static uint8_t cs4270Write(uint16_t data)
{
    uint8_t res=0;
    uint8_t sendBfr[2];
    sendBfr[0] = (uint8_t)((data >> 8)&0xFF);
    sendBfr[1] = (uint8_t)(data&0xFF);
    I2CsendMultiple(sendBfr,2,CS4270_I2C_ADDRESS);
    return res;
}

static uint8_t cs4270Read(uint8_t reg)
{
    uint8_t buffer[1];

    buffer[0]=reg;
    I2CsendMultiple(buffer,1,CS4270_I2C_ADDRESS);
    I2CReceiveMultiple(buffer,1,CS4270_I2C_ADDRESS);
    return buffer[0];
}

void cs4270PowerDown()
{
    uint16_t regdata;
    if (getTargetAddress()!=CS4270_I2C_ADDRESS)
    {
        setTargetAddress(CS4270_I2C_ADDRESS);
    }
    regdata = (CS4270_R2 << 8) | (1 << CS4270_R2_PDN_Pos); // power down
    cs4270Write(regdata);
}

void setupCS4270()
{
    // reset


    if (getTargetAddress()!=CS4270_I2C_ADDRESS)
    {
        setTargetAddress(CS4270_I2C_ADDRESS);
    }
    *GPIO_OE |= (1 << AUDIO_CODEC_RESET);
    *AUDIO_CODEC_RESET_PIN_CNTR = 5;


    // reset high
    *(GPIO_OUT + 1) = (1 << AUDIO_CODEC_RESET);
    waitSysticks(1);
    // reset low
    *(GPIO_OUT + 2) = (1 << AUDIO_CODEC_RESET);
    #ifndef TRIGGER_UI_BY_CORE_0
    uint32_t regdata;
    volatile uint8_t i2c_error = 0;
    waitSysticks(1);
    // reset high
    *(GPIO_OUT + 1) = (1 << AUDIO_CODEC_RESET);
    for(uint8_t c=0;c<10;c++) // wait at least 500ns
    {
        __asm__("nop");
    }
    regdata = (CS4270_R2 << 8) | (1 << CS4270_R2_PDN_Pos); // power down
    i2c_error += cs4270Write(regdata);

    regdata = (CS4270_R3 << 8); // set single speed master, no pop protection
    i2c_error += cs4270Write(regdata);

    regdata = (CS4270_R4 << 8) | (1 << CS4270_R4_DAC_DIF0_Pos) | (1 << CS4270_R4_ADC_DIF0_Pos) | (0 << CS4270_R4_DIG_LOOPBK_Pos); // set i2s for dac and adc
    i2c_error += cs4270Write(regdata);

    regdata = (CS4270_R6 << 8); // disable mute
    i2c_error += cs4270Write(regdata);


    regdata = (CS4270_R2 << 8); // power up again
    i2c_error += cs4270Write(regdata);
    #endif
}

/*
    channel is either CS4270_CHANNEL_A or CS4270_CHANNEL_B,
    val: 1 means unmuted (on) and 0 means muted (off)
*/
void cs4270SetInputState(uint8_t channel,uint8_t val)
{
    uint16_t regdata;
    uint8_t regContent;
    if (getTargetAddress()!=CS4270_I2C_ADDRESS)
    {
        setTargetAddress(CS4270_I2C_ADDRESS);
    }
    regdata = (CS4270_R6 << 8);
    regContent = cs4270Read(CS4270_R6);
    if (val!=0)
    {
        if (channel == CS4270_CHANNEL_A || channel == CS4270_CHANNEL_BOTH)
        {
            regContent &= ~(1 << 3);
        }
        if (channel == CS4270_CHANNEL_B || channel == CS4270_CHANNEL_BOTH)
        {
            regContent &= ~(1 << 4);
        }
    }
    else
    {
        if (channel == CS4270_CHANNEL_A || channel == CS4270_CHANNEL_BOTH)
        {
            regContent |= (1 << 3);
        }
        if (channel == CS4270_CHANNEL_B || channel == CS4270_CHANNEL_BOTH)
        {
            regContent |= (1 << 4);
        }
    }
    regdata |= regContent;
    cs4270Write(regdata);
}

/*
    channel is either CS4270_CHANNEL_A or CS4270_CHANNEL_B,
    byte 0: Channel A, byte 1: Channel B
*/
uint8_t cs4270GetInputState()
{
    uint8_t regContent;
    if (getTargetAddress()!=CS4270_I2C_ADDRESS)
    {
        setTargetAddress(CS4270_I2C_ADDRESS);
    }
    regContent = cs4270Read(CS4270_R6);
    regContent >>= 3;
    regContent &= (0x3);
    regContent ^= (0x3);
    return regContent;
}

void cs4270SetOutputVolume(uint8_t channel,uint8_t volume)
{
    uint16_t regdata;
    uint8_t sendBfr[3];
    if (channel == CS4270_CHANNEL_A) 
    {
        regdata = (CS4270_R7 << 8);
    }
    else if (channel == CS4270_CHANNEL_B)
    {
        regdata = (CS4270_R8 << 8);
    }
    else
    {
        regdata = ((0x80 | CS4270_R7) << 8);
    }
    regdata |= (0xFF - volume);
    if (channel != CS4270_CHANNEL_BOTH)
    {
        cs4270Write(regdata);
    }
    else
    {
        sendBfr[0] = (0x80 | CS4270_R7);
        sendBfr[1] = (0xFF - volume);
        sendBfr[2] = (0xFF - volume);
        I2CsendMultiple(sendBfr,3,CS4270_I2C_ADDRESS);
    }
}

/*
returns the output volume for both channels
A is in the MSB, B in the LSB 
*/
uint16_t cs4270GetOutputVolume()
{
    uint16_t outval=0;
    uint8_t channelVal;
    uint8_t buffer[2];
    buffer[0] = (0x80 | CS4270_R7);
    I2CsendMultiple(buffer,1,CS4270_I2C_ADDRESS);
    I2CReceiveMultiple(buffer,2,CS4270_I2C_ADDRESS);
    channelVal = 0xFF - buffer[0];
    outval |= (channelVal << 8);
    channelVal = 0xFF - buffer[1];
    outval |= channelVal;
    return outval;
}