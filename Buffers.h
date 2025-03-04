#ifndef BUFFERS_H
#define BUFFERS_H

#include <Arduino.h> // Ensures Teensy-specific macros like EXTMEM are available
#include "config.h"

// Declare buffers as extern (definitions will be in Buffers.cpp)
extern int16_t audioBuffer[BUFFER_SIZE];
extern uint16_t realtimeWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
extern uint16_t bufferedWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
extern float fftBuffer[1024];
extern float fftOutput[1024];
extern int16_t demodulationBuffer[BUFFER_SIZE];
extern char receiveBuffer[1024];
extern char inputBuffer[53];
extern char outputBuffer[1024];
extern char auxInputBuffer[53];

#endif