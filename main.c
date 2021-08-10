/*
 * NES Controller Basic.c
 *
 * Created: 7/10/2021 11:44:35 AM
 * Author : Christopher Geis
 */
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "Recording_Playback.h"
#include "main.h"

//PortD
const uint8_t NES_data = 4;
const uint8_t NES_latch = 2;
const uint8_t NES_pulse = 3;

//Controller data
static volatile uint8_t firstBits;
static volatile uint8_t lastBits;

volatile uint8_t input = 0xFF;
volatile uint8_t output = 0xFF;
volatile uint8_t pulseCounter = 0;

void GetControllerInput(void);
void SendButtonState(void);
void IncrementLatchCount(void);

int main(void)
{
	//Initialize IO
	DDRB = 0x00;				//Set PortB to input
	PORTB = 0xFF;				//Engage pull-up resistors
	DDRD = 1 << NES_data;		//Set PortD to input except PD4 (Data line)
	PORTD = ~(1 << NES_data);	//Engage all pull-up resistors except PD1
	DDRC = 1 << 1;				//Set PortC 1 to output
	PORTC = 1 << 0;				//Engage pull-up resistors
	
	//Enable Interrupts
	EICRA = 0b00001111;			//Set INT0 to rising-edge and INT1 to rising-edge triggered
	EIMSK = 0b00000011;			//Enable NES_latch and NES_pulse interrupts
	TIMSK0 = 0b00000001;		//Enable TOV0 interrupt
	TIMSK1 = 0b00000001;		//Enable TOV1 interrupt
	
	//usart_init(9600, 8, 1, NONE, TRANSMIT);
	
	sei();						//Enable interrupts
	
    while(1)
	{
		
		if (!playback) GetControllerInput();
		
		GetRecordButtonState(); //Get record button
		
		//Only light the LED when the controller is recording, but still allow the LED to blink.
		if (!blinking)
		{
			if (recording) ToggleLED(1);
			else ToggleLED(0);
		}
		
		//Check if the maximum number of inputs has been reached
		if (inputCount >= 255)
		{
			recording = 0; //Stop recording if so.
		}
		//Stop playback if sequence is complete.
		if (playbackCount >= inputCount) playback = 0;
	}
}

void GetControllerInput(void)
{
	static volatile uint8_t temp = 0;
	
	//Calculate output byte
	firstBits = 0x00 | (PIND & 0b11000000);			//Capture PD6:7
	lastBits =  0x00 | (PINB & 0b00111111);			//Capture PB0:5
	temp = ((firstBits >> 6) | (lastBits << 2));	//Calculate input
	input = temp;									//Capture input
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

void IncrementLatchCount(void)
{
	if (latchCount < MAX_LATCH) latchCount++;
	else
	{
		if (!inputChanged) RecordInputChange(0, 0x00);
		latchCount = 0;
		inputChanged = 0;
	}
}

//Latch Interrupt
ISR(INT0_vect)
{
	PORTD |= 1 << NES_data;	//Set data line high
	outputOld = output; //Store previous output
	output = input;		//Capture current input
	if (recording && (output != outputOld)) //Record changed inputs
	{ //TODO: Figure out why not all inputs are being recorded!
		Record();
	}
	
	pulseCounter = 0;	//Reset pulse counter
	SendButtonState();	//Send first bit of output to data line
	
	if (playback) Playback();
	
	
	if ((recording) || playback)
	{
		if (latchCount < MAX_LATCH) latchCount++;
		else
		{
			if (!inputChanged) RecordInputChange(0, 0x00);
			latchCount = 0;
			inputChanged = 0;
		}
	}
}

//Pulse Interrupt
ISR(INT1_vect)
{
	pulseCounter++;			//Increment pulse counter
	SendButtonState();		//Send next bit of output to data line
}