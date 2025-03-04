#ifndef JS8_DEMODULATOR_H
#define JS8_DEMODULATOR_H

#include "Buffers.h"

class JS8Demodulator {
public:
  void demodulate(); // Processes demodulationBuffer, outputs to receiveBuffer
};

#endif