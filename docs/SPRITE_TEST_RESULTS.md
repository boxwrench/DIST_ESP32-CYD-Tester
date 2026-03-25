# Sprite Test Results - ESP32 CYD Hardware Validation

**Last Updated:** 2026-01-19  
**Hardware:** ESP32-2432S028R (Cheap Yellow Display)  
**Verified Config:** ILI9341_2_DRIVER, Gamma 0x01, 40MHz SPI

---

## ✅ VERIFIED GROUND TRUTH CONFIGURATION (2026-01-19)

### Critical Discovery: Gamma Correction

**The washed-out sprite issue was caused by the default gamma curve, NOT byte-swapping or RGB order.**

```cpp
// REQUIRED after tft.init() for vibrant sprite colors
tft.writecommand(0x26);  // Gamma Set command
tft.writedata(0x01);     // Gamma curve 1 (more saturated)
```

**Why this matters:**
- `fillRect()` and UI elements look fine with default gamma
- `pushImage()` sprites appear washed out/desaturated with default gamma
- This made diagnosis extremely difficult—colors weren't "wrong," just muted
- Gamma 0x01 restores full vibrancy to sprites

### Verified Display Configuration

```ini
build_flags = 
    -DUSER_SETUP_LOADED=1
    -DILI9341_2_DRIVER=1        # Fixes static/corruption vs ILI9341_DRIVER
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_MISO=12
    -DTFT_MOSI=13
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1
    -DTFT_BL=21
    -DTFT_BACKLIGHT_ON=HIGH
    # DO NOT USE: -DTFT_RGB_ORDER=1 (causes red/blue swap)
    # DO NOT USE: -DUSE_HSPI_PORT=1 (unnecessary)
    -DLOAD_GLCD=1
    -DLOAD_FONT2=1
    -DLOAD_FONT4=1
    -DLOAD_FONT6=1
    -DLOAD_FONT7=1
    -DLOAD_FONT8=1
    -DLOAD_GFXFF=1
    -DSMOOTH_FONT=1
    -DSPI_FREQUENCY=40000000
    -DSPI_READ_FREQUENCY=20000000
    -DSPI_TOUCH_FREQUENCY=2500000
```

### Verified Code Initialization

```cpp
void setup() {
    // Backlight - try BOTH pins
    pinMode(21, OUTPUT); digitalWrite(21, HIGH);
    pinMode(27, OUTPUT); digitalWrite(27, HIGH);
    
    // Display init
    tft.init();
    
    // CRITICAL: Set gamma for vibrant sprite colors
    tft.writecommand(0x26);  // Gamma Set
    tft.writedata(0x01);     // Gamma curve 1
    
    // Required for ILI9341_2_DRIVER variant
    tft.invertDisplay(true);
    
    // Rotation: 1 = Landscape, USB down
    tft.setRotation(1);
    
    // Required for RGB565 sprites
    tft.setSwapBytes(true);
}
```

### Verified Sprite Asset Pipeline

**Format:** RGB565, Little-Endian  
**Tool:** `tools/png_to_rgb565.py` (default RGB mode, NOT --bgr)  
**Firmware:** `setSwapBytes(true)`

```bash
# Generate verified RGB565 sprite
python tools/png_to_rgb565.py input.png output.rgb565
```

**Result:** Accurate colors with crisp outlines

---

## Performance Summary (2026-01-18)

### Loading Speed (B1)
| Format | Load Time | Notes |
|--------|-----------|-------|
| **PNG (decode)** | 34,956 μs | Includes decompression |
| **RGB565 (raw)** | 21,694 μs | **38% faster** ✓ |

**Recommendation:** Pre-convert assets to RGB565 for production.

---

### Rendering Speed (B2)
| Metric | Value |
|--------|-------|
| **Single sprite render** | 696 μs |
| **Theoretical max FPS** | 1,436 FPS |

---

### Sprite Count vs FPS (C1)
| Sprites | FPS | Notes |
|---------|-----|-------|
| 5 | 29.0 | Smooth gameplay |
| 10 | 26.3 | Still smooth |
| 15 | 24.3 | Acceptable |
| 20 | 22.3 | Starting to slow |
| 25 | 21.0 | Minimum playable |

**Key Insight:** FPS degrades gracefully. Bass-Hole target of 15 fish @ 24+ FPS is achievable!

---

### Memory Usage (B3)
| Metric | Value |
|--------|-------|
| **Free heap (start)** | 263,504 bytes |
| **After sprite buffers** | 263,504 bytes |
| **Background (240x240)** | 115,200 bytes (fails on internal RAM) |

**Note:** Large backgrounds require PSRAM or tiling strategy.

---

## Issues Identified & Resolved

| Issue | Cause | Solution |
|-------|-------|----------|
| ❌ Washed out sprites | Default gamma curve | **Set Gamma 0x01** after init |
| ❌ Screen static | Wrong driver variant | Use `ILI9341_2_DRIVER` |
| ❌ Inverted colors | Driver variant default | Add `invertDisplay(true)` |
| ❌ Red/Blue swap | TFT_RGB_ORDER flag | **Remove flag**, use default RGB |
| ❌ BG malloc fail | 115KB exceeds free RAM | Use PSRAM or tiling |

---

## Diagnostic Test Results (2026-01-19)

### Gamma Curve Comparison
| Gamma | Result |
|-------|--------|
| 0x01 | ✅ **Vibrant, accurate colors** |
| 0x02 | Slightly less saturated |
| 0x04 | Noticeably washed out |
| 0x08 | Very washed out |

### Sprite Conversion Tool Comparison
| Tool | Format | swap=ON | swap=OFF |
|------|--------|---------|----------|
| **EXTERNAL** | RGB565 LE | ✅ Correct colors | Red/Blue swap |
| **CURRENT** | BGR565 LE | Red/Blue swap | ✅ Correct colors |
| **VERIFIED** | RGB565 LE | ✅ **Best: Accurate + Crisp** | Red/Blue swap |

**Conclusion:** Use `png_to_rgb565.py` (RGB mode) + `setSwapBytes(true)`

---

## Final Hardware Recommendations

1. **Driver:** Use `ILI9341_2_DRIVER` (not `ILI9341_DRIVER`)
2. **Gamma:** **ALWAYS set Gamma 0x01** after `tft.init()`
3. **Inversion:** Use `invertDisplay(true)` with _2_DRIVER variant
4. **Sprites:** RGB565 Little-Endian with `setSwapBytes(true)`
5. **Rotation:** `1` for landscape (USB down)
6. **Transparency:** Blend PNG alpha against BLACK during conversion

---

## Checklist Status
- [x] Identify washed-out color root cause (Gamma)
- [x] Test all 4 gamma curves on hardware
- [x] Verify RGB vs BGR sprite formats
- [x] Create verified ground truth asset
- [x] Document complete working configuration
- [x] Update all reference documentation
