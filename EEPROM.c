#include <avr/interrupt.h>
#include "EEPROM.h"
#define F_CPU 16000000
#include <util/delay.h>
void eeprom_interrupt_enable()
{
	EECR |= 1 << EERIE;
}

void eeprom_interrupt_disable()
{
	EECR &= ~(1 << EERIE);
}

void eeprom_write(uint16_t addressIn, uint8_t dataIn)
{
	cli();						//Stop interrupts
	
	while (EECR & 1 << EEPE);	//Wait until EEPE is 0
	EEAR = addressIn;			//Write address to EEAR
	EEDR = dataIn;				//Write data to EEDR
	EECR |= 1 << EEMPE;			//Set EEMPE of EECR
	EECR |= 1 << EEPE;			//Set EEPE of EECR
	
	sei();						//Allow interrupts
}

uint8_t eeprom_read(uint16_t addressIn)
{
	cli();						//Stop interrupts
	
	while (EECR & 1 << EEPE);	//Wait until EEPE is 0
	EEAR = addressIn;			//Write address to EEAR
	EECR |= 1 << EERE;			//Set EERE to 1
	
	sei();						//Allow interrupts
	
	return EEDR;				//Read EEDR
}