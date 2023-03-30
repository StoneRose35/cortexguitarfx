#include "sai.h"
#include "adc.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "system.h"
#include "timer.h"
#include "pipicofx/pipicofxui.h"
#include "debugLed.h"

#define AVERAGING_LOWPASS_CUTOFF 0.0001f
#define UI_UPDATE_IN_SAMPLE_BUFFERS 256

static int32_t i2sDoubleBuffer[AUDIO_BUFFER_SIZE*2*2];
#ifdef I2S_INPUT
static int32_t i2sDoubleBufferIn[AUDIO_BUFFER_SIZE*2*2];
#endif
static volatile  uint32_t dbfrPtr; 
static volatile uint32_t dbfrInputPtr;

extern uint32_t task;
extern float avgInOld, avgOutOld;
extern uint32_t cpuLoad;
extern PiPicoFxUiType piPicoUiController;
uint16_t bufferCnt;
volatile uint32_t audioState=0;

int32_t *  audioBufferPtr;
int32_t *  audioBufferInputPtr;
int32_t inputSampleInt,inputSampleInt2;
float inputSample, avgIn, avgOut;
uint32_t ticStart, ticEnd;

void DMA1_Stream0_IRQHandler(void) // adc
{
    if ((task & (1 << TASK_PROCESS_AUDIO_INPUT)) == 0)
    {
        audioState &= ~(1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
    }
    else
    {
        audioState  |= (1 << AUDIO_STATE_INPUT_BUFFER_OVERRUN);
    }


    if ((DMA1->LISR & DMA_LISR_TCIF0) != 0) // receiver trasfer complete
    {
        dbfrInputPtr = AUDIO_BUFFER_SIZE*2;
        DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF1_Pos); 
    }
    if ((DMA1->LISR & DMA_LISR_HTIF0) != 0) // receiver half transfer
    {
        dbfrInputPtr=0;
        DMA1->LIFCR = (1 << DMA_LIFCR_CHTIF1_Pos); 
    }

    // wait for a trasmitter flag to be set
    while (((DMA1->LISR & DMA_LISR_TCIF1) == 0) && ((DMA1->LISR & DMA_LISR_HTIF1) == 0));
    if ((DMA1->LISR & DMA_LISR_TCIF1) != 0)
    {
        dbfrPtr = AUDIO_BUFFER_SIZE*2;
        DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF1_Pos); 
    }
    if ((DMA1->LISR & DMA_LISR_HTIF1) != 0)
    {
        dbfrPtr=0;
        DMA1->LIFCR = (1 << DMA_LIFCR_CHTIF1_Pos); 
    }
    task |= (1 << TASK_PROCESS_AUDIO_INPUT) | (1 << TASK_PROCESS_AUDIO);

    if (((task & (1 << TASK_PROCESS_AUDIO))!= 0) && ((task & (1 << TASK_PROCESS_AUDIO_INPUT))!= 0))
    {
    
		ticStart = getTimeLW();
        //DebugLedOn();
        audioBufferPtr = getEditableAudioBufferHiRes();
        audioBufferInputPtr = getInputAudioBufferHiRes();
        for (uint32_t c=0;c<AUDIO_BUFFER_SIZE*2;c+=2) // count in frame of 4 bytes or two  24bit samples
        {
            
            // convert raw input to float
            inputSampleInt = *(audioBufferInputPtr + c);
            //inputSampleInt = ((int32_t)((((uint32_t)*(audioBufferInputPtr + c) & 0xFFFF) << 16) 
                              //| (((uint32_t)*(audioBufferInputPtr + c) & 0xFFFF0000L) >> 16))) >> 8;  
                              // flip halfwords, then shift right by 8bits since input data is 24bit left-aligned
            inputSample=(float)inputSampleInt;
            inputSample /= 8388608.0f;
            
    

            if (inputSample < 0.0f)
            {
                avgIn = -inputSample;
            }
            else
            {
                avgIn = inputSample;
            }
            avgInOld = AVERAGING_LOWPASS_CUTOFF*avgIn + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgInOld);

            if (audioState & (1 << AUDIO_STATE_ON))
            {
                inputSample = piPicoUiController.currentProgram->processSample(inputSample,piPicoUiController.currentProgram->data);
            }


            if (inputSample < 0.0f)
            {
                avgOut = -inputSample;
            }
            else
            {
                avgOut = inputSample;
            }
            avgOutOld = AVERAGING_LOWPASS_CUTOFF*avgOut + ((1.0f-AVERAGING_LOWPASS_CUTOFF)*avgOutOld);

            inputSampleInt=((int32_t)(inputSample*8388608.0f));
            //inputSampleInt = (((inputSampleInt << 8) & 0xFFFF) << 16) | (((inputSampleInt << 8) & 0xFFFF0000L) >> 16);
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
        //DebugLedOff();
    }
}


static void config_i2s_pin(uint8_t pinnr,uint8_t af)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB4ENR |= (1 << port);
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

void initSAI()
{

    //sai1 for adc and dac
    RCC->APB2ENR &= ~(1 << RCC_APB2ENR_SAI1EN_Pos);
    RCC->APB2ENR |= (1 << RCC_APB2ENR_SAI1EN_Pos);
    //RCC->APB2ENR |= (1 << RCC_APB2ENR_SPI1EN_Pos);

    //configure pins
    config_i2s_pin(I2S_BCLK,6);
    config_i2s_pin(I2S_LRCLK,6);
    config_i2s_pin(I2S_DIN,6);
    config_i2s_pin(I2S_DOUT,6);
    config_i2s_pin(I2S_MCLK,6);


    // configure sai1 
    // block a is master receiver, block b is slave transmitter
    SAI1_Block_A->CR1 = (6 << SAI_xCR1_DS_Pos) // 24 bit data size
                    | (1 << SAI_xCR1_MODE_Pos) // master receiver
                    | ((25-1) << SAI_xCR1_MCKDIV_Pos); // clock division for master clock
    SAI1_Block_A->CR2 = (1 << SAI_xCR2_TRIS_Pos); // sd line becomes hiz after the last bit of the slot
    SAI1_Block_A->FRCR = (1 << SAI_xFRCR_FSDEF_Pos) // fs is start and channel side identification
                        | (1 << SAI_xFRCR_FSOFF_Pos)
                        | ((64 -1) << SAI_xFRCR_FRL_Pos); // fs is asserted one bit before the first slot
    SAI1_Block_A->SLOTR = (3 << SAI_xSLOTR_SLOTEN_Pos)  // enable slot 0 and 1
                        | ((2-1) << SAI_xSLOTR_NBSLOT_Pos) // two slots
                        | (2 << SAI_xSLOTR_SLOTSZ_Pos); // slot size is 32 bit

    SAI1_Block_B->CR1 = (6 << SAI_xCR1_DS_Pos) // 24 bit data size
                    | (2 << SAI_xCR1_MODE_Pos) // slave transmitter
                    | (1 << SAI_xCR1_SYNCEN_Pos) // synchonous with other audio subblock
                    | ((25-1) << SAI_xCR1_MCKDIV_Pos); // clock division for master clock
    SAI1_Block_B->CR2 = (1 << SAI_xCR2_TRIS_Pos); // sd line becomes hiz after the last bit of the slot
    SAI1_Block_B->FRCR = (1 << SAI_xFRCR_FSDEF_Pos) // fs is start and channel side identification
                        | (1 << SAI_xFRCR_FSOFF_Pos); // fs is asserted one bit before the first slot
    SAI1_Block_B->SLOTR = (3 << SAI_xSLOTR_SLOTEN_Pos)  // enable slot 0 and 1
                        | ((2-1) << SAI_xSLOTR_NBSLOT_Pos) // two slots
                        | (2 << SAI_xSLOTR_SLOTSZ_Pos); // slot size is 32 bit

    // enable dma for both subblocks
    SAI1_Block_A->CR1 |= (1 << SAI_xCR1_DMAEN_Pos);
    SAI1_Block_B->CR1 |= (1 << SAI_xCR1_DMAEN_Pos);



    // configure DMA streams

    DMA1_Stream0->PAR=(uint32_t)&(SAI1_Block_A->DR);
    DMA1_Stream0->M0AR=(uint32_t)i2sDoubleBufferIn;
    DMA1_Stream0->M1AR=(uint32_t)i2sDoubleBufferIn;
    DMA1_Stream0->CR = (2 << DMA_SxCR_MSIZE_Pos) | (2 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                       (1 << DMA_SxCR_CIRC_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                       (1 << DMA_SxCR_HTIE_Pos);

    DMA1_Stream0->NDTR=AUDIO_BUFFER_SIZE<<1; //samples*2 (stereo),
    DMAMUX1_Channel0->CCR = ((87) << DMAMUX_CxCR_DMAREQ_ID_Pos); //SAI1 A
    


    DMA1_Stream1->PAR=(uint32_t)&(SAI1_Block_B->DR);
    DMA1_Stream1->M0AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream1->M1AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream1->CR = (2 << DMA_SxCR_MSIZE_Pos) | (2 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                (1 << DMA_SxCR_CIRC_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                (1 << DMA_SxCR_HTIE_Pos) | (1 << DMA_SxCR_DIR_Pos);
    DMA1_Stream1->NDTR=AUDIO_BUFFER_SIZE<<1; 
    DMAMUX1_Channel1->CCR = ((88) << DMAMUX_CxCR_DMAREQ_ID_Pos); //SAI1 B

    // enable i2s devices
    NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    DMA1_Stream0->CR |= (1 << DMA_SxCR_EN_Pos); 
    DMA1_Stream1->CR |= (1 << DMA_SxCR_EN_Pos);

    SAI1_Block_A->CR1 |= (1 << SAI_xCR1_SAIEN_Pos);
    SAI1_Block_B->CR1 |= (1 << SAI_xCR1_SAIEN_Pos);


    //enableAudioEngine();
}

void enableAudioEngine()
{
    audioState |= (1 << AUDIO_STATE_ON);
}
void disableAudioEngine()
{
    audioState &= ~(1 << AUDIO_STATE_ON);
}
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