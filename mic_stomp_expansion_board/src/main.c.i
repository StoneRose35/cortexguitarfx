#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/iom88.h>

#define I2C_ADDRESS 23
#define I2C_MAINBOARD_ADDRESS 27

#define I2C_OWN_ADDRESS_WRITE 0x60
#define I2C_DATA_RECEIVED 0x80
#define I2C_OWN_ADDRESS_READ 0xA8
#define I2C_LAST_DATA_BYTE_TRANSMITTED 0xC0
#define I2C_START_TRANSMITTED 0x08
#define I2C_ADDRESS_TRANSMITTED_ACK 0x18
#define I2C_ADDRESS_TRANSMITTED_NACK 0x20
#define I2C_DATA_TRANSMITTED_ACK 0x28
#define I2C_DATA_TRANSMITTED_NACK 0x30
#define I2C_ARBITRATION_LOST 0x38
volatile int footswitchstate=0, footswitchstateOld=0,sendOperationPending=0;
volatile int ledState=0, ledStateOld=0;

void sendStompSwitchesState(void);
void startDebounceTimer(void);

__attribute__((section(".firmwaresig")))
const volatile uint32_t firmwareVersion=<FIRMWARE_VERSION>;
void startDebounceTimer(void)
{
	TCNT0 = 0;
	TCCR0B = 5; // divide by 1024
	OCR0A = 117; // compare match after roughly 30ms
	TIMSK0 = 2; // output compare a enabled
}

int main(void)
{
	DDRB = 0x0;
	PORTB = 0x7;
	DDRD = 0x3;
	DDRC = 0xF;

	CLKPR = 0x80; // enable
	CLKPR = 0x01; // set prescaler of 8MHz/2 results in 4MHz system clock
	
	TWBR = ((40-16)>>1); // adjust the bit rate register for a bit rate of 100 kHz

	// initialize i2c to listen to address 23
	TWAR = (I2C_ADDRESS << 1);
	TWCR |= (1 << TWIE) | (1 << TWEA) | (1 << TWEN);
	footswitchstate = PINB & 0x7;	
	footswitchstateOld = footswitchstate;
	sei();
	while(1)
	{
		footswitchstate = PINB & 0x7;	
		if (footswitchstateOld != footswitchstate && sendOperationPending == 0)
		{
			if (TCCR0B == 0) // counter didn't run, change is valid
			{
				footswitchstateOld = footswitchstate;
				sendOperationPending = 1;
				TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTA); // start sending the switch state
				startDebounceTimer();
			}
		}
		if (ledState != ledStateOld)
		{
			PORTC = (ledState & 0xF);
			PORTD = ((ledState >> 4) & 0x3); 
			ledStateOld = ledState;
		}
	}
}

void sendStompSwitchesState(void)
{
	TWDR = footswitchstateOld & 0xFF;
	TWCR &= ~(1 << TWEA);
	TWCR |= (1 << TWINT) | (1 << TWEN);
}

ISR ( TWI_vect )
{
	if ((TWSR & 0xF8) == I2C_OWN_ADDRESS_WRITE)
	{
		// got own address and request to write
		// wait for command
		TWCR |= (1 << TWEA) | (1 << TWINT)| (1 << TWEN) | (1 << TWIE);
	}
	else if ((TWSR & 0xF8) == I2C_DATA_RECEIVED)
	{
		// led status data has been received
		ledState = TWDR;
		TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
	}
	else if ((TWSR & 0xF8) == I2C_OWN_ADDRESS_READ)
	{
		// read request has been received, send out stomp switches state
		sendStompSwitchesState();
	}
	else if ((TWSR & 0xF0) == I2C_LAST_DATA_BYTE_TRANSMITTED)
	{
		// last data byte has been transmitted successfully, not ack or ack (both c0 or c8 match) have been received
		TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	}
	else if ((TWSR & 0xF8) == I2C_START_TRANSMITTED)
	{
		// start has been sent, continue with main board address
		TWDR = (I2C_MAINBOARD_ADDRESS << 1);
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
		
	}
	else if ((TWSR & 0xF8) == I2C_ADDRESS_TRANSMITTED_ACK)
	{
		TWDR = footswitchstateOld & 0xFF;
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
		
	}
	else if ((TWSR & 0xF8) == I2C_DATA_TRANSMITTED_ACK)
	{
		sendOperationPending = 0;
		TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
	}
	else if (((TWSR & 0xF8) == I2C_DATA_TRANSMITTED_NACK) || ((TWSR & 0xF8) == I2C_ADDRESS_TRANSMITTED_NACK))
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA) | (1 << TWIE); // immediately retry
	}
	else if (((TWSR & 0xF8) == I2C_ARBITRATION_LOST) && sendOperationPending != 0)
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA) | (1 << TWIE); // immediately retry
	}
	else
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
	}
}


ISR ( TIMER0_COMPA_vect )
{
	TCCR0B = 0;
}