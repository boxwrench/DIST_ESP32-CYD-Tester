# Hardware Guide

This document covers the ESP32 CYD (Cheap Yellow Display) hardware, including pinouts, variants, and modifications.

## What is a CYD?

The "Cheap Yellow Display" is an inexpensive ESP32 development board with an integrated LCD touchscreen. They're manufactured by various companies (TZT, Sunton, etc.) and are popular for DIY projects due to their low cost (~$10-15).

## Supported Hardware

### Primary Target: 2.4" CYD

| Specification | Value |
|---------------|-------|
| MCU | ESP32-WROOM-32 |
| Display | 2.4" ILI9341 TFT LCD |
| Resolution | 240 x 320 pixels |
| Touch | XPT2046 resistive |
| Storage | MicroSD card slot |
| Flash | 4MB |
| RAM | 520KB SRAM |

**Common model names:**
- TZT ESP32 LVGL 2.4" LCD
- Sunton ESP32-2432S028
- Generic "ESP32 CYD 2.4 inch"

## Pinout Reference

### Display (ILI9341) - HSPI

| Function | GPIO | Notes |
|----------|------|-------|
| MOSI | 13 | Data to display |
| MISO | 12 | Data from display (rarely used) |
| SCLK | 14 | SPI clock |
| CS | 15 | Chip select |
| DC | 2 | Data/Command |
| RST | -1 | Connected to EN (auto-reset) |
| BL | 21 | Backlight (some boards use 27) |

### Touch (XPT2046) - Separate SPI

| Function | GPIO | Notes |
|----------|------|-------|
| MOSI | 32 | Dedicated touch data |
| MISO | 39 | Input-only GPIO |
| CLK | 25 | Dedicated touch clock |
| CS | 33 | Touch chip select |
| IRQ | 36 | Touch interrupt (optional) |

### SD Card - VSPI

| Function | GPIO | Notes |
|----------|------|-------|
| MOSI | 23 | |
| MISO | 19 | |
| SCK | 18 | |
| CS | 5 | |

### Other

| Function | GPIO | Notes |
|----------|------|-------|
| RGB LED Red | 4 | Active LOW |
| RGB LED Green | 16 | Active LOW |
| RGB LED Blue | 17 | Active LOW |
| LDR | 34 | Light sensor (some boards) |
| Speaker | 26 | DAC output (some boards) |

## Variant Differences

### 2.4" vs 2.8" CYD

| Feature | 2.4" | 2.8" |
|---------|------|------|
| Resolution | 240x320 | 320x480 |
| Driver | ILI9341 | ILI9341 or ST7789 |
| Touch | XPT2046 | XPT2046 |
| Pins | Standard | May differ |

### ESP32-S3 Variants

Some newer CYDs use ESP32-S3. Key differences:
- Different pin numbering
- USB-C instead of micro-USB
- More RAM (useful for sprites)

Check your board's documentation for exact pinout.

## TFT_eSPI Configuration

Key settings for standard 2.4" CYD:

```cpp
#define ILI9341_DRIVER       // Display driver
#define TFT_WIDTH  240       // Pixels
#define TFT_HEIGHT 320

// SPI pins
#define TFT_MISO  12
#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   -1         // Connected to EN

// Touch
#define TOUCH_CS  33

// Speed
#define SPI_FREQUENCY 40000000  // 40 MHz
```

### Adjusting for Your Board

If your display doesn't work:

1. **White screen** - Wrong driver or SPI pins
2. **Inverted colors** - Add `#define TFT_INVERSION_ON`
3. **Mirrored display** - Try different rotation values
4. **Garbled display** - Reduce `SPI_FREQUENCY` to `27000000`

## Display Configuration

### TZT ESP32 CYD 2.4" - Verified Settings

**Tested Configuration (2025-01-14):**

```cpp
// Display
tft.setRotation(0);          // Portrait, USB at bottom
tft.invertDisplay(true);     // VERIFIED via color test

// Touch
touch.setRotation(1);        // Touch uses different rotation than display

// Color constants (config.h)
#define COLOR_BLACK  0x0000  // Normal RGB565 values
#define COLOR_WHITE  0xFFFF
// ... etc
```

**Why This Works:**
- `invertDisplay(true)` inverts all RGB565 values at hardware level
- Sprites contain normal RGB565 data -> display inverts -> correct screen colors
- Text/UI use normal RGB565 constants -> display inverts -> correct screen colors
- Everything is consistent

### Testing Your Display (Different Variants)

**IMPORTANT:** Different ESP32 CYD manufacturers may need `invertDisplay(false)`. To test yours:

1. **Add color test to setup():**
   ```cpp
   // In main.cpp setup(), after gfxInit()
   gfxDrawColorTest();
   delay(30000);  // Show for 30 seconds
   ```

2. **Upload and observe:**
   - **If "1. RAW" shows Red|Green|Blue|White|Black correctly** -> Use `invertDisplay(true)` + normal RGB565 values
   - **If "2. XOR" shows Red|Green|Blue|White|Black correctly** -> Use `invertDisplay(false)` + normal RGB565 values

3. **Update graphics.cpp:**
   ```cpp
   #define DISPLAY_INVERT true  // or false based on test
   ```

4. **Remove test from setup() and rebuild**

The color test eliminates guesswork and works for any ILI9341 display variant.

## Touch Calibration

The XPT2046 touch controller returns raw ADC values that need mapping to screen coordinates.

### TZT ESP32 CYD 2.4" - Verified Values

**Working configuration (2025-01-14):**

```cpp
// In touch.cpp
#define TOUCH_MIN_X  600
#define TOUCH_MAX_X  3600
#define TOUCH_MIN_Y  500
#define TOUCH_MAX_Y  3600

// Mapping (portrait mode, rotation 0)
touch.setRotation(1);
int16_t mappedX = map(p.x, TOUCH_MAX_X, TOUCH_MIN_X, 0, SCREEN_WIDTH);  // X inverted
int16_t mappedY = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT); // Y normal
```

### Calibration Process

1. Enable debug output:
   ```cpp
   // In config.h
   #define DEBUG_TOUCH 1
   ```

2. Upload and open Serial Monitor (115200 baud)

3. Tap the **four corners** of the screen:
   - Top-left: Note raw X, Y values
   - Top-right: Note raw X, Y values
   - Bottom-left: Note raw X, Y values
   - Bottom-right: Note raw X, Y values

4. Determine min/max for X and Y axes

5. Test mapping - if crosshairs appear in wrong place:
   - **Swap X/Y:** Touch rotation might need changing
   - **Invert X or Y:** Swap MIN/MAX in map() function
   - **Try all 4 rotations:** setRotation(0-3)

6. Update values in `touch.cpp`

## SD Card

### Requirements
- MicroSD card (any capacity)
- FAT32 formatted
- Class 10 recommended for speed

### Troubleshooting

**Card not detected:**
- Reformat as FAT32 (not exFAT)
- Try a different card
- Check SD_CS pin matches your board
- Some boards need the card inserted before power-on

**Slow performance:**
- Use Class 10 or faster card
- Format with 32KB allocation unit size

## Power Considerations

### USB Power
Most CYDs work fine powered by USB. Current draw is typically 100-200mA.

### Battery Power
For portable use:
- 3.7V LiPo battery (500mAh+)
- Connect to battery pins if available
- Or use USB power bank

**Note:** Some CYDs have battery charging circuits, some don't. Check your board.

## Modifications

### Adding a Speaker

Many CYDs have pads for a speaker but no speaker installed.

1. Get a small 8 ohm speaker (<=1W)
2. Solder to speaker pads
3. Audio output is GPIO 26 (DAC)

```cpp
// Basic tone
dacWrite(26, 128);  // Mid-level
delay(1);
dacWrite(26, 0);    // Silent
```

### External RGB LED

The onboard LED is often single-color. For effects, add WS2812B:

1. Connect data pin to any free GPIO
2. Use FastLED or Adafruit NeoPixel library
3. Good for hit feedback, alerts, etc.

### Better Touch

Resistive touch can be imprecise. Options:
- Use larger tap targets in UI
- Add debouncing/averaging
- Upgrade to capacitive touch overlay (advanced)

## Buying Guide

### Recommended Sources
- AliExpress (search "ESP32 CYD 2.4")
- Amazon (higher price, faster shipping)
- Banggood

### What to Look For
- ESP32-WROOM (not just ESP32-C3)
- 2.4" or 2.8" based on your preference
- ILI9341 driver (most common)
- XPT2046 touch (standard)
- SD card slot included

### Price Range
- 2.4" CYD: $8-15
- 2.8" CYD: $12-20
- ESP32-S3 variants: $15-25

## Resources

- [TFT_eSPI Documentation](https://github.com/Bodmer/TFT_eSPI)
- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [CYD Community](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- [Wokwi ESP32 Simulator](https://wokwi.com/)
