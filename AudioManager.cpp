#include "AudioManager.h"
#include <Arduino.h>

EXTMEM int16_t audioManagerBuffer[BUFFER_SIZE];
static uint32_t lastDebug = 0;

AudioManager::AudioManager()
  : patchCord1(audioInput, 0, fft1024, 0),
    patchCord2(audioInput, 0, queue, 0),
    audioBuffer(audioManagerBuffer),
    bufferWritePos(0) {
}

void AudioManager::begin() {
  AudioMemory(60);  // Increase to 60 blocks
  fft1024.windowFunction(AudioWindowHanning1024);
  queue.begin();
  Serial.println("AudioManager initialized");
  Serial.print("AudioMemory allocated: "); Serial.println(60);
}

bool AudioManager::isFFTAvailable() {
  bool available = fft1024.available();
  
  static bool lastState = false;
  if (millis() - lastDebug >= 1000 || available != lastState) {
    //Serial.print("FFT available: "); Serial.println(available ? "Yes" : "No");
    //Serial.print("FFT block count: "); Serial.println(fft1024.available() ? 1 : 0);
    lastDebug = millis();
    lastState = available;
  }
  return available;
}

AudioAnalyzeFFT1024& AudioManager::getFFT() {
  return fft1024;
}

void AudioManager::updateBuffer() {
  int blocks = queue.available();
  static uint32_t lastDebug = 0;
  if (blocks > 0) {
    int16_t* block = queue.readBuffer();
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      audioBuffer[bufferWritePos] = block[i];
      bufferWritePos = (bufferWritePos + 1) % BUFFER_SIZE;
    }
    queue.freeBuffer();
    if (millis() - lastDebug >= 1000) {
      Serial.print("Queue blocks available: "); Serial.println(blocks);
      Serial.print("Audio buffer updated, pos: "); Serial.println(bufferWritePos);
      Serial.print("Buffer[0-2]: ");
      Serial.print(audioBuffer[0]); Serial.print(", ");
      Serial.print(audioBuffer[1]); Serial.print(", ");
      Serial.println(audioBuffer[2]);
      lastDebug = millis();
    }
  }
}

int16_t* AudioManager::getAudioBuffer() {
  return audioBuffer;
}

uint32_t AudioManager::getBufferPos() {
  return bufferWritePos;
}