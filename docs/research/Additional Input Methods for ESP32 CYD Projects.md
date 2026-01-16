# Additional Input Methods for ESP32 CYD Projects

While the Cheap Yellow Display includes a resistive touch screen, adding extra input methods can enhance gameplay or make your projects more accessible.  This document explores physical buttons, rotary encoders, USB and Bluetooth keyboards, IR remotes, gesture sensors, and analog joysticks.  For each method you will find wiring guidance, libraries and example use cases.  Keep in mind the limited number of available GPIO pins on the ESP32‑2432S028R after connecting the TFT, touch and SD card.

## 1. Physical buttons

### Wiring

Connect one terminal of the button to a free GPIO and the other to ground.  Enable the internal pull‑up resistor in software (`pinMode(pin, INPUT_PULLUP)`) or use an external resistor.  When pressed, the pin reads LOW.

### Debouncing

Mechanical buttons bounce, producing multiple transitions.  Debounce by waiting a few milliseconds after a state change before accepting the input, or use a button library.

### Libraries

* **OneButton:** Handles debouncing and detects single/double clicks and long presses.
* **Bounce2:** Provides configurable software debouncing.

### Code example

```cpp
#include <OneButton.h>

OneButton button(32, true, true); // pin, activeLOW, internalPullup

void setup() {
  button.attachClick([]() { /* feed fish */ });
  button.attachLongPressStart([]() { /* open menu */ });
}

void loop() {
  button.tick();
  // game loop logic
}
```

### Practical applications

* Tap to feed fish or start a game.
* Long press to open a settings menu.
* Multiple buttons enable D‑pad style control for menu navigation.

## 2. Rotary encoders

### Wiring

A basic incremental encoder has two output pins (A and B) and a push button.  Connect pins A and B to two GPIOs with pull‑up resistors and the button to a third GPIO.  Turning the knob generates pulses on A and B.

### Reading rotation

When A and B change state, you determine direction by comparing their sequence.  Quadrature encoders produce 4 transitions per detent; you can read them via interrupts or polling.

### Libraries

* `Encoder` or `RotaryEncoder` libraries handle state transitions and provide a position counter.

### Use cases

* Scroll through menus or adjust values (e.g., tank temperature).
* Fine‑tune game settings such as volume or speed.

## 3. USB keyboard input

### Hardware requirements

Standard ESP32 boards (ESP32‑WROOM) lack native USB host capability.  Use an **ESP32‑S2** or **ESP32‑S3**, or add a USB host shield (MAX3421E).  For the CYD (ESP32‑WROOM), USB keyboards are not practical without extra hardware.

### Libraries

* **USBHost** (for ESP32‑S2/S3): supports HID keyboard and mouse.
* For host shields, use the **USB_Host_Shield_2.0** library.

### Limitations

USB consumes additional power; connectors may not fit inside the CYD enclosure.  Only recommended if your project requires a physical keyboard.

## 4. Bluetooth keyboard/gamepad

### Description

The ESP32 can act as a Bluetooth HID host and connect to keyboards, mice or gamepads.  Use `ESP32-BLE-Keyboard` or `NimBLE` libraries to implement host functionality.

### Pairing process

Enable Bluetooth and scan for nearby devices.  Pairing may require the player to press a pairing button on the gamepad.  Save the device MAC address in NVS for automatic reconnection.

### Supported devices

Common Bluetooth gamepads (e.g., 8BitDo, Nintendo Switch Joy‑Con) may work with the ESP32’s BLE HID host.

### Latency

Bluetooth adds input latency (~20–50 ms), which is acceptable for casual games but not for twitch‑style play.

## 5. IR remote control

### Hardware

Use a **VS1838B** or similar IR receiver module.  Connect the receiver output to a free GPIO.  Provide 5 V power through a voltage regulator if required.

### Libraries

* `IRremote` library decodes NEC, Sony and RC5 protocols.

### Use cases

* Navigate menus with directional keys.
* Enter cheat codes via remote control.

## 6. Gesture sensors

### APDS‑9960 (wave gestures)

This I2C sensor detects up, down, left and right hand gestures as well as ambient light and proximity.  Connect **VCC**, **GND**, **SCL**, **SDA** and an optional **INT** pin.  Use the `SparkFun_APDS9960` library to interpret gestures.

### PAJ7620 (more gestures)

Detects swipes, rotations and directional gestures via I2C.  Similar wiring to APDS‑9960.  Useful for gesture‑based controls (wave to scare predators).

### Applications

Control game actions without touching the screen—wave to toss food or tilt the device to tilt the aquarium.

## 7. Joystick/analog input

### Wiring

An analog joystick (e.g., PS2 stick) outputs two analog voltages (X and Y) and a digital button.  Connect X and Y outputs to ADC pins (e.g., GPIO32 and GPIO33) and the button to a digital input with pull‑up.

### Reading values

Use `analogRead()` to get values (0–4095).  Map the range to a position or velocity.  Implement a dead zone around the centre to avoid drift.

### Calibration and dead zones

Calibrate by reading the joystick’s neutral position at startup and subtracting it from subsequent readings.  Define a dead zone threshold to ignore small deviations.

### Applications

* Control fish movement like a virtual joystick.
* Pan around a larger aquarium map.

## Additional notes

* **Available GPIO pins:** After connecting TFT, touch and SD card, typical free pins on the ESP32‑2432S028R include GPIO32, 33, 34, 35, 36, 39 (analog‑only), and GPIO25–GPIO27.  Use I2C devices to share pins and free digital pins for buttons.
* **Power requirements:** Some peripherals (USB host shields, IR transmitters) require 5 V and additional current.  Ensure the CYD’s regulator can supply the load or use an external supply.
* **Practical applications:** Combine multiple input methods to create mini‑games (e.g., feed fish by pressing a button while controlling a net with a joystick).  Provide accessibility options for users who cannot use the touchscreen.

## Summary

Expanding input beyond the CYD’s touchscreen opens new gameplay possibilities.  Simple pushbuttons and rotary encoders are easy to wire and reliable.  Bluetooth keyboards and gamepads add familiar controls but require pairing and add latency.  IR remotes and gesture sensors enable remote or touch‑free interaction.  Analog joysticks provide precise control for fish movement.  Carefully manage GPIO pins, debounce inputs and consider power needs when adding these peripherals to your project.
