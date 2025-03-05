#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <ILI9341_t3n.h>
#include <USBHost_t36.h>
#include "config.h"
#include "operator.h"
#include "Buffers.h"
#include "JS8.h"
#include "JS8Modulator.h" // Added

class UIManager {
public:
    UIManager(ILI9341_t3n &display, JS8& js8, JS8Modulator& mod); // Updated constructor
    void begin();
    void update();
    void handleKeyPress(int key);
    void handleRawKeyPress(uint8_t keycode);
private:
    ILI9341_t3n &tft;
    JS8& js8Ref;
    JS8Modulator& js8ModRef; // Added reference to JS8Modulator
    enum VisualizerMode { VISUALIZER_REALTIME, VISUALIZER_BUFFERED };
    VisualizerMode currentVisualizer;
    size_t inputCursor;
    unsigned long lastStatusUpdate;
    unsigned long lastScreenUpdate;

    void drawUI();
    void renderVisualizer();
    void renderIndicator();
    void renderStatus();
    void renderInput();
    void renderOutput();
    void updateRealtimeWaterfall();
};

#endif