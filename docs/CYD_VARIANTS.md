# CYD Variant Configurations

This file tracks working configurations for different ESP32 CYD (Cheap Yellow Display) board variants. If your board isn't listed, follow the debugging steps and contribute your configuration!

## How to Use This File

1. **Find your board** in the table below
2. **Copy the settings** to your `User_Setup.h` and code
3. **If not listed**, see [Debugging a New Variant](#debugging-a-new-variant)

---

## Tested Configurations

### Variant 1: TZT ESP32 LVGL 2.4" CYD

| Property | Value |
|----------|-------|
| **Purchase** | AliExpress - TZT Official Store |
| **Listing Name** | TZT ESP32 LVGL WIFI&Bluetooth Development Board 2.4 inch LCD TFT Module 240*320 Smart Display Screen With Touch WROOM |
| **Display** | 2.4" ILI9341 240x320 |
| **Touch** | XPT2046 resistive |
| **Chip** | ESP32-WROOM |
| **Features** | WiFi, Bluetooth, SD card slot, voltage regulator |
| **Tested By** | @boxwrench |
| **Status** | **WORKING** - Fully verified (Display + Touch) |

**TFT_eSPI User_Setup.h:**
```cpp
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_RGB_ORDER TFT_BGR  // Required - colors inverted without this

#define TFT_MISO  12
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   -1

#define TOUCH_CS  33

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 2500000

#define SUPPORT_TRANSACTIONS
```

**Critical Startup Sequence (in setup()):**
```cpp
void setup() {
    // 1. Initialize touch FIRST
    touchInit();

    // 2. Initialize display second
    gfxInit();

    // ...
}
```

**Touch Configuration (touch.cpp):**
```cpp
// Use both CS and IRQ pins in constructor
static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// Universal calibration values for TZT CYD 2.4"
#define TOUCH_MIN_X  600
#define TOUCH_MAX_X  3600
#define TOUCH_MIN_Y  500
#define TOUCH_MAX_Y  3600

void touchInit() {
    pinMode(TOUCH_IRQ, INPUT_PULLUP);
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);

    touch.begin(); // Let library initialize SPI
    touch.setRotation(3);
}

void touchUpdate() {
    // ... in the coordinate mapping section:
    int16_t mappedX = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
    // Y-axis is inverted: high raw Y = top, low raw Y = bottom
    int16_t mappedY = map(p.y, TOUCH_MAX_Y, TOUCH_MIN_Y, 0, SCREEN_HEIGHT);
}
```

**Ghost Image Fix (graphics.cpp):**
```cpp
void gfxInit() {
    tft.init();
    // Clear memory in landscape first, then portrait
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
}
```

**Notes:**
- **Touch Calibration**: Y-axis is inverted on this hardware. Raw Y values of ~3600 correspond to the top of the screen (Y=0), and raw Y values of ~500 correspond to the bottom (Y=320).
- **IRQ Pin**: Using the IRQ pin (GPIO 36) in the constructor and for polling is significantly more stable than SPI polling alone.
- **SPI Initialization**: Let the XPT2046 library handle SPI initialization with `touch.begin()` instead of passing the SPI object explicitly.
- **Colors**: Requires `TFT_BGR` and `tft.invertDisplay(true)`.

---

### Variant 2: Sunton ESP32-2432S028R (Single USB)

| Property | Value |
|----------|-------|
| **Model** | ESP32-2432S028R |
| **Display** | 2.8" ILI9341 320x240 |
| **Touch** | XPT2046 resistive |
| **USB** | Single Micro-USB |
| **Status** | Primary Reference Board |

**TFT_eSPI Configuration:**
```cpp
#define ILI9341_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO  12
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   -1
#define TFT_BL    21

#define TOUCH_CS  33

#define SPI_FREQUENCY 40000000
#define USE_HSPI_PORT
```

**Notes:**
- Standard ILI9341 driver
- Backlight on GPIO 21
- No inversion needed

---

### Variant 3: Sunton ESP32-2432S028Rv3 (Dual USB)

| Property | Value |
|----------|-------|
| **Model** | ESP32-2432S028Rv3 |
| **Display** | 2.8" ST7789 320x240 |
| **Touch** | XPT2046 resistive |
| **USB** | Dual (USB-C + Micro-USB) |
| **Status** | Requires driver change |

**TFT_eSPI Configuration:**
```cpp
#define ST7789_DRIVER        // NOT ILI9341!
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_INVERSION_ON     // Critical for ST7789
#define TFT_RGB_ORDER TFT_BGR

#define TFT_MISO  12
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   -1
#define TFT_BL    21

#define TOUCH_CS  33

#define SPI_FREQUENCY 80000000  // ST7789 handles 80MHz
#define USE_HSPI_PORT
```

**Notes:**
- **CRITICAL:** Dual USB = ST7789 driver required
- Uses `TFT_INVERSION_ON`
- Higher SPI frequency supported

---

### Variant 4: [Template - Copy for New Variants]

| Property | Value |
|----------|-------|
| **Seller** | (where you bought it) |
| **Model** | (any model numbers on board) |
| **Display** | (size, driver, resolution) |
| **Touch** | (XPT2046 or other) |
| **Tested By** | @your_github |
| **Status** | Working / Partial / In Progress |

**TFT_eSPI User_Setup.h:**
```cpp
// Your working configuration here
```

**Backlight:**
```cpp
// Which GPIO pin(s) control backlight
```

**Touch Calibration:**
```cpp
// Your calibrated touch values
```

**Notes:**
- Any quirks or issues encountered

---

## Debugging a New Variant

### Step 1: Identify Your Board

Look for:
- Model number printed on PCB (often ESP32-XXXXSXXX format)
- Seller listing title/description
- Any version markings (V1, V2, Rev, etc.)
- **USB connector count** (single vs dual)

### Step 2: Display Not Working

**Black screen:**
1. Try different backlight pins: 21, 27, 4, 16
   ```cpp
   pinMode(PIN, OUTPUT);
   digitalWrite(PIN, HIGH);
   ```
2. Check Serial Monitor - is code running?

**White screen:**
- Wrong display driver or SPI pins
- Try `ST7789_DRIVER` instead of `ILI9341_DRIVER`
- Single USB = ILI9341, Dual USB = ST7789

**Wrong colors (red/blue swapped):**
- Add `#define TFT_RGB_ORDER TFT_BGR`
- Or try `#define TFT_RGB_ORDER TFT_RGB`

**Display garbled/noisy:**
- Reduce SPI_FREQUENCY to 27000000 or 20000000

**Image offset or wrapped:**
- Try different rotation: `tft.setRotation(0)` through `3`
- Check TFT_WIDTH and TFT_HEIGHT match your display

### Step 3: Touch Not Working

**No response:**
1. Enable DEBUG_TOUCH in config.h
2. Check Serial Monitor for any touch output
3. Verify TOUCH_CS pin (common: 33)
4. Try initializing touch on HSPI:
   ```cpp
   SPIClass hspi(HSPI);
   hspi.begin(14, 12, 13, TOUCH_CS);
   touch.begin(hspi);
   ```

**Touch coordinates wrong:**
1. Enable DEBUG_TOUCH to see raw values
2. Tap all four corners and center, note raw X/Y ranges
3. Update TOUCH_MIN/MAX values based on observed range
4. If touches are inverted (tap top -> registers bottom):
   - Swap the parameters: `map(p.y, TOUCH_MAX_Y, TOUCH_MIN_Y, 0, SCREEN_HEIGHT)`
5. May need to swap X/Y axes depending on rotation

### Step 4: SD Card Not Working

**Not detected:**
- Check SD_CS pin (common: 5)
- Try different SD card (FAT32 formatted)
- Some boards need card inserted before power-on

---

## Common CYD Pin Configurations

### 2.4" Variants (240x320)

| Function | Variant A | Variant B | Notes |
|----------|-----------|-----------|-------|
| TFT_MOSI | 13 | 13 | |
| TFT_MISO | 12 | 12 | |
| TFT_SCLK | 14 | 14 | |
| TFT_CS | 15 | 15 | |
| TFT_DC | 2 | 2 | |
| TFT_RST | -1 | -1 | Usually tied to EN |
| TFT_BL | 21 | 27 | Backlight varies! |
| TOUCH_CS | 33 | 33 | |
| TOUCH_IRQ | 36 | 36 | |
| SD_CS | 5 | 5 | |
| SD_MOSI | 23 | 23 | VSPI |
| SD_MISO | 19 | 19 | VSPI |
| SD_SCK | 18 | 18 | VSPI |

### 2.8" Variants (320x480)

| Function | Common | Notes |
|----------|--------|-------|
| Resolution | 320x480 | Different from 2.4" |
| Driver | ILI9341 or ST7789 | Check your display |

---

## Contributing Your Configuration

Found a working config for a new board? Please contribute!

1. Fork the repository
2. Copy the template in this file
3. Fill in your working configuration
4. Submit a Pull Request

Include:
- Where you bought the board
- Any model numbers visible
- Your complete working settings
- Any issues you encountered and solutions

---

## Resources

- [TFT_eSPI Setup Guide](https://github.com/Bodmer/TFT_eSPI)
- [ESP32 GPIO Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [witnessmenow CYD Repository](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
