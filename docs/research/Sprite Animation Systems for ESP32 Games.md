# Sprite Animation Systems for ESP32 Games

Animating sprites gives life to fish, bosses and other entities in CYD games.  On an ESP32 without PSRAM you must balance smooth animation with limited memory and CPU.  This document introduces frame‑based animation fundamentals, memory‑efficient approaches and patterns for managing multiple animated entities.

## 1. Frame‑based animation fundamentals

### Sprite sheet layouts

Animations are typically stored as **sprite sheets**—images containing multiple frames.  Common layouts:

* **Horizontal strip:** All frames are arranged in a single row; the frame width and height are constant.  Simpler indexing but less efficient use of space if frames are small.
* **Grid:** Frames are placed in rows and columns forming a grid.  Each cell holds one frame; the sheet width and height must be divisible by the frame dimensions.

When loading a sprite sheet, you supply the sheet’s width, height and frame dimensions.  To draw frame `i` you compute the source rectangle `(i * frameWidth, 0, frameWidth, frameHeight)` for a horizontal strip or `(col × frameWidth, row × frameHeight)` for a grid.

### Frame timing

There are two common timing approaches:

* **Fixed frame duration:** Each frame lasts for a constant period (e.g., 100 ms).  You track elapsed time (`millis()`) and advance the frame when the duration has passed.
* **Variable delta time:** Frame duration can vary; you accumulate delta time each update and advance frames accordingly.  This allows animations to remain consistent if the game runs at variable frame rates.

### Animation state machines

An entity may have multiple animations (idle, swim, eat, die).  Create a finite state machine (FSM) that determines the current animation based on the entity’s state.  The FSM resets the frame index when entering a new animation and loops or stops based on flags.

## 2. Memory‑efficient approaches

### Single sprite buffer with frame switching

Load the sprite sheet into PROGMEM or an external flash file system and copy only one frame at a time into a small RAM buffer.  For example, allocate a 32×32 buffer and `memcpy()` the current frame before pushing it to the display using `pushImage()`.

### Preloaded vs streamed frames

* **Preloaded:** Copy all frames into RAM at startup.  This allows fast switching but uses more memory.  Suitable for short animations with few frames.
* **Streamed:** Store frames in flash (SPIFFS or PROGMEM) and load them on demand.  Suitable for long or numerous animations.  Reading from flash is slower, so implement caching of recently used frames.

### Compressed animation data

To reduce flash usage, store frames in a compressed format (e.g., RLE, LZ4).  At runtime, decompress into a RAM buffer and push to the display.  Compression trades CPU time for storage space; test to ensure decompression speed is adequate.

## 3. Managing multiple animated entities

When many fish move independently, each with its own animation state, keep overhead low:

* **Animation struct:** Define a struct containing the sprite sheet pointer, frame count, current frame index, frame duration and elapsed time.  Update each animation in the main loop.
* **Pooling frames:** If many entities share the same animation (e.g., identical fish), store the sprite sheet once and share it among all instances.  Only store per‑entity state (frame index and timer).
* **Staggered updates:** To distribute CPU load, update animations in round‑robin fashion rather than all at once.  For example, update the frame index for half the fish each frame; the human eye will not notice the slight skew.

### Example animation struct

```cpp
struct Animation {
  const uint16_t *spriteSheet;   // pointer to PROGMEM or flash
  uint8_t         frameCount;
  uint8_t         frameIndex;
  uint16_t        frameWidth;
  uint16_t        frameHeight;
  uint32_t        frameDuration; // milliseconds per frame
  uint32_t        lastUpdate;

  void update() {
    uint32_t now = millis();
    if (now - lastUpdate >= frameDuration) {
      frameIndex = (frameIndex + 1) % frameCount;
      lastUpdate = now;
    }
  }
};

void drawFrame(const Animation &anim, int x, int y) {
  // Copy one frame from sprite sheet to buffer
  uint16_t buffer[32 * 32];
  uint32_t offset = (uint32_t)anim.frameIndex * anim.frameWidth * anim.frameHeight;
  memcpy_P(buffer, anim.spriteSheet + offset, anim.frameWidth * anim.frameHeight * 2);
  tft.setSwapBytes(true);
  tft.pushImage(x, y, anim.frameWidth, anim.frameHeight, buffer, 0xFFFF);
}
```

## 4. Blending and transitions

Some games require blending between animations (e.g., idle → swim → eat).  Implement transitions by interpolating frame indices or using cross‑fade.  On the ESP32, full alpha blending on a per‑pixel basis is expensive; instead fade brightness or scale the sprite gradually.

## Summary

Frame‑based sprite animation on ESP32 involves storing a sprite sheet, tracking frame indices and timing, and managing memory usage.  Use fixed or variable frame durations, employ state machines for different actions, and share sprite sheets among entities.  Load frames from flash as needed and reuse buffers to stay within RAM limits.  With these patterns you can animate dozens of fish and bosses smoothly on the CYD.
