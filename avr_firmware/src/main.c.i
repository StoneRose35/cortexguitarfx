#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>

__attribute__((section(".firmwaresig")))
const volatile uint32_t firmwareVersion=<FIRMWARE_VERSION>;



#define I2C_ADDRESS 23

#define SW_1 0 //PB0
#define SW_2 7 //PD7
#define SW_3 6 //PD6
#define SW_4 5 //PD5

#define MORSE_SHORT_ON 5
#define MORSE_LONG_ON 60
#define MORSE_PULSE_DURATION 120
#define MORSE_BREAK_SYMBOL 15
#define MORSE_BREAK_CHARACTER 83
#define MORSE_BREAK_WORD 180

volatile uint16_t hueshift_val=0;
volatile uint8_t brightness_val=0;
volatile uint8_t pothaeuer=0, potschnaeuer=0; 
volatile int8_t timer0cycles=0;
volatile uint8_t sendMorseCode=0;
volatile uint16_t ticksCnt=0;

void setRed(int val);
void setGreen(int val);
void setBlue(int val);

const uint8_t morseCodes[] ={0x20 | 0b10, 
                            0x40 | 0b0001, 
							0x40 | 0b0101,
							0x30 | 0b001,
							0x10 | 0b0,
							0x40 | 0b0100, // f
							0x30 | 0b011,
							0x40 | 0b0000,
							0x20 | 0b00,
							0x40 | 0b1110,
							0x30 | 0b101, // k 
							0x40 | 0b0010,
							0x20 | 0b11,
							0x20 | 0b01,
							0x30 | 0b111,
							0x40 | 0b0110, // p
							0x40 | 0b1011,
							0x30 | 0b010,
							0x30 | 0b000, // s
							0x10 | 0b1,
							0x30 | 0b100,
							0x40 | 0b1000,
							0x30 | 0b110, // w
							0x40 | 0b1001,
							0x40 | 0b1101,
							0x40 | 0b0011
							};

void setup()
{
	// start timers, set output compare outs
	DDRB = (1 << 1) | (1 << 2) | (1 << 3);
	// clear on bottom, set on compare match
	TCCR1A = (3 << COM1A0) | (3 << COM1B0) | (5 << WGM10);
	setRed(0);
	setGreen(0);
	TCCR1B=1;
	
	// fast pwm, clear on match
	setBlue(0);
	// fast pwm, clear on match
	TCCR2=(3 << WGM20) | (3 << COM20) | (1);
	
	// initialize adc
	ADMUX = (3 << REFS0) | (1 << ADLAR);
	ADCSRA = (1 << ADEN) | (3 << ADPS0);
	
	// use timer0 as time basis interrupting at 61.035 Hz for led update
	TCCR0 = 3;
	TIMSK |= (1 << TOIE0); 
	
	// define optional switches
	DDRB &= ~(1 << SW_1);
	DDRD &= ~((1 << SW_2) | (1 << SW_3) | (1 << SW_4));
	PORTB |= (1 << SW_1);
	PORTD |= (1 << SW_2) | (1 << SW_3) | (1 << SW_4); 

	sei();
}

void setRed(int val)
{
	OCR1A=val;	
}

void setBlue(int val)
{
	OCR1B=val;
}

void setGreen(int val)
{
	OCR2=val;
}

void applyHueBrightness(uint16_t hueshift_val,uint8_t hueshift_brightness)
{
	uint16_t red_val=0, green_val=0, blue_val=0;
	uint16_t valL;
	if (hueshift_val < 0x100)
			{
				red_val = hueshift_val;
				blue_val = 0xFF;
				green_val = 0x0;
			}	
			else if (hueshift_val < 0x200)
			{
				red_val = 0xFF;
				blue_val = 0xFF - (hueshift_val-0x100);
				green_val = 0x0;
			}
			else if (hueshift_val < 0x300)
			{
				red_val = 0xFF;
				blue_val = 0x0;
				green_val = (hueshift_val - 0x200);
			}
			else if (hueshift_val < 0x400)
			{
				red_val = 0xFF - (hueshift_val - 0x300);
				blue_val = 0x0;
				green_val = 0xFF;
			}
			else if (hueshift_val < 0x500)
			{
				red_val = 0x0;
				blue_val = (hueshift_val - 0x400);
				green_val = 0xFF;
			}
			else if (hueshift_val < 0x600)
			{
				red_val = 0x0;
				blue_val = 0xFF;
				green_val = 0xFF - (hueshift_val - 0x500);
			}

			valL=red_val*hueshift_brightness;
			valL >>= 8;
			setRed((uint8_t)valL);
			valL=blue_val*hueshift_brightness;
			valL >>= 8;
			setBlue((uint8_t)valL);
			valL=green_val*hueshift_brightness;
			valL >>= 8;
			setGreen((uint8_t)valL);
}


void waitForTicks(uint16_t nTicks)
{
	ticksCnt = 0;
	while (ticksCnt < nTicks);
	
}


void shortMorsePulse()
{
	setRed(0x0);
	setBlue(0x0);
	setGreen(0xFF);
	waitForTicks(MORSE_SHORT_ON);
	setRed(0x0);
	setBlue(0x0);
	setGreen(0x0);
	waitForTicks(MORSE_PULSE_DURATION-MORSE_SHORT_ON);
}

void longMorsePulse()
{
	setRed(0x0);
	setBlue(0x0);
	setGreen(0xFF);
	waitForTicks(MORSE_LONG_ON);
	setRed(0x0);
	setBlue(0x0);
	setGreen(0x0);
	waitForTicks(MORSE_PULSE_DURATION - MORSE_LONG_ON);
}

void sendMorseCharacter(uint8_t c)
{
	uint8_t mc = *(morseCodes + c);
	uint8_t mlength = (mc & 0xF0) >> 4;
	uint8_t msequence = mc & 0xF;

	for (uint8_t cnt=0;cnt< mlength;cnt++)
	{
		if (msequence & 0x1)
		{
			longMorsePulse();
		}
		else
		{
			shortMorsePulse();
		}
		msequence >>= 1;
	}
	setBlue(0xFF);
	waitForTicks(MORSE_SHORT_ON);
	setBlue(0x0);
	waitForTicks(MORSE_PULSE_DURATION-MORSE_SHORT_ON);
}

void sendMorseString(char * str)
{
	uint8_t ind=0;
	while(*(str+ind) != 0)
	{
		sendMorseCharacter(*(str + ind++)-'a');
	}
    /*
	sendMorseCharacter(98-97);
	sendMorseCharacter(111-97);
	sendMorseCharacter(108-97);
	sendMorseCharacter(115-97);
    */

}

int main(void)
{
	setup();
	
	while(1)
	{
		if (sendMorseCode==0)
		{
			ADCSRA |= (1 << ADSC);
			while ((ADCSRA & (1 << ADSC))!= 0);
			if((ADMUX & 1) == 0)
			{
				pothaeuer=ADCH;
				ADMUX += 1;
			}
			else
			{	
				potschnaeuer=ADCH;
				ADMUX -= 1;
			}
		}
		else
		{
			sendMorseString("hallo");
			sendMorseCode = 2;
			waitForTicks(30);
			setRed(0xFF);
			waitForTicks(30);
			setRed(0x00);
			waitForTicks(30);
		}
	}
}


ISR ( TIMER0_OVF_vect )
{
	ticksCnt++;
	uint8_t switchescode = ((((PINB >> SW_1) & 0x1) << 0) | 
						   (((PIND >> SW_2) & 0x1) << 1) |
						   (((PIND >> SW_3) & 0x1) << 2) |
						   (((PIND >> SW_4) & 0x1) << 3));
	switchescode ^= 0x0F;

	switch(switchescode)
	{
		case 0:
			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				if (timer0cycles < 6 ) 
				{
					timer0cycles += 1; 
				} 
				else
				{
					timer0cycles = 0;
					hueshift_val += potschnaeuer;
				}
				
				if (hueshift_val > 0x5FF)
				{
					hueshift_val -= 0x5FF;
				}

				applyHueBrightness(hueshift_val,pothaeuer);
			}
			break;
		case 1: // hue and brightness is on each pot, no dynamics
			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				hueshift_val = potschnaeuer * 6;
				applyHueBrightness(hueshift_val,pothaeuer);
			}
			break;
		case 2: // ramp with defineable hue value 

			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode == 0)
			{
				if (potschnaeuer < 4)
				{
					brightness_val++;
				}
				else
				{
					brightness_val += potschnaeuer >> 2;
				}
				hueshift_val = pothaeuer * 6;
				applyHueBrightness(hueshift_val,brightness_val);
			}
			break;
		case 3: // white lamp, schnaeuer has no function 

			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				setRed(pothaeuer);
				setBlue(pothaeuer);
				setGreen(pothaeuer);
			}
			break;
		case 4: // off
			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				setRed(0);
				setBlue(0);
				setGreen(0);
			} 
			break;
		case 5: // red
			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				setRed(0xFF);
				setBlue(0);
				setGreen(0);
			}
			break;
		case 6: // blue
			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				setRed(0);
				setBlue(0xFF);
				setGreen(0);
			}
			break;
		case 7: // green

			if (sendMorseCode == 2)
			{
				sendMorseCode = 0;
			}
			else if (sendMorseCode==0)
			{
				setRed(0);
				setBlue(0);
				setGreen(0xFF);
			}
			break;
		case 10:
			sendMorseCode=1;
			break;	
		default: 
			sendMorseCode = 0;
			break;
	}
}

