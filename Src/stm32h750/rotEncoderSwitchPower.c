
#include "drivers/rotEncoderSwitchPower.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "drivers/cs4270_audio_codec.h"
#include "drivers/systick.h"

static uint32_t oldtickenc,oldtickswitch;
static volatile uint32_t encoderLastVal=0;
static volatile uint32_t currentUsVal;
static volatile uint32_t lastUsVal;
static volatile uint8_t switchVal;
static volatile uint8_t switchPins[8];
static volatile uint8_t switchVals[8]; // bit 0: sticky bit set when button is pressed (chage from 0 to 1), bit 1: sticky bit set when button is released, bit 2: momentary value
static volatile uint32_t oldTickSwitches[8];
static volatile uint8_t lastTrigger;

void processExternalInterrupt()
{
    GPIO_TypeDef * gpio;
    for (uint8_t c=0;c<8;c++)
    {
        if ((EXTI->PR1 & (1 << (switchPins[c] & 0xF)))!= 0)
        {
            gpio = (GPIO_TypeDef*)(GPIOA_BASE + (switchPins[c] >> 4)*0x400);
            if (oldTickSwitches[c] + ROTARY_ENCODER_DEBOUNCE < getTickValue())
            {
                if ((gpio->IDR & (1 << (switchPins[c] & 0xF)))==0)
                {
                    switchVals[c] |= ((1 << 2) | (1 << 0));
                }
                else
                {
                    switchVals[c] &= ~(1 << 2);
                    switchVals[c] |= (1 << 1);
                }
                oldTickSwitches[c]=getTickValue();
            }
            EXTI->PR1 = (1 << (switchPins[c] & 0xF));
        }
    }
    if ((EXTI->PR1 & (1 << (POWERSENSE_PIN & 0xF)))!= 0)
    {
        cs4270PowerDown();
    }
}



void EXTI0_IRQHandler()
{
    processExternalInterrupt();
}

void EXTI1_IRQHandler()
{
    processExternalInterrupt();
}
void EXTI2_IRQHandler()
{
    processExternalInterrupt();
}

void EXTI3_IRQHandler()
{
    processExternalInterrupt();
}

void EXTI4_IRQHandler()
{
    processExternalInterrupt();
}

void EXTI9_5_IRQHandler()
{
    processExternalInterrupt();
}

void EXTI15_10_IRQHandler()
{
    processExternalInterrupt();
}

void enableExternalInterrupt(uint8_t pinnr)
{
        // globally enable the necessary external interrupt lines
    switch (pinnr & 0xF)
    {
        case 0:
            NVIC_EnableIRQ(EXTI0_IRQn);
            break;
        case 1:
            NVIC_EnableIRQ(EXTI1_IRQn);
            break;
        case 2:
            NVIC_EnableIRQ(EXTI2_IRQn);
            break;
        case 3:
            NVIC_EnableIRQ(EXTI3_IRQn);
            break;
        case 4:
            NVIC_EnableIRQ(EXTI4_IRQn);
            break;
        default:
            if ((pinnr & 0xF) < 10)
            {
                NVIC_EnableIRQ(EXTI9_5_IRQn);
            }
            else
            {
                NVIC_EnableIRQ(EXTI15_10_IRQn);
            }
    }
}

// pin numbering is is successive among all the gpio ports
// so pin number 0 is PA0
// pin number 111 is PG15
void initRotaryEncoder(const uint8_t* pins,const uint8_t nswitches)
{
    uint32_t regbfr;
    uint8_t port;
    GPIO_TypeDef * gpio;
    
    // enable syscfg to define external interrupts
    RCC->APB4ENR |= (1 << RCC_APB4ENR_SYSCFGEN_Pos);
    
    // enable the gpio port (A-H) to which the pin belongs
    port = ENCODER_1 >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);

    // set as input with pullup enabled
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((ENCODER_1 & 0xF)<<1));
    regbfr |= (2 << ((ENCODER_1 & 0xF)<<1));
    gpio->MODER=regbfr;    
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((ENCODER_1 & 0xF)<< 1));
    regbfr |= (1 << ((ENCODER_1  & 0xF)<<1));
    gpio->PUPDR = regbfr;
    regbfr = gpio->AFR[(ENCODER_1 & 0xF)>>3];
    regbfr &= ~(0xF << ((ENCODER_1 & 0xF) << 2));
    regbfr |= 2 << ((ENCODER_1 & 0xF) << 2);
    gpio->AFR[(ENCODER_1 & 0xF)>>3] = regbfr;

    // enable the gpio port (A-H) to which the pin belongs
    port = ENCODER_2 >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);

    // set as input with pullup enabled
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((ENCODER_2 & 0xF)<<1));
    regbfr |= (2 << ((ENCODER_2 & 0xF)<<1));
    gpio->MODER=regbfr;    
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((ENCODER_2 & 0xF)<<1));
    regbfr |= (1 << ((ENCODER_2 & 0xF)<<1));
    gpio->PUPDR = regbfr;
    regbfr = gpio->AFR[(ENCODER_2 & 0xF)>>3];
    regbfr &= ~(0xF << ((ENCODER_2 & 0xF) << 2));
    regbfr |= 2 << ((ENCODER_2 & 0xF) << 2);
    gpio->AFR[(ENCODER_2 & 0xF)>>3] = regbfr;


    // enable timer3
    RCC->APB1LENR |= (1 << RCC_APB1LENR_TIM3EN_Pos);
    TIM3->SMCR |= (3 << TIM_SMCR_SMS_Pos); // encoder mode 3
    TIM3->CCMR1 |= (1 << TIM_CCMR1_CC1S_Pos) | (1 << TIM_CCMR1_CC2S_Pos) | 
                   (7 << TIM_CCMR1_IC1F_Pos) | (7 << TIM_CCMR1_IC2F_Pos)  ; 
                   // channel 1 to TI1, channel 2 to TI2, apply filtering
    TIM3->CCER = (0 << TIM_CCER_CC1NP_Pos) | (0 << TIM_CCER_CC1P_Pos) | (0 << TIM_CCER_CC1E_Pos) |
                (0 << TIM_CCER_CC2NP_Pos) | (0 << TIM_CCER_CC2P_Pos) | (0 << TIM_CCER_CC2E_Pos);
    //TIM3->CNT = 0x7FFF;
    TIM3->ARR = 0xFFFF;
    TIM3->CNT = 0x7FFF;
    TIM3->CR1 |= (1 << TIM_CR1_CEN_Pos);

    for (uint8_t c=0;c< nswitches;c++)
    {
        port = pins[c] >> 4;
        RCC->AHB4ENR |= (1 << port);
        gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);

        // set as input with pullup enabled
        regbfr = gpio->MODER;
        regbfr &= ~(3 << ((pins[c] & 0xF)<<1));
        gpio->MODER=regbfr;    
        regbfr = gpio->PUPDR;
        regbfr &= ~(3 << ((pins[c] & 0xF)<<1));
        regbfr |= (1 << ((pins[c] & 0xF)<<1));
        gpio->PUPDR = regbfr;

        // wire up external interrupts
        regbfr = SYSCFG->EXTICR[((pins[c] &0xF) >>2)];
        regbfr &= ~(0xF << ((pins[c] & 0x3) << 2) );
        regbfr |= ((pins[c] >> 4) << ((pins[c] & 0x3) << 2) );
        SYSCFG->EXTICR[((pins[c] &0xF) >>2)] = regbfr;
        EXTI->IMR1 |= (1 << (pins[c] & 0xF));
        EXTI->FTSR1 |= (1 << (pins[c] & 0xF));
        EXTI->RTSR1 |= (1 << (pins[c] & 0xF));
        switchPins[c]=pins[c];
        switchVals[c]=0;
        enableExternalInterrupt(pins[c]);
    }

    port = POWERSENSE_PIN >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);

    // set as input with pullup enabled
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((POWERSENSE_PIN & 0xF)<<1));
    gpio->MODER=regbfr;    
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((POWERSENSE_PIN & 0xF)<<1));
    //regbfr |= (1 << ((POWERSENSE_PIN & 0xF)<<1));
    gpio->PUPDR = regbfr;

    // wire up external interrupts
    regbfr = SYSCFG->EXTICR[((POWERSENSE_PIN &0xF) >>2)];
    regbfr &= ~(0xF << ((POWERSENSE_PIN & 0x3) << 2) );
    regbfr |= ((POWERSENSE_PIN >> 4) << ((POWERSENSE_PIN & 0x3) << 2) );
    SYSCFG->EXTICR[((POWERSENSE_PIN &0xF) >>2)] = regbfr;
    EXTI->IMR1 |= (1 << (POWERSENSE_PIN & 0xF));
    EXTI->FTSR1 |= (1 << (POWERSENSE_PIN & 0xF));
    enableExternalInterrupt(POWERSENSE_PIN);
    
    oldtickenc=getTickValue();
    oldtickswitch=getTickValue();
    lastTrigger = 0;
}

uint32_t getEncoderValue()
{
    return TIM3->CNT;
}

uint8_t getSwitchValue(uint8_t nr)
{
    return switchVals[nr];
}

void clearPressedStickyBit(uint8_t nr)
{
    switchVals[nr] &= ~(1 << 0);
}

void clearReleasedStickyBit(uint8_t nr)
{
    switchVals[nr] &= ~(1 << 1);
}

int16_t getStickyIncrementDelta()
{
    int16_t nval = (int16_t)((TIM3->CNT>>2) - encoderLastVal);
    return nval;
}

uint8_t getMomentarySwitchValue(uint8_t sw)
{
    GPIO_TypeDef * gpio;
    gpio = (GPIO_TypeDef*)(GPIOA_BASE + (switchPins[sw] >> 4)*0x400);

    if ((gpio->IDR & (1 << (switchPins[sw] & 0xF)))==0)
    {
        return 1;
    }
    return 0;
}

// returns the time passed in us between sticky increments oder decrements
uint32_t getRotaryDeltaT()
{
    if (currentUsVal >= lastUsVal)
    {
        return currentUsVal - lastUsVal;
    }
    return 0xFFFFFFFF;
}

void clearStickyIncrementDelta()
{
    encoderLastVal = TIM3->CNT >> 2;
}
