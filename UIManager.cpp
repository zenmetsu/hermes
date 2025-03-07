#include "UIManager.h"
#include <Audio.h>
#include <arm_math.h>
#include <TimeLib.h>

// Global variables from hermes.ino
extern AudioAnalyzeFFT1024 fft1024;
extern bool transmitEnabled;
extern bool isTransmitting;

UIManager::UIManager(ILI9341_t3n &display, JS8& js8, JS8Modulator& mod)
    : tft(display),
      js8Ref(js8),
      js8ModRef(mod), // Initialize JS8Modulator reference
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
    if (key == '\r' || key == '\n') return;
    switch (key) {
        case KEY_BACKSPACE:
            if (inputCursor > 0) {
                inputCursor--;
                inputBuffer[inputCursor] = '\0';
            }
            break;
        case KEYD_LEFT:
            if (inputCursor > 0) inputCursor--;
            break;
        case KEYD_RIGHT:
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
            js8Ref.setOutputBuffer(inputBuffer); // Copy to js8OutputBuffer
            memset(inputBuffer, 0, sizeof(inputBuffer)); // Clear inputBuffer
            inputCursor = 0; // Reset cursor
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
    if (currentVisualizer == VISUALIZER_REALTIME && fft1024.available()) {
        updateRealtimeWaterfall();
    }
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
    tft.setTextSize(1);
    tft.setCursor(0, STATUS_Y + 4);
    tft.print("14.074.000");

    tft.setTextSize(1);
    tft.setTextColor(isTransmitting ? COLOR_GREY : COLOR_GREEN, COLOR_BLACK);
    tft.setCursor(SCREEN_WIDTH - 20, STATUS_Y + 4);
    tft.print("RX");

    uint16_t txColor;
    if (isTransmitting) {
        txColor = COLOR_RED;
        Serial.println("TX: Transmitting (Red)");
    } else if (transmitEnabled && js8Ref.hasPendingMessage()) {
        txColor = COLOR_ORANGE;
        Serial.println("TX: Pending (Orange)");
    } else if (transmitEnabled) {
        txColor = COLOR_GREEN;
        Serial.println("TX: Enabled (Green)");
    } else {
        txColor = COLOR_GREY;
        Serial.println("TX: Disabled (Grey)");
    }
    tft.setTextColor(txColor, COLOR_BLACK);
    tft.setCursor(SCREEN_WIDTH - 20, STATUS_Y + 14);
    tft.print("TX");

    // Existing callsign and time
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

void UIManager::updateRealtimeWaterfall() {
  for (int x = 0; x < VISUALIZER_HEIGHT - 1; x++) {
    memcpy(realtimeWaterfall[x], realtimeWaterfall[x + 1], SCREEN_WIDTH * sizeof(uint16_t));
  }
  float binHz = SAMPLE_RATE / 1024.0;
  int minBin = MIN_FREQ / binHz;
  int maxBin = MAX_FREQ / binHz;
  int binRange = maxBin - minBin;

  float localMax = 0.0;
  float avgMagnitude = 0.0;
  int binCount = 0;
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = fft1024.read(bin);
    if (mag > localMax) localMax = mag;
    avgMagnitude += mag;
    binCount++;
  }
  avgMagnitude /= binCount;

  static float maxMagnitudeRT = 0.0;
  static float gainFactorRT = 1.0;
  const float GAIN_DECAY = 0.99;
  if (localMax > maxMagnitudeRT) maxMagnitudeRT = localMax;
  else maxMagnitudeRT *= GAIN_DECAY;
  gainFactorRT = maxMagnitudeRT > 0 ? 1.0 / maxMagnitudeRT : 1.0;

  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float magnitude = fft1024.read(bin) * gainFactorRT;
    float logMag = log10(1.0 + magnitude * 10.0);
    float normalizedMag = logMag / log10(11.0);
    float t1 = 0.4, t2 = 0.5, t3 = 0.6, t4 = 0.7, t5 = 0.8;
    if (normalizedMag > t5) realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_SEAFOAM;
    else if (normalizedMag > t4) realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_WHITE;
    else if (normalizedMag > t3) realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_LIGHTGREY;
    else if (normalizedMag > t2) realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_GREY;
    else if (normalizedMag > t1) realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_DARKGREY;
    else realtimeWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_BLACK;
  }
}