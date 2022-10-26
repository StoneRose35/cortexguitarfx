#include "i2s.h"
#include "adc.h"
#include "stm32f446/stm32f446xx.h"
#include "stm32f446/stm32f446_cfg_pins.h"
#include "system.h"
#include "timer.h"
#include "pipicofx/pipicofxui.h"
#define AVERAGING_LOWPASS_CUTOFF 0.0001f
#define UI_UPDATE_IN_SAMPLE_BUFFERS 256

static int32_t i2sDoubleBuffer[AUDIO_BUFFER_SIZE*2*2];
#ifdef I2S_INPUT
static int32_t i2sDoubleBufferIn[AUDIO_BUFFER_SIZE*2*2];
#endif
static volatile  uint32_t dbfrPtr; 
static volatile uint32_t dbfrInputPtr;
volatile uint32_t audioState;
extern uint32_t task;
extern float avgInOld, avgOutOld;
extern uint32_t cpuLoad;
extern PiPicoFxUiType piPicoUiController;
uint16_t bufferCnt;

int32_t *  audioBufferPtr;
int32_t *  audioBufferInputPtr;
int32_t inputSampleInt,inputSampleInt2;
float inputSample, avgIn, avgOut;
uint32_t ticStart, ticEnd;

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
    task |= (1 << TASK_PROCESS_AUDIO_INPUT);

    if (((task & (1 << TASK_PROCESS_AUDIO))!= 0) && ((task & (1 << TASK_PROCESS_AUDIO_INPUT))!= 0))
    {

		ticStart = getTimeLW();
        audioBufferPtr = getEditableAudioBufferHiRes();
        audioBufferInputPtr = getInputAudioBufferHiRes();
        for (uint32_t c=0;c<AUDIO_BUFFER_SIZE*2;c+=2) // count in frame of 4 bytes or two  24bit samples
        {
            
            // convert raw input to float
            inputSampleInt2 = *(audioBufferInputPtr + c);
            inputSampleInt = ((int32_t)((((uint32_t)*(audioBufferInputPtr + c) & 0xFFFF) << 16) 
                              | (((uint32_t)*(audioBufferInputPtr + c) & 0xFFFF0000L) >> 16))) >> 8;  
                              // flip halfwords, then shift right by 8bits since input data is 24bit left-aligned
            inputSample=(float)inputSampleInt;
            
    

            if (inputSample < 0.0f)
            {
                avgIn = -inputSample;
            }
            else
            {
                avgIn = inputSample;
            }
            avgInOld = AVERAGING_LOWPASS_CUTOFF*avgIn + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgInOld);

            inputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);


            if (inputSample < 0.0f)
            {
                avgOut = -inputSample;
            }
            else
            {
                avgOut = inputSample;
            }
            avgOutOld = AVERAGING_LOWPASS_CUTOFF*avgOut + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgOutOld);

            inputSampleInt=((int32_t)inputSample);
            inputSampleInt = (((inputSampleInt << 8) & 0xFFFF) << 16) | (((inputSampleInt << 8) & 0xFFFF0000L) >> 16);
            *(audioBufferPtr+c) = inputSampleInt;  
            *(audioBufferPtr+c+1) = inputSampleInt;
        }
        task &= ~((1 << TASK_PROCESS_AUDIO) | (1 << TASK_PROCESS_AUDIO_INPUT));
        bufferCnt++;
        if (bufferCnt == UI_UPDATE_IN_SAMPLE_BUFFERS)
		{
			bufferCnt = 0;
			task |= (1 << TASK_UPDATE_AUDIO_UI);
		}

        ticEnd = getTimeLW();
		if(ticEnd > ticStart)
		{
			cpuLoad = ticEnd-ticStart;
			cpuLoad = cpuLoad*196; // *256*256*F_SAMPLING/AUDIO_BUFFER_SIZE/1000000;
			cpuLoad = cpuLoad >> 8;
		}
    }
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
        DMA1->HIFCR = (1 << DMA_HIFCR_CHTIF4_Pos); 
    }
    task |= (1 << TASK_PROCESS_AUDIO);
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
    regbfr |= af << ((pinnr & 0x7) << 2);
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
    int32_t* res = (int32_t*)((dbfrPtr<<2) + (uint32_t)i2sDoubleBuffer);
    return res;
}
#ifdef I2S_INPUT
int32_t* getInputAudioBufferHiRes()
{
    return (int32_t*)((dbfrInputPtr<<2) + (uint32_t)i2sDoubleBufferIn);
}
void toggleAudioInputBuffer()
{

}
#endif