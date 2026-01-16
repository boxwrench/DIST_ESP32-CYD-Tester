# Sensor Integration for ESP32 CYD Projects

Adding sensors allows your Cheap Yellow Display games to interact with the physical world.  This document surveys common sensor types, wiring methods and libraries, and suggests creative applications.  Because the CYD uses many GPIO pins for the TFT and touch, plan your wiring carefully.

## 1. Environmental sensors

### a. Temperature/Humidity (DHT22, BME280)

* **Description:** Measure ambient temperature and humidity.  The **BME280** also measures barometric pressure.  It operates over I2C or SPI and uses less than **1 mA** during measurement and < 5 µA in sleep mode【508023232584317†L4-L9】.
* **Wiring:** Connect **3.3 V**, **GND**, and I2C lines (**SCL**, **SDA**) to available pins.  For SPI, also connect **SCK**, **MISO**, **MOSI** and **!CS**; level shifting may be required for 5 V logic【508023232584317†L112-L160】.
* **Library:** `Adafruit_BME280` (for BME280) or `DHT` library (for DHT22).
* **Code example:**

```cpp
#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

void setup() {
  Wire.begin();
  bme.begin(0x76); // I2C address 0x76 or 0x77
}

void loop() {
  float temp = bme.readTemperature();
  float humidity = bme.readHumidity();
  // Use values to influence fish behaviour (e.g., fish swim faster when warmer)
}
```

* **Creative ideas:** Make fish swim sluggishly when the room is cold; spawn special fish on rainy days if barometric pressure drops.

### b. Light sensors (LDR, BH1750)

* **Description:** Measure ambient light intensity.  An analog **LDR** uses a voltage divider on an ADC pin; the **BH1750** is a digital lux sensor using I2C.
* **Wiring:** For LDR, connect one end to 3.3 V and the other to an analog pin with a pull‑down resistor.  For BH1750, connect to I2C bus.
* **Library:** `BH1750` library.
* **Applications:** Adjust display brightness or switch to night‑mode graphics when the room is dark; spawn nocturnal fish at night.

### c. Air quality (MQ‑series, CCS811)

* **Description:** Gas sensors measure concentrations of VOCs, CO₂ or smoke.  CCS811 uses I2C and measures total VOC and eCO₂ levels.
* **Wiring:** Connect to 3.3 V, GND and I2C lines.  Some sensors require a warm‑up period and significant current.
* **Applications:** Display air quality inside the game; fish might “cough” when air quality is poor.

## 2. Motion and position sensors

### a. Accelerometer/Gyroscope (MPU6050, LSM6DS3)

* **Description:** Detect acceleration and rotation.  Both sensors connect via I2C and provide three axes of acceleration and gyroscopic rate.
* **Wiring:** Connect **VCC**, **GND**, **SCL** and **SDA** to the I2C bus.
* **Library:** `Adafruit_MPU6050` or `SparkFun_LSM6DS3`.
* **Applications:** Control fish movement by tilting the display; shake the device to scatter fish food.

### b. PIR motion sensor

* **Description:** Passive Infrared (PIR) sensors detect human presence.  They output a digital HIGH when motion is detected.
* **Wiring:** Connect **VCC**, **GND** and output to a free GPIO.  Require 5 V and have a warm‑up period.
* **Applications:** Wake the display when someone approaches; have fish hide when no one is nearby.

### c. Ultrasonic distance sensor (HC‑SR04)

* **Description:** Measures distance using ultrasonic pulses.  Requires trigger and echo pins.
* **Wiring:** Connect **VCC**, **GND**, **Trig** and **Echo**.  Use voltage divider or level shifter for Echo pin.
* **Library:** `NewPing` library.
* **Applications:** Proximity‑based interactions (feed fish when you wave your hand close to the display).

## 3. Biometric sensors

### a. Heart rate sensor (MAX30102)

* **Description:** Measures pulse rate using optical sensing.  Connects via I2C and requires a finger on the sensor.
* **Wiring:** **3.3 V**, **GND**, **SCL**, **SDA**.
* **Applications:** Adjust game difficulty based on player heart rate (e.g., fish swim faster when heart rate increases).

### b. Fingerprint sensor (R307)

* **Description:** Optical fingerprint reader for user authentication.  Communicates over serial UART.
* **Wiring:** **VCC**, **GND**, **TX**, **RX** to an available UART (e.g., Serial2).  Requires 3.3 V regulator and level shifting.
* **Applications:** Unlock special fish or load personalised game profiles when the player scans their finger.

## 4. RFID/NFC (RC522)

* **Description:** RFID reader that scans 13.56 MHz cards and tags.
* **Wiring:** Connect **SDA (SS)**, **SCK**, **MOSI**, **MISO**, **IRQ** and **RST** to the SPI bus and an available GPIO.  Use 3.3 V.
* **Library:** `MFRC522` library.
* **Applications:** Use physical cards to unlock fish species or levels; implement Amiibo‑style collectibles.

## 5. Sound sensors

### Microphone modules (MAX9814, KY‑038)

* **Description:** Amplified electret microphones detect sound levels.  Analog output connects to the ADC; digital versions output a HIGH when sound exceeds a threshold.
* **Applications:** Clap to make fish jump; shout to scare away predators.

## 6. GPS (NEO‑6M)

* **Description:** Provides latitude, longitude and time via serial UART.
* **Wiring:** **VCC**, **GND**, **TX**, **RX**.  Requires a clear view of the sky and 3.3 V regulator.
* **Applications:** Create location‑based games or track outdoor aquariums.

## Pin availability on CYD

The CYD uses SPI and several GPIOs for the display, touch and SD card.  On the ESP32‑2432S028R, typical pin assignments are: TFT **MOSI = GPIO23**, **MISO = GPIO19**, **SCLK = GPIO18**, **CS = GPIO5**, **DC = GPIO16**, **RST = GPIO17**; touch uses **GPIO15** (CS) and **GPIO4** (IRQ); SD card uses **GPIO13**–**GPIO15**.  This leaves a few pins (e.g., GPIO25–GPIO33) for sensors.  Choose sensors that support I2C to minimise pin usage and avoid conflicts.

## I2C address conflicts

When connecting multiple I2C sensors, ensure that no two devices share the same address (e.g., both BME280 and BMP280 default to 0x76).  Many sensors have configurable address pins.  Use a logic analyser or scanning sketch (`Wire.scan()`) to verify addresses.

## Power considerations

* Some sensors (MQ gas sensors, ultrasonic sensors) consume significant current and require a stable 5 V supply.  The CYD’s onboard regulator may not supply enough current; use an external power source and level shifting where necessary.
* Use sleep modes or disable sensors when not needed to reduce overall power consumption.

## Summary

The ESP32 CYD can interface with a wide variety of sensors to create interactive games.  Environmental sensors like BME280 measure temperature and humidity【508023232584317†L4-L9】; motion sensors like accelerometers enable tilt‑based control; biometric sensors personalise gameplay; RFID readers add collectible elements.  When integrating sensors, plan wiring to avoid conflicts with the TFT and touch, choose I2C devices when possible, and consider power and pin constraints.  Creative use of sensors can make your fish tank game responsive to its environment and more engaging.
