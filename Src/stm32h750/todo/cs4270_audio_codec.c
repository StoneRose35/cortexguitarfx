#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "stm32h750/helpers.h"
#include "systick.h"
#include "i2c.h"
#include "cs4270_audio_codec.h"


void cs4270_write(uint16_t data)
{
    masterTransmit((uint8_t)((data >> 8)&0xFF),0);
    masterTransmit((uint8_t)(data&0xFF),1);
}

void init_cs4270_audio_codec()
{
    // reset
    uint32_t port;
    uint32_t regbfr;
    GPIO_TypeDef * gpio;
    uint16_t regdata;

    port = CS4270_RESET >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((CS4270_RESET & 0xF)<<1));
    regbfr |= (1 << ((CS4270_RESET & 0xF)<<1));
    gpio->MODER=regbfr;
    gpio->OTYPER &= ~(1 << (CS4270_RESET & 0xF));
    regbfr = gpio->OSPEEDR;
    //regbfr &=~(3 << ((CS4270_RESET & 0xF)<<1));
    regbfr |= (3 << ((CS4270_RESET & 0xF)<<1));
    gpio->OSPEEDR = regbfr;
    gpio->PUPDR &= ~(3 << ((CS4270_RESET & 0xF)<<1));


    // reset high
    gpio->BSRR = (1 << (CS4270_RESET & 0xF));
    waitSysticks(1);
    //reset low
    gpio->BSRR = (1 << ((CS4270_RESET & 0xF)+16));
    waitSysticks(1);
    // reset high
    gpio->BSRR = (1 << (CS4270_RESET & 0xF));
    for(uint8_t c=0;c<64;c++) // wait at least 500ns
    {
        __asm__("nop");
    }
    regdata = (CS4270_R2 << 8) | (1 << CS4270_R2_PDN_Pos); // power down
    cs4270_write(regdata);

    regdata = (CS4270_R3 << 8); // set single speed master, no pop protection
    cs4270_write(regdata);

    regdata = (CS4270_R4 << 8) | (1 << CS4270_R4_DAC_DIF0_Pos) | (1 << CS4270_R4_ADC_DIF0_Pos); // set i2s for dac and adc
    cs4270_write(regdata);

    regdata = (CS4270_R2 << 8); // power up again
    cs4270_write(regdata);

}