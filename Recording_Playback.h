/*
 * IncFile1.h
 *
 * Created: 8/8/2021 10:17:31 AM
 *  Author: geisc
 */ 


#ifndef RECORDING_PLAYBACK_H_
#define RECORDING_PLAYBACK_H_
#include <stdint.h>
#include <avr/io.h>

//Button and LED indicator
extern int buttonPressed;
extern int buttonWasPressed;
extern int buttonReleasedAfterRecording;
extern int waiting;
extern int blinking;
extern int recording;

//Recording Data
extern int inputPressed;
extern int inputChanged;
extern uint8_t outputOld;
extern int latchCount;
#define MAX_LATCH 0b11111111
extern int inputCount;
extern uint8_t inputChangeArray[255];
extern uint8_t inputTimingArray[255];

//Playback data
extern int playback;
extern int playbackCount;

//Function Declarations
extern void Playback(void);
extern void Record(void);
extern void GetRecordButtonState(void);
extern void RecordInputChange(uint8_t timingIn, uint8_t inputIn);
extern void ToggleLED(uint8_t state);
extern void BlinkLED(void);
extern void StartTimer0(int clocks);
extern void StopTimer0(void);
extern void StartTimer1(int clocks);
extern void StopTimer1(void);

#endif /* INCFILE1_H_ */