#include <Arduino.h>  // Required for EXTMEM and Teensy-specific definitions
#include "AudioManager.h"

// Global PSRAM buffer for audio
EXTMEM int16_t audioManagerBuffer[BUFFER_SIZE];  // 60s at 44.1 kHz (~5.3 MB)

// Constructor: Initializes audio connections
AudioManager::AudioManager() 
  : patchCord1(audioInput, 0, fft1024, 0), 
    patchCord2(audioInput, 0, queue, 0),
    audioBuffer(audioManagerBuffer),  // Point to global buffer
    bufferWritePos(0) {
  // Note: Objects are initialized in declaration order
}

void AudioManager::begin() {
  // Initialize audio system
  AudioMemory(20);  // Allocate audio memory blocks
  fft1024.windowFunction(AudioWindowHanning1024);  // Apply Hanning window
  queue.begin();  // Start recording queue
  // Note: Audio input starts automatically with AudioConnection
}

bool AudioManager::isFFTAvailable() {
  // Check if new FFT data is ready
  return fft1024.available();
}

AudioAnalyzeFFT1024& AudioManager::getFFT() {
  // Return reference to FFT object for processing
  return fft1024;
}

void AudioManager::updateBuffer() {
  // Update PSRAM buffer with new audio blocks
  if (queue.available()) {
    int16_t* block = queue.readBuffer();
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      audioBuffer[bufferWritePos] = block[i];
      bufferWritePos = (bufferWritePos + 1) % BUFFER_SIZE;
    }
    queue.freeBuffer();
  }
}

int16_t* AudioManager::getAudioBuffer() {
  // Return pointer to PSRAM audio buffer
  return audioBuffer;
}

uint32_t AudioManager::getBufferPos() {
  // Return current write position
  return bufferWritePos;
}