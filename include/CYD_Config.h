#ifndef CYD_CONFIG_H
#define CYD_CONFIG_H

// ==========================================
// BASS HOLE - HARDWARE CONFIGURATION
// Machine: ESP32-2432S028R (CYD)
// Generated: 2026-01-14
// ==========================================

// --- Touch Screen Calibration ---
// VALID FOR: Rotation 0 (USB at Bottom)
// Note: If you rotate the screen, you must remap these values.
#define TOUCH_MIN_X 3570
#define TOUCH_MAX_X 544
#define TOUCH_MIN_Y 3429
#define TOUCH_MAX_Y 532

// --- Pin Configuration ---
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

// Backlight: CYD variants use either 21 or 27 (or both)
#define TFT_BL_1  21
#define TFT_BL_2  27

#define TOUCH_CS  33
#define TOUCH_IRQ 36

#define SD_CS     5

// --- Audio (Optional) ---
// Connector usually labeled "SPEAK" or "SPK"
// Common pin on standard CYD is 26 or 17 (requires dac/pwm)
#define SPEAKER_PIN 26

// --- System Info ---
// Chip Model: ESP32-D0WD-V3
// Revision: 3
// Core Count: 2
// Flash Size: 4 MB

#endif // CYD_CONFIG_H
