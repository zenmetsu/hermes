#include <Audio.h>
#include <SPI.h>
#include <ILI9341_t3n.h>
#include <Wire.h>
#include <arm_math.h>
#include <USBHost_t36.h>
#include <ctype.h>
#include <TimeLib.h>
#include "operator.h"

// Display pin definitions
#define TFT_DC   29
#define TFT_CS   28
#define TFT_RST  24
#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_MISO 12

// UI regions (top to bottom)
#define VISUALIZER_Y 0
#define VISUALIZER_HEIGHT 60
#define INDICATOR_Y (VISUALIZER_Y + VISUALIZER_HEIGHT)
#define INDICATOR_HEIGHT 20
#define STATUS_Y (INDICATOR_Y + INDICATOR_HEIGHT)
#define STATUS_HEIGHT 40
#define INPUT_Y (STATUS_Y + STATUS_HEIGHT)
#define INPUT_HEIGHT 20
#define OUTPUT_Y (INPUT_Y + INPUT_HEIGHT)
#define OUTPUT_HEIGHT 100
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Audio sampling parameters
#define SAMPLE_RATE 44100U
#define BUFFER_SECONDS 60U
#define BUFFER_SIZE (SAMPLE_RATE * BUFFER_SECONDS)
#define HALF_BUFFER (SAMPLE_RATE * 30U)
#define SAMPLES_PER_ROW (HALF_BUFFER / VISUALIZER_HEIGHT)

// Color definitions (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_DARKGREY  0x4208
#define COLOR_GREY      0x8410
#define COLOR_LIGHTGREY 0xC618
#define COLOR_SEAFOAM   0x4EF4

// Frequency range
#define MIN_FREQ 300    // Hz
#define MAX_FREQ 3600   // Hz

// USB Host and Keyboard
USBHost usbHost;
USBHIDParser hid(usbHost);
KeyboardController keyboard(usbHost);

// Audio objects
AudioControlSGTL5000 sgtl5000;
AudioInputI2S         audioInput;
AudioAnalyzeFFT1024   fft1024;
AudioRecordQueue      queue1;
AudioConnection       patchCord1(audioInput, 0, fft1024, 0);
AudioConnection       patchCord2(audioInput, 0, queue1, 0);

// Display object
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);

// PSRAM circular buffer
EXTMEM int16_t audioBuffer[BUFFER_SIZE];
volatile uint32_t bufferWritePos = 0;
volatile uint32_t samplesCollected = 0;

// Waterfall buffers
uint16_t realtimeWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];
uint16_t bufferedWaterfall[VISUALIZER_HEIGHT][SCREEN_WIDTH];

// FFT for buffered data
float fftBuffer[1024];
float fftOutput[1024];
arm_rfft_fast_instance_f32 fft_instance;
float maxMagnitudeRT = 0.0, maxMagnitudeBuf = 0.0;
float gainFactorRT = 1.0, gainFactorBuf = 1.0;
const float GAIN_DECAY = 0.99;

// Precomputed FFT magnitudes for buffered waterfall
float bufferedMagnitudes[VISUALIZER_HEIGHT][SCREEN_WIDTH];
uint32_t lastUpdatePos = 0;

// UI state
enum VisualizerMode { VISUALIZER_REALTIME, VISUALIZER_BUFFERED };
VisualizerMode currentVisualizer = VISUALIZER_BUFFERED;  // Default to buffered
char inputBuffer[53] = "";  // 52 chars + null terminator
size_t inputCursor = 0;
unsigned long lastStatusUpdate = 0;
unsigned long lastScreenUpdate = 0;
const unsigned long SCREEN_UPDATE_INTERVAL = 33;  // ~30 Hz (33.33 ms)

void setup() {
  Serial.begin(115200);
  
  AudioMemory(60);
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000.volume(0.5);
  fft1024.windowFunction(AudioWindowHanning1024);
  queue1.begin();

  arm_rfft_fast_init_f32(&fft_instance, 1024);

  usbHost.begin();
  // Author uses a steno keyboard. This complicates matters
  // as some keys are sent translated, and some have to be detected as raw.
  // For instance, there is no ENTER key on a steno keyboard, rather a chord
  // such as R*R is used which sends CR/LF.  We must attach both callbacks
  // and ensure that there are no duplicates betwixt the two.
  keyboard.attachPress(OnPress);    // For most key detection
  keyboard.attachRawPress(OnRawPress); // For raw CR/LF detection

  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) setTime(0, 0, 0, 1, 1, 2025);

  delay(3000);
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(COLOR_BLACK);
  memset(realtimeWaterfall, 0, sizeof(realtimeWaterfall));
  memset(bufferedWaterfall, 0, sizeof(bufferedWaterfall));
  memset(bufferedMagnitudes, 0, sizeof(bufferedMagnitudes));
  tft.useFrameBuffer(true);
  
  memset(audioBuffer, 0, sizeof(audioBuffer));
  
  drawUI();
}

void loop() {
  usbHost.Task();
  
  if (fft1024.available()) {
    updateRealtimeWaterfall();
    updateBufferedWaterfall();
    renderVisualizer();
  }
  
  if (queue1.available() >= 1) {
    int16_t *audioData = queue1.readBuffer();
    updateAudioBuffer(audioData, 128);
    queue1.freeBuffer();
  }

  // Update status every second
  if (millis() - lastStatusUpdate >= 1000) {
    renderStatus();
    lastStatusUpdate = millis();
  }

  // Update screen at 30 Hz
  if (millis() - lastScreenUpdate >= SCREEN_UPDATE_INTERVAL) {
    tft.updateScreenAsync();
    lastScreenUpdate = millis();
  }
}

void updateAudioBuffer(const int16_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    audioBuffer[bufferWritePos] = data[i];
    bufferWritePos = (bufferWritePos + 1) % BUFFER_SIZE;
    if (samplesCollected < BUFFER_SIZE) samplesCollected++;
  }
}

void updateRealtimeWaterfall() {
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

void updateBufferedWaterfall() {
  uint32_t samplesToProcess = min(samplesCollected, HALF_BUFFER);
  if (samplesToProcess < 1024) return;

  uint32_t samplesSinceLast = (bufferWritePos >= lastUpdatePos) ? 
                              (bufferWritePos - lastUpdatePos) : 
                              (BUFFER_SIZE - lastUpdatePos + bufferWritePos);
  if (samplesSinceLast < SAMPLES_PER_ROW) return;

  for (int x = 0; x < VISUALIZER_HEIGHT - 1; x++) {
    memcpy(bufferedMagnitudes[x], bufferedMagnitudes[x + 1], SCREEN_WIDTH * sizeof(float));
    memcpy(bufferedWaterfall[x], bufferedWaterfall[x + 1], SCREEN_WIDTH * sizeof(uint16_t));
  }

  uint32_t fftStartPos = (bufferWritePos - 1024 + BUFFER_SIZE) % BUFFER_SIZE;
  for (int i = 0; i < 1024; i++) {
    fftBuffer[i] = audioBuffer[(fftStartPos + i) % BUFFER_SIZE] / 32768.0f;
  }
  
  for (int i = 0; i < 1024; i++) {
    fftBuffer[i] *= 0.5 * (1 - cos(2 * PI * i / 1023.0));
  }
  
  arm_rfft_fast_f32(&fft_instance, fftBuffer, fftOutput, 0);
  
  float binHz = SAMPLE_RATE / 1024.0;
  int minBin = MIN_FREQ / binHz;
  int maxBin = MAX_FREQ / binHz;
  int binRange = maxBin - minBin;
  
  float localMax = 0.0;
  float avgMagnitude = 0.0;
  int binCount = 0;
  
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = sqrt(fftOutput[2 * bin] * fftOutput[2 * bin] + 
                     fftOutput[2 * bin + 1] * fftOutput[2 * bin + 1]);
    bufferedMagnitudes[VISUALIZER_HEIGHT - 1][y] = mag;
    if (mag > localMax) localMax = mag;
    avgMagnitude += mag;
    binCount++;
  }
  avgMagnitude /= binCount;
  if (localMax > maxMagnitudeBuf) maxMagnitudeBuf = localMax;
  else maxMagnitudeBuf *= GAIN_DECAY;
  gainFactorBuf = maxMagnitudeBuf > 0 ? 1.0 / maxMagnitudeBuf : 1.0;

  for (int y = 0; y < SCREEN_WIDTH; y++) {
    float magnitude = bufferedMagnitudes[VISUALIZER_HEIGHT - 1][y] * gainFactorBuf;
    float logMag = log10(1.0 + magnitude * 10.0);
    float normalizedMag = logMag / log10(11.0);
    float t1 = 0.4, t2 = 0.5, t3 = 0.6, t4 = 0.7, t5 = 0.8;
    if (normalizedMag > t5) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_SEAFOAM;
    else if (normalizedMag > t4) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_WHITE;
    else if (normalizedMag > t3) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_LIGHTGREY;
    else if (normalizedMag > t2) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_GREY;
    else if (normalizedMag > t1) bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_DARKGREY;
    else bufferedWaterfall[VISUALIZER_HEIGHT - 1][y] = COLOR_BLACK;
  }

  lastUpdatePos = bufferWritePos;
}

void drawUI() {
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

void renderVisualizer() {
  for (int x = 0; x < VISUALIZER_HEIGHT; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      if (currentVisualizer == VISUALIZER_REALTIME) {
        tft.drawPixel(y, VISUALIZER_Y + (VISUALIZER_HEIGHT - 1 - x), realtimeWaterfall[x][y]);
      } else {
        tft.drawPixel(y, VISUALIZER_Y + (VISUALIZER_HEIGHT - 1 - x), bufferedWaterfall[x][y]);
      }
    }
  }
  // Removed updateScreenAsync() from here
}

void renderIndicator() {
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

void renderStatus() {
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

void renderInput() {
  tft.fillRect(0, INPUT_Y, SCREEN_WIDTH, INPUT_HEIGHT, COLOR_GREY);
  tft.setTextColor(COLOR_BLACK, COLOR_GREY);
  tft.setTextSize(1);
  tft.setCursor(10, INPUT_Y + 6);
  tft.print(inputBuffer);
  int cursorX = 10 + inputCursor * 6;
  tft.drawFastVLine(cursorX, INPUT_Y + 2, INPUT_HEIGHT - 4, COLOR_BLACK);
}

void renderOutput() {
  tft.fillRect(0, OUTPUT_Y, SCREEN_WIDTH, OUTPUT_HEIGHT, COLOR_BLACK);
  tft.setTextColor(COLOR_WHITE, COLOR_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, OUTPUT_Y + 10);
  tft.print("No messages yet");
}

void OnPress(int key) {
  // Ignore CR and LF here since we'll handle them in OnRawPress
  if (key == '\r' || key == '\n') {
    return;
  }

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
        inputBuffer[inputCursor] = (char)toupper(key); // Convert to uppercase
        inputCursor++;
        inputBuffer[inputCursor] = '\0';
      }
      break;
  }
  renderInput();
}

// Handle raw keycodes (CR/LF detection)
void OnRawPress(uint8_t keycode) {
  Serial.println(keycode);
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
    default:
      break;      
  }
  renderInput();
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}