
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
uint32_t enterTick=0,exitTick=0,currentTick=0;
#define UI_UPDATE_IN_SAMPLE_BUFFERS 300
#define AVERAGING_LOWPASS_CUTOFF 10
uint8_t treatEnterReleased = 1;
volatile uint16_t secCnt=0;


void checkAvrFirmware()
{
	uint8_t signatureBytes[3];
	uint8_t retval;
	retval = enableAvrProgrammingMode();
	if (retval != 0)
	{
		// blink once: chip not present, entering programming mode failed
		led1.nRepetitions = 0x1;
		led1.sysTicksOn=75;
		led1.sysTicksOff=75;
		resetLedBlinkProgram(&led1);
		led1.state = LED_BLINK_STATE_RUNNING;
		return;
	}

	readSignatureBytes(signatureBytes);
	if (!(signatureBytes[0]==0x1e && signatureBytes[1]==0x93 && signatureBytes[2]==0x7))
	{
		// blink twice: chip present, but wrong signature bytes, possibly wrong atmega version
		led1.nRepetitions = 0x2;
		led1.sysTicksOn=75;
		led1.sysTicksOff=75;
		resetLedBlinkProgram(&led1);
		led1.state = LED_BLINK_STATE_RUNNING;
		return;
	}

	int8_t firmwareMatch = matchAvrFirmwareVersion(AVR_SYNC_NUMBER);
	if (firmwareMatch != 0)
	{
		// blink five times: firmware signature read correctly
		led1.nRepetitions = 0x5;
		led1.sysTicksOn=75;
		led1.sysTicksOff=75;
		resetLedBlinkProgram(&led1);
		led1.state = LED_BLINK_STATE_RUNNING;
		return;
	}
	uint16_t firmwareImage[4096];
	downloadAvrFirmware(firmwareImage);
	uint8_t foundProgrammedHalfword=0;
	for (uint16_t c=0;c<4096;c++)
	{
		if (firmwareImage[c] != 0xFFFF)
		{
			foundProgrammedHalfword = 1;
			break;
		}
	}
	if (foundProgrammedHalfword != 0)
	{
		// blink four times: something programmed found, by signature is incorrect
		led1.nRepetitions = 0x4;
		led1.sysTicksOn=75;
		led1.sysTicksOff=75;
		resetLedBlinkProgram(&led1);
		led1.state = LED_BLINK_STATE_RUNNING;
		return;
	}
	else
	{
		// blink three times: chip is cleared
		led1.nRepetitions = 0x3;
		led1.sysTicksOn=75;
		led1.sysTicksOff=75;
		resetLedBlinkProgram(&led1);
		led1.state = LED_BLINK_STATE_RUNNING;
		return;
	}

}

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
    setPin(CLIPPING_LED_OUTPUT,CLIPPING_LED_POLARITY ^ 1);

	clearPressedStickyBit(0);
	clearReleasedStickyBit(0);
	clearPressedStickyBit(1);
	clearReleasedStickyBit(1);
    /* Loop forever */
	for(;;)
	{
		currentTick = getTickValue();
        switchVals[0] = getSwitchValue(0);
        if ((switchVals[0] & 1) > 0)
        {   // enter pressed 
			enterTick = getTickValue();
			treatEnterReleased = 1;
            clearPressedStickyBit(0);
        }
        if((switchVals[0] & 2) > 0 )
        {
			
			if (treatEnterReleased != 0)
			{
				//enter released
				led1.nRepetitions = 0x8;
				led1.sysTicksOn=15;
				led1.sysTicksOff=15;
				resetLedBlinkProgram(&led1);
				led1.state = LED_BLINK_STATE_RUNNING;
				if (currentTick - enterTick < LONG_PRESS_DURATION)
				{
					clearAvrFlash();
					uploadAvrFirmware((uint16_t*)&_binary___avr_firmware_three_led_lamp_bin_start,((uint32_t)&_binary___avr_firmware_three_led_lamp_bin_end-(uint32_t)&_binary___avr_firmware_three_led_lamp_bin_start)>>1 );
				}
				disableAvrProgrammingMode();
			}
			enterTick = 0;
			clearReleasedStickyBit(0);
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
			led1.nRepetitions = 0xa;
			led1.sysTicksOn=7;
			led1.sysTicksOff=7;
			resetLedBlinkProgram(&led1);
			led1.state = LED_BLINK_STATE_RUNNING;
			if (currentTick - exitTick < LONG_PRESS_DURATION)
			{
				clearAvrFlash();
			}
            clearReleasedStickyBit(1);
			//led1.state = LED_BLINK_STATE_STOPPED;
			disableAvrProgrammingMode();
        }

		if (currentTick - enterTick >= LONG_PRESS_DURATION && treatEnterReleased == 1 && enterTick != 0)
		{
			treatEnterReleased = 0;
			checkAvrFirmware();
			disableAvrProgrammingMode();
		}
	}
}
}


#endif
