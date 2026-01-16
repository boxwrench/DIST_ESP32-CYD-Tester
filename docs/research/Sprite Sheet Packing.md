# Sprite Sheet Packing

A sprite sheet (texture atlas) combines multiple images into a single bitmap to minimise draw calls and reduce storage overhead.  Packing sprites efficiently can significantly improve load times and memory usage.

## Tools for packing

Several tools can generate sprite sheets:

- **TexturePacker** is a commercial tool that supports many game engines.  It offers efficient packing algorithms such as grid, basic, maxrects and polygon, trims transparent pixels, detects identical sprites, and can output multiple atlases when one sheet is full【547005632541732†L20-L55】.  It also supports PNG optimisation using `pngquant` and `zopfli`【547005632541732†L50-L64】.

- **Free Texture Packer** is an open‑source alternative with a GUI and CLI.  It provides features including rotation, trimming, identical sprite detection, multiple texture output, TinyPNG integration for optimisation, and various export formats (JSON, XML, CSS)【518814011615902†L45-L63】.

Both tools can export metadata describing each sprite’s rectangle within the atlas.

## Optimal sheet sizes for ESP32 memory

When loading a sprite sheet into RAM you must ensure it fits your available heap.  A 240×240 sheet at 16‑bit colour consumes 115 200 bytes (240×240×2), while a 320×240 sheet consumes 153 600 bytes.  On an ESP32 without PSRAM you typically have ~320 kB free, so a single large atlas plus other buffers may exceed memory.  Using 8‑bit sheets halves the memory requirement, allowing larger atlases.  Alternatively, split your sprites across multiple smaller sheets (e.g. 128×256) and load them on demand.

## UV coordinate extraction at runtime

The atlas metadata file (JSON/XML) lists each sprite’s position (`x`, `y`) and size (`w`, `h`) in pixels.  At runtime you parse this file (or convert it into a C array) and store the coordinates in a structure:

```c
typedef struct {
  uint16_t x, y, w, h;
} SpriteFrame;

extern const SpriteFrame frames[];
extern const uint16_t sheetPixels[];
```

To draw a sprite, you copy its rectangle from the sheet into a temporary buffer or directly draw a portion of the sheet using clipping:

```cpp
void drawSpriteFromSheet(int16_t x, int16_t y, const SpriteFrame &frame) {
  tft.setAddrWindow(x, y, frame.w, frame.h);
  for (uint16_t row=0; row<frame.h; row++) {
    const uint16_t* src = &sheetPixels[(frame.y + row) * SHEET_WIDTH + frame.x];
    tft.pushPixels(src, frame.w);
  }
}
```

This approach avoids storing each sprite separately and leverages the packed sheet.

## Data structures in C

Store the sprite sheet as a single array of `uint16_t` (RGB565) or `uint8_t` (indexed) values.  Create an array of `SpriteFrame` structures containing the coordinates and dimensions for each sprite.  Optionally maintain an animation table mapping sequences of frames to animation names.

```c
typedef struct {
  uint16_t startFrame;
  uint8_t frameCount;
  uint16_t frameTimeMs;
} Animation;

extern const Animation fishAnimations[];
```

## Code example: animation player using packed sheet

Combining the structures above, an animation player can look up the current frame and draw it:

```cpp
class SheetAnimator {
public:
  SheetAnimator(const SpriteFrame *frames, const Animation *anims, uint16_t sheetW)
    : frames(frames), animations(anims), sheetWidth(sheetW) {}

  void play(uint8_t animIndex) {
    current = animations[animIndex];
    currentTime = 0;
    frameIdx = 0;
  }

  void update(uint32_t delta) {
    currentTime += delta;
    if (currentTime >= current.frameTimeMs) {
      currentTime = 0;
      frameIdx = (frameIdx + 1) % current.frameCount;
    }
  }

  void draw(int16_t x, int16_t y) {
    const SpriteFrame &frame = frames[current.startFrame + frameIdx];
    drawSpriteFromSheet(x, y, frame);
  }

private:
  const SpriteFrame *frames;
  const Animation *animations;
  Animation current;
  uint16_t sheetWidth;
  uint8_t frameIdx;
  uint32_t currentTime;
};
```

This player reads rectangle definitions from the atlas and draws the corresponding portion of the sheet each frame.  By pre‑packing all animation frames, you reduce the overhead of switching between separate bitmaps and improve cache locality.