
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
#include "drivers/timer.h"
#include "drivers/gpio.h"
#include "drivers/display128x64.h"
#include "usb/usb_common.h"
#include "usb/usb_cdc.h"
#include "drivers/debugLed.h"
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
#include "drivers/multicore.h"
#include "core1Main.h"
#include "globalConfig.h"
#include "stdio.h"
#include "ledBlink.h"
#include "avrProgrammer.h"
#include "gen/version.h"
volatile uint32_t task=0;
volatile uint8_t context;


#ifdef USB_UART
CommBufferType usbCommBuffer __attribute__((aligned (256)));
ConsoleType usbConsole;
ApiType usbApi;
BufferedInputType bufferedInput;
#endif


// long press duation in systicks
#define LONG_PRESS_DURATION 200


extern uint32_t  _binary___avr_firmware_three_led_lamp_bin_start;
extern uint32_t  _binary___avr_firmware_three_led_lamp_bin_end;
extern LedBlinkType led1;
const uint8_t switchesPins[2]={ENTER_SWITCH,EXIT_SWITCH};
uint8_t switchVals[2]={0,0};
uint32_t enterTick,exitTick,currentTick;
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 10

volatile uint16_t secCnt=0;

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
	initTimer();


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

    initRotaryEncoder(switchesPins,2);
    initAvrProgrammer();

    setAsOutput(CLIPPING_LED_INPUT);
    setAsOutput(CLIPPING_LED_OUTPUT);
    setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY);
    setPin(CLIPPING_LED_OUTPUT,CLIPPING_LED_POLARITY);

    /* Loop forever */
	for(;;)
	{
		currentTick = getTickValue();
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {   // enter pressed 
			enterTick = getTickValue();
            clearPressedStickyBit(0);
        }
        if((switchVals[0] & 2) > 0)
        {
            //enter released
			led1.nRepetitions = 0xFF;
			led1.sysTicksOn=25;
			led1.sysTicksOff=25;
			resetLedBlinkProgram(&led1);
			led1.state = LED_BLINK_STATE_RUNNING;
			if (currentTick - enterTick < LONG_PRESS_DURATION)
			{
				uploadAvrFirmware((uint16_t*)&_binary___avr_firmware_three_led_lamp_bin_start,((uint32_t)&_binary___avr_firmware_three_led_lamp_bin_end-(uint32_t)&_binary___avr_firmware_three_led_lamp_bin_start)>>1 );
			}
            clearReleasedStickyBit(0);
			led1.state = LED_BLINK_STATE_STOPPED;
        }

        switchVals[1] = getSwitchValue(1);
        if ((switchVals[1] & 1) > 0)
        {
            // exit pressed
			exitTick = getTickValue();

            clearPressedStickyBit(1);
        }
        if ((switchVals[1] & 2) > 0)
        {
            // exit released
			led1.nRepetitions = 0xFF;
			led1.sysTicksOn=15;
			led1.sysTicksOff=15;
			resetLedBlinkProgram(&led1);
			led1.state = LED_BLINK_STATE_RUNNING;
			if (currentTick - exitTick < LONG_PRESS_DURATION)
			{
				clearAvrFlash();
			}
            clearReleasedStickyBit(1);
			led1.state = LED_BLINK_STATE_STOPPED;
        }

		if (currentTick - enterTick >= LONG_PRESS_DURATION && (switchVals[0] & 0x4))
		{
			int8_t firmwareMatch = matchAvrFirmwareVersion(AVR_SYNC_NUMBER);
			if (firmwareMatch != 0)
			{
				// blink three times
				led1.nRepetitions = 0x3;
				led1.sysTicksOn=75;
				led1.sysTicksOff=75;
				resetLedBlinkProgram(&led1);
				led1.state = LED_BLINK_STATE_RUNNING;
			}
		}
	}
}
}


#endif
