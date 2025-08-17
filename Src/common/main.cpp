
#include "systemChoice.h"

#ifdef HARDWARE
extern "C" {
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
#include "system.h"
#include "drivers/core.h"
#include "drivers/systemClock.h"
#include "drivers/datetimeClock.h"
#include "drivers/systick.h"
#include "drivers/uart.h"
//#include "consoleBase.h"
#include "drivers/dma.h"
#include "drivers/pio.h"
#include "drivers/adc.h"
#include "drivers/timer.h"
#include "drivers/gpio.h"
#include "drivers/oled_display.h"
#include "drivers/wm8731.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "drivers/cs4270_audio_codec.h"
#include "drivers/pcm3060.h"
#include "drivers/debugLed.h"
#include "consoleHandler.h"
#include "apiHandler.h"
#include "bufferedInputHandler.h"
#include "stringFunctions.h"
#include "fastExpLog.h"
#include "charDisplay.h"
#include "drivers/rotEncoderSwitchPower.h"
#include "cliApiTask.h"
#include "drivers/i2s.h"
#include "drivers/i2c.h"
#include "drivers/stompswitches.h"
#include "audio/sineplayer.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "drivers/multicore.h"
#include "core1Main.h"
#include "pipicofx/fxPrograms.h"
#include "pipicofx/pipicofxui.h"
#include "pipicofx/FxProgram.hpp"
#include "pipicofx/delayMemoryHandler.h"
#include "globalConfig.h"
#include "stdio.h"


volatile uint32_t task=0;
volatile uint8_t context;


#ifdef USB_UART
CommBufferType usbCommBuffer __attribute__((aligned (256)));
ConsoleType usbConsole;
ApiType usbApi;
BufferedInputType bufferedInput;
#endif


PiPicoFxUiType piPicoUiController;
uint32_t core1Handshake;
volatile int16_t avgOutOld=0,avgInOld=0;
volatile uint16_t bufferCnt=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t ticStart,ticEnd,cpuLoad;
volatile uint8_t programsActivated=0;
const uint8_t stompswitch_progs[]={8,7,1};
volatile uint8_t programToInitialize;
FxPresetType presets[3];
volatile uint8_t currentBank=0;
volatile uint8_t currentPreset=0;

// 0: done
// 1: change request
// 2: fade out
// 3: in bypass / change in progress
// 4: fade in
volatile uint8_t programChangeState;
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 10

volatile uint16_t secCnt=0;

char charbfr[8];
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
	#ifdef STM32
	enableFpu();
	#endif
    setupClock();
	initUsbPll();
	initSystickTimer();
	initDMA();
	initGpio();
	initPio();
	initTimer();
	initAdc();
	#ifdef WM8731
	initI2c(26);
	#endif
	#ifdef CS4270_AUDIO_CODEC
	initI2c(CS4270_I2C_ADDRESS); //72 
	#endif
	#ifdef PCM3060_AUDIO_CODEC
	initI2c(PCM3060_I2C_ADDRESS);
	#endif
	#ifdef WM8731
	setupWm8731(SAMPLEDEPTH_16BIT,SAMPLERATE_48KHZ);
	#endif
	#ifdef CS4270_AUDIO_CODEC
	setupCS4270();
	#endif
	#ifdef PCM3060_AUDIO_CODEC
	setupPCM3060();
	#endif

	initDelayMemoryHandler();
	initDebugLed();
	startCore1(&core1Main);
	// sync with core 1
	while ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB)) != (1 << SIO_FIFO_ST_VLD_LSB));
	core1Handshake=*SIO_FIFO_RD;
	while (core1Handshake != 0xcafeface)
	{
		DebugLedOn();
		core1Handshake = *SIO_FIFO_RD;
	}
	
	#ifdef USB
	initUSB();
	#endif
	#ifdef USB_UART
	initUart(57600,&usbCommBuffer);
	#endif

	/*
	 *
	 * Initialise Component-specific drivers
	 * 
	 * */

	#ifdef USB_UART
	initCliApi(&bufferedInput,NULL,&usbApi,&usbCommBuffer,sendCharAsyncUsb);
	#endif

	initI2SSlave();


	ticEnd=0;
	ticStart=0;
	programToInitialize=0xFF;



    /* Loop forever */
	for(;;)
	{
		/* uncomment to to ui updates independent from audio codec*/
		#ifdef TRIGGER_UI_BY_CORE_0
		waitSysticks(10);
		bufferCnt = UI_UPDATE_IN_SAMPLE_BUFFERS;
		#endif
        
		if (bufferCnt >= UI_UPDATE_IN_SAMPLE_BUFFERS)
		{
			bufferCnt = 0;
			task |= (1 << TASK_UPDATE_AUDIO_UI);
		}
		#ifdef USB_UART
		cliApiTask(&bufferedInput);
		#endif	
	}
}
}


#endif
