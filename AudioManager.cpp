#include "AudioManager.h"
#include <Audio.h>

extern AudioControlSGTL5000 sgtl5000;
extern AudioInputI2S audioInput;
extern AudioAnalyzeFFT1024 fft1024;
extern AudioRecordQueue queue1;

AudioManager::AudioManager() : bufferWritePos(0), samplesCollected(0) {}

void AudioManager::begin() {
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000.volume(0.5);
  fft1024.windowFunction(AudioWindowHanning1024);
  queue1.begin();
}

void AudioManager::update() {
  if (fft1024.available()) {
    // Placeholder for FFT processing (moved to JS8Demodulator later)
  }
  if (queue1.available() >= 1) {
    int16_t *audioData = queue1.readBuffer();
    updateAudioBuffer(audioData, 128);
    queue1.freeBuffer();
  }
}

void AudioManager::updateAudioBuffer(const int16_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    audioBuffer[bufferWritePos] = data[i];
    bufferWritePos = (bufferWritePos + 1) % BUFFER_SIZE;
    if (samplesCollected < BUFFER_SIZE) samplesCollected++;
  }
}