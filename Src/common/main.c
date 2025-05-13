
#include "systemChoice.h"

#ifdef HARDWARE



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
#include "adc.h"
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
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "drivers/usb.h"
#include "usb/usb_cdc.h"
#include "usb/usb_dfu.h"

#define LD1 0
#define LD2 7
#define LD3 14

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;

volatile float avgOutOld=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t cpuLoad=0;

const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define ADC_LOWPASS 60
#define UI_DMIN 16
uint32_t encoderVal,encoderCntr,encNew;
int16_t encoderDelta;
volatile uint8_t programsActivated=0;


int16_t avgOldOutBfr;
int16_t avgOldInBfr;
uint16_t cpuLoadBfr;
uint8_t switchVals[2]={0,0};
uint32_t tickStart, tickEnd;

static volatile uint32_t * audioStatePtr;



volatile int16_t *currentSamplePointer;
volatile uint32_t currentSamplePosition;
volatile uint8_t sampleSelectorVal=0;

extern uint32_t  _binary___track001_raw_start;
extern uint32_t  _binary___track001_raw_end;
extern uint32_t  _binary___track002_raw_start;
extern uint32_t  _binary___track002_raw_end;
extern uint32_t  _binary___track003_raw_start;
extern uint32_t  _binary___track003_raw_end;
extern uint32_t  _binary___track004_raw_start;
extern uint32_t  _binary___track004_raw_end;
extern uint32_t  _binary___track005_raw_start;
extern uint32_t  _binary___track005_raw_end;
extern uint32_t  _binary___track006_raw_start;
extern uint32_t  _binary___track006_raw_end;

int16_t * samplePointers[6]={
    (int16_t*)&_binary___track001_raw_start,
    (int16_t*)&_binary___track002_raw_start,
    (int16_t*)&_binary___track003_raw_start,
    (int16_t*)&_binary___track004_raw_start,
    (int16_t*)&_binary___track005_raw_start,
    (int16_t*)&_binary___track006_raw_start};
uint32_t sampleLengths[6];


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
	initDatetimeClock();
	initUart(2000000);
	initDMA();
    initFmcSdram();
    initQspi();
    initUSB();

    initTimer();
	#ifndef PCM3060_CODEC
    initI2c(WM8731_ADDRESS,STOMPSWITCHES_I2C_ADDRESS); // 26 for wm8731, 72 for cs4270, none for pcm3060 (first argument)
    #endif

	//Initialise Component-specific drivers

    initRotaryEncoder(switchesPins,2);

    // wait for flashing when button 0 (Enter switch) is pressed during startup 
    // allows flashing the QSPI from a corrupted state
    uint8_t currentSwitchVal = getMomentarySwitchValue(0);
    if ((currentSwitchVal & 0x01)==1)
    {
        while ((task & (1 << TASK_FLASH_QSPI)) == 0);    
        flashingTask();
        task &= ~(1 << TASK_FLASH_QSPI);
    }


	#ifdef PCM3060_CODEC
    setupPCM3060();
    #endif
    #ifdef WM8731_CODEC
    setupWm8731(SAMPLEDEPTH_24BIT,SAMPLERATE_48KHZ);
    #endif

    initDebugLed();


	context |= (1 << CONTEXT_USB);
	

    // calculate sample lengths
    sampleLengths[0]=((uint32_t)(&_binary___track001_raw_end) - (uint32_t)(&_binary___track001_raw_start)) >> 1;
    sampleLengths[1]=((uint32_t)(&_binary___track002_raw_end) - (uint32_t)(&_binary___track002_raw_start)) >> 1;
    sampleLengths[2]=((uint32_t)(&_binary___track003_raw_end) - (uint32_t)(&_binary___track003_raw_start)) >> 1;
    sampleLengths[3]=((uint32_t)(&_binary___track004_raw_end) - (uint32_t)(&_binary___track004_raw_start)) >> 1;
    sampleLengths[4]=((uint32_t)(&_binary___track005_raw_end) - (uint32_t)(&_binary___track005_raw_start)) >> 1;
    sampleLengths[5]=((uint32_t)(&_binary___track006_raw_end) - (uint32_t)(&_binary___track006_raw_start)) >> 1;
    currentSamplePointer = (int16_t*)0xFFFFFFFF;

    
    //enable audio engine last (when fx programs have been set up)
    initSAI();
    enableAudioEngine();
    
    
    audioStatePtr = getAudioStatePtr();


    #ifdef USB_DBG
    initUart(115200);
    #endif
    /* Loop forever */
    // Enter switch: trigger pedal
    // Exit Switch: Loop Through Sounds
	for(;;)
	{
        uint8_t currentVal;
		//cliApiTask(task);

        if ((task & (1 << TASK_FLASH_QSPI)) != 0)
        {
            flashingTask();
            task &= ~(1 << TASK_FLASH_QSPI);
        }
        
        currentVal = getSwitchValue(0);
        if ((currentVal & 1) != 0)
        {
            clearPressedStickyBit(0);
            currentSamplePointer = samplePointers[sampleSelectorVal];
            currentSamplePosition=0;
        }
        currentVal = getSwitchValue(1);
        if ((currentVal & 1 ) != 0)
        {
            clearPressedStickyBit(1);
            if (sampleSelectorVal < 5)
            {
                sampleSelectorVal++;
            }
            else
            {
                sampleSelectorVal = 0;
            }
            if (currentSamplePosition >= sampleLengths[sampleSelectorVal] || ((uint32_t)currentSamplePointer==0xFFFFFFFF))
            {
                currentSamplePointer = (int16_t*)0xFFFFFFFF;
                currentSamplePosition = 0;
            }
            else
            {
                currentSamplePointer = samplePointers[sampleSelectorVal];
            }
        }
	}
}
#endif
