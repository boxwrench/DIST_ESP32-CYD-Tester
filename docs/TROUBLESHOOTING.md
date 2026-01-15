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

### Step 3.1: Identify the problem

| What you see | Likely cause |
|--------------|--------------|
| Red and blue swapped | RGB order wrong |
| Everything inverted (negative) | Panel inversion setting |
| Sprites OK, text wrong | Sprite vs primitive conflict |
| Washed out / pale | Brightness or color depth |

### Step 3.2: Red/Blue swapped

Add to build_flags:
```ini
-DTFT_RGB_ORDER=TFT_BGR
```

Or in code:
```cpp
tft.setSwapBytes(true);
```

### Step 3.3: Colors inverted (negative image)

**For dual-USB boards:**
```cpp
#define TFT_INVERSION_ON
```

**For code-level control:**
```cpp
tft.invertDisplay(true);   // or false - try both
```

### Step 3.4: Sprites OK but UI wrong (or vice versa)

This is the sprite vs primitive conflict. Two approaches:

**Approach A: Use invertDisplay(true) + normal colors**
```cpp
tft.invertDisplay(true);
#define COLOR_BLACK 0x0000  // Normal RGB565
#define COLOR_WHITE 0xFFFF
```

**Approach B: Use invertDisplay(false) + inverted colors**
```cpp
tft.invertDisplay(false);
#define COLOR_BLACK 0xFFFF  // XOR with 0xFFFF
#define COLOR_WHITE 0x0000
```

Run the Color Test in this tool to determine which works for your board.

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
