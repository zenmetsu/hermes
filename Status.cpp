#include "Status.h"
#include "font_Arial.h"  // For Arial font
#include "Operator.h"    // For operator defines

Status::Status(DisplayManager& disp, Utils& u)
  : display(disp), utils(u) {
  // Initialize buffer to black
  memset(buffer, 0, sizeof(buffer));
}

void Status::update() {
  // Clear buffer (black background)
  for (int x = 0; x < 40; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      buffer[x][y] = COLOR_BLACK;
    }
  }

  // Prepare text: callsign, date, time
  String callsign = CALLSIGN;
  String date = String(year()) + "-" + String(month()) + "-" + String(day());
  String time = String(hour()) + ":" + String(minute()) + ":" + String(second());

  // Set font and color for callsign (Arial 14)
  display.setFont(Arial_14);
  display.setTextColor(COLOR_WHITE);
  
  // Calculate centering for callsign (top row, ~y=70)
  int16_t callsignWidth = callsign.length() * 10;  // Approx 10px/char for Arial 14
  int16_t callsignX = (SCREEN_WIDTH - callsignWidth) / 2;
  display.setCursor(callsignX, 70);
  display.print(callsign.c_str());

  // Switch to built-in font (size 1) for date and time
  display.setFont();  // Reset to built-in font
  display.setTextSize(1);  // Smallest size, 6x8 pixels
  display.setTextColor(COLOR_WHITE);

  // Date (middle row, ~y=84)
  int16_t dateWidth = date.length() * 6;  // Approx 6px/char for built-in font
  int16_t dateX = (SCREEN_WIDTH - dateWidth) / 2;
  display.setCursor(dateX, 84);
  display.print(date.c_str());

  // Time (bottom row, ~y=98)
  int16_t timeWidth = time.length() * 6;  // Approx 6px/char for built-in font
  int16_t timeX = (SCREEN_WIDTH - timeWidth) / 2;
  display.setCursor(timeX, 98);
  display.print(time.c_str());
}

void Status::render() {
  // Render status region (y=68 to y=107)
  for (int x = 0; x < 40; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      display.drawPixel(y, 68 + x, buffer[x][y]);
    }
  }
  display.updateScreenAsync();  // Full screen update
}