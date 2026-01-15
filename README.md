# ESP32-CYD-Tester v2.0

**A comprehensive hardware test suite and configuration generator for the ESP32 "Cheap Yellow Display" (CYD) family.**

If you bought a CYD board from AliExpress, Amazon, or eBay and want to get it working reliably, this tool is your starting point. It validates your hardware, helps you identify the correct settings, and generates configuration code you can use in your own projects.

## What This Tool Does

1. **Driver Detection** - Guides you to identify ILI9341 vs ST7789 based on USB connector count
2. **Color Inversion Test** - Determines the correct `invertDisplay()` setting for your specific board
3. **Display Validation** - Tests colors, patterns, and rendering
4. **RGB LED Test** - Validates the onboard LED (if present)
5. **Touch Calibration** - Captures your specific touch screen calibration values
6. **Memory Analysis** - Shows available heap and recommends max sprite sizes
7. **WiFi Scan** - Tests WiFi hardware
8. **SD Card Test** - Validates SD card slot
9. **Config Generation** - Outputs a complete configuration block to Serial Monitor

## Quick Start

### Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO IDE Extension](https://platformio.org/install/ide?install=vscode)

### Running the Tests

1. **Connect** your CYD board via USB
2. **Open** this folder in VS Code (`File -> Open Folder`)
3. **Upload** firmware (click the -> arrow in the PlatformIO toolbar)
   - If upload fails: Hold **BOOT** button while uploading
4. **Open Serial Monitor** (plug icon, 115200 baud)
5. **Follow on-screen instructions** - tap to proceed through tests
6. **Copy the config block** from Serial Monitor when done

## Test Sequence

### 1. Driver Detection
```
How many USB ports does your board have?

[1 USB]          [2 USB]
(Micro)          (USB-C + Micro)

Single USB = ILI9341 driver
Dual USB = ST7789 driver
```
This is the #1 cause of white screens - using the wrong driver.

### 2. Color Inversion Test
```
+------------------+------------------+
|   1. RAW COLORS  |   2. XOR COLORS  |
+------------------+------------------+
|      RED         |      CYAN        |
|     GREEN        |     MAGENTA      |
|      BLUE        |      YELLOW      |
|     WHITE        |      BLACK       |
+------------------+------------------+
        Tap the side with correct colors
```
- If LEFT (RAW) is correct: `invertDisplay(false)`
- If RIGHT (XOR) is correct: `invertDisplay(true)`

### 3. Touch Calibration
Tap the red circles in each corner. The test captures your specific touch controller's raw values and calculates the correct mapping.

### 4. Memory Analysis
Shows:
- Total heap available
- Free heap
- PSRAM detection (if present)
- Maximum recommended sprite size

## Generated Configuration

After completing tests, Serial Monitor outputs:

```cpp
/**************************************************************************/
/*               CYD HARDWARE CONFIGURATION BLOCK (GENERATED)             */
/**************************************************************************/
#ifndef CYD_CONFIG_H
#define CYD_CONFIG_H

// --- Display Driver ---
#define ILI9341_DRIVER
#define DISPLAY_INVERT true

// --- Touch Screen Calibration ---
#define TOUCH_MIN_X 3570
#define TOUCH_MAX_X 544
#define TOUCH_MIN_Y 3429
#define TOUCH_MAX_Y 532

// --- Pin Configuration ---
#define TFT_MISO 12
#define TFT_MOSI 13
// ... etc

#endif // CYD_CONFIG_H
/**************************************************************************/
```

**Copy this block** and save as `CYD_Config.h` in your project!

## Documentation

Comprehensive guides are in the `docs/` folder:

| Document | Description |
|----------|-------------|
| [ESP32_CYD_REFERENCE.md](docs/ESP32_CYD_REFERENCE.md) | Complete development reference - pinouts, library compatibility, performance limits |
| [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Decision tree for diagnosing issues |
| [HARDWARE.md](docs/HARDWARE.md) | Hardware guide - variants, modifications, buying advice |
| [CYD_VARIANTS.md](docs/CYD_VARIANTS.md) | Working configurations for specific board variants |

## Common Issues

| Problem | Quick Fix |
|---------|-----------|
| **Black screen** | Check backlight - code enables both pin 21 and 27 |
| **White screen** | Wrong driver - check USB count (single=ILI9341, dual=ST7789) |
| **Colors inverted** | Run the color test to determine `invertDisplay()` setting |
| **Touch not working** | Touch uses separate SPI pins (25, 32, 33, 36, 39) |
| **Touch misaligned** | Run calibration, check axis inversion in mapping |
| **Upload fails** | Hold BOOT button while uploading |

## Project Structure

```
DIST_ESP32-CYD-Tester/
├── docs/
│   ├── ESP32_CYD_REFERENCE.md    # Complete dev reference
│   ├── TROUBLESHOOTING.md        # Decision tree
│   ├── HARDWARE.md               # Hardware guide
│   └── CYD_VARIANTS.md           # Board-specific configs
├── include/
│   ├── CYD_2432S028R.h           # Default config
│   └── CYD_Config.h              # Generated config (after test)
├── src/
│   └── main.cpp                  # Test suite (~400 lines)
├── platformio.ini                # Build configuration
└── README.md                     # This file
```

## Using the Generated Config

In your project:

1. Copy `CYD_Config.h` to your project's `include/` folder
2. Include it in your code:
   ```cpp
   #include "CYD_Config.h"

   void setup() {
       tft.init();
       tft.invertDisplay(DISPLAY_INVERT);

       // Touch mapping using calibration values
       int x = map(raw.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
       int y = map(raw.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);
   }
   ```

3. Use the platformio.ini build_flags from the serial output

## Community Resources

- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) - Primary community hub
- [rzeldent/esp32-smartdisplay](https://github.com/rzeldent/esp32-smartdisplay) - Zero-config LVGL library
- [Random Nerd Tutorials CYD Guide](https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/)

## License

MIT License - see [LICENSE](LICENSE) file.

## Credits

Built for the **Bass Hole** project by Keith Wilkinson.

Designed to eliminate the frustration of CYD hardware configuration by providing deterministic tests and documented solutions.

---

**Having issues?** Check `docs/TROUBLESHOOTING.md` or open an issue.
