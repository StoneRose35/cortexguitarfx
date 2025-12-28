extern "C" {
#include "globalConfig.h"
#include "system.h"
#include "drivers/dma.h"
#include "drivers/multicore.h"
#include "core1Main.h"
#include "drivers/gpio.h"
#include "drivers/irq.h"
#include "drivers/display128x64.h"
#include "avrProgrammer.h"
#include "audio/firFilter.h"
#include "audio/audiotools.h"
#include "audio/delay.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/adc.h"
#include "stringFunctions.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "drivers/stompswitches.h"
#include "drivers/systick.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/regs/dma.h"
#include "hardware/regs/m0plus.h"
#include "hardware/rp2040_registers.h"
#include "consoleBase.h"
#include "gen/version.h"

}
#include "pipicofx/picofxCore.hpp"
#include "pipicofx/FxProgramLoader.hpp"

extern "C" {


void core1Main()
{
    /*
    initSystickTimer();
    
    audioStatePtr = getAudioStatePtr();



    #ifdef EXTENSION_BOARD
    #ifdef AVR_FLASHER
    initAvrProgrammer();
    int8_t firmwareMatch = matchAvrFirmwareVersion(AVR_SYNC_NUMBER);
    if (firmwareMatch == 0)
    {
        clearAvrFlash();
        uploadAvrFirmware((uint16_t*)&_binary___mic_stomp_expansion_board_mic_stomp_bin_start,((uint32_t)&_binary___mic_stomp_expansion_board_mic_stomp_bin_end-(uint32_t)&_binary___mic_stomp_expansion_board_mic_stomp_bin_start)>>1 );
        disableAvrProgrammingMode();
        waitSysticks(10);
    }
    #endif
    #endif

    initDisplay();

     //Initialize Background Services

	
	piPicoFxUiSetup(&piPicoUiController);
	ClearDisplay();
    *DMA_INTE0 |= (1 << 4);

	#ifndef FORCE_TEST_MODE
		enterLevel0(&piPicoUiController);
	#else
	    // switch on program "off"
		piPicoUiController.currentProgramIdx = 2;
		piPicoUiController.currentProgram=loadProgram(piPicoUiController.currentProgramIdx);
	    enterLevel7(&piPicoUiController);
	#endif

    // initalized the rotary encoder and the switches so that core 1 handler the interrupts of the ui elements
    initRotaryEncoder(switchesPins,2);

    initRoundRobinReading(); // internal adc for reading parameters

    setAsOutput(CLIPPING_LED_INPUT);
    setAsOutput(CLIPPING_LED_OUTPUT);
    setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY);
    setPin(CLIPPING_LED_OUTPUT,CLIPPING_LED_POLARITY);

    #ifdef EXTENSION_BOARD
    initStompSwitchesInterface();
    setStompswitchColorRaw(0);
    #endif

    *NVIC_ISER = (1 << 16) | (1 << 23) | (1 << 11); 
    // enable interrupt for sio: FIR filteraudio processing, i2c: stomp extension and dma: display update of proc1     
    setInterruptPriority(11,1);
    *SIO_FIFO_ST = (1 << 2);
    *SIO_FIFO_WR=0xcafeface; // write sync word for core 0 to wait for core 1

    for(;;)
    {        
        if ((task & (1 << TASK_UPDATE_POTENTIOMETER_VALUES)) == (1 << TASK_UPDATE_POTENTIOMETER_VALUES))
        {
            // call the update function of the chosen program
            adcChannel = getChannel0Value();
            adcChannel0 = adcChannel0 + ((ADC_LOWPASS*(adcChannel - adcChannel0)) >> 8);
            if ((adcChannel0 > adcChannelOld0) && (adcChannel0-adcChannelOld0) > UI_DMIN )
            {
                onKnob0(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }
            else if ((adcChannel0 < adcChannelOld0) && (adcChannelOld0-adcChannel0) > UI_DMIN )
            {
                onKnob0(adcChannel0,&piPicoUiController);
                adcChannelOld0=adcChannel0;
            }

            adcChannel = getChannel1Value();
            adcChannel1 = adcChannel1 + ((ADC_LOWPASS*(adcChannel - adcChannel1)) >> 8);
            if ((adcChannel1 > adcChannelOld1) && (adcChannel1-adcChannelOld1) > UI_DMIN )
            {
                onKnob1(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }
            else if ((adcChannel1 < adcChannelOld1) && (adcChannelOld1-adcChannel1) > UI_DMIN )
            {
                onKnob1(adcChannel1,&piPicoUiController);
                adcChannelOld1=adcChannel1;
            }

            adcChannel = getChannel2Value();
            adcChannel2 = adcChannel2 + ((ADC_LOWPASS*(adcChannel - adcChannel2)) >> 8);
            if ((adcChannel2 > adcChannelOld2) && (adcChannel2-adcChannelOld2) > UI_DMIN )
            {
                onKnob2(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel2;
            }
            else if ((adcChannel2 < adcChannelOld2) && (adcChannelOld2-adcChannel) > UI_DMIN )
            {
                onKnob2(adcChannel2,&piPicoUiController);
                adcChannelOld2=adcChannel;
            }
            task &= ~(1 << TASK_UPDATE_POTENTIOMETER_VALUES);
        }
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = avgInOld >> 8;
            avgOldOutBfr = avgOutOld >> 8;
            cpuLoadBfr = (cpuLoad >> 1);
            onUpdate(avgOldInBfr,avgOldOutBfr,cpuLoadBfr,&piPicoUiController);
            #ifndef FORCE_TEST_MODE
            if ((*audioStatePtr & (1 << AUDIO_STATE_INPUT_CLIPPED)) == (1 << AUDIO_STATE_INPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY ^ 1);
                *audioStatePtr &= ~(1 << AUDIO_STATE_INPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY);
            }
            if ((*audioStatePtr & (1 << AUDIO_STATE_OUTPUT_CLIPPED)) == (1 << AUDIO_STATE_OUTPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_OUTPUT,CLIPPING_LED_POLARITY ^ 1);
                *audioStatePtr &= ~(1 << AUDIO_STATE_OUTPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_OUTPUT,CLIPPING_LED_POLARITY);
            }
            #endif
            
            task &= ~(1 << TASK_UPDATE_AUDIO_UI);
        }

        if ((task & (1 << TASK_I2C_DATA_RECEIVED))!=0)
        {

            handleSwitchesUpdate(I2CGetReceivedData());
            task &= ~(1 << TASK_I2C_DATA_RECEIVED);
        }
        
        
         
         // UI Switches Callback
          
        

        
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {
            onEnterPressed(&piPicoUiController);
            clearPressedStickyBit(0);
        }
        if((switchVals[0] & 2) > 0)
        {
            onEnterReleased(&piPicoUiController);
            clearReleasedStickyBit(0);
        }

        switchVals[1] = getSwitchValue(1);
        if ((switchVals[1] & 1) > 0)
        {
            onExitPressed(&piPicoUiController);
            clearPressedStickyBit(1);
        }
        if ((switchVals[1] & 2) > 0)
        {
            onExitReleased(&piPicoUiController);
            clearReleasedStickyBit(1);
        }

        getStickyIncrementAndSpeed(&rotaryEncoderInfo);
        if (rotaryEncoderInfo.increment != 0)
        {
            int32_t d_enc; 
            #ifdef ENCODER_TUNE
            char chrbfr[32];
            chrbfr[0]=0;
            #endif
            if (rotaryEncoderInfo.speed == 0)
            {
                if (rotaryEncoderInfo.increment > 0)
                {
                    d_enc = 1;
                }
                else
                {
                    d_enc = -1;
                }
            }
            else 
            {
                d_enc = rotaryEncoderInfo.increment*rotaryEncoderInfo.speed*ROTARY_ENCODER_SPEED_FACTOR;
                if (d_enc > ROTARY_ENCODER_MAX_INCR)
                {
                    d_enc = ROTARY_ENCODER_MAX_INCR;
                }
                else if (d_enc < -ROTARY_ENCODER_MAX_INCR)
                {
                    d_enc = -ROTARY_ENCODER_MAX_INCR;
                }
                #ifdef ENCODER_TUNE
                appendToString(chrbfr,"d_time: ");
                printf(chrbfr);
                UInt32ToChar(rotaryEncoderInfo.deltaTime,chrbfr);
                printf(chrbfr);
                chrbfr[0]='\r';
                chrbfr[1]='\n';
                chrbfr[2]=0;
                printf(chrbfr);
                #endif
            }
            encoderDelta = (int16_t)d_enc;
            #ifdef ENCODER_TUNE
            appendToString(chrbfr, "enc delta: ");
            printf(chrbfr);
            Int16ToChar(encoderDelta,chrbfr);
            printf(chrbfr);
            chrbfr[0]='\r';
            chrbfr[1]='\n';
            chrbfr[2]=0;
            printf(chrbfr);
            #endif
            onRotaryChange(encoderDelta,&piPicoUiController);
            clearStickyIncrementDelta();
        }
            


        // Stomp Switches Callback

        
        #ifdef EXTENSION_BOARD
        stompSwitchState = getStompSwitchState(0);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(0);
            onStompSwitch1Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(0);
            onStompSwitch1Released(&piPicoUiController);
        }
        stompSwitchState = getStompSwitchState(1);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(1);
            onStompSwitch2Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(1);
            onStompSwitch2Released(&piPicoUiController);
        }
        stompSwitchState = getStompSwitchState(2);
        if ((stompSwitchState & (1 << 1)) != 0) 
        {
            clearStompSwitchStickyPressed(2);
            onStompSwitch3Pressed(&piPicoUiController);
        }
        if ((stompSwitchState & (1 << 2)) != 0)
        {
            clearStompSwitchStickyReleased(2);
            onStompSwitch3Released(&piPicoUiController);
        }


        if (programChangeState == 3)
        {            
            if (programToInitialize != 0xFF)
            {
                delete piPicoUiController.currentProgram;
                piPicoUiController.currentProgram = loadProgram(programToInitialize);
                piPicoUiController.currentParameterIdx = 0;
                piPicoUiController.currentParameter = piPicoUiController.currentProgram->getParameter(piPicoUiController.currentParameterIdx);
                if (piPicoUiController.currentProgram != nullptr)
                {
                    
                    if (currentPreset != 0xFF)
                    {
                        applyPreset(presets+currentPreset,piPicoUiController.currentProgram);
                    }
                    
                    programChangeState = 4;
                }
                onCreate(&piPicoUiController);
            }
            else
            {
                programChangeState = 4;
            }
        }
        #endif
        
    }*/
}

}