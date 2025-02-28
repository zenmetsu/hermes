#ifndef STATUS_H
#define STATUS_H

#include "DisplayManager.h"
#include "Utils.h"
#include "Config.h"

// Class to manage the Status window (320x40 region)
// Purpose: Displays operator callsign, date, and time
class Status {
public:
  Status(DisplayManager& display, Utils& utils);  // Constructor
  void update();  // Update status buffer with current data
  void render();  // Render status region (y=68 to y=107)

private:
  DisplayManager& display;  // Reference to display manager
  Utils& utils;             // Reference to utils for RTC
  uint16_t buffer[40][SCREEN_WIDTH];  // Status pixel buffer (320x40)
};

#endif