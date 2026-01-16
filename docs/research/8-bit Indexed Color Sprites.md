# 8‑bit Indexed Color Sprites

Using 8‑bit indexed colour sprites instead of 16‑bit RGB565 can halve your memory usage at the cost of a small amount of extra processing.  The TFT_eSPI library supports 1‑bit, 4‑bit and 8‑bit sprites via the `setColorDepth()` function【417674275483255†L24-L28】.

## Palette design

A palette sprite stores a lookup table of 16 or 256 16‑bit colours and then references them with 4‑ or 8‑bit indices.  For small sprites such as fish or UI icons, a 16‑colour palette is often sufficient; backgrounds may require 256 entries.  When designing palettes ensure that transparent colours are mapped to a reserved index.  TFT_eSPI reserves the 16‑bit constant `TFT_TRANSPARENT` as a special transparent colour【488272378528787†L120-L125】 which is encoded into an 8‑bit index automatically.

## Converting PNGs to indexed C arrays

PNG files typically use 24‑bit colour and an alpha channel; they must be converted to an indexed format at build time.  Tools such as **GIMP**, **Aseprite** or command‑line utilities like `ImageMagick` can quantise an image to 256 or 16 colours and export raw indexed data.  You can then convert the indexed pixel data and palette into C arrays using scripts (e.g. `png2c.py`).  The palette should be stored as an array of `uint16_t` values in 565 format:

```c
const uint16_t fish_palette[16] = {
  0xFFFF, /* white */
  0x0000, /* black */
  // … define up to 16 colours
};

const uint8_t fish_pixels[32*32] PROGMEM = {
  /* each byte indexes into fish_palette */
};
```

## Rendering indexed sprites

Create a `TFT_eSprite` instance, set its colour depth to 8 and assign the palette:

```cpp
TFT_eSprite spr = TFT_eSprite(&tft);
spr.setColorDepth(8);         // 8‑bit indexed
spr.createSprite(width, height);
spr.createPalette(fish_palette, 16);  // load 16‑colour palette
spr.setPaletteColor(0, TFT_TRANSPARENT); // reserve index 0 for transparency
```

When drawing, fill the sprite’s pixel array and then call `spr.pushSprite(x, y, 0)` to draw it using index 0 as the transparent background.  The Transparent Sprite Demo shows that an 8‑bit 70×80 sprite uses 5 600 bytes compared with 11 200 bytes for a 16‑bit sprite【273131442873782†L14-L22】【273131442873782†L81-L101】, demonstrating the 50 % memory savings.

## Memory savings and trade‑offs

The `Sprite.h` documentation states that 1‑bit sprites use 1 bit per pixel, 4‑bit sprites use a nibble per pixel, 8‑bit sprites use one byte per pixel and 16‑bit sprites use two bytes per pixel【417674275483255†L24-L28】.  This means a 240×240 16‑bit background consumes 115 200 bytes, whereas the same sprite at 8 bit consumes 57 600 bytes.  Halving the memory allows you to allocate larger buffers or additional sprites.  However, each pixel must be looked up in the palette and converted to 16‑bit colour when drawn, which adds a small overhead.  For static images or slow‑moving sprites the overhead is negligible; for high‑frame‑rate animations the cost can be measurable but is still far faster than drawing pixel by pixel.

## Animated sprite class example

To encapsulate an indexed animation you can store each frame in PROGMEM and assign a common palette.  The class holds the palette and an array of pointers to frame data; at runtime it selects the frame based on elapsed time and draws it:

```cpp
class AnimatedSprite8 {
public:
  AnimatedSprite8(TFT_eSPI *tft, const uint16_t *palette, uint8_t frameCount)
    : tft(tft), frameCount(frameCount) {
      spr.setColorDepth(8);
      spr.createPalette((uint16_t*)palette, 256);
    }

  void drawFrame(int x, int y, const uint8_t *frame) {
    spr.setBuffer((uint8_t*)frame); // point sprite at frame data in PROGMEM
    spr.pushSprite(x, y, 0);
  }
  // ... implement timing and frame switching ...
private:
  TFT_eSPI *tft;
  TFT_eSprite spr = TFT_eSprite(tft);
  uint8_t frameCount;
};
```

When using 8‑bit sprites you can preload all frames into flash or external storage.  During the game you change the pointer to the desired frame and push it; the palette remains fixed, so memory bandwidth is reduced.

## Performance impact

Indexed colour rendering requires an extra lookup per pixel, but the TFT_eSPI library is optimised and still draws palette sprites rapidly.  Transparent blending is handled automatically: `TFT_TRANSPARENT` encodes to an 8‑bit index and decodes back to 16 bit【488272378528787†L120-L125】, so you can specify a transparent background without additional processing.  Overall, using 8‑bit sprites is an effective way to save RAM on boards without PSRAM while maintaining acceptable frame rates.