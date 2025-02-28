#include <Arduino.h>  // Required for EXTMEM and Teensy-specific definitions
#include "SignalProcessor.h"
#include <cstdio>     // For snprintf

// Global PSRAM buffers
EXTMEM int16_t signalProcessorResampleBuffer[JS8_RESAMPLE_SIZE];  // 17.36s at 12.8 kHz (~444 KB)
EXTMEM float tonePower[JS8_RESAMPLE_SIZE / JS8_FFT_SIZE][8];      // Power for each tone per block (~3.5 KB)
EXTMEM float costasPower[JS8_RESAMPLE_SIZE / JS8_FFT_SIZE][3];    // Power at each Costas position (~1.3 KB)
EXTMEM char debugMsg[100];  // Global debug message buffer to reduce stack usage

SignalProcessor::SignalProcessor(Utils& u, Indicator& ind)
  : utils(u),
    indicator(ind),
    resampleBuffer(signalProcessorResampleBuffer),  // Point to global buffer
    candidateCount(0) {
  // Initialize FFT instance
  arm_cfft_radix2_init_f32(&fftInstance, JS8_FFT_SIZE, 0, 1);
}

void SignalProcessor::resample(int16_t* input, uint32_t inputPos) {
  // Simple linear interpolation resampling from 44.1 kHz to 12.8 kHz
  float ratio = (float)SAMPLE_RATE / JS8_RESAMPLE_RATE;  // ~3.4453125

  for (uint32_t i = 0; i < JS8_RESAMPLE_SIZE; i++) {
    float srcPos = i * ratio;
    uint32_t idx = (inputPos + (uint32_t)srcPos - (uint32_t)(JS8_SEARCH_DURATION * SAMPLE_RATE) + BUFFER_SIZE) % BUFFER_SIZE;
    uint32_t nextIdx = (idx + 1) % BUFFER_SIZE;
    float frac = srcPos - (uint32_t)srcPos;

    // Linear interpolation
    float sample = input[idx] * (1.0f - frac) + input[nextIdx] * frac;
    resampleBuffer[i] = (int16_t)sample;
  }
  utils.debugPrint("Resampling complete");
}

void SignalProcessor::detectTones() {
  // Clear tone indicators at start
  indicator.clear();

  // Process resampled buffer in 2048-sample blocks (0.16s each, ~108 blocks in 17.36s)
  uint32_t blockCount = JS8_RESAMPLE_SIZE / JS8_FFT_SIZE;  // ~108
  float totalPower[529] = {0};  // Power for bins 48-576 (300-3600 Hz) over all blocks

  utils.debugPrint("Starting FFT processing");
  for (uint32_t block = 0; block < blockCount; block++) {
    // Fill FFT input (real samples, imag = 0)
    for (uint32_t i = 0; i < JS8_FFT_SIZE; i++) {
      fftInput[2*i] = resampleBuffer[block * JS8_FFT_SIZE + i] / 32768.0f;  // Normalize to [-1, 1]
      fftInput[2*i + 1] = 0.0f;  // Imaginary part
    }

    // Perform FFT
    arm_cfft_radix2_f32(&fftInstance, fftInput);
    arm_cmplx_mag_f32(fftInput, fftOutput, JS8_FFT_SIZE);

    // Accumulate power for bins 48-576 (300-3600 Hz)
    for (int bin = 48; bin <= 576; bin++) {
      totalPower[bin - 48] += fftOutput[bin] * 0.0001f;  // Scale and sum over time
    }
  }
  utils.debugPrint("FFT processing complete");

  // Structure to store top tone group candidates
  struct ToneGroup {
    float power;      // Combined power of 8 tones
    float centerFreq; // Frequency of tone #4 (middle of 8)
  };
  ToneGroup topGroups[10] = {0};  // Initialize to zero
  int groupCount = 0;

  // Identify top 10 tone groups (8 adjacent bins)
  for (int startBin = 48; startBin <= 576 - 7; startBin++) {  // Slide from 48 to 569
    float groupPower = 0.0f;
    for (int tone = 0; tone < 8; tone++) {
      groupPower += totalPower[startBin - 48 + tone];  // Sum power of 8 adjacent bins
    }

    // Check if this group is strong enough
    if (groupPower > 0.01f) {  // Threshold for combined power
      float centerFreq = 300.0f + ((startBin + 4) * 6.25f);  // Tone #4 frequency

      // Insert into topGroups, maintaining top 10
      int insertIdx = groupCount < 10 ? groupCount : 9;
      for (int i = 0; i < groupCount && i < 10; i++) {
        if (groupPower > topGroups[i].power) {
          insertIdx = i;
          break;
        }
      }

      // Shift lower entries down
      for (int i = (groupCount < 10 ? groupCount : 9); i > insertIdx; i--) {
        topGroups[i] = topGroups[i-1];
      }

      // Insert new group
      topGroups[insertIdx].power = groupPower;
      topGroups[insertIdx].centerFreq = centerFreq;
      if (groupCount < 10) groupCount++;
    }
  }

  // Report top 10 tone groups and update indicator
  for (int i = 0; i < groupCount; i++) {
    snprintf(debugMsg, sizeof(debugMsg), "Top tone group %d: %.1f Hz, power %.4f",
             i + 1, topGroups[i].centerFreq, topGroups[i].power);
    utils.debugPrint(debugMsg);
    indicator.update(topGroups[i].centerFreq);  // Add tone indicator
  }
  utils.debugPrint("Tone detection complete");
}

void SignalProcessor::syncCostas() {
  // Process FFT output to find Costas arrays
  uint32_t blockCount = JS8_RESAMPLE_SIZE / JS8_FFT_SIZE;  // ~108

  utils.debugPrint("Starting Costas sync");
  for (uint32_t block = 0; block < blockCount - JS8_NORMAL_TONES + 1; block++) {  // Limit to valid message length
    // Check for Costas arrays at start, mid, end
    for (int pos = 0; pos < 3; pos++) {
      uint32_t offset = (pos == 0) ? block : (pos == 1) ? block + 36 : block + 72;
      if (offset + 7 >= blockCount) continue;  // Prevent out-of-bounds access

      float power = 0.0f;
      for (int i = 0; i < 7; i++) {
        uint32_t bin = 48 + costasArray[i];  // Starting at 300 Hz
        if (offset * JS8_FFT_SIZE + bin < JS8_FFT_SIZE) {  // Bounds check
          power += fftOutput[offset * JS8_FFT_SIZE + bin];
        }
      }
      costasPower[block][pos] = power / 7.0f;  // Average power
    }

    // Check for message candidate
    if (block + 72 < blockCount &&  // Ensure all positions are valid
        costasPower[block][0] > 0.01f &&
        costasPower[block + 36][1] > 0.01f &&
        costasPower[block + 72][2] > 0.01f &&
        candidateCount < 10) {
      candidates[candidateCount].timestamp = block * 0.16f;  // 0.16s per block
      candidates[candidateCount].centerFreq = 300.0f + (block * 6.25f);  // Rough estimate
      // TODO: Extract 79 symbols
      snprintf(debugMsg, sizeof(debugMsg), "Costas sync at %.2fs, %.1f Hz",
               candidates[candidateCount].timestamp, candidates[candidateCount].centerFreq);
      utils.debugPrint(debugMsg);
      candidateCount++;
    }
  }
  utils.debugPrint("Costas sync complete");
}

void SignalProcessor::processNormal(int16_t* input, uint32_t inputPos) {
  resample(input, inputPos);
  detectTones();
  syncCostas();
}

void SignalProcessor::processFast(int16_t* input, uint32_t inputPos) {
  utils.debugPrint("Fast mode not implemented yet");
}

void SignalProcessor::processTurbo(int16_t* input, uint32_t inputPos) {
  utils.debugPrint("Turbo mode not implemented yet");
}

void SignalProcessor::processSlow(int16_t* input, uint32_t inputPos) {
  utils.debugPrint("Slow mode not implemented yet");
}