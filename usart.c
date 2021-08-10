#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"

char string[30];
int stringLength;
int charCount = 0;
int completed = 1;

void usart_init(int baudRate, int characterSize, int stopBits, PARITY parityType, DATA_DIRECTION direction)
{
	int baudCalculation = ((int)(1000000 / baudRate)) - 1; //Calculate BAUD input
	
	//Set BAUD rate
	UBRR0 = baudCalculation; 
	
	//Set character size
	switch (characterSize) 
	{
		case 5: UCSR0C = 0;
		case 6: UCSR0C = (1 << UCSZ00);
		case 7: UCSR0C = (1 << UCSZ01);
		case 8: UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
		case 9: 
		{
			UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
			UCSR0B = (1 << UCSZ02);
		}
		default: UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
	}
	
	//Set stop bits
	if (stopBits == 2) UCSR0C |= (1 << USBS0);
	
	//Set parity
	switch (parityType)
	{
		case EVEN: UCSR0C |= (1 << UPM01);
		case ODD:  UCSR0C |= (1 << UPM01) | (1 << UPM00);
		case NONE: break;
	}

	if (direction == TRANSMIT) usart_enable_tx();
	else if (direction == RECEIVE) usart_enable_rx();
	else 
	{
		usart_enable_tx();
		usart_enable_rx();
	}
}

void usart_enable_tx()
{
	UCSR0B |= (1 << TXEN0) | (1 << UDRIE0);
}

void usart_enable_rx()
{
	UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
}

void usart_disable_tx()
{
	UCSR0B &= ~((1 << TXEN0) | (1 << UDRIE0));
}

void usart_disable_rx()
{
	UCSR0B &= ~((1 << RXEN0) | (1 << RXCIE0));
}

void usart_send(char stringIn[])
{
	charCount = 0;
	stringLength = strlen(stringIn);
	strcpy(string, stringIn);
	completed = 0;
	
}

int usart_completed()
{
	return completed;
}

//Send ISR
ISR(USART_UDRE_vect)
{
	if (completed) return;

	UDR0 = string[charCount++];
	
	if (charCount >= stringLength)
	{
		completed = 1;
		charCount = 0;
	}
}