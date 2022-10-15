#include "i2s.h"
#include "adc.h"
#include "stm32f446/stm32f446xx.h"
#include "stm32f446/stm32f446_cfg_pins.h"
#include "system.h"
static int32_t i2sDoubleBuffer[AUDIO_BUFFER_SIZE*2*2];
#ifdef I2S_INPUT
static int32_t i2sDoubleBufferIn[AUDIO_BUFFER_SIZE*2*2];
#endif
static volatile  uint32_t dbfrPtr; 
static volatile uint32_t dbfrInputPtr;
volatile uint32_t audioState;
extern uint32_t task;

void DMA2_Stream2_IRQHandler() // adc
{
    if ((task & (1 << TASK_PROCESS_AUDIO_INPUT)) == 0)
    {
        audioState &= ~(1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
    }
    else
    {
        audioState  |= (1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
    }
    if ((DMA2->LISR & DMA_LISR_TCIF2) != 0)
    {
        dbfrInputPtr = AUDIO_BUFFER_SIZE*2;
        DMA2->LIFCR = (1 << DMA_LIFCR_CTCIF2_Pos); 
    }
    else if ((DMA2->LISR & DMA_LISR_HTIF2) != 0)
    {
        dbfrInputPtr=0;
        DMA2->LIFCR = (1 << DMA_LIFCR_CHTIF2_Pos); 
    }
    task |= TASK_PROCESS_AUDIO_INPUT;
}


void DMA1_Stream4_IRQHandler() // dac
{
    if ((task & (1 << TASK_PROCESS_AUDIO)) == 0)
    {
        audioState &= ~(1 << AUDIO_STATE_BUFFER_UNDERRUN);
    }
    else
    {
        audioState  |= (1 << AUDIO_STATE_BUFFER_UNDERRUN);
    }
    if ((DMA1->HISR & DMA_HISR_TCIF4) != 0)
    {
        dbfrPtr = AUDIO_BUFFER_SIZE*2;
        DMA1->HIFCR = (1 << DMA_HIFCR_CTCIF4_Pos); 
    }
    else if ((DMA1->HISR & DMA_HISR_HTIF4) != 0)
    {
        dbfrPtr=0;
        DMA2->HIFCR = (1 << DMA_HIFCR_CHTIF4_Pos); 
    }
    task |= TASK_PROCESS_AUDIO;
}

static void config_i2s_pin(uint8_t pinnr,uint8_t af)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (2 << ((pinnr & 0xF)<<1));
    gpio->MODER=regbfr;
    gpio->PUPDR &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr = gpio->AFR[(pinnr & 0xF)>>3];
    regbfr &= ~(0xF << ((pinnr & 0x7) << 2));
    regbfr |= 5 << ((pinnr & 0x7) << 2);
    gpio->AFR[(pinnr & 0xF)>>3] = regbfr; 
}

void initI2S()
{

    //SPI1 to adc, spi2 to dac
    RCC->APB1ENR |= (1 << RCC_APB1ENR_SPI2EN_Pos);
    RCC->APB2ENR |= (1 << RCC_APB2ENR_SPI1EN_Pos);

    //configure pins
    config_i2s_pin(I2S_BCLK,5);
    config_i2s_pin(I2S_LRCLK,5);
    config_i2s_pin(I2S_BCLK2,5);
    config_i2s_pin(I2S_LRCLK2,5);
    config_i2s_pin(I2S_DIN,5);
    config_i2s_pin(I2S_DOUT,7);


    // configure i2s/spi interface
    // i2s2 is transmitter, i2s1 is  receiver
    SPI1->CR2 |= (1 << SPI_CR2_RXDMAEN_Pos);
    SPI2->CR2 |= (1 << SPI_CR2_TXDMAEN_Pos);
    SPI1->I2SCFGR = (1 << SPI_I2SCFGR_I2SMOD_Pos) | (1 << SPI_I2SCFGR_DATLEN_Pos) | 
                    (1 << SPI_I2SCFGR_CHLEN_Pos) | (1 << SPI_I2SCFGR_I2SCFG_Pos);
    SPI2->I2SCFGR = (1 << SPI_I2SCFGR_I2SMOD_Pos) | (1 << SPI_I2SCFGR_DATLEN_Pos) | 
                    (1 << SPI_I2SCFGR_CHLEN_Pos) | (0 << SPI_I2SCFGR_I2SCFG_Pos); 

    // configure DMA streams
    // ADC: DMA2, Stream 2, Channel 3 (SPI1_RX)
    // DAC: DMA1, Stream 4,Channel 0 (SPI2_TX)
    DMA2_Stream2->PAR=(uint32_t)&(SPI1->DR);
    DMA2_Stream2->M0AR=(uint32_t)i2sDoubleBufferIn;
    DMA2_Stream2->M1AR=(uint32_t)i2sDoubleBufferIn;
    DMA2_Stream2->CR = (2 << DMA_SxCR_MSIZE_Pos) | (1 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                       (1 << DMA_SxCR_CIRC_Pos) | (3 << DMA_SxCR_CHSEL_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                       (1 << DMA_SxCR_HTIE_Pos);
    DMA2_Stream2->NDTR=AUDIO_BUFFER_SIZE<<3; //*2 since SPI->DR is 16bits and memory address is 32bits, *2 (stereo), *2 (double buffer)
    NVIC_EnableIRQ(DMA2_Stream2_IRQn);

    DMA1_Stream4->PAR=(uint32_t)&(SPI2->DR);
    DMA1_Stream4->M0AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream4->M1AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream4->CR = (2 << DMA_SxCR_MSIZE_Pos) | (1 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                (1 << DMA_SxCR_CIRC_Pos) | (0 << DMA_SxCR_CHSEL_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                (1 << DMA_SxCR_HTIE_Pos) | (1 << DMA_SxCR_DIR_Pos);
    DMA1_Stream4->NDTR=AUDIO_BUFFER_SIZE<<3; //*2 since SPI->DR is 16bits and memory address is 32bits, *2 (stereo), *2 (double buffer)
    NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    //enable DMA Streams
    DMA2_Stream2->CR |= (1 << DMA_SxCR_EN_Pos);
    DMA1_Stream4->CR |= (1 << DMA_SxCR_EN_Pos);

    // enable i2s devices
    SPI1->I2SCFGR |= (1 << SPI_I2SCFGR_I2SE_Pos);
    SPI2->I2SCFGR |= (1 << SPI_I2SCFGR_I2SE_Pos);
}

void enableAudioEngine()
{

}
void disableAudioEngine();
void toggleAudioBuffer()
{}
int32_t* getEditableAudioBufferHiRes()
{
    return (int32_t*)dbfrPtr;
}
#ifdef I2S_INPUT
int32_t* getInputAudioBufferHiRes()
{
    return (int32_t*)dbfrInputPtr;
}
void toggleAudioInputBuffer()
{

}
#endif