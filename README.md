# ESP32-CYD-Tester

**A "Golden Master" hardware test & configuration generator for the Cheap Yellow Display (ESP32-2432S028R).**

If you just bought a random "CYD" board from AliExpress, Amazon, or eBay, this tool is your starting point. It validates that your hardware works and generates the correct configuration code for your own projects.

## What This Tool Does
1.  **Validates Hardware**: Checks the Display (Colors, Inversion), Touch Screen, WiFi, and SD Card.
2.  **Fixes Common Issues**: Automatically handles "Black Screen" issues (Dual Backlight Pins) and Color Inversion.
3.  **Generates Config**: Outputs a `CYD_Config.h` file with **your** specific Touch Calibration data and Pin Definitions.

## Prerequisite: The "AI-Assisted" Workflow (Recommended)
If you are new to coding or hardware, we strongly recommend using an AI-integrated IDE like **Cursor** or **VS Code with AI extensions**.

1.  **Install VS Code**: [Download Visual Studio Code](https://code.visualstudio.com/).
2.  **Install PlatformIO**: Search for the "PlatformIO IDE" extension in VS Code and install it. (This handles all the messy toolchains for you).
3.  **Open This Folder**: `File -> Open Folder...` and select this `ESP32-CYD-Tester` folder.

## How to Run the Test

### 1. Connect Your Device
Plug your ESP32 CYD into your computer via USB.

### 2. Upload the Firmware
- Look for the **PlatformIO Alien Head** icon in the sidebar, or the Blue Bar at the bottom.
- Click the **Right Arrow (→)** icon ("Upload").
- **Troubleshooting**: If it says "Connecting..." and fails:
    - Hold the **BOOT** button (usually next to the RST button on the side) while clicking Upload.
    - Release it once you see `Writing at 0x...`.

### 3. Open Serial Monitor
- Once uploaded, click the **Plug Icon** (Serial Monitor) in the bottom blue bar.
- Set the baud rate to **115200** if asked (it usually auto-detects).

### 4. Interactive Test
The screen should show "CYD Hardware Test".
1.  **Tap the Screen** to start.
2.  **Touch Calibration**: Tap the red dots as they appear.
3.  **Watch the output**: It will test WiFi and SD Card automatically.

## Interpreting the Output

At the end of the test, the Serial Monitor will print a block like this:

```cpp
/**************************************************************************/
/*               CYD HARDWARE CONFIGURATION BLOCK (GENERATED)             */
/**************************************************************************/
#ifndef CYD_CONFIG_H
#define CYD_CONFIG_H

// --- Touch Screen Calibration ---
#define TOUCH_MIN_X 3570
#define TOUCH_MAX_X 544
...
```

**Copy this entire block!** Save it as `CYD_Config.h` in your own project's `include/` folder. This is your "Golden Ticket" to working hardware in any future game or app you build.

## Troubleshooting

- **Still Black Screen?**
    - Ensure you are using the provided `src/main.cpp`. It activates **BOTH** Pin 21 and Pin 27 for backlight, covering 99% of CYD variants.
- **Inverted Colors?**
    - The code uses `tft.invertDisplay(true)`. If yours looks weird (Blue is Red), try changing this to `false` in `src/main.cpp`.
- **Upload Failed?**
    - Check your USB cable (some are power-only!).
    - Hold the BOOT button!

## Credits
Built for the **Bass Hole** project.
Designed to be used with AI coding assistants to speed up your hardware bring-up.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Attribution
If you use this project in your work, please credit:
**Keith Wilkinson**
[https://github.com/boxwrench/DIST_ESP32-CYD-Tester](https://github.com/boxwrench/DIST_ESP32-CYD-Tester)

