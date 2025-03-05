#include "RadioControl.h"
#include <Arduino.h>

void RadioControl::startTransmission() {
    // Placeholder: Set PTT pin high (e.g., pin 10)
    Serial.println("PTT ON"); // For debugging
    // digitalWrite(10, HIGH); // Uncomment when pin is assigned
}

void RadioControl::stopTransmission() {
    // Placeholder: Reset PTT pin
    Serial.println("PTT OFF"); // For debugging
    // digitalWrite(10, LOW); // Uncomment when pin is assigned
}