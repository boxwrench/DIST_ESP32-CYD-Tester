# Wokwi Research: ESP32 Game Development Gems

This document catalogs high-performance implementations and "gems" found in the Wokwi community. These patterns are highly relevant to the Bass-Hole project and general ESP32 CYD development.

> [!NOTE]
> **Resource Access**
> While most Wokwi projects are public and can be viewed without an account, **signing in** is required to save projects to your dashboard, upload custom files, or use the private "Wokwi Club" features.

## 💎 The Gems

### 1. Arcade-Level Optimization: "Galagino"
- **Project:** [Galagino Emulation](https://wokwi.com/projects/383837995080765441)
- **The Gem:** Shows how to run complex arcade games on ESP32.
- **Key Patterns:**
  - **Row-Buffer Rendering:** Renders 8 lines at a time into RAM instead of full-screen buffers. This is a "must-use" for high-resolution graphics on memory-constrained ESP32s.
  - **Task Pinning:** Logic on Core 0, Rendering on Core 1.
  - **DMA Blitting:** Custom sprite blitters that use DMA to push data while the CPU prepares the next line.

### 2. The CYD Performance Standard: LovyanGFX
- **Project:** [LVGL Layout for CYD](https://wokwi.com/projects/378637365901112321)
- **The Gem:** Demonstrates **LovyanGFX** as a high-speed alternative to `TFT_eSPI`.
- **Key Patterns:**
  - **Precise Calibration:** Contains verified XPT2046 touch calibration for the Cheap Yellow Display.
  - **DMA Handling:** LovyanGFX has superior built-in DMA management compared to standard libraries.

### 3. Advanced Input: 4-Way Swipe Detection
- **Project:** [Touch Interaction Demo](https://wokwi.com/projects/420679370071526401)
- **The Gem:** A robust gesture state machine for resistive touch screens.
- **Key Patterns:**
  - **`checkSwipe()`:** Filters jittery XPT2046 input to detect clean Up/Down/Left/Right swipes.
  - **Buffer Manipulation:** Includes RGB565 "Image Processing" functions for effects like screen flashes or color grading.

---

## 🚀 Recommended Integration Patterns for Bass-Hole

| Pattern | Benefit | Implementation Effort |
|---------|---------|-----------------------|
| **Row-Buffer Rendering** | Saves ~100KB RAM | High (Requires custom renderer) |
| **LovyanGFX** | Higher FPS / Smoother DMA | Medium (Library switch) |
| **Swipe Gestures** | Modern feel (Feed/Fish) | Low (Copied algorithm) |
| **Multi-Core tasking** | Prevents "stutter" | Medium (FreeRTOS tasks) |

---

## 🔗 Useful Wokwi Search Queries (Google)
- `site:wokwi.com ESP32 ILI9341 DMA`
- `site:wokwi.com ESP32 "dirty rectangle"`
- `site:wokwi.com ESP32 "LovyanGFX" CYD`
