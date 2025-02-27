#include <Arduino.h>
#include "signal_proc.h"
#include <vector>
#include <arm_math.h>
#include "utils.h"

static float fft_input[FFT_SIZE * 2];
static arm_cfft_radix2_instance_f32 fft_instance;

EXTMEM int16_t audio_buffer[SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE]; // 5.17 MB, 16-bit, 44.1 kHz
EXTMEM int16_t resampled_buffer[RESAMPLED_BUFFER_SIZE]; // ~444 KB, 16-bit, 12.8 kHz
volatile size_t write_idx = 0;

static bool transmitting = false;
static std::vector<int> tx_symbols;
static size_t tx_sym_idx = 0;
static float tx_time = 0.0f;
float tone_frequencies[NUM_TONES];

static const int COSTAS_PATTERN[7] = {4, 2, 5, 6, 1, 3, 0};
struct Conversation {
    float center_freq;
    int symbols[SYMBOLS_PER_MESSAGE];
    size_t sym_count;
    bool active;
};
static std::vector<Conversation> conversations;

// Profiler variables
static uint32_t init_start_time, init_end_time;
static uint32_t peripheral_start_time, peripheral_end_time;
static uint32_t demod_start_time, demod_end_time;

void init_signal_proc() {
    init_start_time = micros();
    for (int k = 0; k < NUM_TONES; k++) {
        tone_frequencies[k] = BASE_FREQUENCY + k * TONE_SPACING;
    }

    // FFT initialization with diagnostics
    extern unsigned long _ebss, _estack;
    unsigned long free_heap = (unsigned long)&_estack - (unsigned long)&_ebss;
    char msg[128];
    sprintf(msg, "[%lu] Before FFT init - FFT_SIZE: %d, Free heap: %lu bytes", micros(), FFT_SIZE, free_heap);
    Serial.println(msg);

    arm_status status = arm_cfft_radix2_init_f32(&fft_instance, FFT_SIZE, 0, 1);
    if (status != ARM_MATH_SUCCESS) {
        sprintf(msg, "[%lu] FFT init failed - Status: %d", micros(), status);
        Serial.println(msg);
    } else {
        sprintf(msg, "[%lu] FFT init succeeded", micros());
        Serial.println(msg);
    }

    write_idx = 0;
    conversations.clear();

    memset(audio_buffer, 0, sizeof(int16_t) * SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
    memset(resampled_buffer, 0, sizeof(int16_t) * RESAMPLED_BUFFER_SIZE);
    init_end_time = micros();
    sprintf(msg, "[%lu] Program initialized in %lu us", init_end_time, init_end_time - init_start_time);
    Serial.println(msg);
}

void report_buffer_write_speed() {
    peripheral_start_time = micros();
    size_t test_samples = 44100;
    uint32_t start_time = micros();
    for (size_t i = 0; i < test_samples; i++) {
        audio_buffer[i] = 0;
    }
    uint32_t end_time = micros();
    float elapsed_seconds = (end_time - start_time) / 1000000.0f;
    float write_speed = test_samples / elapsed_seconds / 1000.0f;
    peripheral_end_time = micros();
    char msg[128];
    sprintf(msg, "[%lu] Peripheral (buffer write) initialized in %lu us, speed: %.2f kSamples/sec", 
            peripheral_end_time, peripheral_end_time - peripheral_start_time, write_speed);
    Serial.println(msg);
}

void transfer_to_audio_buffer() {
    if (audio_available()) {
        int16_t* block = get_audio_block();
        if (block) {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
                audio_buffer[write_idx] = block[i];
                write_idx = (write_idx + 1) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
            }
            queue_in.freeBuffer();
        }
    }
}

void start_transmission(const char* message) {
    for (size_t i = 0; i < strlen(message); i++) {
        tx_symbols.push_back(message[i] % NUM_TONES);
    }
    if (tx_symbols.empty()) {
        tx_symbols = {0, 1, 2, 3, 4, 5, 6, 7};
    }
    tx_sym_idx = 0;
    tx_time = 0.0f;
    transmitting = true;
}

bool is_transmitting() {
    return transmitting;
}

void generate_tx_audio(int16_t* block) {
    const float sample_period = 1.0f / static_cast<float>(SAMPLE_RATE_NATIVE);
    const float symbol_period = 1.0f / SYMBOL_RATE_NORMAL;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        if (tx_sym_idx < tx_symbols.size()) {
            int tone_idx = tx_symbols[tx_sym_idx];
            float frequency = tone_frequencies[tone_idx];
            block[i] = static_cast<int16_t>(sinf(2.0f * PI * frequency * tx_time) * 32767.0f);
            tx_time += sample_period;
            if (tx_time >= symbol_period) {
                tx_time -= symbol_period;
                tx_sym_idx++;
            }
        } else {
            block[i] = 0;
            transmitting = false;
        }
    }
}

void demodulate_js8_normal() {
    demod_start_time = micros();
    char msg[128];
    sprintf(msg, "[%lu] Starting demodulation process", demod_start_time);
    Serial.println(msg);

    // Resample 44.1 kHz to 12.8 kHz
    size_t input_size = (SLOT_DURATION_NORMAL + 2.36) * SAMPLE_RATE_NATIVE; // 17.36s = 765,126 samples
    size_t output_size = RESAMPLED_BUFFER_SIZE; // ~222,208 samples
    size_t current_write_idx = write_idx;
    size_t read_start_idx = (current_write_idx - input_size + SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);

    float resample_ratio = static_cast<float>(SAMPLE_RATE_NATIVE) / SAMPLE_RATE_RESAMPLED; // ~3.4453
    float resample_acc = 0.0f;

    for (size_t i = 0, j = 0; i < input_size && j < output_size; i++) {
        resample_acc += 1.0f / resample_ratio;
        if (resample_acc >= 1.0f) {
            size_t buffer_idx = (read_start_idx + i) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
            resampled_buffer[j++] = audio_buffer[buffer_idx];
            resample_acc -= 1.0f;
        }
    }

    // Power integration over 17.36s at 12.8 kHz
    std::vector<float> power_spectrum(FFT_SIZE / 2, 0.0f);
    std::vector<float> process_buffer(FFT_SIZE);
    uint32_t window_count = 0;

    for (size_t offset = 0; offset + FFT_SIZE <= output_size; offset += FFT_SIZE) {
        for (int i = 0; i < FFT_SIZE; i++) {
            process_buffer[i] = resampled_buffer[offset + i] / 32768.0f * (0.54f - 0.46f * cosf(2.0f * PI * i / (FFT_SIZE - 1)));
            fft_input[2 * i] = process_buffer[i];
            fft_input[2 * i + 1] = 0.0f;
        }
        arm_cfft_radix2_f32(&fft_instance, fft_input);
        arm_cmplx_mag_f32(fft_input, std::vector<float>(FFT_SIZE / 2).data(), FFT_SIZE / 2);

        // Accumulate power
        for (size_t bin = 0; bin < FFT_SIZE / 2; bin++) {
            power_spectrum[bin] += fft_input[bin];
        }
        window_count++;
    }

    // Average power per bin
    for (size_t bin = 0; bin < FFT_SIZE / 2; bin++) {
        power_spectrum[bin] /= window_count;
    }

    // Find top 10 tone sets by power
    struct ToneSet {
        float center_freq; // Frequency of tone 4
        float total_power;
    };
    std::vector<ToneSet> tone_sets;
    for (int bin = 48; bin <= 568 - NUM_TONES; bin++) {
        float total_power = 0.0f;
        bool is_tone_set = true;
        for (int k = 0; k < NUM_TONES; k++) {
            int tone_bin = bin + k;
            if (tone_bin >= FFT_SIZE / 2 || power_spectrum[tone_bin] < 0.5f) {
                is_tone_set = false;
                break;
            }
            total_power += power_spectrum[tone_bin];
        }
        if (is_tone_set) {
            float freq = (bin + 4) * (static_cast<float>(SAMPLE_RATE_RESAMPLED) / FFT_SIZE); // Tone 4’s frequency
            tone_sets.push_back({freq, total_power});
        }
    }

    // Sort and limit to top 10
    std::sort(tone_sets.begin(), tone_sets.end(), 
              [](const ToneSet& a, const ToneSet& b) { return a.total_power > b.total_power; });
    if (tone_sets.size() > 10) tone_sets.resize(10);

    // Log top 10 tone sets
    for (size_t i = 0; i < tone_sets.size(); i++) {
        sprintf(msg, "[%lu] Top tone set %u at %.2f Hz, power: %.4f", micros(), (unsigned)i + 1, tone_sets[i].center_freq, tone_sets[i].total_power);
        Serial.println(msg);
    }

    std::vector<std::vector<int>> symbol_streams(tone_sets.size(), std::vector<int>(SYMBOLS_PER_SLOT, -1));
    std::vector<float> power_spectrum_temp(FFT_SIZE / 2);

    if (!tone_sets.empty()) {
        conversations.resize(tone_sets.size());
        for (size_t c = 0; c < tone_sets.size(); c++) {
            conversations[c].center_freq = tone_sets[c].center_freq;
            conversations[c].sym_count = 0;
            conversations[c].active = true;

            // Extract 108 symbols per tone set
            for (size_t sym_idx = 0; sym_idx < SYMBOLS_PER_SLOT; sym_idx++) {
                size_t offset = sym_idx * SYMBOL_SAMPLES_NORMAL; // 2048 samples (~0.16s at 12.8 kHz)
                if (offset + FFT_SIZE > output_size) break;

                for (int i = 0; i < FFT_SIZE; i++) {
                    process_buffer[i] = resampled_buffer[offset + i] / 32768.0f * (0.54f - 0.46f * cosf(2.0f * PI * i / (FFT_SIZE - 1)));
                    fft_input[2 * i] = process_buffer[i];
                    fft_input[2 * i + 1] = 0.0f;
                }
                arm_cfft_radix2_f32(&fft_instance, fft_input);
                arm_cmplx_mag_f32(fft_input, power_spectrum_temp.data(), FFT_SIZE / 2);

                int base_bin = static_cast<int>(tone_sets[c].center_freq / TONE_SPACING) - 4; // Tone 0’s bin
                int max_tone = 0;
                float max_power = 0.0f;
                for (int k = 0; k < NUM_TONES; k++) {
                    int tone_bin = base_bin + k;
                    if (tone_bin >= 0 && tone_bin < FFT_SIZE / 2) {
                        if (power_spectrum_temp[tone_bin] > max_power) {
                            max_power = power_spectrum_temp[tone_bin];
                            max_tone = k;
                        }
                    }
                }
                symbol_streams[c][sym_idx] = max_tone;
            }
        }

        // Costas synchronization
        sprintf(msg, "[%lu] Starting Costas synchronization", micros());
        Serial.println(msg);
        for (size_t c = 0; c < tone_sets.size(); c++) {
            float max_corr = 0.0f;
            size_t best_offset = 0;
            for (size_t offset = 0; offset + SYMBOLS_PER_MESSAGE <= SYMBOLS_PER_SLOT; offset++) {
                float corr_start = 0.0f, corr_mid = 0.0f, corr_end = 0.0f;
                for (int k = 0; k < 7; k++) {
                    if (symbol_streams[c][offset + k] == COSTAS_PATTERN[k]) corr_start += 1.0f;
                    if (offset + 36 + k < SYMBOLS_PER_SLOT && symbol_streams[c][offset + 36 + k] == COSTAS_PATTERN[k]) corr_mid += 1.0f;
                    if (offset + 72 + k < SYMBOLS_PER_SLOT && symbol_streams[c][offset + 72 + k] == COSTAS_PATTERN[k]) corr_end += 1.0f;
                }
                float total_corr = (corr_start + corr_mid + corr_end) / 3.0f;
                if (total_corr > max_corr) {
                    max_corr = total_corr;
                    best_offset = offset;
                }
            }

            if (max_corr >= 4.0f) {
                sprintf(msg, "[%lu] Tone set at %.2f Hz, Costas corr: %.1f", micros(), tone_sets[c].center_freq, max_corr);
                Serial.println(msg);

                if (max_corr >= 5.0f) {
                    for (int i = 0; i < SYMBOLS_PER_MESSAGE; i++) {
                        conversations[c].symbols[i] = symbol_streams[c][best_offset + i];
                    }
                    conversations[c].sym_count = SYMBOLS_PER_MESSAGE;

                    sprintf(msg, "[%lu] Message at %.2f Hz: ", micros(), tone_sets[c].center_freq);
                    Serial.print(msg);
                    for (int i = 0; i < SYMBOLS_PER_MESSAGE; i++) {
                        Serial.print(conversations[c].symbols[i]);
                        Serial.print(" ");
                    }
                    Serial.println();
                    Serial.flush();
                    conversations[c].active = false;
                }
            }
        }
    }

    demod_end_time = micros();
    sprintf(msg, "[%lu] Demodulation completed in %lu us", demod_end_time, demod_end_time - demod_start_time);
    Serial.println(msg);
    sprintf(msg, "[%lu] End of demodulation process", demod_end_time);
    Serial.println(msg);
}