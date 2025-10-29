
#ifdef HARDWARE

extern "C" {
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"
#include "globalConfig.h"
#include "system.h"
#include "core.h"
#include "systemClock.h"
#include "systick.h"
#include "datetimeClock.h"
#include "uart.h"
#include "dma.h"
#include "fmc.h"
#include "qspi.h"
#include "drivers/adc.h"
#include "timer.h"
#include "gpio.h"
#include "drivers/oled_display.h"
#include "drivers/display128x64.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "debugLed.h"
#include "consoleHandler.h"
#include "consoleBase.h"
#include "apiHandler.h"
#include "bufferedInputHandler.h"
#include "qspiFlasher.h"
#include "stringFunctions.h"
#include "charDisplay.h"
#include "drivers/stompswitches.h"
#include "cliApiTask.h"
#include "sai.h"
#include "drivers/i2c.h"
#include "drivers/wm8731.h"
#include "pcm3060.h"
#include "memchecker.h"
#include "speedtest_logexp.h"
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/audiotools.h"
#include "pipicofx/delayMemoryHandler.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/usb.h"
#include "usb/usb_cdc.h"
#include "usb/usb_dfu.h"
}
#include "pipicofx/picofxCore.hpp"
#include "pipicofx/FxProgramLoader.hpp"

extern "C" {

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;
extern uint32_t FLASH_SYNC_NUMBER; 
extern uint32_t QSPI_SYNC_NUMBER;

float inputSampleScaled;
volatile float avgOutOld=0,avgInOld=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t cpuLoad=0;
PiPicoFxUiType piPicoUiController;

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define ADC_LOWPASS 60
#define UI_DMIN 8
uint32_t encoderVal,encoderCntr,encNew;
int16_t encoderDelta;
uint8_t enterSwitchVal;
uint8_t exitSwitchVal;
char displayData[128];
uint16_t adcVal;
volatile uint8_t programsActivated=0;
const uint8_t stompswitch_progs[]={8,7,1};
volatile uint8_t programsToInitialize[3];
FxPresetType presets[3];
volatile uint8_t currentBank=0;
volatile uint8_t currentPreset=0xFF;
volatile uint8_t programToInitialize=0xFF;

#ifdef EXTENSION_BOARD
// 0: done
// 1: change request
// 2: fade out
// 3: in bypass / change in progress
// 4: fade in
volatile uint8_t programChangeState=0;
volatile uint8_t stompSwitchState;
#endif


int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint16_t cpuLoadBfr;
uint32_t encoderValOld=0;
uint8_t switchVals[2]={0,0};
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;
uint16_t adcChannel=0;
uint8_t * fb;

uint32_t tickStart, tickEnd;
char chrbfr[16];

static volatile uint32_t * audioStatePtr;


/**
 * @brief the main entry point, should never exit
 * 
 * @return int by definition but should never return a value
 */
int main(void)
{


	/*
	 *
	 * Initialize Hardware components
	 * 
	 * */
    setupClock();
	initSystickTimer();
    configureAndEnableMPU();
    SCB_EnableICache();
    SCB_EnableDCache();
	initUart(2000000);
	initDMA();
    initFmcSdram();
    initDelayMemoryHandler();
    initQspi();
    initUSB();

	initAdc();
    initTimer();
    #ifdef WM8731_CODEC
    initI2c(WM8731_ADDRESS,STOMPSWITCHES_I2C_ADDRESS); // 26 for wm8731, 72 for cs4270, none for pcm3060 (first argument)
    #endif
    #ifdef CS4270_CODEC
    initI2c(CS4270_I2C_ADDRESS,STOMPSWITCHES_I2C_ADDRESS); // 26 for wm8731, 72 for cs4270, none for pcm3060 (first argument)
    #endif
	#ifdef PCM3060_CODEC_EXTERNAL // TODO: still init i2c
    initI2c(0,STOMPSWITCHES_I2C_ADDRESS); // 26 for wm8731, 72 for cs4270, none for pcm3060 (first argument)
    #endif

	//Initialise Component-specific drivers
	initDisplay();

    initRotaryEncoder(switchesPins,2);

    // wait for flashing when button 0 (Enter switch) is pressed during startup 
    // allows flashing the QSPI from a corrupted state
    uint8_t currentSwitchVal = getMomentarySwitchValue(0);
    if ((currentSwitchVal & 0x01)==1 || FLASH_SYNC_NUMBER != QSPI_SYNC_NUMBER)
    {
        while ((task & (1 << TASK_FLASH_QSPI)) == 0);    
        flashingTask();
        task &= ~(1 << TASK_FLASH_QSPI);
    }

    // start memchecker if exit is pressed during startup
    //currentSwitchVal = getMomentarySwitchValue(1);
    //if ((currentSwitchVal & 0x01)==1)
    //{
    //    while (1)
    //    {
    //        testSpeed();
    //    }
    //}

	#ifdef PCM3060_CODEC
    setupPCM3060();
    #endif
    #ifdef WM8731_CODEC
    setupWm8731(SAMPLEDEPTH_24BIT,SAMPLERATE_48KHZ);
    #endif

	encoderVal=getEncoderValue();
    initDebugLed();



    //Initialize Background Services
	//initCliApi();
    #ifdef EXTENSION_BOARD
    initStompSwitchesInterface();
    #endif
    initRoundRobinReading(); // internal adc for reading parameters

    setAsOutput(CLIPPING_LED_INPUT);
    setPin(CLIPPING_LED_INPUT,1);
    setAsOutput(CLIPPING_LED_OUTPUT);
    setPin(CLIPPING_LED_OUTPUT,1);

    #ifdef EXTENSION_BOARD
    setStompswitchColorRaw(0);
    #endif
	context |= (1 << CONTEXT_USB);
	
    piPicoFxUiSetup(&piPicoUiController);
	ClearDisplay();
	#ifndef FORCE_TEST_MODE
		enterLevel0(&piPicoUiController);
	#else
	    // switch on program "off"
		//piPicoUiController.currentProgramIdx = 2;
		//piPicoUiController.currentProgram=loadProgram(piPicoUiController.currentProgramIdx);
	    enterLevel7(&piPicoUiController);
	#endif
    
    
    //enable audio engine last (when fx programs have been set up)
    initSAI();
    enableAudioEngine();
    
    
    audioStatePtr = getAudioStatePtr();
    BwImageType * imgBfr = getImageBuffer();
    fb = imgBfr->data;


    #ifdef USB_DBG
    initUart(115200);
    #endif
    /* Loop forever */
	for(;;)
	{
		//cliApiTask(task);

        if ((task & (1 << TASK_FLASH_QSPI)) != 0)
        {
            flashingTask();
            task &= ~(1 << TASK_FLASH_QSPI);
        }
        

        
		
        if ((task & (1 << TASK_UPDATE_AUDIO_UI)) == (1 << TASK_UPDATE_AUDIO_UI))
        {
            avgOldInBfr = (int32_t)(avgInOld*128.0f);
            avgOldOutBfr = (int32_t)(avgOutOld*128.0f);
            cpuLoadBfr = cpuLoad >> 1;
            onUpdate(avgOldInBfr,avgOldOutBfr,cpuLoadBfr,&piPicoUiController);
            DisplayWriteFramebufferAsync(fb);
            if ((*audioStatePtr & (1 << AUDIO_STATE_INPUT_CLIPPED)) == (1 << AUDIO_STATE_INPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_INPUT,0);
                *audioStatePtr &= ~(1 << AUDIO_STATE_INPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_INPUT,1);
            }
            if ((*audioStatePtr & (1 << AUDIO_STATE_OUTPUT_CLIPPED)) == (1 << AUDIO_STATE_OUTPUT_CLIPPED))
            {
                setPin(CLIPPING_LED_OUTPUT,0);
                *audioStatePtr &= ~(1 << AUDIO_STATE_OUTPUT_CLIPPED);
            }
            else
            {
                setPin(CLIPPING_LED_OUTPUT,1);
            }
            task &= ~(1 << TASK_UPDATE_AUDIO_UI);

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
                restartAdc();
            }
                    

        }

        if ((task & (1 << TASK_DISPLAY_NEXT_LINE)) != 0)
        {
            // wait until transmission through spi is done 
            while ((SPI1->SR & (1 << SPI_SR_TXC_Pos))==0); 
            DisplayWriteNextLine();
            task &= ~(1 << TASK_DISPLAY_NEXT_LINE);
        }
		
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {
            onEnterPressed(&piPicoUiController);
            clearPressedStickyBit(0);
        }
        if ((switchVals[0] & 2) > 0)
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
        encoderDelta=getStickyIncrementDelta();


        if (encoderDelta != 0)
        {
            onRotaryChange(encoderDelta,&piPicoUiController);
            clearStickyIncrementDelta();
        }


        /*
        *
        * Stomp Switches Callback
        * 
       */
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
      if ((task & (1 << TASK_I2C_DATA_RECEIVED))!=0)
      {

        handleSwitchesUpdate(I2CGetReceivedData());
        task &= ~(1 << TASK_I2C_DATA_RECEIVED);
      }
      if ((task & (1 << TASK_PREPARE_FOR_DFU))!=0)
      {
        prepareSystemForDFU();
        task &= ~(1 << TASK_PREPARE_FOR_DFU);
      }
      #endif
	}
}
}
#endif
