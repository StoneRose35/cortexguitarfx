
#include "systemChoice.h"

#ifdef HARDWARE
#ifndef SIMPLE_TIMERTEST
#ifndef SIMPLE_NEOPIXEL

#include "hardware/regs/addressmap.h"
#include "hardware/regs/sio.h"
#include "hardware/rp2040_registers.h"
#include "system.h"
#include "systemClock.h"
#include "systick.h"
#include "uart.h"
#include "consoleBase.h"
#include "dma.h"
#include "pio.h"
#include "adc.h"
#include "timer.h"
#include "gpio.h"
#include "ssd1306_display.h"
#include "wm8731.h"
#include "cs4270_audio_codec.h"
#include "debugLed.h"
#include "consoleHandler.h"
#include "apiHandler.h"
#include "bufferedInputHandler.h"
#include "stringFunctions.h"
#include "rotaryEncoder.h"
#include "cliApiTask.h"
#include "i2s.h"
#include "i2c.h"
#include "audio/simpleChorus.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/firFilter.h"
#include "audio/waveShaper.h"
#include "audio/oversamplingWaveshaper.h"
#include "multicore.h"
#include "core1Main.h"
#include "audio/fxprogram/fxProgram.h"
#include "pipicofx/pipicofxui.h"

volatile uint32_t task=0;
volatile uint8_t context;

extern CommBufferType usbCommBuffer;
extern CommBufferType btCommBuffer;


PiPicoFxUiType piPicoUiController;
uint32_t core1Handshake;
volatile int16_t avgOutOld=0,avgInOld=0;
volatile uint16_t bufferCnt=0;
volatile uint8_t fxProgramIdx = 1;
volatile uint32_t ticStart,ticEnd,cpuLoad;
const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 10



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
	initPio();
	initGpio();
	initTimer();
	initAdc();
	#ifdef WM8731
	initI2c(26);
	#endif
	#ifdef CS4270
	initI2c(72); //72 
	#endif


	/*
	 *
	 * Initialise Component-specific drivers
	 * 
	 * */
	#ifdef WM8731
	setupWm8731(SAMPLEDEPTH_16BIT,SAMPLERATE_48KHZ);
	#endif
	#ifdef CS4270
	setupCS4270();
	#endif

	initSsd1306Display();

	initDebugLed();
	initRotaryEncoder(switchesPins,2);


	/*
     *
     * Initialize Background Services
     *
	 */

	initRoundRobinReading(); // internal adc for reading parameters
	piPicoFxUiSetup(&piPicoUiController);
	ssd1306ClearDisplay();
	for (uint8_t c=0;c<N_FX_PROGRAMS;c++)
	{
		if ((uint32_t)fxPrograms[c]->setup != 0)
		{
			fxPrograms[c]->setup(fxPrograms[c]->data);
		}
	}
	drawUi(&piPicoUiController);


	startCore1(&core1Main);
	// sync with core 1
	while ((*SIO_FIFO_ST & (1 << SIO_FIFO_ST_VLD_LSB)) != (1 << SIO_FIFO_ST_VLD_LSB));
	core1Handshake=*SIO_FIFO_RD;
	while (core1Handshake != 0xcafeface)
	{
		DebugLedOn();
		core1Handshake = *SIO_FIFO_RD;
	}

	ticEnd=0;
	ticStart=0;

	initI2SSlave();

    /* Loop forever */
	for(;;)
	{
		if (bufferCnt >= UI_UPDATE_IN_SAMPLE_BUFFERS)
		{
			bufferCnt = 0;
			task |= (1 << TASK_UPDATE_AUDIO_UI);
		}

	}
}
#endif
#endif
#endif
