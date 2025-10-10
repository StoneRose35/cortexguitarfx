extern "C" {
#include "sai.h"
#include "drivers/adc.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "system.h"
#include "timer.h"
#include "audioEngine.h"
#include "pipicofx/pipicofxui.h"
#include "gpio.h"
#include "audio/audiotools.h"
#include "stm32h750/daisy_seed_pins.h"
#include "memoryRegions.h"

#ifdef EXTERNAL_CODEC

static int32_t i2sDoubleBuffer[AUDIO_BUFFER_SIZE*2*2];
#ifdef I2S_INPUT
static int32_t i2sDoubleBufferIn[AUDIO_BUFFER_SIZE*2*2];
#endif
static volatile  uint32_t dbfrPtr; 
static volatile uint32_t dbfrInputPtr;

extern uint32_t task;
extern float avgInOld, avgOutOld;

volatile uint32_t audioState=0;
volatile uint16_t audioTransferState=0;

__QSPI_CODE
void DMA1_Stream0_IRQHandler(void) // adc
{
    if ((DMA1->LISR & DMA_LISR_TCIF0) != 0) // receiver transfer complete
    {
        dbfrInputPtr = AUDIO_BUFFER_SIZE*2;
        DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF0_Pos); 
        audioTransferState += 1;
    }
    if ((DMA1->LISR & DMA_LISR_HTIF0) != 0) // receiver half transfer
    {
        dbfrInputPtr=0;
        DMA1->LIFCR = (1 << DMA_LIFCR_CHTIF0_Pos); 
        audioTransferState += 1;
    }

    if (audioTransferState == 2)
    {
        // jump to audio processing
        processAudioBuffers();
    }
    else
    {
        return;
    }

    // wait for a transmitter flag to be set
    if ((DMA1->LISR & DMA_LISR_TCIF1) != 0)
    {
        dbfrPtr = AUDIO_BUFFER_SIZE*2;
        DMA1->LIFCR = (1 << DMA_LIFCR_CTCIF1_Pos); 
        audioTransferState += 1;
    }
    if ((DMA1->LISR & DMA_LISR_HTIF1) != 0)
    {
        dbfrPtr=0;
        DMA1->LIFCR = (1 << DMA_LIFCR_CHTIF1_Pos); 
        audioTransferState += 1;
    }

    if (audioTransferState == 2)
    {
        // jump to audio processing
        processAudioBuffers();
    }
    else
    {
        return;
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

    //sai2 for adc and dac
    RCC->APB2ENR &= ~(1 << RCC_APB2ENR_SAI2EN_Pos);
    RCC->APB2ENR |= (1 << RCC_APB2ENR_SAI2EN_Pos);

    //configure pins
    config_i2s_pin(I2S_EXT_BCLK,8);
    config_i2s_pin(I2S_EXT_LRCLK,10);
    config_i2s_pin(I2S_EXT_DIN,10);
    config_i2s_pin(I2S_EXT_DOUT,10);
    config_i2s_pin(I2S_EXT_MCLK,10);


    // configure sai2 
    // block a is slave receiver, block b is slave transmitter
    SAI2_Block_A->CR1 = (6 << SAI_xCR1_DS_Pos) // 24 bit data size
                    | (3 << SAI_xCR1_MODE_Pos) // slave receiver
                    | (1 << SAI_xCR1_SYNCEN_Pos) // synchonous with other audio subblock
                    | (1 << SAI_xCR1_CKSTR_Pos) // receive changes on the rising edge
                    | ((2) << SAI_xCR1_MCKDIV_Pos); // clock division for master clock: N/A
    SAI2_Block_A->CR2 = (1 << SAI_xCR2_TRIS_Pos); // sd line becomes hiz after the last bit of the slot
    SAI2_Block_A->FRCR = (1 << SAI_xFRCR_FSDEF_Pos) // fs is start and channel side identification
                        | (1 << SAI_xFRCR_FSOFF_Pos)
                        | ((32-1) << SAI_xFRCR_FSALL_Pos)
                        | ((64 -1) << SAI_xFRCR_FRL_Pos); // fs is asserted one bit before the first slot
    SAI2_Block_A->SLOTR = (3 << SAI_xSLOTR_SLOTEN_Pos)  // enable slot 0 and 1
                        | ((2-1) << SAI_xSLOTR_NBSLOT_Pos) // two slots
                        | (2 << SAI_xSLOTR_SLOTSZ_Pos); // slot size is 32 bit

    SAI2_Block_B->CR1 = (6 << SAI_xCR1_DS_Pos) // 24 bit data size
                    | (2 << SAI_xCR1_MODE_Pos) // slave transmitter
                    | (1 << SAI_xCR1_CKSTR_Pos) // send changes on the rising edge
                    | ((2) << SAI_xCR1_MCKDIV_Pos); // clock division for master clock
    SAI2_Block_B->CR2 = (1 << SAI_xCR2_TRIS_Pos); // sd line becomes hiz after the last bit of the slot
    SAI2_Block_B->FRCR = (1 << SAI_xFRCR_FSDEF_Pos) // fs is start and channel side identification
                        | ((32-1) << SAI_xFRCR_FSALL_Pos)
                        |  ((64 -1) << SAI_xFRCR_FRL_Pos)
                        | (1 << SAI_xFRCR_FSOFF_Pos); // fs is asserted one bit before the first slot
    SAI2_Block_B->SLOTR = (3 << SAI_xSLOTR_SLOTEN_Pos)  // enable slot 0 and 1
                        | ((2-1) << SAI_xSLOTR_NBSLOT_Pos) // two slots
                        | (2 << SAI_xSLOTR_SLOTSZ_Pos); // slot size is 32 bit

    // enable dma for both subblocks
    SAI2_Block_A->CR1 |= (1 << SAI_xCR1_DMAEN_Pos);
    SAI2_Block_B->CR1 |= (1 << SAI_xCR1_DMAEN_Pos);



    // configure DMA streams

    DMA1_Stream0->PAR=(uint32_t)&(SAI2_Block_A->DR);
    DMA1_Stream0->M0AR=(uint32_t)i2sDoubleBufferIn;
    DMA1_Stream0->M1AR=(uint32_t)i2sDoubleBufferIn;
    DMA1_Stream0->CR = (2 << DMA_SxCR_MSIZE_Pos) | (2 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                       (1 << DMA_SxCR_CIRC_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                       (1 << DMA_SxCR_HTIE_Pos);

    DMA1_Stream0->NDTR=AUDIO_BUFFER_SIZE<<2; //samples*2 (stereo),
    DMAMUX1_Channel0->CCR = ((89) << DMAMUX_CxCR_DMAREQ_ID_Pos); //SAI2 A
    


    DMA1_Stream1->PAR=(uint32_t)&(SAI2_Block_B->DR);
    DMA1_Stream1->M0AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream1->M1AR=(uint32_t)i2sDoubleBuffer;
    DMA1_Stream1->CR = (2 << DMA_SxCR_MSIZE_Pos) | (2 << DMA_SxCR_PSIZE_Pos) | (1 << DMA_SxCR_MINC_Pos) | 
                (1 << DMA_SxCR_CIRC_Pos) | (1 << DMA_SxCR_TCIE_Pos) |
                (1 << DMA_SxCR_HTIE_Pos) | (1 << DMA_SxCR_DIR_Pos);
    DMA1_Stream1->NDTR=AUDIO_BUFFER_SIZE<<2; 
    DMAMUX1_Channel1->CCR = ((90) << DMAMUX_CxCR_DMAREQ_ID_Pos); //SAI2 B

    // enable i2s devices

    DMA1_Stream0->CR |= (1 << DMA_SxCR_EN_Pos); 
    DMA1_Stream1->CR |= (1 << DMA_SxCR_EN_Pos);

    SAI2_Block_A->CR1 |= (1 << SAI_xCR1_SAIEN_Pos);
    SAI2_Block_B->CR1 |= (1 << SAI_xCR1_SAIEN_Pos);

    disableAudioEngine();
}

void enableAudioEngine()
{
    NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    audioState |= (1 << AUDIO_STATE_ON);
}
void disableAudioEngine()
{
    NVIC_DisableIRQ(DMA1_Stream0_IRQn);
    audioState &= ~(1 << AUDIO_STATE_ON);
}
void toggleAudioBuffer()
{}
int32_t* getEditableAudioBufferHiRes()
{
    int32_t* res = (dbfrPtr + i2sDoubleBuffer);
    return res;
}
#ifdef I2S_INPUT
int32_t* getInputAudioBufferHiRes()
{
    return (dbfrInputPtr + i2sDoubleBufferIn);
}
void toggleAudioInputBuffer()
{

}
#endif
#endif
}
