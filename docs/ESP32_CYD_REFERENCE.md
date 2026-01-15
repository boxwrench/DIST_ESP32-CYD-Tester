# ESP32 CYD Development Reference Manual

The ESP32 "Cheap Yellow Display" (CYD) family represents the most accessible entry point for embedded touchscreen development, but successful implementation requires precise configuration knowledge. This reference consolidates hardware-verified configurations, library compatibility data, and failure pattern solutions across all major CYD variants. The critical finding: **hardware identification by USB connector count** (single = ILI9341, dual USB-C + Micro = ST7789) prevents the most common failure mode—blank screens from driver mismatch.

---

## Known-good configuration matrix

The following configurations have been validated through community testing on real hardware, with evidence from GitHub issues, project repositories, and forum discussions spanning thousands of implementations.

### ESP32-WROOM variants (no PSRAM)

| Model | Display | Touch | Working Stack | SPI Freq | Status |
|-------|---------|-------|---------------|----------|--------|
| **ESP32-2432S028R** (single USB) | ILI9341 | XPT2046 | TFT_eSPI 2.5.43 + Arduino-ESP32 2.0.14 | 40 MHz | Primary reference |
| **ESP32-2432S028Rv2** (USB-C) | ILI9341 | XPT2046 | TFT_eSPI 2.5.43 + `TFT_INVERSION_ON` | 40 MHz | Verified |
| **ESP32-2432S028Rv3** (dual USB) | **ST7789** | XPT2046 | TFT_eSPI 2.5.43 + ST7789_DRIVER | 80 MHz | Requires driver change |
| ESP32-2432S024R | ILI9341 | XPT2046 | LovyanGFX 1.2.7 | 40 MHz | Verified |
| ESP32-2432S032R | ST7789 | XPT2046 | esp32-smartdisplay 2.1.0 | 80 MHz | Verified |
| ESP32-2432S032C | ST7789 | GT911 | LovyanGFX 1.2.7 + I2C touch | 80 MHz | Verified |
| ESP32-3248S035R | ST7796 | XPT2046 | TFT_eSPI 2.5.43 | 80 MHz | Verified |
| ESP32-3248S035C | ST7796 | GT911 | esp32-smartdisplay 2.1.0 | 80 MHz | Verified |

### ESP32-S3 variants (8MB PSRAM, 16MB Flash)

| Model | Display | Touch | Working Stack | Interface | Status |
|-------|---------|-------|---------------|-----------|--------|
| ESP32-4827S043C | ST7262 | GT911 | esp32-smartdisplay 2.1.0 | 16-bit RGB | Verified |
| ESP32-8048S050C | ST7262 | GT911 | LovyanGFX 1.2.7 | 16-bit RGB | Verified |
| ESP32-8048S070C | ST7262 | GT911 | LVGL 9.2.2 + esp_lcd | 16-bit RGB | Verified |

### Minimal working configuration (ESP32-2432S028R)

```ini
# platformio.ini - Conservative stable stack
[env:cyd-basic]
platform = espressif32@6.4.0
framework = arduino
board = esp32dev
lib_deps =
    bodmer/TFT_eSPI@2.5.43
    paulstoffregen/XPT2046_Touchscreen@1.4

build_flags =
    -DUSER_SETUP_LOADED=1
    -DILI9341_DRIVER=1
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_MOSI=13
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1
    -DTFT_BL=21
    -DTOUCH_CS=33
    -DSPI_FREQUENCY=40000000
    -DUSE_HSPI_PORT
```

---

## Definitive GPIO pin mapping

The ESP32-2432S028R serves as the reference design for all 2.8" CYD variants. Understanding this pinout is essential because **CYD uses two separate SPI buses**—a design choice that causes the majority of touch/SD card conflicts.

### Display SPI (HSPI bus)

| Function | GPIO | Notes |
|----------|------|-------|
| TFT_MOSI | 13 | Data to display |
| TFT_MISO | 12 | Optional; conflicts with touch when defined |
| TFT_SCLK | 14 | Clock |
| TFT_CS | 15 | Chip select |
| TFT_DC | 2 | Data/Command |
| TFT_RST | -1 | Connected to board reset |
| TFT_BL | 21 | Backlight PWM; **shared with I2C expansion** |

### Touch SPI (separate from display)

| Function | GPIO | Notes |
|----------|------|-------|
| TOUCH_MOSI | 32 | Dedicated touch data |
| TOUCH_MISO | 39 | Input-only GPIO |
| TOUCH_CLK | 25 | Dedicated touch clock |
| TOUCH_CS | 33 | Touch chip select |
| TOUCH_IRQ | 36 | Interrupt; input-only GPIO |

### SD Card (VSPI bus)

| Function | GPIO | Notes |
|----------|------|-------|
| SD_MOSI | 23 | Shared with VSPI |
| SD_MISO | 19 | Shared with VSPI |
| SD_SCK | 18 | Shared with VSPI |
| SD_CS | 5 | Chip select |

### Peripherals

| Function | GPIO | Notes |
|----------|------|-------|
| RGB LED Red | 4 | **Active LOW** (inverted logic) |
| RGB LED Green | 16 | Active LOW |
| RGB LED Blue | 17 | Active LOW |
| LDR Sensor | 34 | Analog input only |
| Speaker | 26 | DAC output |
| Available expansion | 22, 27, 35 | GPIO 35 is input-only |

---

## Library truth table

This matrix represents tested compatibility as of January 2025. The critical insight: **TFT_eSPI struggles with Arduino-ESP32 Core 3.x** while LovyanGFX handles it gracefully.

| Library | Version | Arduino 2.0.14 | Arduino 3.2.x | ESP-IDF 5.x | CYD Support | Notes |
|---------|---------|----------------|---------------|-------------|-------------|-------|
| **TFT_eSPI** | 2.5.43 | Stable | Issues | No | Built-in | 231+ open issues; limited maintenance |
| **LovyanGFX** | 1.2.7 | Yes | Yes | Yes | AUTODETECT | **Recommended for new projects** |
| **LVGL** | 9.2.2 | Yes | Yes | Yes | Via driver | Requires display driver integration |
| **LVGL** | 8.3.11 | Yes | Yes | Yes | Via driver | Legacy; still supported |
| **esp32-smartdisplay** | 2.1.0 | Yes | Yes | Yes | Zero-config | **Best for LVGL + CYD** |
| **Adafruit_GFX** | 1.11.9 | Yes | Yes | No | Manual | Requires Adafruit_ILI9341 |

### Recommended stacks by use case

**Game development (performance-critical):**
```ini
lib_deps =
    lovyan03/LovyanGFX@^1.2.7
platform = espressif32@6.9.0
```

**UI applications (LVGL):**
```ini
lib_deps =
    rzeldent/esp32_smartdisplay@^2.1.0
    lvgl/lvgl@^9.2.0
```

**Maximum compatibility (legacy projects):**
```ini
lib_deps =
    bodmer/TFT_eSPI@2.5.43
platform = espressif32@6.4.0  # Pins to Arduino-ESP32 2.0.14
```

---

## Failure pattern reference

These patterns emerge from analysis of GitHub issues, forum posts, and community troubleshooting across TFT_eSPI, LovyanGFX, and esp32-smartdisplay repositories.

### Display failures

| Symptom | Root Cause | Fix |
|---------|------------|-----|
| **White/blank screen** | Wrong driver (ILI9341 vs ST7789) | Check USB count: single=ILI9341, dual=ST7789 |
| White screen | User_Setup.h not found | Add `-DUSER_SETUP_LOADED=1` to build_flags |
| White screen | SPI pins misconfigured | Verify HSPI: MOSI=13, SCLK=14, CS=15, DC=2 |
| **Black screen** | Backlight not initialized | `pinMode(21, OUTPUT); digitalWrite(21, HIGH);` |
| **Inverted colors** | Panel inversion mismatch | Add `#define TFT_INVERSION_ON` (dual-USB variants) |
| **Wrong colors (red/blue swapped)** | RGB byte order wrong | Add `#define TFT_RGB_ORDER TFT_BGR` |
| Flickering | No double buffering | Use TFT_eSprite or LVGL double buffer |
| Partial/corrupted display | SPI too fast | Reduce `SPI_FREQUENCY` to 27000000 |
| Only works after reset | Init timing | Add `delay(100);` before `tft.init();` |

### Touch failures

| Symptom | Root Cause | Fix |
|---------|------------|-----|
| **Touch not responding** | Wrong SPI bus | Touch uses separate pins: CLK=25, MOSI=32, MISO=39 |
| Touch not responding | CS pin wrong | Touch CS is GPIO 33, not GPIO 15 |
| **X/Y swapped** | Rotation mismatch | `touchscreen.setRotation(display_rotation);` |
| **Touch offset** | Needs calibration | Calibrate with raw values: x_min~280, x_max~3860 |
| Touch works, display doesn't | SPI bus conflict | Use XPT2046_Bitbang_Slim or LovyanGFX bitbang mode |
| Erratic readings | No threshold | Set `threshold: 400` or filter by `p.z > 400` |

### Memory/performance failures

| Symptom | Root Cause | Fix |
|---------|------------|-----|
| **Crash on sprite create** | Sprite too large | Max ~200x200 @ 16-bit without PSRAM (80KB) |
| Allocation failure | Heap fragmented | Pre-allocate sprites at startup |
| PSRAM not detected | Wrong board selected | Select "ESP32 Wrover Module" in Arduino IDE |
| Slow performance | DMA disabled | Ensure `TFT_MISO` defined; add `-DESP32=1` |

### Build failures

| Symptom | Root Cause | Fix |
|---------|------------|-----|
| Multiple library conflicts | Duplicate symbols | Use only one display library per project |
| TFT_eSPI + Core 3.x crashes | API incompatibility | Use Arduino-ESP32 2.0.14 or switch to LovyanGFX |
| LGFX_AUTODETECT fails | ESP32 Core 3.2.0 bug | Use Core 3.1.3 or LovyanGFX 1.2.7+ |

---

## Performance limits reference

These numbers represent practical achievable performance on real CYD hardware, not theoretical maximums.

### Frame rate limits

| Operation | Achievable FPS | Configuration |
|-----------|----------------|---------------|
| Full screen fill (320x240) | **25-40 FPS** | SPI @ 40 MHz with DMA |
| Full screen sprite push | **~22 FPS** | From PSRAM; ~45ms per frame |
| Partial update (1/10 screen) | **60+ FPS** | LVGL dirty rectangles |
| Sprite animation (small) | **100-200 FPS** | Internal RAM: 200 FPS; PSRAM: 100 FPS |
| LVGL UI (typical) | **14-31 FPS** | Depends on buffer size |

### SPI frequency limits

| Controller | Stable Max | Notes |
|------------|------------|-------|
| **ILI9341** | 40 MHz | Datasheet says 10 MHz; 40 MHz tested stable 8+ hours |
| **ST7789** | 80 MHz | Higher throughput than ILI9341 |
| ST7796 | 80 MHz | 3.5" displays |
| XPT2046 touch | 2.5 MHz | Higher causes erratic readings |

### Memory budget (ESP32-2432S028R, no PSRAM)

| Resource | Available | Usage |
|----------|-----------|-------|
| Total heap | ~300 KB | System + application |
| **Usable for graphics** | ~160 KB | After FreeRTOS, WiFi stack |
| Full framebuffer (320x240x16-bit) | 150 KB | Exceeds safe allocation |
| **Safe sprite maximum** | ~80 KB | ~200x200 @ 16-bit |
| LVGL minimum buffer | 15 KB | 320x24 (1/10 screen) |
| 8-bit sprite (320x240) | 76.8 KB | Fits in internal RAM |

### Bandwidth calculations

```
SPI @ 40 MHz, 16-bit color:
  Theoretical: 40,000,000 / 16 = 2,500,000 pixels/second
  Frame time: (320 x 240) / 2,500,000 = 30.7ms
  Max FPS: 1000 / 30.7 = 32 FPS (theoretical)
  Practical: ~26 FPS (with SPI overhead)

SPI @ 80 MHz:
  Practical max: ~52 FPS
```

---

## Display initialization sequences

### ILI9341 (single-USB CYD variants)

```cpp
// TFT_eSPI configuration
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH
#define USE_HSPI_PORT
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 20000000
```

### ST7789 (dual-USB CYD variants)

```cpp
// TFT_eSPI configuration for v3 boards
#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_INVERSION_ON      // Critical for ST7789
#define TFT_RGB_ORDER TFT_BGR // Color correction
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   21
#define USE_HSPI_PORT
#define SPI_FREQUENCY 80000000  // ST7789 handles 80MHz
```

### LovyanGFX universal configuration

```cpp
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel;  // or Panel_ST7789
    lgfx::Bus_SPI _bus;
    lgfx::Touch_XPT2046 _touch;

public:
    LGFX(void) {
        auto bus_cfg = _bus.config();
        bus_cfg.spi_host = HSPI_HOST;
        bus_cfg.freq_write = 40000000;
        bus_cfg.pin_sclk = 14;
        bus_cfg.pin_mosi = 13;
        bus_cfg.pin_miso = 12;
        bus_cfg.pin_dc   = 2;
        _bus.config(bus_cfg);
        _panel.setBus(&_bus);

        auto panel_cfg = _panel.config();
        panel_cfg.pin_cs   = 15;
        panel_cfg.pin_rst  = -1;
        panel_cfg.pin_busy = -1;
        _panel.config(panel_cfg);
        _panel.setLight(&_light);
        setPanel(&_panel);

        auto touch_cfg = _touch.config();
        touch_cfg.freq = 2500000;
        touch_cfg.pin_sclk = 25;
        touch_cfg.pin_mosi = 32;
        touch_cfg.pin_miso = 39;
        touch_cfg.pin_cs   = 33;
        touch_cfg.pin_int  = 36;
        _touch.config(touch_cfg);
        setTouch(&_touch);
    }
};
```

---

## Touch calibration procedure

Resistive touchscreens (XPT2046) require per-device calibration. The calibration maps raw ADC values (typically **280-3860 range**) to screen coordinates.

### Calibration code

```cpp
// Log raw values for calibration
void calibrateTouchDebug() {
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();
        Serial.printf("Raw: x=%d y=%d z=%d\n", p.x, p.y, p.z);
    }
}

// Apply calibration
int16_t mapTouchX(int16_t raw) {
    return map(raw, 280, 3860, 0, 320);  // Adjust min/max per device
}
int16_t mapTouchY(int16_t raw) {
    return map(raw, 340, 3860, 0, 240);
}
```

### ESPHome calibration format

```yaml
touchscreen:
  platform: xpt2046
  spi_id: touch
  cs_pin: 33
  interrupt_pin: 36
  calibration:
    x_min: 280
    x_max: 3860
    y_min: 340
    y_max: 3860
  transform:
    swap_xy: false
    mirror_x: false
    mirror_y: false
```

---

## Hardware variant identification guide

Physical inspection reliably identifies CYD variants:

| Feature | ILI9341 Variant | ST7789 Variant |
|---------|-----------------|----------------|
| **USB connectors** | Single (Micro-USB) | Dual (USB-C + Micro-USB) |
| PCB marking | Sometimes "9341" | Sometimes "7789" |
| Model suffix | R, Rv1, Rv2 | Rv3 |
| Display init | Standard | Requires TFT_INVERSION_ON |

### Capacitive vs resistive touch

| Touch Type | Controller | Interface | Identification |
|------------|------------|-----------|----------------|
| Resistive | XPT2046 | SPI | Model ends in "R" |
| Capacitive | GT911 | I2C | Model ends in "C" |
| Capacitive | CST816S | I2C | Smaller displays (2.2") |
| None | - | - | Model ends in "N" |

---

## Quick reference card

### Critical numbers to remember

| Parameter | Value |
|-----------|-------|
| ILI9341 safe SPI | **40 MHz** |
| ST7789 safe SPI | **80 MHz** |
| Touch SPI | **2.5 MHz** |
| Max sprite (no PSRAM) | **~80 KB** (200x200) |
| LVGL min buffer | **1/10 screen** |
| Touch calibration range | **280-3860** typical |
| Backlight GPIO | **21** |
| Touch CS GPIO | **33** |

---

## Community Resources

- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) - Primary community hub (3,300+ stars)
- [rzeldent/esp32-smartdisplay](https://github.com/rzeldent/esp32-smartdisplay) - Zero-config LVGL library
- [Random Nerd Tutorials CYD Guide](https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/)

This reference represents consolidated knowledge from the ESP32 CYD community as of January 2025. Hardware variants continue to evolve; always verify USB connector count and test initialization sequences when working with new boards.
