#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Audio.h>
#include "Config.h"

// Class to manage audio input and FFT processing
// Purpose: Encapsulates Teensy Audio Library usage for I2S input, FFT, and raw audio buffering
class AudioManager {
public:
  AudioManager();             // Constructor
  void begin();               // Initialize audio components
  bool isFFTAvailable();      // Check if new FFT data is ready
  AudioAnalyzeFFT1024& getFFT();  // Access FFT object
  void updateBuffer();        // Update PSRAM audio buffer
  int16_t* getAudioBuffer();  // Get pointer to PSRAM buffer
  uint32_t getBufferPos();    // Get current write position

private:
  AudioInputI2S audioInput;         // I2S audio input
  AudioAnalyzeFFT1024 fft1024;      // FFT processor for waterfall
  AudioRecordQueue queue;           // Queue for raw audio capture
  AudioConnection patchCord1;       // Input to FFT
  AudioConnection patchCord2;       // Input to queue
  int16_t* audioBuffer;             // Pointer to global PSRAM buffer
  volatile uint32_t bufferWritePos; // Current write position
};

#endif