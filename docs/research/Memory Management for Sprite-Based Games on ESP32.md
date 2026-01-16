# Memory Management for Sprite‑Based Games on ESP32

Developing games on the **ESP32 Cheap Yellow Display** (CYD) without PSRAM requires careful memory management because RAM is limited.  A 320×240 16‑bit frame buffer consumes **153 600 bytes**【192273562336216†L25-L35】, leaving little room for sprites, logic and other buffers.  This document describes how to store sprites, avoid fragmentation and estimate practical limits for CYD games.

## 1. PROGMEM vs heap storage for sprites

### Using PROGMEM

* **What it is:** PROGMEM stores constant data in flash memory instead of RAM.  Flash on the ESP32 can hold large arrays (sprites, fonts, strings) without using the heap.
* **When to use:** Use PROGMEM for **static assets** such as background images, sprite tables and text strings.  Reading from PROGMEM takes slightly longer than RAM, but this overhead is negligible compared to SPI transfers.
* **Reading data:** Use `pgm_read_byte()` or `pgm_read_word()` to access PROGMEM data, or copy it into a RAM buffer before drawing.  Example:

```cpp
// Store a 16‑bit sprite in flash
const uint16_t fishSprite[32 * 32] PROGMEM = { /* ... */ };

// Copy into RAM before pushing
uint16_t buffer[32 * 32];
memcpy_P(buffer, fishSprite, sizeof(buffer));
tft.setSwapBytes(true);
tft.pushImage(x, y, 32, 32, buffer, 0xFFFF);
```

### Using heap (dynamic memory)

* **What it is:** The heap is free RAM available at runtime.  You can allocate buffers using `new`, `malloc()` or library functions (e.g., `TFT_eSprite.createSprite()`).
* **When to use:** Use heap when assets are generated at runtime (e.g., procedural backgrounds), when you need to modify sprite data, or when decoding compressed formats such as PNG to RGB565.
* **Monitoring heap:** On ESP32 you can call `ESP.getFreeHeap()` to measure available memory and `ESP.getMaxAllocHeap()` for the largest block.  These functions help you decide whether a new sprite can be created.
* **Fragmentation:** Frequent allocation and deallocation of varying‑sized sprites can lead to heap fragmentation.  Over time this prevents large contiguous allocations even if total free memory remains.  To reduce fragmentation:
  * Allocate long‑lived buffers (e.g., background buffer) once at startup.
  * Use fixed‑size pools for transient objects like bullets.
  * Deallocate sprites in reverse order of allocation when possible.

## 2. Heap fragmentation and prevention

Fragmentation occurs when blocks of memory are repeatedly allocated and freed in different sizes.  Unused gaps form between allocated blocks, making it impossible to allocate a large contiguous region.  Strategies to mitigate fragmentation include【686340179156026†L94-L116】:

* **Avoid frequent `new`/`free`:** Use static or global arrays for sprites that persist throughout the game.
* **Object pools:** Create a pool of fixed‑size objects (e.g., bullets, particles) and reuse them instead of allocating and destroying dynamically.  This pattern reduces the overhead of creating and destroying objects【217222634938005†L12-L35】.
* **Memory arenas:** Allocate a large block of memory at startup and partition it manually for sprites and buffers.  When an object is no longer needed, mark its region as free without returning it to the system allocator.

## 3. Practical limits for CYD games

**Background buffer:** A full 320×240 16‑bit background requires ≈150 kB【192273562336216†L25-L35】.  Since the ESP32 has roughly 320 kB of usable RAM after code and stack, storing a full‑screen buffer leaves limited room for sprites and logic.  To save memory:

* Use 8‑bit or 1‑bit sprites: TFT\_eSPI allows 8‑bit sprites (≈76 kB for full screen) and 1‑bit sprites (≈9.6 kB)【708024030522094†L564-L598】.  For small sprites (e.g., 32×32) the difference is negligible but for large backgrounds it matters.
* Split the background into tiles (e.g., 32×32 or 64×64) stored in PROGMEM and draw only the visible tiles.
* Avoid storing the entire background if the scene is mostly static; instead redraw static elements each frame or use dirty rectangle techniques.

**Maximum number of sprites:** A typical 32×32 16‑bit sprite uses 2 048 bytes.  On a system with 100 kB of free heap (after code, buffers and Wi‑Fi), you could theoretically allocate ~50 such sprites (100 kB / 2 kB each).  In practice you must reserve memory for game logic, sound buffers, and network stacks; plan for half that number.  Using 8‑bit sprites halves RAM usage.

## 4. Memory‑saving techniques

1. **Use palette‑based sprites:** Instead of 16‑bit RGB565, store sprites in 8‑bit or 4‑bit indexed color.  Each sprite includes a small palette (16 or 256 colors).  When drawing, map each index to a 16‑bit color.  This reduces RAM and flash usage but requires an extra lookup per pixel.
2. **Sprite sheets:** Combine multiple frames of an animation into a single large bitmap.  You then read sub‑regions instead of storing separate arrays.  This reduces overhead per frame and improves caching.
3. **Tile‑based backgrounds:** For large backgrounds, divide the world into smaller tiles (e.g., 16×16 or 32×32) stored in PROGMEM.  Draw only the tiles visible in the viewport.  This reduces memory and eliminates the need for a full background buffer.
4. **Streaming from SD card or flash:** If your game includes many large images or level data, store them on an SD card or external SPI flash and stream them into RAM as needed.  Use a small buffer to decode part of the image and draw gradually.
5. **Use smaller data types:** When storing positions or velocities, use `int16_t` or `int8_t` instead of `int` to reduce RAM.  Avoid `float` if not necessary【686340179156026†L24-L81】.

## 5. Example: calculating memory usage

Suppose your fish game uses:

* One 240×240 background at 16‑bit (240×240×2 = 115 200 bytes).
* Ten fish sprites at 32×32 16‑bit (10 × 2 048 = 20 480 bytes).
* A sprite for the user interface at 120×32 8‑bit (120×32×1 = 3 840 bytes).

Total memory used for graphics buffers: **139 520 bytes**, leaving ~180 kB for code, stack, network, audio buffers and dynamic allocations.  You must also account for the size of global variables and libraries.  Always measure free heap at runtime (`ESP.getFreeHeap()`) to ensure there is enough headroom.

## Summary

Efficient memory management is critical when writing CYD games.  Store immutable data in PROGMEM, allocate large buffers at startup, and reuse objects via pools to avoid fragmentation.  Plan sprite sizes and quantities to fit within the available heap.  With careful planning and memory‑saving techniques you can implement visually rich games on the ESP32 without PSRAM.
