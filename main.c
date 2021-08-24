/*
 * NES Controller Basic.c
 *
 * Created: 7/10/2021 11:44:35 AM
 * Author : Christopher Geis
 */
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Recording_Playback.h"
#include "EEPROM.h"
#include "main.h"

//PortD
const uint8_t NES_latch = 2;
const uint8_t NES_pulse = 3;
const uint8_t NES_data = 4;

//Controller data
static volatile uint8_t firstBits;
static volatile uint8_t lastBits;

volatile uint8_t input = 0xFF;
volatile uint8_t output = 0xFF;
volatile uint8_t latchHit;
volatile uint8_t pulseCounter = 0;

void GetControllerInput(void);
void SendButtonState(void);
void IncrementLatchCount(void);

int main(void)
{
	//Initialize IO
	DDRC = 0x00;						//Set PortB to input
	PORTC = 0b00111111;					//Engage pull-up resistors
	DDRB = 0x00;
	PORTB = 0b00000110;
	DDRD = 1 << NES_data | 1 << CONT_LED;	//Set PortD to input except the data and LED line
	PORTD = ~(1 << NES_data | 1 << CONT_LED);//Engage all pull-up resistors except the data and LED line
	
	//DEBUG: LED
	//DDRC |= 1 << 5;
	
	//Enable Interrupts
	EICRA = 0b00001111;			//Set INT0 to rising-edge and INT1 to rising-edge triggered
	EIMSK = 0b00000011;			//Enable NES_latch and NES_pulse interrupts
	TIMSK0 = 0b00000001;		//Enable TOV0 interrupt
	TIMSK1 = 0b00000001;		//Enable TOV1 interrupt
	
	sei();						//Enable interrupts
	
	//Get last recorded input sequence
	ReadRecording();
	
    while(1)
	{
		GetControllerInput();
		
		GetRecordButtonState(); //Get record button
		
		//Only light the LED when the controller is recording, but still allow the LED to blink.
		if (!blinking)
		{
			if (recording) ToggleLED(1);
			else ToggleLED(0);
		}
		
		//DEBUG: LED
		if (!playback)
		{
			if (!(output & 1)) PORTC |= 1 << 5;
			else PORTC &= ~(1 << 5);
 		}
	}
}

void GetControllerInput(void)
{
	savedTime = TCNT1;
	uint8_t temp = 0;
	
	//Calculate output byte
	firstBits = 0x00 | (PINB & 0b00000110);			//Capture PB1:2
	lastBits =  0x00 | (PINC & 0b00111111);			//Capture PC0:5
	temp = ((firstBits >> 1) | (lastBits << 2));			//Calculate input
	if (!playback)
	{
		
		input = temp;								//Capture input
		outputOld = output;							//Store previous output
		output = input;								//Capture current input
	}
	else if (playback && temp != 0xFF) playback = 0;//Stop playback on player input
	
	if (recording && input != outputOld) Record();
}

void SendButtonState(void)
{
	if (output & (1 << pulseCounter))	//If current bit of output is HIGH
	{
		PORTD |= 1 << NES_data;			//Set data line HIGH
	}
	else
	{
		PORTD &= ~(1 << NES_data);		//Else set data line LOW
	}
}

//Latch Interrupt
ISR(INT0_vect)
{
	if (playbackWaitingToStart) 
	{
		playbackWaitingToStart = 0;
		Playback();
	}
	else if (recordWaitingToStart)
	{
		recordWaitingToStart = 0;
		recording = 1;
	}
	
	PORTD |= 1 << NES_data;	//Set data line high
	
	pulseCounter = 0;	//Reset pulse counter
	SendButtonState();	//Send first bit of output to data line
}

//Pulse Interrupt
ISR(INT1_vect)
{
	pulseCounter++;			//Increment pulse counter
	SendButtonState();		//Send next bit of output to data line
}