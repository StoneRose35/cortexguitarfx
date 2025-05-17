#include "globalConfig.h"
#include "system.h"
#include "drivers/dma.h"
#include "drivers/multicore.h"
#include "core1Main.h"
#include "drivers/gpio.h"
#include "drivers/irq.h"
#include "drivers/oled_display.h"
#include "audio/firFilter.h"
#include "audio/audiotools.h"
#include "audio/delay.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "pipicofx/picofxCore.h"
#include "drivers/adc.h"
#include "stringFunctions.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "drivers/stompswitches.h"
#include "drivers/systick.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"


int16_t firstHalfOut;
FirFilterType**core1FirData;
extern volatile uint32_t task;
extern volatile int16_t avgOutOld;
extern volatile int16_t avgInOld;
extern volatile uint8_t fxProgramIdx;
extern volatile uint32_t cpuLoad;
extern const uint8_t stompswitch_progs[];
int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint8_t cpuLoadBfr;
int16_t encoderDelta;
uint8_t switchVals[2]={0,0};
uint8_t stompSwitchState;
FxPresetType preset1, preset2;

extern volatile int16_t *currentSamplePointer;
extern volatile uint32_t currentSamplePosition;
extern volatile uint8_t sampleSelectorVal;
extern int16_t **samplePointers;
extern uint32_t sampleLengths[];

static volatile uint32_t * audioStatePtr;
#define UI_DMIN 1
#define ADC_LOWPASS 2

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};

void isr_c1_sio_irq_proc1_irq16() // only fires when a fir computation has to be made
{
    if ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB))!= 0)
    {
        core1FirData = (FirFilterType**)*SIO_FIFO_RD;
        firstHalfOut = processFirstHalf(*core1FirData);
        *SIO_FIFO_WR = firstHalfOut;
    }
    else if (((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_ROE_LSB)) != 0) || ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_WOF_LSB)) != 0))
    {
        *SIO_FIFO_ST = (1 << 2);
    }
}

void core1Main()
{
    audioStatePtr = getAudioStatePtr();

    initRotaryEncoder(switchesPins,2);

    *SIO_FIFO_ST = (1 << 2);
    *SIO_FIFO_WR=0xcafeface; // write sync word for core 0 to wait for core 1
    *NVIC_ISER = (1 << 16) | (1 << 11); // enable interrupt for dma and sio of proc1 
    //setInterruptPriority(11,1);

    for(;;)
    {

    }
}