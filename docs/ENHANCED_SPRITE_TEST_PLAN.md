# Enhanced Sprite Test Plan - PNG vs RGB565 Performance Analysis

This document extends the baseline [SPRITE_TEST_PLAN.md](SPRITE_TEST_PLAN.md) with SD card format comparison and performance stress testing.

**Test Duration:** ~30-45 minutes  
**Hardware Required:** ESP32 CYD + microSD card (formatted FAT32)

---

## Test Overview

### Part A: Baseline Tests (5-10 minutes)
Run tests 1-5 from [SPRITE_TEST_PLAN.md](SPRITE_TEST_PLAN.md) first to establish correct display configuration.

### Part B: Format Comparison (15-20 minutes)
Compare PNG vs RGB565 loading from SD card for:
- Loading speed
- Rendering speed
- Memory usage
- Display quality

### Part C: Performance Stress Test (15-20 minutes)
Test both formats under load:
- Multiple sprites on screen (5, 10, 15, 20, 25)
- FPS measurement
- CPU usage
- Memory pressure

---

## Part A: Baseline Configuration ✓

**Complete tests 1-5 from SPRITE_TEST_PLAN.md FIRST.**

Once complete, record your verified settings here:

```
TFT_RGB_ORDER: ___________
TFT_INVERSION_ON: [ ] Yes [ ] No
setSwapBytes: [ ] true [ ] false
```

You'll use these settings for all tests in Parts B and C.

---

## Part B: Format Comparison Tests

### Prerequisites

#### SD Card File Structure
```
/sprite_tests/
├── test_sprite_32x32.png        (32×32 RGB image)
├── test_sprite_32x32.rgb565     (2048 bytes raw)
├── test_sprite_64x64.png        (64×64 RGB image)
├── test_sprite_64x64.rgb565     (8192 bytes raw)
├── fish_trout.png               (32×32 actual game sprite)
├── fish_trout.rgb565            (2048 bytes raw)
├── background_240x240.png       (240×240 RGB image)
└── background_240x240.rgb565    (115200 bytes raw)
```

#### Generate RGB565 Files From PNG
Use this Python script to convert test assets:

```python
# tools/png_to_rgb565.py
from PIL import Image
import struct

def png_to_rgb565(png_path, output_path):
    img = Image.open(png_path).convert('RGB')
    with open(output_path, 'wb') as f:
        for y in range(img.height):
            for x in range(img.width):
                r, g, b = img.getpixel((x, y))
                # Convert to RGB565
                r5 = (r >> 3) & 0x1F
                g6 = (g >> 2) & 0x3F
                b5 = (b >> 3) & 0x1F
                rgb565 = (r5 << 11) | (g6 << 5) | b5
                # Write as little-endian uint16
                f.write(struct.pack('<H', rgb565))

# Example usage:
png_to_rgb565('test_sprite_32x32.png', 'test_sprite_32x32.rgb565')
```

---

### Test B1: Loading Speed Comparison

**Purpose:** Measure file loading time from SD card

**Test Code:**
```cpp
void testB1_LoadingSpeed() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("B1: Loading Speed Test", 10, 10, 2);
    
    // Test PNG loading (using PNGdec library)
    unsigned long pngStart = micros();
    PNG png;
    File pngFile = SD.open("/sprite_tests/test_sprite_32x32.png");
    int pngResult = png.open(pngFile, pngDecode);
    unsigned long pngLoad = micros() - pngStart;
    pngFile.close();
    
    // Test RGB565 loading (raw binary)
    unsigned long rgbStart = micros();
    File rgbFile = SD.open("/sprite_tests/test_sprite_32x32.rgb565");
    uint16_t buffer[32*32];
    rgbFile.read((uint8_t*)buffer, 32*32*2);
    unsigned long rgbLoad = micros() - rgbStart;
    rgbFile.close();
    
    // Display results
    tft.drawString("32x32 PNG load:", 10, 50, 2);
    tft.drawString(String(pngLoad) + " us", 10, 70, 2);
    
    tft.drawString("32x32 RGB565 load:", 10, 100, 2);
    tft.drawString(String(rgbLoad) + " us", 10, 120, 2);
    
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Faster: " + String(pngLoad < rgbLoad ? "PNG" : "RGB565"), 10, 150, 2);
    
    delay(5000);
    
    // Repeat for 240x240 background
    // ... (same pattern for larger files)
}
```

**Record Results:**
```
32×32 Sprite:
  PNG load time:    _______ µs
  RGB565 load time: _______ µs
  Winner: _______

240×240 Background:
  PNG load time:    _______ µs
  RGB565 load time: _______ µs
  Winner: _______
```

---

### Test B2: Rendering Speed Comparison

**Purpose:** Measure display rendering time after loading

**Test Code:**
```cpp
void testB2_RenderingSpeed() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("B2: Rendering Speed Test", 10, 10, 2);
    
    // Load both formats into memory first
    uint16_t pngBuffer[32*32];
    uint16_t rgbBuffer[32*32];
    
    // Load PNG and decode to buffer
    PNG png;
    File pngFile = SD.open("/sprite_tests/test_sprite_32x32.png");
    png.open(pngFile, pngDecode);
    png.decode(pngBuffer, 0);  // Decode to buffer
    pngFile.close();
    
    // Load RGB565
    File rgbFile = SD.open("/sprite_tests/test_sprite_32x32.rgb565");
    rgbFile.read((uint8_t*)rgbBuffer, 32*32*2);
    rgbFile.close();
    
    // Test PNG rendering (10 iterations)
    unsigned long pngStart = micros();
    for (int i = 0; i < 10; i++) {
        tft.pushImage(50, 50, 32, 32, pngBuffer);
    }
    unsigned long pngRender = (micros() - pngStart) / 10;
    
    // Test RGB565 rendering (10 iterations)
    unsigned long rgbStart = micros();
    for (int i = 0; i < 10; i++) {
        tft.pushImage(100, 50, 32, 32, rgbBuffer);
    }
    unsigned long rgbRender = (micros() - rgbStart) / 10;
    
    // Display results
    tft.drawString("PNG render (avg):", 10, 100, 2);
    tft.drawString(String(pngRender) + " us", 10, 120, 2);
    
    tft.drawString("RGB565 render (avg):", 10, 150, 2);
    tft.drawString(String(rgbRender) + " us", 10, 170, 2);
    
    delay(5000);
}
```

**Record Results:**
```
32×32 Sprite (avg of 10 renders):
  PNG render:    _______ µs
  RGB565 render: _______ µs
  Winner: _______
```

---

### Test B3: Memory Usage Comparison

**Purpose:** Compare RAM requirements for each format

**Test Code:**
```cpp
void testB3_MemoryUsage() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("B3: Memory Usage Test", 10, 10, 2);
    
    // Baseline memory
    uint32_t memBefore = ESP.getFreeHeap();
    tft.drawString("Baseline free:", 10, 50, 2);
    tft.drawString(String(memBefore) + " bytes", 10, 70, 2);
    
    // Load PNG (PNGdec creates internal buffers)
    PNG png;
    File pngFile = SD.open("/sprite_tests/background_240x240.png");
    png.open(pngFile, pngDecode);
    uint32_t memAfterPNG = ESP.getFreeHeap();
    uint32_t pngUsed = memBefore - memAfterPNG;
    
    tft.drawString("PNG loaded:", 10, 100, 2);
    tft.drawString(String(memAfterPNG) + " free", 10, 120, 2);
    tft.drawString("Used: " + String(pngUsed), 10, 140, 2);
    
    png.close();
    pngFile.close();
    
    // Load RGB565 (raw buffer)
    uint16_t *rgbBuffer = (uint16_t*)malloc(240*240*2);
    File rgbFile = SD.open("/sprite_tests/background_240x240.rgb565");
    rgbFile.read((uint8_t*)rgbBuffer, 240*240*2);
    rgbFile.close();
    uint32_t memAfterRGB = ESP.getFreeHeap();
    uint32_t rgbUsed = memBefore - memAfterRGB;
    
    tft.drawString("RGB565 loaded:", 10, 170, 2);
    tft.drawString(String(memAfterRGB) + " free", 10, 190, 2);
    tft.drawString("Used: " + String(rgbUsed), 10, 210, 2);
    
    free(rgbBuffer);
    
    delay(5000);
}
```

**Record Results:**
```
240×240 Background Memory:
  PNG overhead:    _______ bytes
  RGB565 overhead: _______ bytes
  Difference:      _______ bytes
```

---

### Test B4: Display Quality Comparison

**Purpose:** Visual comparison of rendered output quality

**Test Code:**
```cpp
void testB4_QualityComparison() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("B4: Quality Comparison", 10, 10, 2);
    
    // Load and render PNG (left half)
    PNG png;
    File pngFile = SD.open("/sprite_tests/fish_trout.png");
    png.open(pngFile, pngDecode);
    uint16_t pngBuffer[32*32];
    png.decode(pngBuffer, 0);
    tft.pushImage(30, 60, 32, 32, pngBuffer);
    pngFile.close();
    
    tft.drawString("PNG", 35, 100, 1);
    
    // Load and render RGB565 (right half)
    File rgbFile = SD.open("/sprite_tests/fish_trout.rgb565");
    uint16_t rgbBuffer[32*32];
    rgbFile.read((uint8_t*)rgbBuffer, 32*32*2);
    tft.pushImage(120, 60, 32, 32, rgbBuffer);
    rgbFile.close();
    
    tft.drawString("RGB565", 115, 100, 1);
    
    tft.drawString("Any visual difference?", 10, 130, 2);
    tft.drawString("(Should be identical)", 10, 150, 1);
    
    delay(10000);  // 10 seconds to compare visually
}
```

**Visual Assessment:**
```
[ ] PNG and RGB565 look identical
[ ] PNG looks better (how?): _______________________
[ ] RGB565 looks better (how?): ____________________
[ ] Other difference: ______________________________
```

---

## Part C: Performance Stress Tests

### Test C1: Sprite Count vs FPS

**Purpose:** Measure FPS degradation as sprite count increases

**Test Code:**
```cpp
// Global vars
uint16_t spriteBuffer[32*32];
int spriteCounts[] = {5, 10, 15, 20, 25};
int spriteX[25], spriteY[25], spriteDX[25], spriteDY[25];

void testC1_SpriteFPS(bool usePNG) {
    // Load sprite once
    if (usePNG) {
        PNG png;
        File f = SD.open("/sprite_tests/fish_trout.png");
        png.open(f, pngDecode);
        png.decode(spriteBuffer, 0);
        f.close();
    } else {
        File f = SD.open("/sprite_tests/fish_trout.rgb565");
        f.read((uint8_t*)spriteBuffer, 32*32*2);
        f.close();
    }
    
    // Initialize sprite positions
    for (int i = 0; i < 25; i++) {
        spriteX[i] = random(0, 208);
        spriteY[i] = random(0, 288);
        spriteDX[i] = random(1, 3);
        spriteDY[i] = random(1, 3);
    }
    
    // Test each sprite count
    for (int countIdx = 0; countIdx < 5; countIdx++) {
        int numSprites = spriteCounts[countIdx];
        
        // Run for 5 seconds, count frames
        unsigned long start = millis();
        int frames = 0;
        
        while (millis() - start < 5000) {
            tft.fillScreen(TFT_BLACK);
            
            // Move and draw sprites
            for (int i = 0; i < numSprites; i++) {
                spriteX[i] += spriteDX[i];
                spriteY[i] += spriteDY[i];
                
                // Bounce at edges
                if (spriteX[i] <= 0 || spriteX[i] >= 208) spriteDX[i] = -spriteDX[i];
                if (spriteY[i] <= 0 || spriteY[i] >= 288) spriteDY[i] = -spriteDY[i];
                
                tft.pushImage(spriteX[i], spriteY[i], 32, 32, spriteBuffer);
            }
            
            frames++;
        }
        
        float fps = frames / 5.0;
        
        // Display result
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(String(numSprites) + " sprites:", 10, 10, 2);
        tft.drawString(String(fps, 1) + " FPS", 10, 40, 4);
        delay(2000);
        
        Serial.print(usePNG ? "PNG" : "RGB565");
        Serial.print(" - ");
        Serial.print(numSprites);
        Serial.print(" sprites: ");
        Serial.print(fps);
        Serial.println(" FPS");
    }
}
```

**Record Results:**
```
Format: PNG
  5 sprites:  _____ FPS
  10 sprites: _____ FPS
  15 sprites: _____ FPS
  20 sprites: _____ FPS
  25 sprites: _____ FPS

Format: RGB565
  5 sprites:  _____ FPS
  10 sprites: _____ FPS
  15 sprites: _____ FPS
  20 sprites: _____ FPS
  25 sprites: _____ FPS
```

---

### Test C2: Background + Sprites Test

**Purpose:** Test realistic game scenario (background + moving sprites)

**Test Code:**
```cpp
void testC2_BackgroundPlusSprites(bool usePNG) {
    // Load background (240x240)
    uint16_t *bgBuffer = (uint16_t*)malloc(240*240*2);
    if (usePNG) {
        PNG png;
        File f = SD.open("/sprite_tests/background_240x240.png");
        png.open(f, pngDecode);
        png.decode(bgBuffer, 0);
        f.close();
    } else {
        File f = SD.open("/sprite_tests/background_240x240.rgb565");
        f.read((uint8_t*)bgBuffer, 240*240*2);
        f.close();
    }
    
    // Load fish sprite
    uint16_t fishBuffer[32*32];
    if (usePNG) {
        PNG png;
        File f = SD.open("/sprite_tests/fish_trout.png");
        png.open(f, pngDecode);
        png.decode(fishBuffer, 0);
        f.close();
    } else {
        File f = SD.open("/sprite_tests/fish_trout.rgb565");
        f.read((uint8_t*)fishBuffer, 32*32*2);
        f.close();
    }
    
    // Initialize 10 fish
    for (int i = 0; i < 10; i++) {
        spriteX[i] = random(0, 208);
        spriteY[i] = random(0, 208);  // Within 240x240 bg
        spriteDX[i] = random(1, 3);
        spriteDY[i] = random(1, 3);
    }
    
    // Run for 10 seconds
    unsigned long start = millis();
    int frames = 0;
    
    while (millis() - start < 10000) {
        // Draw background
        tft.pushImage(0, 0, 240, 240, bgBuffer);
        
        // Move and draw fish
        for (int i = 0; i < 10; i++) {
            spriteX[i] += spriteDX[i];
            spriteY[i] += spriteDY[i];
            
            if (spriteX[i] <= 0 || spriteX[i] >= 208) spriteDX[i] = -spriteDX[i];
            if (spriteY[i] <= 0 || spriteY[i] >= 208) spriteDY[i] = -spriteDY[i];
            
            tft.pushImage(spriteX[i], spriteY[i], 32, 32, fishBuffer);
        }
        
        frames++;
    }
    
    float fps = frames / 10.0;
    
    // Display result
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(usePNG ? "PNG Format:" : "RGB565 Format:", 10, 10, 2);
    tft.drawString("BG + 10 fish", 10, 40, 2);
    tft.drawString(String(fps, 1) + " FPS", 10, 70, 4);
    
    free(bgBuffer);
    delay(3000);
    
    Serial.print(usePNG ? "PNG" : "RGB565");
    Serial.print(" - Background + 10 sprites: ");
    Serial.print(fps);
    Serial.println(" FPS");
}
```

**Record Results:**
```
Background (240×240) + 10 Fish (32×32):
  PNG format:    _____ FPS
  RGB565 format: _____ FPS
  Winner: _______
```

---

### Test C3: Dirty Rectangle Optimization Test

**Purpose:** Compare full redraw vs dirty rectangle approach

**Test Code:**
```cpp
void testC3_DirtyRectangles(bool usePNG) {
    // Load sprites
    uint16_t bgBuffer[240*240];
    uint16_t fishBuffer[32*32];
    // ... (load both as before)
    
    // TEST 1: Full screen redraw every frame
    unsigned long start = millis();
    int frames = 0;
    while (millis() - start < 5000) {
        tft.pushImage(0, 0, 240, 240, bgBuffer);  // Full BG
        tft.pushImage(100, 100, 32, 32, fishBuffer);  // One fish
        frames++;
    }
    float fullRedrawFPS = frames / 5.0;
    
    // TEST 2: Dirty rectangle (only redraw changed areas)
    frames = 0;
    int oldX = 100, oldY = 100;
    int newX = 100, newY = 100;
    tft.pushImage(0, 0, 240, 240, bgBuffer);  // Initial BG
    
    start = millis();
    while (millis() - start < 5000) {
        // Restore old position from BG
        uint16_t bgPatch[32*32];
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                bgPatch[y*32 + x] = bgBuffer[(oldY+y)*240 + (oldX+x)];
            }
        }
        tft.pushImage(oldX, oldY, 32, 32, bgPatch);
        
        // Move sprite
        newX += 1;
        if (newX > 208) newX = 0;
        
        // Draw at new position
        tft.pushImage(newX, newY, 32, 32, fishBuffer);
        
        oldX = newX;
        oldY = newY;
        frames++;
    }
    float dirtyRectFPS = frames / 5.0;
    
    // Display results
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Full redraw:", 10, 10, 2);
    tft.drawString(String(fullRedrawFPS, 1) + " FPS", 10, 30, 2);
    tft.drawString("Dirty rect:", 10, 60, 2);
    tft.drawString(String(dirtyRectFPS, 1) + " FPS", 10, 80, 2);
    
    delay(5000);
}
```

**Record Results:**
```
Optimization Comparison (PNG/RGB565):
  Full redraw:        _____ FPS
  Dirty rectangles:   _____ FPS
  Improvement:        _____x faster
```

---

## Test Results Summary

### Format Winner Matrix

| Test | PNG | RGB565 | Notes |
|------|-----|--------|-------|
| B1: Loading Speed (32×32) | | | |
| B1: Loading Speed (240×240) | | | |
| B2: Rendering Speed | | | |
| B3: Memory Usage | | | |
| B4: Visual Quality | | | |
| C1: FPS @ 5 sprites | | | |
| C1: FPS @ 15 sprites | | | |
| C1: FPS @ 25 sprites | | | |
| C2: Background + 10 fish | | | |

### Overall Recommendation

Based on test results:

**Use PNG if:**
- [ ] Loading speed is acceptable
- [ ] Memory overhead is acceptable
- [ ] Quality is noticeably better
- [ ] Easier asset pipeline (no conversion needed)

**Use RGB565 if:**
- [ ] Loading speed is critical
- [ ] Memory is constrained
- [ ] Rendering performance is better
- [ ] Maximum FPS needed

**Recommended format for Bass-Hole:** ___________

**Reasoning:**
```
_________________________________________________________
_________________________________________________________
_________________________________________________________
```

---

## Required Libraries

Add to `platformio.ini`:
```ini
lib_deps =
    bodmer/TFT_eSPI@^2.5.0
    bodmer/PNGdec@^1.0.1
```

---

## Next Steps After Testing

1. **Update Bass-Hole** with winning format
2. **Document findings** in `Bass-Hole/docs/SPRITE_FORMAT_CHOICE.md`
3. **Create conversion scripts** for chosen format
4. **Update DEVLOG** with hardware test results
5. **Apply verified display settings** from Part A to Bass-Hole

---

**Good luck with testing! 🎣**
