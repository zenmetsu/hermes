#include "Buffers.h"

// Define PSRAM buffers with EXTMEM
EXTMEM int16_t audioBuffer[BUFFER_SIZE];
EXTMEM uint16_t realtimeWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
EXTMEM uint16_t bufferedWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
EXTMEM float fftBuffer[1024];
EXTMEM float fftOutput[1024];
EXTMEM int16_t demodulationBuffer[BUFFER_SIZE];
EXTMEM char receiveBuffer[1024];
EXTMEM char inputBuffer[53];
EXTMEM char outputBuffer[1024];
EXTMEM char auxInputBuffer[53];