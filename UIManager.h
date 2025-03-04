#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <ILI9341_t3n.h>
#include "config.h"
#include "operator.h"
#include "Buffers.h"

class UIManager {
public:
  UIManager(ILI9341_t3n &display);
  void begin();
  void update();
  void handleKeyPress(int key);
  void handleRawKeyPress(uint8_t keycode);

private:
  ILI9341_t3n &tft;
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
};

#endif