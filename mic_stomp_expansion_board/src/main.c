#include <avr/io.h>
#include <avr/interrupt.h>

#define I2C_ADDRESS 10

volatile int footswitchstate=0, footswitchstateOld=0;
volatile int ledState=0, ledStateOld=0;

//int transmissionOngoing = 0;

void sendStompSwitchesState(void);
void startDebounceTimer(void);


void startDebounceTimer(void)
{
	TCNT0 = 0;
	TCCR0B = 4; // divide by 256
	OCR0A = 117; // compare match after roughly 30ms
	TIMSK0 = 2; // output compare a enabled
}

int main(void)
{
	DDRB = 0x0;
	PORTB = 0x7;
	DDRD = 0x3;
	DDRC = 0xF;
	
	// initialize i2c to listen to address 10
	TWAR = (I2C_ADDRESS << 1);
	TWCR |= (1 << TWIE);
	sei();
	while(1)
	{
		footswitchstate = PINB & 0x7;	
		if (footswitchstateOld != footswitchstate)
		{
			if (TCCR0B == 0) // counter didn't run, change is valid
			{
				footswitchstateOld = footswitchstate;
				startDebounceTimer();
			}
		}
		if (ledState != ledStateOld)
		{
			PORTC |= (ledState & 0xF);
			PORTD |= ((ledState >> 4) & 0x3); 
			ledStateOld = ledState;
		}	
	}
}

void sendStompSwitchesState(void)
{
	TWDR = footswitchstateOld & 0xFF;
	TWCR &= ~(1 >> TWEA);
	TWCR |= (1 << TWINT) | (1 << TWEN);
}

ISR ( TWI_vect )
{
	//PORTD &= ~0x2;
	if ((TWSR & 0xF8) == 0x60)
	{
		// got own address and request to write
		// wait for command
		TWCR |= (1 << TWEA) | (1 << TWINT)| (1 << TWEN);
		//transmissionOngoing = 1;
	}
	else if ((TWSR & 0xF8) == 0x80)
	{
		// led status data has been received
		TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
		ledState = TWDR;
	}
	else if ((TWSR & 0xF8) == 0xA8)
	{
		// read request has been received, send out stomp switches state
		sendStompSwitchesState();
	}
	else if ((TWSR & 0xF0) == 0xC0)
	{
		// last data byte has been transmitted successfully, not ack or ack (both c0 or c8 match) have been received
		TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	}
	else
	{
		TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	}
}


ISR ( TIMER0_COMPA_vect )
{
	TCCR0B = 0;
}