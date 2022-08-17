#include "system.h"
#include "dma.h"
#include "multicore.h"
#include "core1Main.h"
#include "ssd1306_display.h"
#include "audio/firFilter.h"
#include "audio/fxprogram/fxProgram.h"
#include "pipicofx/pipicofxui.h"
#include "adc.h"
#include "rotaryEncoder.h"
//#include "i2s.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"


int16_t firstHalfOut;
FirFilterType**core1FirData;
extern volatile uint32_t task;
extern volatile int16_t avgOutOld,avgInOld;
extern volatile uint8_t fxProgramIdx;
extern volatile uint32_t cpuLoad;
extern volatile uint32_t audioState;
int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint8_t cpuLoadBfr;
uint8_t switchValsOld[2]={0,0};
uint32_t encoderValOld=0,encoderVal;
uint8_t switchVals[2]={0,0};
//PiPicoFxUiType uiControllerData;
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;
uint16_t adcChannel=0;
#define UI_DMIN 1
#define ADC_LOWPASS 2

void isr_sio_irq_proc1_irq16() // only fires when a fir computation has to be made
{
    if ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB))!= 0)
    {
        core1FirData = (FirFilterType**)*SIO_FIFO_RD;
        firstHalfOut = processFirstHalf(*core1FirData);
        *SIO_FIFO_WR = firstHalfOut;
        //*SIO_FIFO_ST = (1 << 2);
    }
    else if (((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_ROE_LSB)) != 0) || ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_WOF_LSB)) != 0))
    {
        *SIO_FIFO_ST = (1 << 2);
    }
}

void core1Main()
{

    *SIO_FIFO_ST = (1 << 2);
    *SIO_FIFO_WR=0xcafeface; // write sync word for core 0 to wait for core 1
    *NVIC_ISER = (1 << 16); //enable interrupt for adc and sio of proc1 

    encoderValOld=getEncoderValue();
    for(;;)
    {
        if ((task & (1 << TASK_UPDATE_POTENTIOMETER_VALUES)) == (1 << TASK_UPDATE_POTENTIOMETER_VALUES))
        {
            // call the update function of the chosen program
            adcChannel = getChannel0Value();
            adcChannel0 = adcChannel0 + ((ADC_LOWPASS*(adcChannel - adcChannel0)) >> 8);
            if ((adcChannel0 > adcChannelOld0) && (adcChannel0-adcChannelOld0) > UI_DMIN )
            {
                knob0Callback(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }
            else if ((adcChannel0 < adcChannelOld0) && (adcChannelOld0-adcChannel0) > UI_DMIN )
            {
                knob0Callback(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }

            adcChannel = getChannel1Value();
            adcChannel1 = adcChannel1 + ((ADC_LOWPASS*(adcChannel - adcChannel1)) >> 8);
            if ((adcChannel1 > adcChannelOld1) && (adcChannel1-adcChannelOld1) > UI_DMIN )
            {
                knob1Callback(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }
            else if ((adcChannel1 < adcChannelOld1) && (adcChannelOld1-adcChannel1) > UI_DMIN )
            {
                knob1Callback(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }

            adcChannel = getChannel2Value();
            adcChannel2 = adcChannel2 + ((ADC_LOWPASS*(adcChannel - adcChannel2)) >> 8);
            if ((adcChannel2 > adcChannelOld2) && (adcChannel2-adcChannelOld2) > UI_DMIN )
            {
                knob2Callback(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel2;
            }
            else if ((adcChannel2 < adcChannelOld2) && (adcChannelOld2-adcChannel) > UI_DMIN )
            {
                knob2Callback(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel;
            }
            task &= ~(1 << TASK_UPDATE_POTENTIOMETER_VALUES);
            //*ADC_CS |= (1 << ADC_CS_START_MANY_LSB);
        }
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = avgInOld >> 8;
            avgOldOutBfr = avgOutOld >> 8;
            cpuLoadBfr = (cpuLoad >> 1);
            updateAudioUi(avgOldInBfr,avgOldOutBfr,cpuLoadBfr,&piPicoUiController);
            task &= ~(1 << TASK_UPDATE_AUDIO_UI);
        }
        switchVals[0] = getSwitchValue(0);
        if (switchVals[0] == 0 && switchValsOld[0] == 1)
        {
            button1Callback(&piPicoUiController);
        }
        switchValsOld[0] = getSwitchValue(0);

        switchVals[1] = getSwitchValue(1);
        if (switchVals[1] == 0 && switchValsOld[1] == 1)
        {
            button2Callback(&piPicoUiController);
        }
        switchValsOld[1] = getSwitchValue(1);
       encoderVal=getEncoderValue();
       if (encoderValOld > encoderVal + 2 || encoderValOld < encoderVal - 2)
       {
           rotaryCallback(encoderVal,&piPicoUiController);
           encoderValOld=encoderVal;
       }
    }
}