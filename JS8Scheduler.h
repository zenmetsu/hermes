#ifndef JS8_SCHEDULER_H
#define JS8_SCHEDULER_H

#include "Buffers.h"

class JS8Scheduler {
public:
  void begin();
  void update();
  
private:
  void triggerDemodulation();
  void triggerTransmission();
};

#endif