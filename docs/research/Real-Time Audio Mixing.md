# Real‑Time Audio Mixing

Many games need to play multiple sound effects simultaneously while music is playing.  Real‑time mixing combines multiple audio streams into a single output, scaling each by a volume weight, and feeds the result to the DAC or I2S codec.  On the ESP32 you can implement mixing in software using libraries or simple algorithms.

## Mixer concepts

The Espressif **esp‑audio‑effects** library includes a mixer component that merges multiple audio signals by adjusting weights and transition times to produce a single harmonious output【29782365554611†L117-L120】.  The algorithm sums each sample from the input streams multiplied by its volume coefficient and divides by the sum of the coefficients to normalise the result.  All streams must have the same sample rate, channel count and bit depth.

A simple example using the Arduino Audio Tools library creates two sine wave generators and mixes them with the `Mixer` class【682609192217116†L27-L82】:

```cpp
AudioGeneratorSine sine1(440); // 440 Hz tone
AudioGeneratorSine sine2(660);
AudioEffectMixer<2> mixer;     // two channels

void setup() {
  AudioOutputI2S i2s;
  sine1.begin();
  sine2.begin();
  mixer.input(0, sine1);
  mixer.input(1, sine2);
  mixer.setInputGain(0, 0.5);  // weight each stream
  mixer.setInputGain(1, 0.5);
  AudioOutputQueue out(i2s);
  out.begin();
  while (true) {
    out.write(mixer.read());
  }
}
```

## Using sndmixer (MicroPython)

MicroPython’s `sndmixer` module illustrates a lightweight mixing engine.  It runs its mixing task on the second core of the ESP32.  You call `sndmixer.begin(n_channels, enable_stereo)` to start the mixer, then `sndmixer.voice` to play a sound.  The documentation notes that you cannot change the number of channels without restarting the mixer【653418738420789†L50-L69】, and that starting with fewer channels but stereo enables higher quality at the cost of mixing fewer streams.

## DAC versus I2S

The built‑in DACs on pins 25 and 26 are 8‑bit and thus suitable only for simple beeps or telephone‑quality audio【879333597936804†L109-L151】.  They support continuous output via DMA, but audio quality is limited by the 8‑bit resolution.  For better audio fidelity use the I2S peripheral and an external DAC such as the I2S‑based MAX98357.  I2S supports 16‑bit or 24‑bit audio and can output stereo at sample rates up to 44.1 kHz.

## Implementing mixing on the ESP32

A basic mixing routine in C adds samples from two `int16_t` streams and divides by 2:

```c
int16_t mix(int16_t a, int16_t b) {
  int32_t sum = (int32_t)a + (int32_t)b;
  return sum >> 1; // divide by 2 to avoid clipping
}
```

For more streams, sum each weighted sample and divide by the number of streams or by the sum of weights.  Implement the mixer in a dedicated FreeRTOS task pinned to core 0 so that it does not interfere with rendering.

When streaming from SD card or flash, decode each audio file into a small buffer, mix the buffers sample by sample, and feed the result to the I2S write queue.  To avoid buffer underruns, keep buffers large enough for a few milliseconds of audio and prefill them before starting playback.

## Tips and considerations

- Ensure all audio streams have the same sampling rate and bit depth.  Resample or convert offline if necessary【682609192217116†L27-L82】.
- Use 16‑bit audio for better quality; avoid the internal DAC for anything more than retro bleeps【879333597936804†L109-L151】.
- Run the mixer on a separate core or low‑priority task to avoid audio glitches.
- Use a ring buffer or double buffering for each sound stream to allow asynchronous decoding while the mixer consumes the previous buffer.
- Gradually adjust volume weights to avoid audible pops; e.g. increase or decrease weight over several samples.

With careful implementation, the ESP32 can play multiple sound effects and music simultaneously, enriching your game’s soundscape.