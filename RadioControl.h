#ifndef RADIO_CONTROL_H
#define RADIO_CONTROL_H

class RadioControl {
public:
    void startTransmission(); // Set PTT high or send CAT command
    void stopTransmission();  // Reset PTT or send CAT stop
};

#endif