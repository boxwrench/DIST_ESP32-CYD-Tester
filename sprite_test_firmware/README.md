# Sprite Test Firmware for ESP32 CYD

Comprehensive sprite testing firmware implementing the **Enhanced Sprite Test Plan**.

## Features

- **Part A: Baseline Tests** - RGB order, inversion, byte swap validation
- **Part B: Format Comparison** - PNG vs RGB565 loading speed, rendering, memory
- **Part C: Performance Tests** - FPS stress testing with multiple sprites

## Hardware Requirements

- ESP32 CYD (ESP32-2432S028R or compatible)
- microSD card (FAT32) with test sprites in `/sprite_tests/` folder
- USB cable for serial monitoring

## SD Card Setup

1. Format SD card as FAT32
2. Copy test files from `../test_assets/` to SD card `/sprite_tests/` folder:
   - `fish_bluegill_32x32.png` and `.rgb565`
   - `enemy_clanker_32x32.png` and `.rgb565`
   - `background_240x240.png` and `.rgb565`

See `../test_assets/SD_CARD_SETUP.md` for detailed instructions.

## Build and Upload

```bash
# Open in PlatformIO
cd sprite_test_firmware

# Build
pio run

# Upload to CYD
pio run --target upload

# Monitor serial output
pio device monitor
```

Or use the PlatformIO IDE extension in VS Code.

## Running Tests

1. **Insert SD card** with test assets into CYD
2. **Upload firmware** to CYD
3. **Open serial monitor** at 115200 baud
4. **Reset CYD** - tests will run automatically
5. Tests auto-advance every 2-3 seconds
6. **View serial output** for detailed timing results

## Test Sequence

1. **A1: RGB Order Test** - Verify color channels (red/blue swap check)
2. **A2: Inversion Test** - Verify panel inversion setting
3. **A3: Byte Swap Test** - Verify setSwapBytes setting
4. **B1: Loading Speed** - Compare PNG vs RGB565 file loading
5. **B2: Rendering Speed** - Measure display performance
6. **B3: Memory Usage** - Track RAM consumption
7. **C1: FPS Stress Test** - Test 5, 10, 15, 20, 25 sprites
8. **C2: Background + Sprites** - Realistic game scenario test
9. **Results Summary** - Display all test results

## Expected Output

Serial monitor will show:
```
===== Enhanced Sprite Test Firmware =====
SD Card initialized
Starting tests...

PNG Load: 45230 us
RGB565 Load: 12450 us
Render Time: 8920 us
...
===== TEST RESULTS =====
B1_PNG_Load: 45230.0 us
B1_RGB_Load: 12450.0 us
...
```

## Recording Results

Fill out the Results Summary in `../docs/ENHANCED_SPRITE_TEST_PLAN.md` with your findings.

## Troubleshooting

**SD Card Failed:**
- Check SD card is formatted FAT32
- Verify `/sprite_tests/` folder exists
- Check SD card connections

**Display Issues:**
- Adjust `TFT_RGB_ORDER` in `platformio.ini`
- Toggle `TFT_INVERSION_ON` if colors look inverted
- Part A tests will help diagnose

**Low FPS:**
- Normal for ESP32 without PSRAM
- Background + 10 sprites: 5-15 FPS is expected
- Adjust `SPI_FREQUENCY` in `platformio.ini` (max 40MHz for ILI9341)

## Next Steps

After testing, apply verified settings to Bass-Hole project.
