#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Audio.h>
#include "config.h"
#include "Buffers.h"

class AudioManager {
public:
  AudioManager();
  void begin();
  void update();
  
private:
  void updateAudioBuffer(const int16_t *data, size_t len);
  volatile uint32_t bufferWritePos;
  volatile uint32_t samplesCollected;
};

#endif