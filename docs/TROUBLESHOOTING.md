# CYD Troubleshooting Decision Tree

This guide walks you through diagnosing and fixing common ESP32 CYD issues. Follow the flowchart based on your symptoms.

---

## Quick Diagnosis

```
START HERE: What do you see?
    |
    +---> Black screen ---------> Go to Section 1
    |
    +---> White screen ---------> Go to Section 2
    |
    +---> Colors wrong ---------> Go to Section 3
    |
    +---> Touch not working ----> Go to Section 4
    |
    +---> Touch misaligned -----> Go to Section 5
    |
    +---> SD card issues -------> Go to Section 6
    |
    +---> Upload fails ---------> Go to Section 7
    |
    +---> Crashes/reboots ------> Go to Section 8
```

---

## Section 1: Black Screen

### Step 1.1: Is the backlight on?

Look at the screen from an angle. Can you see faint content?

**NO - Backlight is off:**
```cpp
// Try BOTH common backlight pins
pinMode(21, OUTPUT);
digitalWrite(21, HIGH);
pinMode(27, OUTPUT);
digitalWrite(27, HIGH);
```

**YES - Backlight works but nothing displays:**
Go to Step 1.2

### Step 1.2: Is code running?

Add to setup():
```cpp
Serial.begin(115200);
Serial.println("Setup started!");
```

**No serial output:**
- USB cable might be power-only (no data lines)
- Try a different USB cable
- Hold BOOT button while uploading

**Serial works:**
- Check `tft.init()` is being called
- Add `delay(100);` before `tft.init();`
- Verify SPI pins in platformio.ini

---

## Section 2: White Screen

### Step 2.1: Check USB connector count

**Single USB (Micro-USB only):**
```cpp
#define ILI9341_DRIVER
```

**Dual USB (USB-C + Micro-USB):**
```cpp
#define ST7789_DRIVER
```

This is the #1 cause of white screens!

### Step 2.2: Verify build flags

Ensure platformio.ini has:
```ini
build_flags =
    -DUSER_SETUP_LOADED=1
    -DILI9341_DRIVER=1    ; or ST7789_DRIVER=1
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_MOSI=13
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1
```

### Step 2.3: Reduce SPI speed

If still white, try slower SPI:
```ini
-DSPI_FREQUENCY=27000000   ; Instead of 40000000
```

---

## Section 3: Colors Wrong

### Understanding the THREE Separate Color Settings

**CRITICAL:** These are THREE INDEPENDENT settings that people often conflate:

| Setting | What it controls | Symptoms when wrong |
|---------|------------------|---------------------|
| **RGB Order** | Red vs Blue channel position | Red/blue swapped (skin looks cyan) |
| **Byte Swap** | Endianness of pixel data in buffers | UI fine, sprites garbled/wrong colors |
| **Inversion** | Panel-level color inversion | Everything looks like a photo negative |

You can have RGB order correct but byte swap wrong. You can have inversion correct but RGB order wrong. **Test each independently.**

---

### Step 3.1: Identify the problem

| What you see | Likely cause | Fix |
|--------------|--------------|-----|
| Red and blue swapped | RGB order wrong | Step 3.2 |
| Everything inverted (negative) | Panel inversion | Step 3.3 |
| **UI fine, sprites/images wrong** | **Byte swap mismatch** | **Step 3.4** |
| Washed out / pale | Double conversion or COLMOD | Step 3.5 |

---

### Step 3.2: Red/Blue swapped (RGB Order)

**Test:** Draw solid color bars with `fillRect()`:
```cpp
tft.fillRect(0, 0, 80, 50, 0xF800);   // Should be RED
tft.fillRect(80, 0, 80, 50, 0x07E0);  // Should be GREEN
tft.fillRect(160, 0, 80, 50, 0x001F); // Should be BLUE
```

If red shows as blue (and vice versa), fix RGB order:

**In platformio.ini:**
```ini
-DTFT_RGB_ORDER=TFT_BGR   ; or TFT_RGB - try both
```

**Note:** `setSwapBytes()` does NOT fix RGB order - that's a different issue!

---

### Step 3.3: Colors inverted (negative image)

If colors look like a photo negative (white↔black, colors inverted):

**Build flag approach (preferred):**
```ini
-DTFT_INVERSION_ON
```

**Runtime approach (for testing only):**
```cpp
tft.invertDisplay(true);   // or false - try both
```

**Important:** If using `invertDisplay()` makes colors look "washed out", that's a sign the underlying driver/init is wrong for your panel. Prefer fixing via build flags.

---

### Step 3.4: UI fine but sprites/images wrong (BYTE SWAP - Most Common!)

This is THE classic CYD sprite problem:
- `fillRect()`, `drawString()`, `drawPixel()` look correct
- `pushImage()`, `pushSprite()`, sprite buffers look wrong (garbled, inverted-ish, wrong palette)

**Root cause:** Your sprite pixel buffer is RGB565 but bytes are in opposite order from what the driver expects.

**The fix - set swap bytes ONCE in init:**
```cpp
void gfxInit() {
    tft.init();
    tft.setSwapBytes(true);  // Apply to ALL pushImage/pushSprite calls
    // ... rest of init
}
```

**Critical understanding:**
- `setSwapBytes()` ONLY affects `pushImage()` / `pushSprite()` / buffer operations
- `setSwapBytes()` does NOT affect `drawPixel()` / `fillRect()` / primitives
- If you're doing pixel-by-pixel sprite rendering with `drawPixel()`, you need MANUAL byte swap:
  ```cpp
  uint16_t pixel = pgm_read_word(&sprite[offset]);
  tft.drawPixel(x, y, (pixel >> 8) | (pixel << 8));  // Manual swap
  ```

**Best practice:** Use `pushImage()` with `setSwapBytes(true)` instead of pixel-by-pixel loops. It's faster AND handles byte order automatically.

---

### Step 3.5: Washed out / pale colors

Usually caused by:
1. Double byte-swapping (sprite data already swapped + setSwapBytes(true))
2. Wrong color depth (COLMOD) in driver init
3. PNG decoder doing unexpected gamma/color conversion

**Debug approach:**
1. Use RAW RGB565 sprite data (not PNG) to eliminate decoder variables
2. Verify sprite data byte order matches your setSwapBytes setting
3. Check you're not swapping bytes twice (once in data, once in code)

---

### Step 3.6: Sprites OK but UI wrong (or vice versa)

If primitives and sprites show OPPOSITE behavior, you likely have:
- Correct byte swap for one path but not the other
- Or inversion affecting them differently

**Solution:** Ensure consistent settings across ALL rendering:
```cpp
void gfxInit() {
    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);      // For all pushImage operations
    // invertDisplay handled by TFT_INVERSION_ON build flag
}
```

Run the Color Test in this tool to determine which combination works for your board.

---

## Section 4: Touch Not Working

### Step 4.1: Verify touch pins

Touch uses SEPARATE SPI pins from display:
```cpp
#define XPT2046_CLK  25   // NOT 14!
#define XPT2046_MOSI 32   // NOT 13!
#define XPT2046_MISO 39
#define XPT2046_CS   33   // NOT 15!
#define XPT2046_IRQ  36
```

### Step 4.2: Initialize touch SPI correctly

```cpp
SPIClass touchSPI(VSPI);
touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
touch.begin(touchSPI);
```

### Step 4.3: Check IRQ pin

```cpp
pinMode(36, INPUT);
// GPIO 36 is input-only - cannot use pinMode(36, INPUT_PULLUP)
```

### Step 4.4: Test raw touch

```cpp
void loop() {
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        Serial.printf("X=%d Y=%d Z=%d\n", p.x, p.y, p.z);
    }
}
```

If you get values, touch hardware works - calibration needed.

---

## Section 5: Touch Misaligned

### Step 5.1: Get raw values

Touch all four corners and note the raw X, Y values:
- Top-left corner
- Top-right corner
- Bottom-left corner
- Bottom-right corner

### Step 5.2: Determine axis behavior

**X-axis inverted?**
If touching LEFT gives HIGH X values:
```cpp
int16_t mappedX = map(p.x, TOUCH_MAX_X, TOUCH_MIN_X, 0, SCREEN_WIDTH);
```

**Y-axis inverted?**
If touching TOP gives HIGH Y values:
```cpp
int16_t mappedY = map(p.y, TOUCH_MAX_Y, TOUCH_MIN_Y, 0, SCREEN_HEIGHT);
```

### Step 5.3: Set rotation

Touch rotation is INDEPENDENT from display rotation:
```cpp
tft.setRotation(0);    // Display rotation
touch.setRotation(1);  // Touch rotation - try 0, 1, 2, 3
```

### Step 5.4: Common working values

TZT ESP32 CYD 2.4":
```cpp
#define TOUCH_MIN_X  600
#define TOUCH_MAX_X  3600
#define TOUCH_MIN_Y  500
#define TOUCH_MAX_Y  3600
```

Generic CYD:
```cpp
#define TOUCH_MIN_X  280
#define TOUCH_MAX_X  3860
#define TOUCH_MIN_Y  340
#define TOUCH_MAX_Y  3860
```

---

## Section 6: SD Card Issues

### Step 6.1: Card not detected

1. Format as FAT32 (not exFAT)
2. Use allocation size 32KB
3. Insert card BEFORE powering on
4. Try a different card

### Step 6.2: Verify CS pin

```cpp
#define SD_CS 5
if (!SD.begin(SD_CS)) {
    Serial.println("SD mount failed");
}
```

### Step 6.3: SPI conflict

SD uses VSPI (separate from display HSPI):
```cpp
// SD pins - VSPI
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK  18
#define SD_CS   5
```

---

## Section 7: Upload Fails

### Step 7.1: "Connecting..." timeout

1. Hold **BOOT** button while clicking Upload
2. Release when you see `Writing at 0x...`

### Step 7.2: Wrong port

- Windows: Check Device Manager for COM port
- Linux: Check `/dev/ttyUSB*` or `/dev/ttyACM*`
- May need CH340 drivers

### Step 7.3: Linux-specific

```bash
# Remove brltty (conflicts with CH340)
sudo apt remove brltty

# Add user to dialout group
sudo usermod -a -G dialout $USER
# Then logout and login again
```

### Step 7.4: Cable issues

Some USB cables are power-only. Try a different cable that you know works for data transfer.

---

## Section 8: Crashes/Reboots

### Step 8.1: Memory issues

**Sprite too large:**
```cpp
// Max safe sprite without PSRAM: ~200x200 pixels
sprite.createSprite(200, 200);  // 80KB - OK
sprite.createSprite(320, 240);  // 150KB - CRASH
```

**Solution:** Use smaller sprites or partial screen updates.

### Step 8.2: Check free heap

```cpp
Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
// Should be > 50KB for stable operation
```

### Step 8.3: Stack overflow

If crashing in specific functions:
```cpp
// Increase task stack size
xTaskCreate(myTask, "Task", 8192, NULL, 1, NULL);  // 8KB stack
```

### Step 8.4: Power issues

- USB hub may not provide enough current
- Try direct connection to computer
- Use powered USB hub or USB charger (2A+)

---

## Color Test Procedure

This tool includes a color test. Here's how to interpret it:

```
+------------------+------------------+
|   1. RAW COLORS  |   2. XOR COLORS  |
+------------------+------------------+
|      RED         |      CYAN        |
|     GREEN        |     MAGENTA      |
|      BLUE        |      YELLOW      |
|     WHITE        |      BLACK       |
|     BLACK        |      WHITE       |
+------------------+------------------+
```

**Which side shows correct colors?**

- **LEFT (RAW) correct:** Use `invertDisplay(true)` + normal RGB565
- **RIGHT (XOR) correct:** Use `invertDisplay(false)` + normal RGB565

---

## Quick Reference: Common Fixes

| Problem | One-liner fix |
|---------|---------------|
| Black screen | `digitalWrite(21, HIGH);` |
| White screen | Check USB count -> correct driver |
| Red/blue swap | Add `-DTFT_RGB_ORDER=TFT_BGR` |
| Inverted colors | Add `-DTFT_INVERSION_ON` |
| **UI fine, sprites wrong** | **`tft.setSwapBytes(true);` in init** |
| Touch dead | Use pins 25/32/39/33/36 not display pins |
| Touch flipped | Swap min/max in map() function |
| Upload fail | Hold BOOT button |
| SD fail | Format FAT32, insert before power |

---

## Still Stuck?

1. Run the full test suite in this tool
2. Check Serial Monitor output (115200 baud)
3. Copy the generated config block
4. Compare against known-working configs in `docs/CYD_VARIANTS.md`

Community resources:
- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- [rzeldent/esp32-smartdisplay](https://github.com/rzeldent/esp32-smartdisplay)
