# Sprite Display Test Plan

This document outlines the systematic testing procedure to validate sprite rendering on CYD boards. Complete these tests IN ORDER - each test isolates one variable.

---

## Prerequisites

- ESP32 CYD board with known-working display (basic colors work)
- Test sprite data in PROGMEM (RGB565 format)
- Serial monitor connected at 115200 baud

---

## Test Sequence Overview

```
Test 1: RGB Order        -> Validates red/green/blue channel mapping
Test 2: Inversion        -> Validates panel inversion setting
Test 3: Byte Swap        -> Validates pushImage endianness
Test 4: Transparency     -> Validates transparent sprite rendering
Test 5: Integration      -> Full sprite pipeline end-to-end
```

---

## Test 1: RGB Order Validation

**Purpose:** Confirm TFT_RGB_ORDER is set correctly

**Code:**
```cpp
void testRGBOrder() {
    tft.fillScreen(TFT_BLACK);

    // Draw labeled color bars using fillRect (NOT pushImage)
    tft.setTextColor(TFT_WHITE);
    tft.drawString("RGB ORDER TEST", 10, 10, 2);

    // Pure RGB565 colors
    tft.fillRect(10, 50, 60, 60, 0xF800);   // RED
    tft.fillRect(80, 50, 60, 60, 0x07E0);   // GREEN
    tft.fillRect(150, 50, 60, 60, 0x001F);  // BLUE

    tft.drawString("R", 35, 120, 2);
    tft.drawString("G", 105, 120, 2);
    tft.drawString("B", 175, 120, 2);

    tft.drawString("Labels match colors?", 10, 150, 2);
}
```

**Expected Result:**
- Left box is RED, labeled "R"
- Middle box is GREEN, labeled "G"
- Right box is BLUE, labeled "B"

**If FAILED (red/blue swapped):**
- Toggle `TFT_RGB_ORDER` in platformio.ini
- `TFT_RGB_ORDER=TFT_BGR` (value 0) vs `TFT_RGB_ORDER=TFT_RGB` (value 1)

**Record Result:**
- [ ] PASS - Colors match labels
- [ ] FAIL - Red and Blue swapped -> Need TFT_BGR / TFT_RGB change

---

## Test 2: Inversion Validation

**Purpose:** Confirm panel inversion is set correctly

**Code:**
```cpp
void testInversion() {
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.drawString("INVERSION TEST", 10, 10, 2);

    // Draw a gradient that's easy to judge
    for (int i = 0; i < 200; i++) {
        uint8_t gray = map(i, 0, 200, 0, 255);
        uint16_t color = tft.color565(gray, gray, gray);
        tft.drawFastVLine(20 + i, 50, 60, color);
    }

    tft.drawString("BLACK", 20, 120, 2);
    tft.drawString("WHITE", 180, 120, 2);

    tft.drawString("Gradient: Black->White?", 10, 150, 2);
}
```

**Expected Result:**
- Gradient goes from BLACK (left) to WHITE (right)
- Background is BLACK
- Text is WHITE

**If FAILED (looks like negative):**
- Add `-DTFT_INVERSION_ON` to platformio.ini, OR
- Remove it if already present

**Record Result:**
- [ ] PASS - Black on left, white on right
- [ ] FAIL - Inverted (white on left) -> Toggle TFT_INVERSION_ON

---

## Test 3: Byte Swap Validation (CRITICAL)

**Purpose:** Confirm setSwapBytes setting for pushImage operations

**Preparation:** Create a simple test sprite in code:
```cpp
// 10x10 test sprite: left half RED, right half BLUE
const uint16_t testSprite[100] PROGMEM = {
    // Row 0-9: 5 red pixels (0xF800), 5 blue pixels (0x001F)
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
    0xF800,0xF800,0xF800,0xF800,0xF800, 0x001F,0x001F,0x001F,0x001F,0x001F,
};
```

**Code:**
```cpp
void testByteSwap() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("BYTE SWAP TEST", 10, 10, 2);

    // Reference using fillRect
    tft.drawString("Reference (fillRect):", 10, 40, 1);
    tft.fillRect(10, 55, 50, 50, 0xF800);   // RED
    tft.fillRect(60, 55, 50, 50, 0x001F);   // BLUE

    // Test with setSwapBytes(false)
    tft.drawString("pushImage swap=false:", 10, 115, 1);
    tft.setSwapBytes(false);
    tft.pushImage(10, 130, 10, 10, testSprite);
    // Scale up for visibility
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 100; x++) {
            int sx = x / 10;
            int sy = y / 10;
            uint16_t pixel = pgm_read_word(&testSprite[sy * 10 + sx]);
            tft.drawPixel(10 + x, 130 + y, pixel);  // No swap
        }
    }

    // Test with setSwapBytes(true)
    tft.drawString("pushImage swap=true:", 130, 115, 1);
    tft.setSwapBytes(true);
    tft.pushImage(130, 130, 10, 10, testSprite);
    // Scale up for visibility (pushImage handles swap)
    for (int y = 0; y < 5; y++) {
        tft.pushImage(130, 130 + y*10, 10, 10, testSprite);
    }

    tft.drawString("Which matches reference?", 10, 190, 1);
}
```

**Expected Result:**
- Reference shows RED on left, BLUE on right
- ONE of the pushImage results should match the reference

**If swap=false matches:** Use `tft.setSwapBytes(false)` (or don't call it)
**If swap=true matches:** Use `tft.setSwapBytes(true)` in gfxInit()

**Record Result:**
- [ ] swap=false matches reference
- [ ] swap=true matches reference

---

## Test 4: Transparency Validation

**Purpose:** Validate transparent sprite rendering

**Preparation:** Create sprite with transparency color (magenta 0xF81F):
```cpp
// 10x10 sprite with transparent corners
const uint16_t transSprite[100] PROGMEM = {
    0xF81F,0xF81F,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xF81F,0xF81F,
    0xF81F,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xF81F,
    0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
    // ... (middle rows all green 0x07E0)
    0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,
    0xF81F,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xF81F,
    0xF81F,0xF81F,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0x07E0,0xF81F,0xF81F,
};
#define TRANSPARENT_COLOR 0xF81F
```

**Code:**
```cpp
void testTransparency() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("TRANSPARENCY TEST", 10, 10, 2);

    // Draw red background
    tft.fillRect(50, 50, 100, 100, TFT_RED);

    // Draw sprite with transparency using pushImage
    tft.setSwapBytes(true);  // Use result from Test 3
    tft.pushImage(80, 80, 10, 10, transSprite, TRANSPARENT_COLOR);

    // Scale version
    for (int sy = 0; sy < 10; sy++) {
        for (int sx = 0; sx < 10; sx++) {
            uint16_t pixel = pgm_read_word(&transSprite[sy * 10 + sx]);
            if (pixel != TRANSPARENT_COLOR) {
                for (int dy = 0; dy < 5; dy++) {
                    for (int dx = 0; dx < 5; dx++) {
                        tft.drawPixel(80 + sx*5 + dx, 80 + sy*5 + dy, pixel);
                    }
                }
            }
        }
    }

    tft.drawString("Corners show red BG?", 10, 160, 2);
}
```

**Expected Result:**
- Green sprite visible on red background
- CORNERS of sprite show RED background (transparency working)
- No magenta visible

**Record Result:**
- [ ] PASS - Corners show red background through
- [ ] FAIL - Magenta corners visible (transparency color wrong or not working)

---

## Test 5: Integration Test

**Purpose:** Validate full sprite pipeline as used in Bass-Hole

**Code:**
```cpp
void testIntegration() {
    // This test uses actual game sprites if available
    // Or simulates the game rendering pipeline

    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);

    // 1. Draw background using pushImage
    // tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, backgroundSprite);

    // 2. Draw sprites with transparency
    // gfxDrawSpriteTransparent(fishSprite, x, y, w, h);

    // 3. Draw UI with primitives
    tft.fillRect(0, 0, 240, 40, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("$1234", 10, 10, 2);

    // 4. Verify all three look correct together
    tft.setTextColor(TFT_WHITE);
    tft.drawString("All elements correct?", 10, 280, 2);
}
```

**Expected Result:**
- Background displays with correct colors
- Sprites display with correct colors AND transparency
- UI text displays with correct colors
- No color conflicts between layers

---

## Results Summary

Fill this out after running all tests:

```
Date: ___________
Board: ESP32-2432S028R / Other: ___________
USB Ports: Single / Dual

Test 1 - RGB Order:     [ ] PASS  [ ] FAIL -> Setting: ___________
Test 2 - Inversion:     [ ] PASS  [ ] FAIL -> Setting: ___________
Test 3 - Byte Swap:     [ ] PASS  [ ] FAIL -> Setting: ___________
Test 4 - Transparency:  [ ] PASS  [ ] FAIL
Test 5 - Integration:   [ ] PASS  [ ] FAIL

Final platformio.ini build_flags:
    -DTFT_RGB_ORDER=_____
    -DTFT_INVERSION_ON (yes/no): _____
    setSwapBytes: true / false
```

---

## Applying Results to Bass-Hole

Once tests pass, update Bass-Hole:

### 1. platformio.ini
```ini
build_flags =
    ; ... existing flags ...
    -DTFT_RGB_ORDER=___    ; From Test 1
    -DTFT_INVERSION_ON     ; If Test 2 required it
```

### 2. graphics.cpp gfxInit()
```cpp
void gfxInit() {
    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);  // From Test 3 - apply ONCE here
    // Remove any other setSwapBytes calls
    // Remove manual byte swap from gfxDrawSpriteTransparent
}
```

### 3. gfxDrawSpriteTransparent() - Use pushImage
```cpp
void gfxDrawSpriteTransparent(const uint16_t *sprite, int16_t x, int16_t y,
                              int16_t width, int16_t height) {
    // FAST: Use pushImage with transparency color
    tft.pushImage(x, y, width, height, sprite, SPRITE_TRANSPARENT_COLOR);
}
```

---

## Common Issues After Testing

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Tests pass but game sprites wrong | Sprite data has different byte order | Re-export sprites OR swap in converter |
| Background wrong, fish correct | Background uses different render path | Ensure gfxDrawSprite also uses setSwapBytes |
| Random corruption | SPI speed too high | Lower SPI_FREQUENCY to 27000000 |

---

## References

- [TFT_eSPI setSwapBytes documentation](https://github.com/Bodmer/TFT_eSPI)
- [witnessmenow CYD repo](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- [CYD color issues discussion](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/issues/76)
