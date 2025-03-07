#include "Buffers.h"

// Define PSRAM buffers with EXTMEM
EXTMEM int16_t audioBuffer[BUFFER_SIZE] = {0};
EXTMEM uint16_t realtimeWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH] = {0};
EXTMEM uint16_t bufferedWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH] = {0};
EXTMEM float fftBuffer[4096] = {0};
EXTMEM float fftOutput[4096] = {0};
EXTMEM int16_t demodulationBuffer[BUFFER_SIZE] = {0};
EXTMEM char receiveBuffer[1024] = {0};
EXTMEM char inputBuffer[53] = {0};
EXTMEM char outputBuffer[1024] = {0};
EXTMEM char auxInputBuffer[53] = {0};
EXTMEM unsigned int transmitBuffer[330751] = {0}; // Unchanged