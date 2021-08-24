/*
 * Recording_Playback.c
 *
 * Created: 8/8/2021 10:16:09 AM
 *  Author: geisc
 */ 

#include "Recording_Playback.h"
#include "main.h"
#include "EEPROM.h"
#include <avr/interrupt.h>
#include <stdlib.h>

//Button and LED indicator
int buttonPressed, buttonWasPressed, buttonReleasedAfterRecording;
int waiting, blinking, recording;

//Recording Data
volatile int inputPressed;
volatile uint8_t outputOld;
volatile uint8_t inputCount = 0;
volatile uint16_t savedTime;
volatile uint8_t inputChangeArray[MAX_INPUTS];
volatile uint16_t inputTimingArray[MAX_INPUTS];

volatile uint8_t recordWaitingToStart, playbackWaitingToStart;

//Playback data
int playback, playbackCount = 0;

//Functions
void GetRecordButtonState(void)
{
	buttonPressed = !(PIND & 1 << CONT_button); //Check if the button is being pressed
	
	if (buttonPressed && !buttonWasPressed)
	{
		buttonWasPressed = 1;
		//If the button is being pressed for the first time in it's sequence, and the controller is not recording
		if (!waiting && !recording)
		{
			StartTimer1_1024(15625); //Start the one second hold timer
			BlinkLED();			//Indicate that the button was pressed
			waiting = 1;		//Indicate that the button has begun to be held, and the hold timer has not finished
		}
		
		if (recording && buttonReleasedAfterRecording) //If the button was pressed after letting go once initiating the recording sequence...
		{
			StopRecord();
			waiting = 0;
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
				StartPlayback();
			}
		}
	}
}

void StartPlayback(void)
{
	playbackCount = 0;
	output = 0xFF;
	playback = 1;
	playbackWaitingToStart = 1;
}

void Playback(void)
{
	output ^= inputChangeArray[playbackCount];
	StartTimer1_64(inputTimingArray[playbackCount++]);
	
	//DEBUG: LED
// 	if (!(output & 1)) PORTC |= 1 << 5;
// 	else PORTC &= ~(1 << 5);
	
	//Stop playback if sequence is complete.
	if (playback && playbackCount > inputCount) StopPlayback();
}

void StopPlayback(void)
{
	playback = 0;
	StopTimer1();
}

void StartRecord(void)
{
	inputCount = 0;
	buttonReleasedAfterRecording = 0;
	recordWaitingToStart = 1;
	recording = 1;
}

void Record(void)
{
	//TODO: Figure out how to match the recording with the Latches!
	//Record the changed inputs and timing
	RecordInputChange(savedTime, input ^ outputOld);
	
	//Stop if the maximum number of inputs has been reached
	if (inputCount >= MAX_INPUTS) StopRecord();
}

void StopRecord(void)
{
	recording = 0;
	StopTimer1();
	//StoreRecording();
}

void StoreRecording(void)
{
	PORTD |= 1 << 4; //Set data line HIGH while storing data to prevent pausing
	
	if (inputCount <= 1)
	{
		//Remove the stored recording indicator
		eeprom_write(RECORDING_EE_ADDRESS, 0x00);
	}
	else
	{
		uint16_t timingData;
		int addressIterator = 1;
		//Write the recording begin signal
		eeprom_write(RECORDING_EE_ADDRESS, RECORDING_BEGIN_BYTE);
		//Store the inputCount
		eeprom_write(RECORDING_EE_ADDRESS + addressIterator++, inputCount);
		for (int i = 0; i < inputCount; i++)
		{
			timingData = inputTimingArray[i];
			//Write the timing HIGH byte first!
			eeprom_write(RECORDING_EE_ADDRESS + addressIterator++, (timingData & 0xFF00) >> 8);
			//Write the timing LOW byte next!
			eeprom_write(RECORDING_EE_ADDRESS + addressIterator++, timingData & 0x00FF);
			//Write the input last!
			eeprom_write(RECORDING_EE_ADDRESS + addressIterator++, inputChangeArray[i]);
		}
	}
}

void ReadRecording(void)
{
	if (eeprom_read(RECORDING_EE_ADDRESS) == RECORDING_BEGIN_BYTE) //Check if a recording was stored
	{
		uint16_t timingH;
		uint16_t timingL;
		int addressIterator = 1;
		inputCount = eeprom_read(RECORDING_EE_ADDRESS + addressIterator++);
		for (int i = 0; i < inputCount; i++)
		{
			//Get first byte of timing data
			timingH = eeprom_read(RECORDING_EE_ADDRESS + addressIterator++);
			//Get second byte of timing data
			timingL = eeprom_read(RECORDING_EE_ADDRESS + addressIterator++);
			
			//Store timing data in array
			inputTimingArray[i] = (timingL | (timingH << 8));
			//Get and store input data in array
			inputChangeArray[i] = eeprom_read(RECORDING_EE_ADDRESS + addressIterator++);
		}
	}
}

void RecordInputChange(uint16_t timingIn, uint8_t inputIn)
{
	//Get the timing
	inputTimingArray[inputCount] = timingIn;
	
	//Get the toggle mask
	inputChangeArray[inputCount++] = inputIn;
	
	StartTimer1_64(0xFFFF);
}

void ToggleLED(uint8_t state)
{
	if (state) PORTD |= 1 << CONT_LED;
	else PORTD &= ~(1 << CONT_LED);
}

void BlinkLED(void)
{
	blinking = 1;
	ToggleLED(1);
	StartTimer0(0xFF);
}

void StartTimer0(uint8_t clocks)
{
	TCNT0 = 0xFF - clocks;		//Load Timer0 with the number of pre-scaled clocks in a second
	TCCR0B = 0b00000101;		//Set Timer0 pre-scaler to CLK/1024 and start the clock
}

void StopTimer0(void)
{
	TCCR0B = 0;					//Stop Timer0
}

void StartTimer1_1024(int clocks)
{
	TCNT1 = 0xFFFF - clocks;	//Load Timer1 with the number of pre-scaled clocks in a second
	TCCR1B = 0b00000101;		//Set Timer1 pre-scaler to CLK/1024 and start the clock
}

void StartTimer1_64(int clocks)
{
	TCNT1 = 0xFFFF - clocks;	//Load Timer1 with the number of pre-scaled clocks in a second
	TCCR1B = 0b00000011;		//Set Timer1 pre-scaler to CLK/64 and start the clock
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
	if (!recording && !playback && waiting)
	{
		waiting = 0;
		StartRecord();
	}
	else if (recording)
	{
			RecordInputChange(0xFFFF, 0);
	}
	else if (playback) Playback();
}