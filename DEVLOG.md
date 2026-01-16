# Development Log

This file tracks development sessions, decisions, and context for continuity across sessions and devices. Claude (or any AI assistant) should read this first when picking up the project.

---

## Project Status

**Current State:** v2.0 - Functional test suite with color inversion detection
**Primary Goal:** Hardware validation tool for ESP32 CYD boards
**Related Project:** [Bass-Hole](../Bass-Hole) - ESP32 fishing game (uses configs generated here)

---

## Quick Context for New Sessions

```
This repo: Hardware testing/config generation for CYD boards
Key problem solved: Color inversion detection (RAW vs XOR test)
Key finding: USB count determines driver (single=ILI9341, dual=ST7789)
Tech stack: PlatformIO, TFT_eSPI, XPT2046_Touchscreen
```

## Debugging Workflow (CRITICAL - Follow This Order!)

When encountering hardware issues:

1. **Check docs/ FIRST** before any code changes

   - `ESP32_CYD_REFERENCE.md` - Hardware specs, known ranges, library behavior
   - `TROUBLESHOOTING.md` - Decision tree for common issues
   - `HARDWARE.md` - Pinouts, variants, physical hardware info
   - `CYD_VARIANTS.md` - Board-specific quirks and working configs

2. **Search docs for specific issue** (e.g., "touch", "calibration", "rotation")

3. **Only after docs** - Try code modifications based on documented behavior

**Lesson learned (2026-01-15):** Touch issues could have been solved in 5 minutes by reading docs instead of 90 minutes of trial-and-error with `touch.setRotation()` calls that docs explicitly say NOT to use.

---

## Session Log

### 2026-01-16 - Sprite Display Research & Test Planning

**What was done:**
- Deep dive research into CYD sprite display issues
- Identified THREE SEPARATE settings that affect sprite colors (commonly conflated):
  1. **RGB Order** (TFT_RGB vs TFT_BGR) - red/blue channel swap
  2. **Byte Swap** (setSwapBytes) - endianness for pushImage/sprite buffers
  3. **Inversion** (TFT_INVERSION_ON) - panel-level color inversion
- Analyzed Bass-Hole graphics.cpp - found inconsistent byte swapping
- Created test plan for validating sprite pipeline end-to-end

**Key findings from research:**
```
Symptom                          | Likely Cause
---------------------------------|----------------------------------
Red/Blue swapped                 | RGB order wrong (TFT_RGB vs BGR)
Negative/inverted look           | Panel inversion setting
UI fine, sprites wrong           | Byte swap mismatch (MOST COMMON)
Washed out colors                | Double conversion or wrong COLMOD
```

**Critical insight:**
- `setSwapBytes(true)` only affects `pushImage()` / `pushSprite()`
- `drawPixel()` is NOT affected by setSwapBytes
- Bass-Hole does manual byte swap in `gfxDrawSpriteTransparent()` but NOT in `gfxDrawSprite()` or `gfxRestoreBackground()` - this is the bug!

**Recommended fix approach:**
1. Set `tft.setSwapBytes(true)` ONCE in gfxInit()
2. Remove manual byte swap from gfxDrawSpriteTransparent()
3. Use `pushImage()` with transparency parameter instead of pixel-by-pixel
4. Ensure sprite data is generated with correct byte order

**Files created:**
- docs/SPRITE_TEST_PLAN.md - Hardware test checklist for sprite validation
- docs/RESEARCH_AGENDA.md - 13 research topics with discrete prompts for future development
- docs/research/ - Folder for research output documents

**Next steps (requires hardware):**
- Run RGB test bars to verify RGB/BGR order
- Run sprite pushImage test with setSwapBytes(true)
- Validate background restoration uses same byte order
- Update Bass-Hole once ground truth established

**Blocking issues:** No hardware available for testing

---

### 2026-01-15 - v2.0 Major Enhancement

**What was done:**

- Added driver detection wizard (USB count -> ILI9341/ST7789)
- Added color inversion test (RAW vs XOR side-by-side, user taps correct side)
- Added RGB LED test, memory analysis
- Created comprehensive docs/ folder:
  - ESP32_CYD_REFERENCE.md (copied from Bass-Hole, 578 lines)
  - TROUBLESHOOTING.md (decision tree flowchart)
  - HARDWARE.md (pinouts, variants, mods)
  - CYD_VARIANTS.md (board-specific configs)
- Updated README with full test methodology
- Enhanced config output includes platformio.ini build_flags

**Key decisions:**

- Color test uses invertDisplay(false) baseline, shows RAW vs XOR side-by-side
- Touch mapping in test mode uses calibration values if available
- Config output now includes driver type and inversion setting

**Files changed:**

- src/main.cpp - Major rewrite (~400 lines now)
- README.md - Complete rewrite
- docs/\* - All new

**Next steps:**

- Test on actual hardware with new test sequence
- Validate SPI speed test results across board variants
- Consider adding speaker/DAC test as future enhancement

**Blocking issues:** None

---

### 2026-01-15 - v2.1 Enhancement

**What was done:**

- Fixed touch mapping in color inversion test and driver detection
  - Now uses calibrated touch values if available
  - Falls back to rough mapping (200-3800) if not calibrated
- Reordered test sequence:
  - Touch calibration now runs FIRST (#1)
  - Driver detection and color test now use calibrated touch (#2, #3)
- Added SPI Speed Test feature
  - Tests frequencies: 10, 20, 27, 40, 55, 80 MHz
  - Driver-specific limits (ILI9341: 55MHz, ST7789: 80MHz)
  - Results added to config output
- Updated config generation
  - Now includes "Max Stable SPI" recommendation
  - platformio.ini build_flags use tested SPI frequency

**Key decisions:**

- Calibration must run before interactive tests for accurate touch detection
- SPI test uses driver-specific safe limits rather than visual corruption detection
  - ILI9341: up to 55MHz
  - ST7789: up to 80MHz
- Config output now uses dynamic SPI frequency from test results

**Files changed:**

- src/main.cpp - ~100 lines added/modified
  - Added `maxStableSPI` global variable
  - Updated `printConfig()` to include SPI recommendation
  - Fixed `testColorInversion()` touch mapping
  - Fixed `detectDriver()` touch mapping
  - Added `testSPISpeed()` function (~90 lines)
  - Reordered setup() test sequence

**Next steps:**

- Test on actual hardware
- May need to refine touch mapping in color test (currently rough)
- Consider adding SPI speed test

**Blocking issues:** None

---

### 2026-01-14 - Initial Commit

**What was done:**

- Basic test suite: display, touch calibration, WiFi, SD
- Config generation to serial
- Dual backlight pin support (21 + 27)

---

## Architecture Notes

```
Test Sequence:
1. Driver Detection (user input - USB count)
2. Color Inversion Test (interactive - tap correct side)
3. Display Test (visual - color fills + patterns)
4. RGB LED Test (visual - cycle colors)
5. Touch Calibration (interactive - tap corners)
6. Memory Test (informational)
7. WiFi Scan (automatic)
8. SD Card Test (automatic)
9. Config Output (serial)
10. Touch Test Mode (loop - draw to verify)
```

## Known Issues / Tech Debt

- [ ] Touch mapping in color test uses hardcoded range (200-3800), should use calibration
- [ ] No test for speaker/DAC output
- [ ] Could add SPI frequency sweep test to find max stable speed
- [ ] **Add sprite byte-swap test to main test suite** (see docs/SPRITE_TEST_PLAN.md)
- [ ] Add pushImage vs fillRect color comparison test
- [ ] Generate config output should include setSwapBytes recommendation

## Related Resources

- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- [rzeldent/esp32-smartdisplay](https://github.com/rzeldent/esp32-smartdisplay)
