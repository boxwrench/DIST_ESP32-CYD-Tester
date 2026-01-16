# Research Documents

This folder contains research documentation generated from the prompts in `../RESEARCH_AGENDA.md`.

## Status: ✅ All Research Complete (2026-01-16)

**21 total documents** covering core CYD development and advanced techniques.

---

## Phase 1: Core CYD Development (Topics 1-13)

| File | Topic |
|------|-------|
| `TFT_eSPI Sprite Performance on ESP32 (ILI9341).md` | Sprite rendering methods comparison |
| `Memory Management for Sprite-Based Games on ESP32.md` | ESP32 memory limits and strategies |
| `Sprite Animation Systems for ESP32 Games.md` | Animation frame management |
| `Dirty Rectangle Rendering for TFT Displays.md` | Partial screen update optimization |
| `Audio Output Options for ESP32 CYD Projects.md` | Sound on CYD boards |
| `Handling Touch Input During Heavy Rendering on ESP32.md` | Responsive touch during rendering |
| `Game State Persistence on ESP32.md` | Save systems and persistence |
| `Performance Optimization for Boss Battles and Bullet-Hell Moments.md` | Many sprites performance |
| `Text and Dialogue Rendering on ESP32 TFT Games.md` | Fonts and dialogue systems |
| `Sensor Integration for ESP32 CYD Projects.md` | Environmental and motion sensors |
| `Cloud Connectivity and API Integration for ESP32 CYD Projects.md` | APIs, leaderboards, AI integration |
| `Additional Input Methods for ESP32 CYD Projects.md` | Buttons, keyboards, gamepads |
| `Output Options for ESP32 CYD Projects Beyond the Display.md` | Printers, email, notifications |

---

## Phase 2: Advanced Techniques (Topics 14-21)

| File | Topic |
|------|-------|
| `DMA Transfer Deep Dive.md` | pushImageDMA implementation guide |
| `8-bit Indexed Color Sprites.md` | 50% RAM savings with indexed color |
| `Dual-Core Task Architecture.md` | FreeRTOS multi-core patterns |
| `Sprite Sheet Packing.md` | Texture atlas workflow |
| `LVGL Integration.md` | Comparison with raw TFT_eSPI |
| `Real-Time Audio Mixing.md` | Multi-channel game SFX |
| `SD Card Streaming.md` | Buffered loading patterns |
| `Anti-Aliased Sprites.md` | Alpha blending approaches |

---

## Key Findings

### Immediate Actionable (for Bass-Hole)

1. **Byte Swap Bug** - Most sprite color issues likely caused by inconsistent byte swapping. Fix by calling `setSwapBytes(true)` once in `gfxInit()`.

2. **Use pushImage for Transparency** - Research confirms `pushImage(x, y, w, h, data, transparentColor)` is much faster than pixel-by-pixel loops.

3. **Skip DMA** - Not enough RAM on standard CYD for double-buffering. Standard rendering already achieves 30+ FPS.

4. **Skip LVGL** - Raw TFT_eSPI is better for games. LVGL overhead not justified for simple UI.

### Future Optimization Options

5. **8-bit Indexed Sprites** - Could halve sprite memory. Fish need only ~10 colors, good candidate for 16-color palettes.

6. **Dual-Core Architecture** - Touch on Core 0, rendering on Core 1. Good for responsiveness during complex scenes.

7. **Sprite Sheets** - Packing animations into atlases reduces draw overhead.

---

## Next Steps

All planned research is complete. Future topics should emerge from **hardware testing via SPRITE_TEST_PLAN.md**.
