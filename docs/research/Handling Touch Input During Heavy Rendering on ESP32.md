# Handling Touch Input During Heavy Rendering on ESP32

The Cheap Yellow Display (CYD) uses an XPT2046 or similar resistive touch controller connected via SPI.  Games with continuous rendering must process touch events promptly without causing missed taps.  This document describes polling versus interrupt modes, scheduling touch reads in the game loop and using dual cores on ESP32 to maintain responsiveness.

## 1. Touch polling vs interrupts

### XPT2046 polling

Libraries such as `XPT2046_Touchscreen` periodically read the touch controller over SPI.  The ESPHome documentation notes that if no interrupt pin is specified, the driver polls the controller at the `update_interval` (default **50 ms**)【727410691330105†L40-L84】.  Increasing the polling rate (e.g., 10 ms) improves responsiveness at the cost of more SPI bus usage.

### Interrupt pin (IRQ)

Many CYD boards break out the **IRQ** pin of the XPT2046.  When a touch occurs, the controller pulls IRQ low.  By configuring the driver with an `interrupt_pin`, the library can trigger a touch read immediately rather than waiting for the next poll【727410691330105†L40-L84】.  This reduces latency and SPI traffic because the controller is only queried when there is a touch.

#### Pros and cons

| Mode       | Pros                                  | Cons |
|------------|---------------------------------------|------|
| **Polling**| Simple implementation; no external interrupt wiring. | Wastes SPI bandwidth when idle; latency tied to polling interval. |
| **Interrupt**| Immediate touch detection; reduces bus traffic when not touched. | Requires wiring IRQ to a GPIO and handling debouncing; may require a pull‑up resistor. |

## 2. Sampling touches in the game loop

### Where to sample

Read touch coordinates either **before** or **after** rendering.  Reading during a long `pushSprite()` or `pushImage()` can delay detection until the draw finishes.  To avoid missing taps:

* Insert a touch read at the beginning of each frame update, then process game logic and rendering.
* Alternatively, read touch in an **interrupt service routine (ISR)** and push events to a queue processed by the main loop.

### Buffering touch events

Store touch events (position, time, state) in a ring buffer.  The game loop can process multiple events each frame, ensuring none are lost even if the loop occasionally takes longer than expected.

### Debouncing and filtering

Touch controllers sometimes generate multiple readings for a single press.  Implement debounce logic by requiring a stable state for a few milliseconds before accepting a tap.  For drag gestures, apply simple filters (e.g., average consecutive samples) to smooth jitter.

## 3. Touch responsiveness targets

Human reaction time to touch events is around 100–150 ms, but players notice delays above ~50 ms in interactive games.  Strive to sample touch at least every **20 ms** (50 Hz) to ensure responsive tapping.  This corresponds to `update_interval: 20ms` when polling【727410691330105†L40-L84】.

Test your game by logging the time between touch events and on‑screen response.  If latency exceeds 100 ms, consider reducing frame time or offloading rendering to the second core.

## 4. Dual‑core solutions

The ESP32 has two cores (Core 0 and Core 1).  Use them to separate touch input from rendering:

* **Core 0:** Run the touch handling task.  Configure XPT2046 driver with an IRQ pin.  When an interrupt fires, read touch coordinates and place events in a queue.
* **Core 1:** Run the main game loop, reading events from the queue and updating game state.  Render frames using `pushSprite()` or `pushImage()`.

Tasks communicate via FreeRTOS queues or ring buffers.  Ensure the SPI bus is not accessed by both tasks simultaneously—use a mutex around SPI transfers.

## 5. Practical patterns for CYD touch games

* **Use the IRQ pin if available:** Wire the XPT2046 IRQ to an unused GPIO.  In your driver’s initialization, specify `interrupt_pin` to enable immediate notifications【727410691330105†L40-L84】.
* **Reduce polling interval:** If IRQ is not available, set `update_interval` to 10–20 ms to improve responsiveness, balancing against SPI load.
* **Process touch before drawing:** Read touch at the start of the frame to reduce perceived latency.
* **Implement a touch queue:** Buffer events so that quick taps are not lost even if rendering occasionally takes longer than the polling interval.
* **Use dual cores:** For games with heavy rendering or audio decoding, move input handling to one core and rendering to the other.
* **Calibrate touch:** Many CYD boards require calibration values (min/max raw X/Y) to map raw readings to screen coordinates.  Provide a calibration routine and store values in NVS.

## Code example using XPT2046_Touchscreen

```cpp
#include <XPT2046_Touchscreen.h>
#define CS_PIN  15
#define IRQ_PIN 4

XPT2046_Touchscreen touch(CS_PIN, IRQ_PIN);

volatile bool touched = false;

void IRAM_ATTR touchIsr() {
  touched = true;
}

void setup() {
  SPI.begin();
  touch.begin();
  pinMode(IRQ_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), touchIsr, FALLING);
}

void loop() {
  if (touched) {
    touched = false;
    if (touch.touched()) {
      TS_Point p = touch.getPoint();
      // map raw coordinates to screen space
      int16_t x = map(p.x, 200, 3800, 0, 319);
      int16_t y = map(p.y, 240, 3800, 0, 239);
      // push event into queue or handle directly
    }
  }
  // update game state
  // render frame
}
```

## Summary

Responsive touch input on CYD requires balancing SPI bandwidth and latency.  Using the XPT2046 IRQ pin or reducing the polling interval ensures taps are detected quickly.  Schedule touch reads before or after rendering, buffer events, and leverage the ESP32’s dual cores to maintain smooth rendering while handling input.  With these practices your fish tank game will respond promptly to taps, feeds and drag gestures even during heavy rendering.
