# Audio Output Options for ESP32 CYD Projects

The ESP32 Cheap Yellow Display (CYD) boards typically include a 2.8 inch TFT and a touch controller but no integrated speaker.  Adding sound effects and music to your game requires choosing a suitable audio output method.  This document describes built‑in capabilities of the ESP32 and practical approaches for generating audio on CYD boards.

## 1. Built‑in audio capabilities

### DAC pins

The ESP32 has two **8‑bit Digital‑to‑Analog Converter (DAC)** channels connected internally to **GPIO25** and **GPIO26**.  Each DAC can convert a digital value (0–255) to an analog voltage between 0 V and a reference voltage【59653963732276†L75-L94】.  You can output simple waveforms by writing values to these DAC channels at a fixed rate.  However, 8‑bit resolution limits audio fidelity, and driving a speaker directly from the DAC may require an amplifier.

### PWM via LEDC

The **LEDC** peripheral in ESP32 generates high‑frequency pulse‑width modulation (PWM) signals on arbitrary GPIO pins【190466416380224†L75-L107】.  Although designed for LED brightness control, PWM can be filtered to produce audio.  The LEDC module supports up to 8 channels and allows you to set frequency and resolution (e.g., 8–16 bits).  Use `ledcWriteTone()` or `ledcWrite()` to generate square waves or tones; this method is convenient for beeps and 8‑bit style sounds.

### I2S interface

For high‑quality audio, connect an **I2S DAC** (e.g., MAX98357A, PCM5102) to the ESP32.  The ESP32’s I2S peripheral can output 16‑bit or 24‑bit PCM audio at sample rates up to 48 kHz.  Many libraries (e.g., ESP8266Audio) support streaming WAV, MP3 or Ogg files via I2S from flash or SD card.

## 2. Simple sound generation

### Using tone() (not recommended)

The Arduino `tone()` function produces square waves on a single pin.  On ESP32 the implementation uses the LEDC peripheral.  `tone(pin, frequency, duration)` is simple but limited—you cannot play multiple tones simultaneously and the frequency resolution is coarse.

### Using ledcWriteTone() and PWM

Call `ledcSetup(channel, freq, resolutionBits)` to configure a PWM channel and `ledcAttachPin(pin, channel)` to assign it to a pin.  Then call `ledcWriteTone(channel, frequency)` or adjust the duty cycle with `ledcWrite()` to generate custom waveforms.  For example, to play a 440 Hz tone on GPIO27:

```cpp
const int pwmChannel = 0;
const int audioPin = 27;

void setup() {
  ledcSetup(pwmChannel, 2000, 8);        // 2 kHz carrier, 8‑bit resolution
  ledcAttachPin(audioPin, pwmChannel);
}

void playTone(int freq, int duration) {
  ledcWriteTone(pwmChannel, freq);
  delay(duration);
  ledcWriteTone(pwmChannel, 0);          // stop
}

// Example: play C major arpeggio
playTone(523, 200); // C4
playTone(659, 200); // E4
playTone(784, 200); // G4
```

A simple RC low‑pass filter (resistor and capacitor) on the audio pin smooths the PWM signal before feeding it into a small amplifier and speaker.

### Playing simple 8‑bit style effects

For retro games, you can synthesise sound effects algorithmically using square waves, noise and envelopes.  Generate these waveforms in software and write samples to the DAC at regular intervals.  Many “chiptune” effects are single‑channel and small in size.

## 3. Playing audio files

### From SD card or SPIFFS

Libraries like **ESP8266Audio** and **AudioFile** can decode WAV, MP3 and Ogg files directly from SD card or SPIFFS and stream them to I2S or the DAC.  For example, to play a WAV file using ESP8266Audio:

```cpp
#include <AudioFileSourceSPIFFS.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

AudioFileSourceSPIFFS source("/sfx.wav");
AudioGeneratorWAV    decoder;
AudioOutputI2S       output;

void setup() {
  SPIFFS.begin();
  output.begin();
  decoder.begin(&source, &output);
}

void loop() {
  if (decoder.isRunning()) decoder.loop();
}
```

For small 8‑bit samples you can read raw PCM data from PROGMEM and stream it to the DAC using a timer interrupt.

### Limitations when playing audio during rendering

* **SPI contention:** Most TFT displays and SD cards share the same SPI bus.  Playing audio from an SD card while updating the display can cause stuttering.  Use separate SPI buses or schedule audio reads during vertical blanking intervals.
* **CPU load:** Decoding MP3 or Ogg uses significant CPU.  On an ESP32 without PSRAM, complex graphics and audio decoding may compete for cycles.
* **Dual‑core advantage:** Place audio decoding on one core (e.g., core 0) and graphics rendering on the other (core 1) to reduce contention.
* **DMA:** Use I2S with DMA to offload audio transfer from the CPU.

## 4. Practical recommendations for retro game sound

* **Beep and tone effects:** Use `ledcWriteTone()` or `tone()` for simple beeps (coin pickups, button presses).
* **Wave‑table synthesis:** Generate simple waveforms (square, triangle, noise) in a buffer and stream them to the DAC in an interrupt.  You can adjust frequency and amplitude to create varied effects.
* **Short WAV samples:** Store uncompressed 8‑bit WAV files in flash or SD card and stream them to the DAC or I2S.  Keep sample length short (< 1 s) to reduce memory and latency.
* **Volume control:** Include a potentiometer or digital volume control IC to adjust output level.  The DAC outputs a signal centred at half supply voltage (1.65 V), so AC‑couple to your amplifier through a capacitor.

## Summary

The ESP32 CYD does not include a built‑in speaker, but you can add sound using the on‑chip DAC pins, PWM via LEDC, or an external I2S DAC.  For simple retro effects, generate tones or short waveforms using `ledcWriteTone()` or by streaming PCM data to the DAC.  For high‑fidelity music, use an external I2S DAC and audio decoding library.  Be mindful of SPI bus contention and CPU load when playing audio alongside graphics rendering.
