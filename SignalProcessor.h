#ifndef SIGNAL_PROCESSOR_H
#define SIGNAL_PROCESSOR_H

#include <arm_math.h>
#include "Config.h"
#include "Utils.h"
#include "Indicator.h"

// Structure to store detected JS8 message candidates
struct JS8MessageCandidate {
  float timestamp;    // Start time in seconds since minute
  float centerFreq;   // Center frequency (tone #4 freq) in Hz
  uint8_t symbols[JS8_NORMAL_TONES];  // 79 8FSK symbols (0-7)
};

// Class for JS8 signal processing
// Purpose: Handles resampling, tone detection, and Costas synchronization
class SignalProcessor {
public:
  SignalProcessor(Utils& utils, Indicator& indicator);  // Constructor with utils and indicator
  void resample(int16_t* input, uint32_t inputPos);  // Resample 17.36s to 12.8 kHz
  void detectTones();           // Detect 8FSK tones from FFT
  void syncCostas();            // Synchronize with Costas arrays
  void processNormal(int16_t* input, uint32_t inputPos);  // Process Normal mode
  void processFast(int16_t* input, uint32_t inputPos);    // Stub: Fast mode
  void processTurbo(int16_t* input, uint32_t inputPos);   // Stub: Turbo mode
  void processSlow(int16_t* input, uint32_t inputPos);    // Stub: Slow mode

private:
  Utils& utils;                 // Reference to utils for debugging
  Indicator& indicator;         // Reference to indicator for tone markers
  int16_t* resampleBuffer;      // Pointer to global PSRAM buffer
  float32_t fftInput[JS8_FFT_SIZE * 2];  // Complex FFT input (real+imag)
  float32_t fftOutput[JS8_FFT_SIZE];     // FFT magnitude output
  arm_cfft_radix2_instance_f32 fftInstance;  // CMSIS FFT instance
  JS8MessageCandidate candidates[10];    // Array for up to 10 simultaneous messages
  uint8_t candidateCount;                // Number of detected candidates
  const uint8_t costasArray[7] = JS8_COSTAS_ARRAY;  // JS8 Normal mode Costas array
};

#endif