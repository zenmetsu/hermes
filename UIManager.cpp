#include "UIManager.h"
#include <arm_math.h>
#include <TimeLib.h> // Added for year(), month(), etc.

UIManager::UIManager(ILI9341_t3n &display) 
  : tft(display), 
    currentVisualizer(VISUALIZER_BUFFERED), 
    inputCursor(0), 
    lastStatusUpdate(0), 
    lastScreenUpdate(0) {}

void UIManager::begin() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(COLOR_BLACK);
  tft.useFrameBuffer(true);
  drawUI();
}

void UIManager::update() {
  if (millis() - lastStatusUpdate >= 1000) {
    renderStatus();
    lastStatusUpdate = millis();
  }
  if (millis() - lastScreenUpdate >= SCREEN_UPDATE_INTERVAL) {
    renderVisualizer();
    tft.updateScreenAsync();
    lastScreenUpdate = millis();
  }
}

void UIManager::handleKeyPress(int key) {
  if (key == '\r' || key == '\n') return; // Handled in raw press
  switch (key) {
    case KEY_BACKSPACE:
      if (inputCursor > 0) {
        inputCursor--;
        inputBuffer[inputCursor] = '\0';
      }
      break;
    case KEY_LEFT:  // Changed from KEYD_LEFT
      if (inputCursor > 0) inputCursor--;
      break;
    case KEY_RIGHT: // Changed from KEYD_RIGHT
      if (inputCursor < strlen(inputBuffer)) inputCursor++;
      break;
    default:
      if (isPrintable(key) && inputCursor < 52) {
        inputBuffer[inputCursor] = (char)toupper(key);
        inputCursor++;
        inputBuffer[inputCursor] = '\0';
      }
      break;
  }
  renderInput();
}

void UIManager::handleRawKeyPress(uint8_t keycode) {
  switch (keycode) {
    case 0x28: // CR
      memset(inputBuffer, 0, sizeof(inputBuffer));
      inputCursor = 0;
      break;
    case 0x2A: // Backspace
      if (inputCursor > 0) {
        inputCursor--;
        inputBuffer[inputCursor] = '\0';
      }
      break;
  }
  renderInput();
}

void UIManager::drawUI() {
  tft.fillRect(0, VISUALIZER_Y, SCREEN_WIDTH, VISUALIZER_HEIGHT, COLOR_BLACK);
  tft.fillRect(0, INDICATOR_Y, SCREEN_WIDTH, INDICATOR_HEIGHT, COLOR_DARKGREY);
  tft.fillRect(0, STATUS_Y, SCREEN_WIDTH, STATUS_HEIGHT, COLOR_BLACK);
  tft.fillRect(0, INPUT_Y, SCREEN_WIDTH, INPUT_HEIGHT, COLOR_GREY);
  tft.fillRect(0, OUTPUT_Y, SCREEN_WIDTH, OUTPUT_HEIGHT, COLOR_BLACK);
  renderIndicator();
  renderStatus();
  renderInput();
  renderOutput();
}

void UIManager::renderVisualizer() {
  // Simplified for now; detailed waterfall logic can be added later
  for (int x = 0; x < VISUALIZER_HEIGHT; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      tft.drawPixel(y, VISUALIZER_Y + (VISUALIZER_HEIGHT - 1 - x), 
                    currentVisualizer == VISUALIZER_REALTIME ? realtimeWaterfall[x][y] : bufferedWaterfall[x][y]);
    }
  }
}

void UIManager::renderIndicator() {
  tft.setTextColor(COLOR_WHITE, COLOR_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(0, INDICATOR_Y + 4);
  tft.print("300 Hz");
  tft.setCursor(SCREEN_WIDTH - 50, INDICATOR_Y + 4);
  tft.print("3600 Hz");
  for (int x = 0; x < SCREEN_WIDTH; x += 32) {
    tft.drawFastVLine(x, INDICATOR_Y, INDICATOR_HEIGHT, COLOR_GREY);
  }
}

void UIManager::renderStatus() {
  tft.fillRect(0, STATUS_Y, SCREEN_WIDTH, STATUS_HEIGHT, COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(2);
  int callsignWidth = strlen(CALLSIGN) * 12;
  int callsignX = (SCREEN_WIDTH - callsignWidth) / 2;
  tft.setCursor(callsignX, STATUS_Y + 2);
  tft.print(CALLSIGN);

  char dateText[11];
  sprintf(dateText, "%04d-%02d-%02d", year(), month(), day());
  tft.setTextSize(1);
  int dateWidth = strlen(dateText) * 6;
  int dateX = (SCREEN_WIDTH - dateWidth) / 2;
  tft.setCursor(dateX, STATUS_Y + 20);
  tft.print(dateText);

  char timeText[9];
  sprintf(timeText, "%02d:%02d:%02d", hour(), minute(), second());
  int timeWidth = strlen(timeText) * 6;
  int timeX = (SCREEN_WIDTH - timeWidth) / 2;
  tft.setCursor(timeX, STATUS_Y + 30);
  tft.print(timeText);
}

void UIManager::renderInput() {
  tft.fillRect(0, INPUT_Y, SCREEN_WIDTH, INPUT_HEIGHT, COLOR_GREY);
  tft.setTextColor(COLOR_BLACK, COLOR_GREY);
  tft.setTextSize(1);
  tft.setCursor(10, INPUT_Y + 6);
  tft.print(inputBuffer);
  int cursorX = 10 + inputCursor * 6;
  tft.drawFastVLine(cursorX, INPUT_Y + 2, INPUT_HEIGHT - 4, COLOR_BLACK);
}

void UIManager::renderOutput() {
  tft.fillRect(0, OUTPUT_Y, SCREEN_WIDTH, OUTPUT_HEIGHT, COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, OUTPUT_Y + 10);
  tft.print("No messages yet");
}