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

// Transmit buffer: 330,751 32-bit words for 30s at 22050 Hz + header
EXTMEM extern unsigned int transmitBuffer[330751];

#endif