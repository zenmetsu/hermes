#ifndef BUFFERS_H
#define BUFFERS_H

#include <Arduino.h>
#include "config.h"

// Existing buffers
extern int16_t audioBuffer[BUFFER_SIZE];
extern uint16_t realtimeWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
extern uint16_t bufferedWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
extern float fftBuffer[4096];
extern float fftOutput[4096];
extern int16_t demodulationBuffer[BUFFER_SIZE];
extern char receiveBuffer[1024];
extern char inputBuffer[53];
extern char outputBuffer[1024];
extern char auxInputBuffer[53];

// New transmit buffer for all modes
EXTMEM extern int16_t transmitBuffer[1323000]; // 30s * 44.1 kHz

#endif