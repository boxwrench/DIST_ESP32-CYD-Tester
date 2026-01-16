# TFT\_eSPI Sprite Performance on ESP32 (ILI9341)

The **TFT\_eSPI** library provides several ways to draw sprites on ESP32‑based boards such as the Cheap Yellow Display (CYD) with an **ILI9341** TFT controller.  A sprite in TFT\_eSPI is an off‑screen buffer you draw into and then push to the display.  The method you choose affects both speed and memory usage.  This document compares common approaches—per‑pixel drawing, `pushImage()`, **TFT\_eSprite**, and DMA—on an ESP32 without PSRAM.  It also explains how transparency works, estimates memory usage, and provides code examples.

## Background: sprite sizes and performance

TFT\_eSPI stores pixel data in 16‑bit RGB565 format by default.  A 320×240 sprite requires `320 × 240 × 2 = 153 600` bytes (≈150 kB) of RAM【708024030522094†L564-L598】; storing a full screen is possible only on ESP32 boards with at least 320 kB of free RAM.  Sprites can instead be created at 8‑bit or 1‑bit depth to reduce RAM consumption.  The TFT\_eSPI README notes that an 8‑bit 320×240 sprite uses ≈76 kB and a 1‑bit sprite uses ≈9 600 bytes【708024030522094†L564-L598】.  Drawing operations on a sprite are very fast—on a 160×128 sprite the full Adafruit graphics test completes in about **18 ms** (~55 FPS) and a full 320×240 sprite renders in about **45 ms**【708024030522094†L564-L598】.

## 1. Pixel‑by‑pixel drawing (drawPixel)

**Method:** Loop over each pixel of the sprite, calling `drawPixel(x, y, color)` to set the color.

**Transparency:** Not supported directly—every pixel is drawn.  You can simulate transparency by checking the source pixel and skipping `drawPixel()` calls when a designated “transparent” color is encountered.

**Memory requirements:** Only the destination display buffer (no sprite) is used.  A 32×32 sprite drawn pixel by pixel holds no off‑screen buffer; you store pixel data in program memory (e.g., PROGMEM) and draw each pixel individually.

**Performance:** Very slow.  Each `drawPixel()` call sends commands and data over SPI, so a 32×32 sprite requires 1 024 SPI transactions.  This method is acceptable for occasional updates but unsuitable for real‑time animation.

**Code example:**

```cpp
// 16‑bit RGB565 sprite data stored in PROGMEM
const uint16_t fishData[32 * 32] PROGMEM = { /* sprite pixels */ };

void drawFish(int16_t x0, int16_t y0) {
  for (uint8_t y = 0; y < 32; y++) {
    for (uint8_t x = 0; x < 32; x++) {
      uint16_t pixel = pgm_read_word(&fishData[y * 32 + x]);
      if (pixel != 0xFFFF) {        // 0xFFFF used as transparent color
        tft.drawPixel(x0 + x, y0 + y, pixel);
      }
    }
  }
}
```

**Limitations & gotchas:**

* The SPI bus remains busy for each pixel, severely limiting frame rates.
* Transparent pixels still require an `if` check, adding CPU overhead.
* Suitable only for tiny sprites or static UI elements.

## 2. pushImage with transparency

**Method:** Use `tft.pushImage(x, y, w, h, pixels, transparentColor)` to send an array of RGB565 values in one burst.  The optional `transparentColor` parameter skips drawing pixels equal to that value, providing simple transparency.

**Transparency:** Single color key (RGB565) is used; any pixel matching the key is not drawn.  There is no built‑in alpha blending.

**Memory requirements:** Requires a contiguous array of `w × h` 16‑bit pixel data in RAM or flash (≈2 bytes × number of pixels).  A 32×32 sprite uses **2 048 bytes**.

**Performance:** Much faster than `drawPixel()` because pixel data is sent in a single transaction.  Transparent pixels are skipped by the library, so it saves SPI bandwidth.  When drawing many sprites, call `tft.setSwapBytes(true)` if your pixel array is little‑endian to ensure correct byte order.

**Code example:**

```cpp
// 32×32 sprite stored in flash
const uint16_t fishPixels[32 * 32] PROGMEM = { /* ... */ };

void drawFish(int16_t x, int16_t y) {
  // Copy data from PROGMEM to a local buffer (could also use external flash)
  static uint16_t buffer[32 * 32];
  memcpy_P(buffer, fishPixels, sizeof(buffer));
  tft.setSwapBytes(true);              // ensure correct endianness
  tft.pushImage(x, y, 32, 32, buffer, 0xFFFF); // 0xFFFF is transparent
}
```

**Limitations & gotchas:**

* Only a single transparent color is supported; partial transparency and alpha blending are unavailable.
* The pixel array must be in RGB565 format; converting from PNG at runtime consumes CPU and RAM.
* Byte order matters—set `setSwapBytes(true)` if your array’s bytes are reversed.

## 3. TFT\_eSprite and pushSprite

**Method:** Use the **TFT\_eSprite** class to create an off‑screen buffer.  You draw onto the sprite using high‑level graphics calls (lines, text, other sprites) and then call `sprite.pushSprite(x, y)` to copy it to the display.

**Transparency:** Sprites support a transparent color (`setColorDepth(8)` or `setColorDepth(16)` and `setTransparentColor()` or pass a transparent color to `pushSprite()`).  The library will skip drawing pixels equal to that color when pushing to the display.

**Memory requirements:** A `w × h` 16‑bit sprite allocates `w × h × 2` bytes of RAM【708024030522094†L564-L598】.  For example, a 32×32 sprite consumes **2 048 bytes**.  Multiple sprites increase memory use, but you can dynamically create and delete sprites to manage RAM.

**Performance:** Very fast for complex operations.  Since drawing commands occur in RAM, the SPI bus is used only when pushing the finished sprite.  The TFT\_eSPI README reports that a 160×128 sprite graphics test completes in **18 ms** and a 320×240 full‑screen sprite renders in ≈45 ms【708024030522094†L564-L598】.  For a 32×32 sprite, push operations take a fraction of a millisecond, enabling smooth animation.

**Code example:**

```cpp
#include <TFT_eSPI.h>

TFT_eSPI tft;
TFT_eSprite sprite = TFT_eSprite(&tft);

void setup() {
  tft.begin();
  sprite.createSprite(32, 32);        // allocate 32×32 sprite
  sprite.setColorDepth(16);
  sprite.fillSprite(TFT_TRANSPARENT); // set entire sprite to transparent color
}

void drawFish(int16_t x, int16_t y) {
  sprite.fillSprite(TFT_TRANSPARENT);
  // draw into sprite using graphics primitives or another pushImage()
  sprite.fillCircle(16, 16, 15, TFT_YELLOW);
  sprite.pushSprite(x, y);            // transparent pixels are skipped
}
```

**Limitations & gotchas:**

* Each sprite occupies RAM continuously.  Creating too many large sprites can exhaust heap memory (see memory management document).
* Transparent drawing uses a single color key—no alpha blending.
* `pushSprite()` does not automatically restore the background; you must save or redraw the background yourself (dirty rectangle techniques).

## 4. pushImageDMA

**Method:** Some drivers, including recent versions of TFT\_eSPI when compiled with DMA support, provide `pushImageDMA()` to transfer pixel data using Direct Memory Access.  DMA can free the CPU while data transfers to the display.

**Transparency:** Similar to `pushImage()`, you can specify a transparent color.  DMA transfers only non‑transparent pixels.

**Memory requirements:** Same as `pushImage()`—you need the source pixel array in memory.

**Performance:** Highest possible throughput because data transfers without CPU involvement.  On some boards the SPI bus can run at 40 MHz.  A 32×32 sprite transfers in ≈51 µs (`32 × 32 × 2 bytes ÷ 40 MHz`).  The CPU can perform other tasks (e.g., collision detection) during transfer.

**Limitations & gotchas:**

* DMA is supported only on certain TFT controllers and requires enabling `#define SUPPORT_DMA` in **User\_Setup.h**.
* Complex to integrate with existing code; you must not modify the source buffer until DMA transfer completes.
* Transparent color support depends on the implementation.

## Choosing the right method

| Method                    | Transparency     | Memory per 32×32 sprite | Approx. speed             | Notes |
|--------------------------|-----------------|-------------------------|---------------------------|-------|
| **`drawPixel()` loop**   | None / manual   | None                    | Very slow (~kHz)          | Only for simple or static images |
| **`pushImage()`**        | Single color    | 2 048 bytes             | Fast (single SPI burst)   | Ensure correct byte order (`setSwapBytes(true)`) |
| **TFT\_eSprite**         | Single color    | 2 048 bytes (16‑bit)    | Very fast (~hundreds FPS) | Supports drawing primitives off‑screen; must manage sprite creation/deletion |
| **`pushImageDMA()`**     | Single color    | 2 048 bytes             | Fastest (CPU offloaded)   | Requires DMA support and careful use |

### Recommendations

* For **static icons** or rare updates, using `drawPixel()` from PROGMEM is acceptable and saves RAM.
* Use **`pushImage()`** for medium‑sized sprites or when decoding images from flash; it offers good speed and a simple API.
* For **animation or complex drawing**, allocate a **`TFT_eSprite`** and draw everything into it.  Push the sprite each frame.  Delete the sprite when no longer needed to free RAM.
* Use **DMA** only if your driver supports it and you require the highest throughput; ensure you handle buffer ownership correctly.

By understanding these techniques and their trade‑offs, you can choose the appropriate sprite rendering method for your CYD‑based game.
