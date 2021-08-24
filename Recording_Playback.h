/*
 * Recording_Playback.h
 *
 * Created: 8/8/2021 10:17:31 AM
 *  Author: geisc
 */ 


#ifndef RECORDING_PLAYBACK_H_
#define RECORDING_PLAYBACK_H_
#include <stdint.h>
#include <avr/io.h>

#define RECORDING_BEGIN_BYTE 0x55
#define RECORDING_EE_ADDRESS 0x04
#define MAX_INPUTS 255

//Button and LED indicator
extern int buttonPressed, buttonWasPressed, buttonReleasedAfterRecording;
extern int waiting, blinking, recording;

//Recording Data
extern volatile int inputPressed;
extern volatile uint8_t outputOld;
extern volatile uint8_t inputCount;
extern volatile uint16_t savedTime;
extern volatile uint8_t inputChangeArray[255];
extern volatile unsigned int inputTimingArray[255];

extern volatile uint8_t recordWaitingToStart, playbackWaitingToStart;

//Playback data
extern int playback, playbackCount;

//Function Declarations
extern void StartPlayback(void);
extern void Playback(void);
extern void StopPlayback(void);
extern void StartRecord(void);
extern void Record(void);
extern void StopRecord(void);
extern void GetRecordButtonState(void);
void StoreRecording(void);
extern void ReadRecording(void);
extern void RecordInputChange(uint16_t timingIn, uint8_t inputIn);
extern void ToggleLED(uint8_t state);
extern void BlinkLED(void);
extern void StartTimer0(uint8_t clocks);
extern void StopTimer0(void);
extern void StartTimer1_1024(int clocks);
extern void StartTimer1_64(int clocks);
extern void StopTimer1(void);

#endif /* RECORDING_PLAYBACK_H_ */