
#ifdef HARDWARE
#define SYNC_NUMBERS
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
#include "drivers/24lc128.h"
#include "avrProgrammer.h"
#include "pcm3060.h"
#include "memchecker.h"
#include "speedtest_logexp.h"
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "audio/looper.h"
#include "audio/audiotools.h"
#include "audioEngine.h"
#include "pipicofx/delayMemoryHandler.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/usb.h"
#include "usb/usb_cdc.h"
#include "usb/usb_dfu.h"
#include "gen/version.h"
#include "flash.h"
}
#include "pipicofx/picofxCore.hpp"
#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/MultiAudioProcessor.hpp"

extern "C" {

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;

extern MultiAudioProcessor audioProcessor; 

extern uint32_t _binary___mic_stomp_expansion_board_mic_stomp_bin_start;
extern uint32_t _binary___mic_stomp_expansion_board_mic_stomp_bin_end;



float inputSampleScaled;
volatile float avgOutOld=0,avgInOld=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t cpuLoad=0;
PiPicoFXUiType ui;

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define ADC_LOWPASS 60
#define UI_DMIN 2
uint32_t encoderVal,encoderCntr,encNew;
int16_t encoderDelta;
RotaryEncoderIncrementType rotaryEncoderInfo;
uint8_t enterSwitchVal;
uint8_t exitSwitchVal;
uint16_t adcVal;
volatile uint8_t programsActivated=0;
const uint8_t stompswitch_progs[]={8,7,1};
FxPresetType presets[3];
volatile uint8_t currentBank=0;
volatile uint8_t currentPreset=0x0;
volatile uint8_t programsToInitialize[3]={0xFF,0xFF,0xFF}; // 0xFF: null values, otherwise bits 0-6: program nr to initialize, bit 7: copy parameters from preset or not
volatile uint16_t initialKnobValues[3];
AudioProcessor * currentFxProgram;
__DTCM_DATA
MultiAudioProcessor audioProcessor;
LooperDataType looper;
#ifdef EXTENSION_BOARD
// 0: done
// 1: change request
// 2: fade out
// 3: in bypass / change in progress
// 4: fade in
volatile uint8_t programChangeState=0;
volatile uint8_t stompSwitchState;
#define ROTARY_ENCODER_MAX_INCR 512
#define ROTARY_ENCODER_SPEED_FACTOR 32
#endif


int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint16_t cpuLoadBfr;
uint8_t switchVals[2]={0,0};
uint16_t adcChannelOld0=0,adcChannel0=0;
uint16_t adcChannelOld1=0,adcChannel1=0;
uint16_t adcChannelOld2=0,adcChannel2=0;

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
	//initUart(2000000);
	initDMA();
    initFmcSdram();
    initDelayMemoryHandler();
    initQspi();
    initUSB();


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
        uint8_t lockStartup= 1;
        while(lockStartup ==1)
        {   
            if ((task & (1 << TASK_FLASH_QSPI)) != 0)
            {
                flashingTask();
                task &= ~(1 << TASK_FLASH_QSPI);
                lockStartup = 0;
            }
            if ((task & (1 << TASK_PREPARE_FOR_DFU))!=0)
            {
                prepareSystemForDFU();
                task &= ~(1 << TASK_PREPARE_FOR_DFU);
            }
        }
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
	
    initAudioEngine();
    piPicoFxUiSetup();
	ClearDisplay();
	#ifndef FORCE_TEST_MODE
		enterLevel0();
	#else
	    // switch on program "off"
		//piPicoUiController.currentProgramIdx = 2;
		//piPicoUiController.currentProgram=loadProgram(piPicoUiController.currentProgramIdx);
	    enterLevel7(&piPicoUiController);
	#endif
    
    LooperInit(&looper);
    
    //enable audio engine last (when fx programs have been set up)
    initSAI();
    enableAudioEngine();
    
    audioStatePtr = getAudioStatePtr();

    clearReleasedStickyBit(0);
    #if defined USB_DBG || defined ENCODER_TUNE
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
            onUpdate(avgOldInBfr,avgOldOutBfr,cpuLoadBfr);
            DisplayWriteFramebufferAsync(getImageBuffer()->data);
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
                adcChannel0 = getChannel0Value();
                //adcChannel0 = adcChannel0 + ((ADC_LOWPASS*(adcChannel - adcChannel0)) >> 8);
                if ((adcChannel0 > adcChannelOld0) && (adcChannel0-adcChannelOld0) > UI_DMIN )
                {
                    onKnob0(adcChannel0);
                    adcChannelOld0=adcChannel0;
                }
                else if ((adcChannel0 < adcChannelOld0) && (adcChannelOld0-adcChannel0) > UI_DMIN )
                {
                    onKnob0(adcChannel0);
                    adcChannelOld0=adcChannel0;
                }
    
                adcChannel1 = getChannel1Value();
                //adcChannel1 = adcChannel1 + ((ADC_LOWPASS*(adcChannel - adcChannel1)) >> 8);
                if ((adcChannel1 > adcChannelOld1) && (adcChannel1-adcChannelOld1) > UI_DMIN )
                {
                    onKnob1(adcChannel1);
                    adcChannelOld1=adcChannel1;
                }
                else if ((adcChannel1 < adcChannelOld1) && (adcChannelOld1-adcChannel1) > UI_DMIN )
                {
                    onKnob1(adcChannel1);
                    adcChannelOld1=adcChannel1;
                }
    
                adcChannel2 = getChannel2Value();
                //adcChannel2 = adcChannel2 + ((ADC_LOWPASS*(adcChannel - adcChannel2)) >> 8);
                if ((adcChannel2 > adcChannelOld2) && (adcChannel2-adcChannelOld2) > UI_DMIN )
                {
                    onKnob2(adcChannel2);
                    adcChannelOld2=adcChannel2;
                }
                else if ((adcChannel2 < adcChannelOld2) && (adcChannelOld2-adcChannel2) > UI_DMIN )
                {
                    onKnob2(adcChannel2);
                    adcChannelOld2=adcChannel2;
                }
                task &= ~(1 << TASK_UPDATE_POTENTIOMETER_VALUES);
                //restartAdc();
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
            onEnterPressed();
            clearPressedStickyBit(0);
        }
        if ((switchVals[0] & 2) > 0)
        {
            onEnterReleased();
            clearReleasedStickyBit(0);
        }

        switchVals[1] = getSwitchValue(1);
        if ((switchVals[1] & 1) > 0)
        {
            onExitPressed();
            clearPressedStickyBit(1);
        }
        if ((switchVals[1] & 2) > 0)
        {
            onExitReleased();
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
            onRotaryChange(encoderDelta);
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
          onStompSwitch1Pressed();
      }
      if ((stompSwitchState & (1 << 2)) != 0)
      {
          clearStompSwitchStickyReleased(0);
          onStompSwitch1Released();
      }
      stompSwitchState = getStompSwitchState(1);
      if ((stompSwitchState & (1 << 1)) != 0) 
      {
          clearStompSwitchStickyPressed(1);
          onStompSwitch2Pressed();
      }
      if ((stompSwitchState & (1 << 2)) != 0)
      {
          clearStompSwitchStickyReleased(1);
          onStompSwitch2Released();
      }
      stompSwitchState = getStompSwitchState(2);
      if ((stompSwitchState & (1 << 1)) != 0) 
      {
          clearStompSwitchStickyPressed(2);
          onStompSwitch3Pressed();
      }
      if ((stompSwitchState & (1 << 2)) != 0)
      {
          clearStompSwitchStickyReleased(2);
          onStompSwitch3Released();
      }

      

      // remove all entries and replaces them by new one 
      // when programsToInitialize is not 0xFF at the given position
      // updates the currently edited parameter of the ui structure
        if (programChangeState == 3)
        {         
            for (uint8_t q = 0;q < 3;q++)
            {   
                if ((programsToInitialize[q] & 0x3F) != 0x3F)
                {
                    currentFxProgram = audioProcessor.removeFxProgram(q);
                    if (currentFxProgram != nullptr)
                    {
                        if (q == ui.currentProgramPosition)
                        {
                            ui.currentProgram = 0;
                            ui.currentParameter = 0;
                            ui.currentParameterIdx = 0;
                        }
                        delete currentFxProgram; 
                        currentFxProgram = nullptr;
                    }


                    currentFxProgram = loadProgram(programsToInitialize[q]&0x7F);
                    audioProcessor.addFxProgram(currentFxProgram,q);
                    if (ui.defaultOn)
                    {
                        ((FxProgram*)currentFxProgram)->switchOn();
                    }
                    else
                    {
                        ((FxProgram*)currentFxProgram)->switchOff();
                    }
                    
                    ui.currentParameterIdx = 0;
                    if (q == ui.currentProgramPosition)
                    {
                        ui.currentProgram = ((FxProgram*)currentFxProgram);
                        ui.currentProgramIdx = programsToInitialize[q]&0x7F;
                        ui.currentParameter = ((FxProgram*)currentFxProgram)->getParameter(ui.currentParameterIdx);
                    }

                    if (programsToInitialize[q] & 0x80)
                    {
                        applyPresetToProgram(presets+currentPreset,&audioProcessor,q);
                    }
                    //onCreate();
                }
            }
            //applyPreset(presets+currentPreset,&audioProcessor);
            programChangeState = 4;
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
