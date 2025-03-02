#include <Audio.h>
#include <extRAM_t4.h>
#include <TimeLib.h>
#include "js8.h" // Simplified, 12800 Hz
#include "pack.h" // Adjusted for 12800 Hz

AudioInputI2S audioInput;
AudioRecordQueue queue;
AudioOutputI2S audioOutput;
AudioPlayQueue playQueue;
AudioConnection patchCord1(audioInput, 0, queue, 0);
AudioConnection patchCord2(playQueue, 0, audioOutput, 0);

ExtRAM_t4 extRAM;
int16_t *inputBuffer; // 44100 Hz PSRAM buffer
int16_t *resampledBuffer; // 12800 Hz PSRAM buffer
volatile uint32_t writeIdx = 0, readIdx = 0;
const uint32_t bufferSize = 44100 * 15; // 15-second buffer
const uint32_t resampledSize = 12800 * 15;
std::vector<double> rxSamples(resampledSize);
uint32_t rxSampleCount = 0;

std::string mycall = "YOURCALL"; // Set via config
std::string mygrid = "YOURGRID";
std::string rx_call, rx_buf, tx_buf;
double rx_hz = 0, rx_snr = 0, tx_hz = -1;
bool transmitting = false;

void setup() {
    Serial.begin(115200);
    AudioMemory(10);
    extRAM.begin();
    inputBuffer = (int16_t *)extRAM.malloc(bufferSize * sizeof(int16_t));
    resampledBuffer = (int16_t *)extRAM.malloc(resampledSize * sizeof(int16_t));
    queue.begin();
    setSyncProvider(getTeensyRTCTime);
}

void loop() {
    // RX: Collect audio
    if (queue.available() >= 1) {
        int16_t *block = queue.readBuffer();
        for (int i = 0; i < 128; i++) {
            inputBuffer[writeIdx] = block[i];
            writeIdx = (writeIdx + 1) % bufferSize;
        }
        queue.freeBuffer();
    }

    // RX: Decode every 15 seconds
    if (second(now()) % 15 == 0 && !transmitting) {
        // Resample (stub, from Developer 1895998174680256538)
        resample_44100_to_12800(inputBuffer, resampledBuffer, bufferSize);
        for (int i = 0; i < resampledSize; i++) {
            rxSamples[i] = resampledBuffer[i] / 32768.0;
        }
        entry(rxSamples.data(), rxSamples.size(), 12800 * 0.5, 12800, 100, 2900, nullptr, nullptr, fate_cb);
    }

    // TX: Check keyboard input (via Serial for now)
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') tx_buf += " "; // Simplified newline handling
        else if (c == 129) tx_buf += (char)129; // CQ
        else if (c == 130) tx_buf += (char)130; // HW CPY?
        else if (c == 131) tx_buf += (char)131; // SNR
        else tx_buf += c;
    }

    // TX: Transmit on cycle
    if (second(now()) % 15 == 0 && tx_buf.length() > 0 && !transmitting) {
        if (tx_hz < 0) tx_hz = 1500; // Simplified choice
        std::vector<double> samples;
        if (tx_buf[0] == (char)129) {
            samples = pack_cq(mycall, mygrid, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else if (tx_buf[0] == (char)130 && rx_call.length() > 0) {
            samples = pack_directed(mycall, rx_call, 19, 0, 3, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else if (tx_buf[0] == (char)131 && rx_call.length() > 0) {
            samples = pack_directed(mycall, rx_call, 25, rx_snr + 31, 3, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else {
            int consumed;
            samples = pack_text(tx_buf, consumed, 2, 12800, tx_hz);
            tx_buf.erase(0, consumed);
        }
        transmit(samples);
    }
}

int fate_cb(int *a87, double hz0, double hz1, double off, const char *comment, double snr, int pass, int correct_bits) {
    std::string other_call, txt = unpack(a87, other_call);
    Serial.printf("Decode: %.0f Hz, SNR=%.1f, %s\n", hz0, snr, txt.c_str());
    if (other_call == rx_call) {
        rx_buf += txt;
        rx_snr = snr;
    }
    return 2; // Simplified, always subtract
}

void transmit(std::vector<double> samples) {
    transmitting = true;
    for (int i = 0; i < samples.size(); i += 128) {
        int16_t *buffer = playQueue.getBuffer();
        for (int j = 0; j < 128 && (i + j) < samples.size(); j++) {
            buffer[j] = (int16_t)(samples[i + j] * 16380);
        }
        playQueue.playBuffer();
        while (playQueue.available() > 0) delay(1); // Wait for buffer to play
    }
    transmitting = false;
}

time_t getTeensyRTCTime() {
    return Teensy3Clock.get();
}

// Stub for resampling (from Developer 1895998174680256538)
void resample_44100_to_12800(int16_t *in, int16_t *out, uint32_t len) {
    // Placeholder until resampling code is provided
    for (int i = 0; i < resampledSize; i++) out[i] = in[i % len];
}