/*
 * CFile1.c
 *
 * Created: 8/8/2021 10:16:09 AM
 *  Author: geisc
 */ 

#include "Recording_Playback.h"
#include "main.h"
#include <avr/interrupt.h>

//Button and LED indicator
int buttonPressed;
int buttonWasPressed;
int buttonReleasedAfterRecording;
int waiting;
int blinking;
int recording;

//Recording Data
int inputPressed;
int inputChanged;
uint8_t outputOld;
int latchCount;
int inputCount = 0;
uint8_t inputChangeArray[255];
uint8_t inputTimingArray[255];

//Playback data
int playback;
int playbackCount = 0;

//Functions
void Playback(void)
{
	if (inputTimingArray[playbackCount] == latchCount) //TODO: Figure out what to do with empty latch cycles!
	{
		input ^= inputChangeArray[playbackCount++];
	}
}

void Record(void)
{
	inputChanged = 1;
	//Record the changed inputs and timing
	RecordInputChange(latchCount, output ^ outputOld);
}

void GetRecordButtonState(void)
{
	buttonPressed = !(PINC & 1 << 0); //Check if the button is being pressed
	
	if (buttonPressed && !buttonWasPressed)
	{
		buttonWasPressed = 1;
		//If the button is being pressed for the first time in it's sequence, and the controller is not recording
		if (!waiting && !recording)
		{
			StartTimer1(15625); //Start the one second hold timer
			BlinkLED();			//Indicate that the button was pressed
			waiting = 1;		//Indicate that the button has begun to be held, and the hold timer has not finished
		}
		
		if (recording && buttonReleasedAfterRecording) //If the button was pressed after letting go once initiating the recording sequence...
		{
			recording = 0; //Stop recording
		}
	}
	//If the button was released
	else if (!buttonPressed && buttonWasPressed)
	{
		buttonWasPressed = 0;
		//And, if the recording sequence has been initiated, and the button hasn't already been released...
		if (recording && !buttonReleasedAfterRecording) buttonReleasedAfterRecording = 1; //Indicate that the button has been released.
		
		//And, if the one second hold timer has not expired...
		if (waiting)
		{
			StopTimer1(); //Stop the one second timer
			
			//And, if the controller is not recording...
			if (!recording)
			{
				waiting = 0;
				//Start button playback
				playbackCount = 0;
				latchCount = 0;
				input = 0xFF;
				playback = 1;
			}
		}
	}
}

void RecordInputChange(uint8_t timingIn, uint8_t inputIn)
{
	//Get the timing
	inputTimingArray[inputCount] = timingIn;
	
	//Get the toggle mask
	inputChangeArray[inputCount++] = inputIn;
}

void ToggleLED(uint8_t state)
{
	if (state) PORTC |= 1 << 1;
	else PORTC &= ~(1 << 1);
}

void BlinkLED(void)
{
	blinking = 1;
	ToggleLED(1);
	StartTimer0(0xFF);
}

void StartTimer0(int clocks)
{
	TCNT0 = 0xFF - clocks;		//Load Timer0 with the number of pre-scaled clocks in a second
	TCCR0B = 0b00000101;		//Set Timer0 pre-scaler to CLK/1024 and start the clock
}

void StopTimer0(void)
{
	TCCR0B = 0;					//Stop Timer0
}

void StartTimer1(int clocks)
{
	TCNT1 = 0xFFFF - clocks;	//Load Timer1 with the number of pre-scaled clocks in a second
	TCCR1B = 0b00000101;		//Set Timer1 pre-scaler to CLK/1024 and start the clock
}

void StopTimer1(void)
{
	TCCR1B = 0;					//Stop Timer1
}

//Timer0 Overflow
ISR(TIMER0_OVF_vect)
{
	blinking = 0;
	StopTimer0();
	ToggleLED(0);
}

//Timer1 Overflow
ISR(TIMER1_OVF_vect)
{
	StopTimer1();
	waiting = 0;
	inputCount = 0;
	recording = 1;
	buttonReleasedAfterRecording = 0;
	latchCount = 0;
}