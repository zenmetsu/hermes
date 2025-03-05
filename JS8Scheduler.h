#ifndef JS8_SCHEDULER_H
#define JS8_SCHEDULER_H

#include "Buffers.h"
#include <TimeLib.h>
#include "JS8.h"
#include "JS8Demodulator.h"
#include "JS8Modulator.h"
#include "RadioControl.h"

class JS8Scheduler {
public:
    JS8Scheduler(JS8& js8, JS8Demodulator& demod, JS8Modulator& mod, RadioControl& radio);
    void begin();
    void update();
private:
    JS8& js8Ref;
    JS8Demodulator& demodRef;
    JS8Modulator& modRef;
    RadioControl& radioRef;
    static time_t lastCycleTime;
    static const uint16_t cycleDuration;
    void triggerDemodulation();
    void triggerTransmission();
};

#endif