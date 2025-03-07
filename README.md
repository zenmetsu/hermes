# Hermes - Teensy 4.1 Multi-Mode Amateur Radio Transceiver

![Build Status](https://github.com/zenmetsu/hermes/actions/workflows/build.yml/badge.svg)
![CodeQL](https://github.com/zenmetsu/hermes/actions/workflows/codeql-analysis.yml/badge.svg)
![License](https://img.shields.io/github/license/zenmetsu/hermes?cacheSeconds=0)
![Stars](https://img.shields.io/github/stars/zenmetsu/hermes?style=social)
![Forks](https://img.shields.io/github/forks/zenmetsu/hermes?style=social)
![Teensy](https://img.shields.io/badge/Teensy-4.1-blue)

## IMPORTANT NOTE
This repository is under active development. Please don’t purchase hardware expecting a fully functional device yet—it will take time for the code to mature into a complete transceiver.

## Overview
Hermes is an experimental software-defined radio (SDR) application designed to transmit and receive digital mode signals using a Teensy 4.1 microcontroller and a Revision D Teensy Audio Adaptor. The initial focus is on implementing JS8, a weak-signal digital mode derived from FT8, due to the scarcity of JS8 microcontroller implementations.

JS8 offers multiple speed modes:
- **Turbo**: 6s slot, 25 baud
- **Fast**: 10s slot, 10 baud
- **Normal**: 15s slot, 6.25 baud
- **Slow**: 30s slot, 2 baud

The project currently supports JS8 Normal mode transmission with a placeholder 1000 Hz sine wave, with plans to expand to full modulation, demodulation, and additional modes. It leverages the Teensy 4.1’s 600 MHz Cortex-M7 processor and 8 MB PSRAM for real-time audio processing, making it a compact, standalone solution for JS8 communication when paired with a compatible radio transceiver.

This is a learning project in C++ and digital signal processing (DSP), iteratively developed to enhance code organization and functionality.

## Design Criteria for Initial JS8 Implementation
- **Receive**: Processes 44.1 kHz audio input with a 4096-point FFT (~10.77 Hz/bin resolution) for the buffered waterfall; 1024-point FFT (~43 Hz/bin) for real-time visualization. Demodulation of JS8 Normal mode signals is planned but not yet implemented.
- **Transmit**: Generates a 12.6-second JS8 Normal mode signal at 22050 Hz, 16-bit PCM, with a 32-bit header (mode `0x82`, sample count). Currently outputs a 1000 Hz sine wave, with silence padding to 30 seconds.
- **Real-Time**: Uses interrupt-driven audio buffering, non-blocking scheduling, and a state machine for transmission to keep the UI responsive.
- **Diagnostics**: Provides timestamped debug messages via Serial (115200 baud).
- **Modular Design**: Organized into classes (`AudioManager`, `UIManager`, `JS8Scheduler`, etc.) for maintainability.
- **Display**: Features a 320x240 ILI9341 TFT with real-time and buffered waterfalls, updated at ~30 Hz.

## Hardware Prerequisites
- **Teensy 4.1**:
  - 600 MHz Cortex-M7, 1 MB RAM (512 KB DTCM, 512 KB OCRAM), 8 MB PSRAM required.
  - PSRAM stores a 5.17 MB `audioBuffer` (60s at 44.1 kHz) and a 2.65 MB `transmitBuffer` (30s at 22050 Hz, packed 32-bit).
- **Revision D Teensy Audio Adaptor**:
  - Stereo I2S audio I/O.
  - Input: Line In (left channel) from radio audio output (e.g., 3.5mm jack).
  - Output: Line Out (left channel) to radio for transmission.
- **Radio Transceiver**:
  - HF bands (e.g., 40m, 20m) for JS8 frequencies.
  - Audio I/O via 3.5mm jacks; PTT via GPIO (pin TBD, currently a stub).
- **ILI9341 TFT Display**:
  - 320x240 resolution, SPI interface (pins defined in `config.h`).
- **USB Connection**:
  - For power, programming, and serial debugging.

## Software Requirements
- **Arduino IDE**: With Teensyduino addon for Teensy 4.1 support.
- **Libraries**:
  - `Audio.h` (Teensy Audio Library): I2S audio processing and playback.
  - `TimeLib.h`: RTC-based timestamps for cycle synchronization.
  - `arm_math.h` (CMSIS-DSP): FFT for signal analysis.
  - `ILI9341_t3n.h`: TFT display control.
  - `USBHost_t36.h`: USB keyboard input.
- **Dependencies**: Standard C++ libraries.

## Installation
1. **Hardware Setup**:
   - Solder the Teensy Audio Adaptor to the Teensy 4.1 (I2S pins aligned).
   - Connect radio Line Out to Audio Adaptor Line In (left channel).
   - Connect Audio Adaptor Line Out (left channel) to radio Line In.
   - Attach ILI9341 TFT to SPI pins (see `config.h`).
   - Add 8 MB PSRAM chip if not pre-installed.
   - (Optional) Connect PTT to a GPIO pin (TBD).
2. **Software Setup**:
   - Install Arduino IDE and Teensyduino.
   - Clone this repository: `git clone [repo-url]`.
   - Open `hermes.ino` in Arduino IDE.
   - Select Teensy 4.1 board and USB Serial port.
   - Compile and upload.

## Usage
- **Running**: Upload the sketch, open Serial Monitor (115200 baud) for debug output.
- **Input**: Type a message via USB keyboard; press Enter to queue for transmission.
- **Output**: Transmission occurs at the next 15-second cycle start (e.g., :00, :15), playing a 12.6s 1000 Hz tone.
- **Display**: Shows real-time (1024-point FFT) and buffered (4096-point FFT) waterfalls, status, and input buffer.

## Project Structure
- `hermes.ino`: Main sketch, setup, and loop.
- `AudioManager.*`: Audio input processing and buffered waterfall FFT.
- `UIManager.*`: TFT display and keyboard input handling.
- `JS8Scheduler.*`: Non-blocking scheduling logic for modulation/demodulation and clock-synchronized transmission.
- `JS8Modulator.*`: Waveform generation (currently a sine wave).
- `JS8Demodulator.*`, `JS8Resampler.*`, `JS8Responder.*`: Stubs for resampling, receiving, and auto-reply.
- `Buffers.*`: PSRAM buffer definitions.
- `RadioControl.*`: PTT control stub.
- `config.h`, `operator.h`: Configuration and constants.

## Status
- **Stable**: No crashes; non-blocking transmission with 4096-point FFT and UI updates.
- **Implemented**:
  - JS8 Normal mode transmission (12.6s, 22050 Hz, placeholder 1000 Hz tone).
  - Non-blocking state machine for scheduling.
  - Real-time and buffered waterfalls on TFT.
  - Keyboard input and message queuing.
- **To-Do**:
  - **Demodulation**: Implement JS8 Normal mode 8-FSK tone detection (~6.25 Hz spacing).
  - **Modulation**: Replace sine wave with proper JS8 8-FSK encoding.
  - **Modes**: Add Turbo (6s), Fast (10s), and Slow (30s) support.
  - **TX Hardware**: Implement PTT (GPIO/VOX/CAT).
  - **Profiling**: Add performance metrics (e.g., FFT timing).
  - **Cleanup**: Refine class interactions and buffer management.

## Contributing
This is a learning project—contributions are welcome! Fork, modify, and submit pull requests via GitHub.

## License
MIT License—see `LICENSE` file (to be added).
