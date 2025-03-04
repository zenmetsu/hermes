#ifndef JS8_RESPONDER_H
#define JS8_RESPONDER_H

#include "Buffers.h"

class JS8Responder {
public:
  void processMessages(); // Parses receiveBuffer, may fill auxInputBuffer
};

#endif