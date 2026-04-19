#include "drivers/adc.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "system.h"

extern uint32_t task;

volatile uint16_t adcChannelValues[3];
volatile uint16_t adcChannelValuesOld[3];

void DMA1_Stream2_IRQHandler() //raised when all 3 adc values have been read
{
    DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF2_Pos);
    adcChannelValuesOld[0] += ((ADC_LOWPASS*(adcChannelValues[0]-adcChannelValuesOld[0]))>>10);
    adcChannelValuesOld[1] += ((ADC_LOWPASS*(adcChannelValues[1]-adcChannelValuesOld[1]))>>10);
    adcChannelValuesOld[2] += ((ADC_LOWPASS*(adcChannelValues[2]-adcChannelValuesOld[2]))>>10);
    restartAdc();
    task |= (1 << TASK_UPDATE_POTENTIOMETER_VALUES);
}

/**
 * @brief Set the encoded sample cycles for a given adc channel for adc1
 * 
•SMP = 000: 1.5 ADC clock cycles
•SMP = 001: 2.5 ADC clock cycles
•SMP = 010: 8.5 ADC clock cycles
•SMP = 011: 16.5 ADC clock cycles
•SMP = 100: 32.5 ADC clock cycles
•SMP = 101: 64.5 ADC clock cycles
•SMP = 110: 387.5 ADC clock cycles
•SMP = 111: 810.5 ADC clock cycles
 * @param channel 
 */
inline void setSampleCycles(uint8_t cycles,uint8_t channel)
{
    if (channel < 10)
    {
        ADC1->SMPR1 |= (cycles << (channel*3));
    }
    else
    {
        ADC1->SMPR2 |= ( cycles << ((channel-10)*3));
    }
}

void initAdc()
{
    GPIO_TypeDef * gpio;
    uint32_t port;

    RCC->AHB1ENR|= (1 << RCC_AHB1ENR_ADC12EN_Pos);
    SYSCFG->PMCR |= (1 << SYSCFG_PMCR_BOOSTEN_Pos);
    // power up sequence
    ADC1->CR &= ~(1 << ADC_CR_DEEPPWD_Pos);
    ADC1->CR |= (1 << ADC_CR_ADVREGEN_Pos);
    while((ADC1->ISR & (1 << 12))==0);
    ADC1->SQR1 = (2 << ADC_SQR1_L_Pos); // 3 conversions per sequence
    ADC1->SQR1 |= (POT1_CHANNEL << 6) | (POT2_CHANNEL << 12) | (POT3_CHANNEL << 18);
    ADC1->PCSEL = (1 << POT1_CHANNEL) | (1 << POT2_CHANNEL) | (1 << POT3_CHANNEL);
    setSampleCycles(4,POT1_CHANNEL);
    setSampleCycles(4,POT2_CHANNEL);
    setSampleCycles(4,POT3_CHANNEL);    

    // calibrate the adc
    ADC1->CR &= ~(1 << ADC_CR_ADCALDIF_Pos); 
    ADC1->CR |= (1 << ADC_CR_ADCALLIN_Pos);
    ADC1->CR |= (1 << ADC_CR_ADCAL_Pos);
    while ((ADC1->CR & (1 << ADC_CR_ADCAL_Pos))!=0);

    // enable the port to which the pots are attached
    // and set mode to analog
    port = POT1 >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->MODER |= (3 << ((POT1 & 0xF)<<1));
    gpio->PUPDR &= ~(3 << ((POT1 & 0xF)<< 1));

    port = POT2 >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->MODER |= (3 << ((POT2 & 0xF)<<1));
    gpio->PUPDR &= ~(3 << ((POT2 & 0xF)<< 1));

    port = POT3 >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->MODER |= (3 << ((POT3 & 0xF)<<1));
    gpio->PUPDR &= ~(3 << ((POT3 & 0xF)<< 1));

   adcChannelValuesOld[0]=0;
   adcChannelValuesOld[1]=0;
   adcChannelValuesOld[2]=0;
}

void initRoundRobinReading()
{

    // setup dma 1 channel 0 stream 2
    DMA1_Stream2->PAR=(uint32_t)&(ADC1->DR);
    DMA1_Stream2->M0AR=(uint32_t)adcChannelValues;
    DMA1_Stream2->M1AR=(uint32_t)adcChannelValues;
    DMA1_Stream2->CR = (1 << DMA_SxCR_MSIZE_Pos) | (1 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | (1 << DMA_SxCR_TCIE_Pos) | (1 << DMA_SxCR_CIRC_Pos);
    DMA1_Stream2->NDTR=3;
    DMAMUX1_Channel2->CCR = ((9) << DMAMUX_CxCR_DMAREQ_ID_Pos);
    NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    DMA1_Stream2->CR |=(1 << DMA_SxCR_EN_Pos);

    // no external trigger, DMA Circular Mode
    ADC1->CFGR = (1 << ADC_CFGR_JQDIS_Pos) | (3 << ADC_CFGR_DMNGT_Pos);
    ADC1->CR |= (1 << ADC_CR_ADEN_Pos);
    while ((ADC1->ISR & (1 << ADC_ISR_ADRDY_Pos))==0);
    ADC1->CR |= (1 << ADC_CR_ADSTART_Pos);
}

void restartAdc()
{

    ADC1->CR |= (1 << ADC_CR_ADSTART_Pos);
}

uint16_t getChannel0Value()
{
    return adcChannelValuesOld[0]>>4;
}

uint16_t getChannel1Value()
{
    return adcChannelValuesOld[1]>>4;
}

uint16_t getChannel2Value()
{
    return adcChannelValuesOld[2]>>4;
}