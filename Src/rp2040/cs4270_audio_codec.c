#include "stdint.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/resets.h"
#include "hardware/rp2040_registers.h"
#include "systick.h"
#include "i2c.h"
#include "cs4270_audio_codec.h"


uint8_t cs4270Write(uint16_t data)
{
    uint8_t res=0;
    res += masterTransmit((uint8_t)((data >> 8)&0xFF),0);
    res += masterTransmit((uint8_t)(data&0xFF),1);
    return res;
}

void setupCS4270()
{
    // reset
    uint32_t regdata;
    volatile uint8_t i2c_error = 0;
    *GPIO_OE |= (1 << AUDIO_CODEC_RESET);
    *AUDIO_CODEC_RESET_PIN_CNTR = 5;


    // reset high
    *(GPIO_OUT + 1) = (1 << AUDIO_CODEC_RESET);
    waitSysticks(1);
    // reset low
    *(GPIO_OUT + 2) = (1 << AUDIO_CODEC_RESET);
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

}