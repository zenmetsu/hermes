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
  fft1024.windowFunction(AudioWindowHanning1024); // Still used for realtime
  queue1.begin();
}

void AudioManager::update() {
  if (fft1024.available()) {
    // Realtime waterfall updated in UIManager
  }
  if (queue1.available() >= 1) {
    int16_t *audioData = queue1.readBuffer();
    updateAudioBuffer(audioData, 128);
    updateBufferedWaterfall();
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

void AudioManager::updateBufferedWaterfall() {
  static uint32_t lastUpdatePos = 0; // Moved to top
  static arm_rfft_fast_instance_f32 fft_instance;
  static bool fftInitialized = false;
  if (!fftInitialized) {
    arm_rfft_fast_init_f32(&fft_instance, 4096);
    fftInitialized = true;
  }

  uint32_t samplesToProcess = min(samplesCollected, HALF_BUFFER);
  if (samplesToProcess < 4096) return;

  uint32_t samplesSinceLast = (bufferWritePos >= lastUpdatePos) ?
                              (bufferWritePos - lastUpdatePos) :
                              (BUFFER_SIZE - lastUpdatePos + bufferWritePos);
  if (samplesSinceLast < SAMPLES_PER_ROW) return;

  for (int x = 0; x < VISUALIZER_HEIGHT - 1; x++) {
    memcpy(bufferedWaterfall[x], bufferedWaterfall[x + 1], SCREEN_WIDTH * sizeof(uint16_t));
  }

  uint32_t fftStartPos = (bufferWritePos - 4096 + BUFFER_SIZE) % BUFFER_SIZE;
  for (int i = 0; i < 4096; i++) {
    fftBuffer[i] = audioBuffer[(fftStartPos + i) % BUFFER_SIZE] / 32768.0f;
    fftBuffer[i] *= 0.5 * (1 - cos(2 * PI * i / 4095.0)); // Hanning window for 4096
  }

  arm_rfft_fast_f32(&fft_instance, fftBuffer, fftOutput, 0);

  float binHz = SAMPLE_RATE / 4096.0; // ~10.766 Hz
  int minBin = MIN_FREQ / binHz;      // 300 / 10.766 ≈ 28
  int maxBin = MAX_FREQ / binHz;      // 3600 / 10.766 ≈ 334
  int binRange = maxBin - minBin;     // 334 - 28 = 306

  static float maxMagnitudeBuf = 0.0;
  static float gainFactorBuf = 1.0;
  const float GAIN_DECAY = 0.99;

  float localMax = 0.0;
  float avgMagnitude = 0.0;
  int binCount = 0;
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = sqrt(fftOutput[2 * bin] * fftOutput[2 * bin] +
                     fftOutput[2 * bin + 1] * fftOutput[2 * bin + 1]);
    if (mag > localMax) localMax = mag;
    avgMagnitude += mag;
    binCount++;
  }
  avgMagnitude /= binCount;

  if (localMax > maxMagnitudeBuf) maxMagnitudeBuf = localMax;
  else maxMagnitudeBuf *= GAIN_DECAY;
  gainFactorBuf = maxMagnitudeBuf > 0 ? 1.0 / maxMagnitudeBuf : 1.0;

  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = sqrt(fftOutput[2 * bin] * fftOutput[2 * bin] +
                     fftOutput[2 * bin + 1] * fftOutput[2 * bin + 1]);
    float magnitude = mag * gainFactorBuf;
    float logMag = log10(1.0 + magnitude * 10.0);
    float normalizedMag = logMag / log10(11.0);
    float t1 = 0.4, t2 = 0.5, t3 = 0.6, t4 = 0.7, t5 = 0.8;
    if (normalizedMag > t5) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_SEAFOAM;
    else if (normalizedMag > t4) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_WHITE;
    else if (normalizedMag > t3) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_LIGHTGREY;
    else if (normalizedMag > t2) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_GREY;
    else if (normalizedMag > t1) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_DARKGREY;
    else bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_BLACK;
  }

  lastUpdatePos = bufferWritePos; // Now in scope
}