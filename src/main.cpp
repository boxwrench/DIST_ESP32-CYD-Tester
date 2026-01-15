#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <SD.h>
#include <math.h>
#include "CYD_2432S028R.h"

// --- Hardware Definitions ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SD_CS 5

// RGB LED pins (active LOW)
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17

// --- Globals ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Test results
bool colorInvertNeeded = true;  // Will be determined by color test
String driverType = "ILI9341";  // Will be set based on user input

// Calibration Data
#ifdef TOUCH_MIN_X
uint16_t touchMinX = TOUCH_MIN_X;
uint16_t touchMaxX = TOUCH_MAX_X;
uint16_t touchMinY = TOUCH_MIN_Y;
uint16_t touchMaxY = TOUCH_MAX_Y;
#else
uint16_t touchMinX = 0;
uint16_t touchMaxX = 0;
uint16_t touchMinY = 0;
uint16_t touchMaxY = 0;
#endif

void printConfig()
{
    Serial.println("\n\n/**************************************************************************/");
    Serial.println("/*               CYD HARDWARE CONFIGURATION BLOCK (GENERATED)             */");
    Serial.println("/**************************************************************************/");
    Serial.println("#ifndef CYD_CONFIG_H");
    Serial.println("#define CYD_CONFIG_H");
    Serial.println("");
    Serial.println("// --- Display Driver ---");
    Serial.printf("#define %s_DRIVER\n", driverType.c_str());
    Serial.printf("#define DISPLAY_INVERT %s\n", colorInvertNeeded ? "true" : "false");
    Serial.println("");
    Serial.println("// --- Touch Screen Calibration ---");
    Serial.printf("#define TOUCH_MIN_X %d\n", touchMinX);
    Serial.printf("#define TOUCH_MAX_X %d\n", touchMaxX);
    Serial.printf("#define TOUCH_MIN_Y %d\n", touchMinY);
    Serial.printf("#define TOUCH_MAX_Y %d\n", touchMaxY);
    Serial.println("");
    Serial.println("// --- Pin Configuration ---");
    Serial.println("#define TFT_MISO 12");
    Serial.println("#define TFT_MOSI 13");
    Serial.println("#define TFT_SCLK 14");
    Serial.println("#define TFT_CS   15");
    Serial.println("#define TFT_DC    2");
    Serial.println("#define TFT_RST  -1");
    Serial.println("#define TFT_BL   21");
    Serial.println("");
    Serial.println("#define TOUCH_CS  33");
    Serial.println("#define TOUCH_IRQ 36");
    Serial.println("#define TOUCH_MOSI 32");
    Serial.println("#define TOUCH_MISO 39");
    Serial.println("#define TOUCH_CLK  25");
    Serial.println("");
    Serial.println("#define SD_CS     5");
    Serial.println("");
    Serial.println("// --- System Info ---");
    Serial.printf("// Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("// Revision: %d\n", ESP.getChipRevision());
    Serial.printf("// Core Count: %d\n", ESP.getChipCores());
    Serial.printf("// Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("// Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("// Generated: %s\n", __DATE__);
    Serial.println("");
    Serial.println("#endif // CYD_CONFIG_H");
    Serial.println("/**************************************************************************/");
    Serial.println("");
    Serial.println("// --- platformio.ini build_flags (copy this section) ---");
    Serial.println("// build_flags = ");
    Serial.println("//     -DUSER_SETUP_LOADED=1");
    Serial.printf("//     -D%s_DRIVER=1\n", driverType.c_str());
    Serial.println("//     -DTFT_WIDTH=240");
    Serial.println("//     -DTFT_HEIGHT=320");
    Serial.println("//     -DTFT_MOSI=13");
    Serial.println("//     -DTFT_SCLK=14");
    Serial.println("//     -DTFT_CS=15");
    Serial.println("//     -DTFT_DC=2");
    Serial.println("//     -DTFT_RST=-1");
    Serial.println("//     -DTFT_BL=21");
    Serial.println("//     -DTOUCH_CS=33");
    if (driverType == "ST7789") {
        Serial.println("//     -DTFT_INVERSION_ON");
        Serial.println("//     -DSPI_FREQUENCY=80000000");
    } else {
        Serial.println("//     -DSPI_FREQUENCY=40000000");
    }
    Serial.println("//     -DUSE_HSPI_PORT");
    Serial.println("/**************************************************************************/");
}

void waitForTouch()
{
    while (!touch.touched())
    {
        delay(10);
    }
    while (touch.touched())
    {
        delay(10);
    }
}

// Wait for touch with timeout, returns true if touched
bool waitForTouchTimeout(int timeoutMs)
{
    unsigned long start = millis();
    while (!touch.touched())
    {
        if (millis() - start > timeoutMs) return false;
        delay(10);
    }
    while (touch.touched())
    {
        delay(10);
    }
    return true;
}

// ============================================================================
// COLOR INVERSION TEST
// Shows RAW colors vs XOR-inverted colors side by side
// User taps the side that looks correct
// ============================================================================
void testColorInversion()
{
    Serial.println("\n=== COLOR INVERSION TEST ===");
    Serial.println("This test determines the correct invertDisplay() setting.");

    // First, set invertDisplay to FALSE so we see "raw" behavior
    tft.invertDisplay(false);
    tft.fillScreen(TFT_BLACK);

    // Draw header
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("TAP THE SIDE WITH", 120, 5);
    tft.drawString("CORRECT COLORS", 120, 20);

    // Left side: RAW colors (as-is)
    int colWidth = 120;
    int rowHeight = 50;
    int startY = 45;

    // Column headers
    tft.setTextDatum(TC_DATUM);
    tft.drawString("1. RAW", 60, 35);
    tft.drawString("2. XOR", 180, 35);

    // Draw divider
    tft.drawLine(120, startY, 120, 290, TFT_WHITE);

    // RAW colors (left side) - what TFT_RED etc. actually produce
    tft.fillRect(0, startY, colWidth, rowHeight, TFT_RED);
    tft.fillRect(0, startY + rowHeight, colWidth, rowHeight, TFT_GREEN);
    tft.fillRect(0, startY + rowHeight * 2, colWidth, rowHeight, TFT_BLUE);
    tft.fillRect(0, startY + rowHeight * 3, colWidth, rowHeight, TFT_WHITE);
    tft.fillRect(0, startY + rowHeight * 4, colWidth, rowHeight, TFT_BLACK);

    // Labels for left side
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("RED", 60, startY + 20);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawString("GREEN", 60, startY + rowHeight + 20);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString("BLUE", 60, startY + rowHeight * 2 + 20);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("WHITE", 60, startY + rowHeight * 3 + 20);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("BLACK", 60, startY + rowHeight * 4 + 20);

    // XOR colors (right side) - colors XORed with 0xFFFF
    tft.fillRect(colWidth, startY, colWidth, rowHeight, TFT_RED ^ 0xFFFF);
    tft.fillRect(colWidth, startY + rowHeight, colWidth, rowHeight, TFT_GREEN ^ 0xFFFF);
    tft.fillRect(colWidth, startY + rowHeight * 2, colWidth, rowHeight, TFT_BLUE ^ 0xFFFF);
    tft.fillRect(colWidth, startY + rowHeight * 3, colWidth, rowHeight, TFT_WHITE ^ 0xFFFF);
    tft.fillRect(colWidth, startY + rowHeight * 4, colWidth, rowHeight, TFT_BLACK ^ 0xFFFF);

    // Labels for right side
    tft.setTextColor((TFT_WHITE ^ 0xFFFF), (TFT_RED ^ 0xFFFF));
    tft.drawString("RED", 180, startY + 20);
    tft.setTextColor((TFT_BLACK ^ 0xFFFF), (TFT_GREEN ^ 0xFFFF));
    tft.drawString("GREEN", 180, startY + rowHeight + 20);
    tft.setTextColor((TFT_WHITE ^ 0xFFFF), (TFT_BLUE ^ 0xFFFF));
    tft.drawString("BLUE", 180, startY + rowHeight * 2 + 20);
    tft.setTextColor((TFT_BLACK ^ 0xFFFF), (TFT_WHITE ^ 0xFFFF));
    tft.drawString("WHITE", 180, startY + rowHeight * 3 + 20);
    tft.setTextColor((TFT_WHITE ^ 0xFFFF), (TFT_BLACK ^ 0xFFFF));
    tft.drawString("BLACK", 180, startY + rowHeight * 4 + 20);

    // Footer instructions
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Tap LEFT or RIGHT", 120, 310);

    Serial.println("Waiting for user to tap correct color side...");

    // Wait for touch and determine which side
    while (true)
    {
        if (touch.touched())
        {
            TS_Point p = touch.getPoint();
            Serial.printf("Touch at raw X=%d\n", p.x);

            // Map touch to screen (roughly - we just need left vs right)
            // For most CYDs, higher X = right side, but we'll use screen center
            int mappedX = map(p.x, 200, 3800, 0, 240);  // Rough mapping

            while (touch.touched()) delay(10);  // Wait for release

            if (mappedX < 120)
            {
                // User tapped LEFT (RAW colors correct)
                // This means invertDisplay(false) shows correct colors
                colorInvertNeeded = false;
                Serial.println("Result: RAW colors correct -> invertDisplay(false)");
                break;
            }
            else
            {
                // User tapped RIGHT (XOR colors correct)
                // This means we need invertDisplay(true) to get correct colors
                colorInvertNeeded = true;
                Serial.println("Result: XOR colors correct -> invertDisplay(true)");
                break;
            }
        }
        delay(10);
    }

    // Apply the result and show confirmation
    tft.invertDisplay(colorInvertNeeded);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Color Test Complete!", 120, 140);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(colorInvertNeeded ? "invertDisplay(true)" : "invertDisplay(false)", 120, 170);
    delay(2000);
}

// ============================================================================
// DRIVER DETECTION WIZARD
// Guides user to identify their CYD variant
// ============================================================================
void detectDriver()
{
    Serial.println("\n=== DRIVER DETECTION ===");

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("DRIVER DETECTION", 120, 10);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(10, 40);
    tft.println("How many USB ports does");
    tft.println("your board have?");
    tft.println("");
    tft.println("Look at the board edge.");

    // Draw two buttons
    int btnY = 150;
    int btnH = 60;

    // Left button - Single USB
    tft.fillRoundRect(10, btnY, 100, btnH, 8, TFT_BLUE);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString("1 USB", 60, btnY + 20);
    tft.drawString("(Micro)", 60, btnY + 40);

    // Right button - Dual USB
    tft.fillRoundRect(130, btnY, 100, btnH, 8, TFT_PURPLE);
    tft.setTextColor(TFT_WHITE, TFT_PURPLE);
    tft.drawString("2 USB", 180, btnY + 20);
    tft.drawString("(USB-C+Micro)", 180, btnY + 40);

    // Info text
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Single = ILI9341", 120, 230);
    tft.drawString("Dual = ST7789", 120, 250);

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Tap your answer", 120, 300);

    Serial.println("Waiting for user to select USB count...");

    // Wait for touch
    while (true)
    {
        if (touch.touched())
        {
            TS_Point p = touch.getPoint();
            int mappedX = map(p.x, 200, 3800, 0, 240);

            while (touch.touched()) delay(10);

            if (mappedX < 120)
            {
                driverType = "ILI9341";
                Serial.println("Result: Single USB -> ILI9341 driver");
            }
            else
            {
                driverType = "ST7789";
                Serial.println("Result: Dual USB -> ST7789 driver");
            }
            break;
        }
        delay(10);
    }

    // Confirmation
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Driver Detected!", 120, 140);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(driverType, 120, 170);
    delay(1500);
}

// ============================================================================
// RGB LED TEST
// ============================================================================
void testRGBLED()
{
    Serial.println("\n=== RGB LED TEST ===");

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("RGB LED Test", 120, 140);
    tft.drawString("Watch the LED!", 120, 170);

    // Setup LED pins (active LOW)
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);

    // All off first
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    delay(500);

    // Red
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("RED", 120, 160);
    digitalWrite(LED_RED, LOW);
    delay(800);
    digitalWrite(LED_RED, HIGH);

    // Green
    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.drawString("GREEN", 120, 160);
    digitalWrite(LED_GREEN, LOW);
    delay(800);
    digitalWrite(LED_GREEN, HIGH);

    // Blue
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString("BLUE", 120, 160);
    digitalWrite(LED_BLUE, LOW);
    delay(800);
    digitalWrite(LED_BLUE, HIGH);

    // White (all on)
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("WHITE (ALL)", 120, 160);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    delay(800);

    // All off
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);

    Serial.println("RGB LED test complete");
}

// ============================================================================
// MEMORY TEST
// ============================================================================
void testMemory()
{
    Serial.println("\n=== MEMORY TEST ===");

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("MEMORY INFO", 10, 10);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 40);

    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();

    tft.printf("Total Heap: %d KB\n", totalHeap / 1024);
    tft.printf("Free Heap: %d KB\n", freeHeap / 1024);
    tft.printf("Min Free: %d KB\n", minFreeHeap / 1024);
    tft.println("");

    // Check for PSRAM
    uint32_t psramSize = ESP.getPsramSize();
    if (psramSize > 0)
    {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.printf("PSRAM: %d MB\n", psramSize / (1024 * 1024));
        Serial.printf("PSRAM detected: %d bytes\n", psramSize);
    }
    else
    {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.println("PSRAM: Not detected");
        Serial.println("No PSRAM detected");
    }

    // Sprite size recommendation
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println("");
    int maxSpriteKB = (freeHeap * 0.4) / 1024;  // 40% of free heap
    int maxSpriteSize = sqrt(maxSpriteKB * 1024 / 2);  // 16-bit color
    tft.printf("Max Sprite: ~%dx%d px\n", maxSpriteSize, maxSpriteSize);
    tft.printf("(%d KB @ 16-bit)\n", maxSpriteKB);

    Serial.printf("Free heap: %d, Max sprite: %dx%d\n", freeHeap, maxSpriteSize, maxSpriteSize);

    delay(3000);
}

TS_Point getTouchPoint()
{
    while (!touch.touched())
    {
        delay(10);
    }
    TS_Point p = touch.getPoint();
    while (touch.touched())
    {
        delay(10);
    }
    return p;
}

void calibrateTouch()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Touch Calibration", 120, 160);
    delay(1000);

    // Top Left
    tft.fillScreen(TFT_BLACK);
    tft.fillCircle(10, 10, 5, TFT_RED);
    tft.drawCircle(10, 10, 8, TFT_WHITE);
    tft.drawString("TOUCH HERE", 60, 20);
    TS_Point p1 = getTouchPoint();

    // Bottom Right
    tft.fillScreen(TFT_BLACK);
    tft.fillCircle(230, 310, 5, TFT_RED);
    tft.drawCircle(230, 310, 8, TFT_WHITE);
    tft.drawString("TOUCH HERE", 180, 300);
    TS_Point p2 = getTouchPoint();

    touchMinX = p1.x;
    touchMinY = p1.y;
    touchMaxX = p2.x;
    touchMaxY = p2.y;

    tft.fillScreen(TFT_BLACK);
    tft.drawString("Calibration Complete!", 120, 160);
    delay(1000);
}

void testDisplay()
{
    tft.fillScreen(TFT_RED);
    delay(500);
    tft.fillScreen(TFT_GREEN);
    delay(500);
    tft.fillScreen(TFT_BLUE);
    delay(500);
    tft.fillScreen(TFT_WHITE);
    delay(500);

    // Grid
    tft.fillScreen(TFT_BLACK);
    for (int i = 0; i < 320; i += 20)
        tft.drawLine(0, i, 240, i, TFT_DARKGREY);
    for (int i = 0; i < 240; i += 20)
        tft.drawLine(i, 0, i, 320, TFT_DARKGREY);
    tft.drawRect(0, 0, 240, 320, TFT_RED);
    tft.drawLine(0, 0, 240, 320, TFT_GREEN);
    tft.drawLine(240, 0, 0, 320, TFT_GREEN);

    tft.drawString("Display Test OK", 120, 160);
    delay(1000);
}

void testWiFi()
{
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Scanning WiFi...", 120, 160);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    int n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("WiFi Scan Results:");
    if (n == 0)
    {
        tft.println("No networks found");
    }
    else
    {
        tft.printf("Found %d networks\n", n);
        for (int i = 0; i < n && i < 15; ++i)
        {
            tft.printf("%d: %s (%d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        }
    }
    delay(2000);
}

void testSD()
{
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Testing SD Card...", 120, 160);
    if (!SD.begin(SD_CS))
    {
        tft.drawString("SD Card Mount Failed!", 120, 180);
    }
    else
    {
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        tft.fillScreen(TFT_BLACK);
        tft.drawString("SD Card OK!", 120, 140);
        tft.setCursor(60, 160);
        tft.printf("Size: %lluMB", cardSize);
    }
    delay(2000);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("   ESP32 CYD Hardware Tester v2.0");
    Serial.println("========================================");
    Serial.println("Starting initialization...\n");

    // Init Touch SPI FIRST (before display - prevents ghosting)
    Serial.println("Initializing touch...");
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(0);

    // --- DISPLAY INIT (Robust) ---
    Serial.println("Initializing display...");

    // Turn on backlight - try BOTH common CYD pins
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);

    tft.init();

    // Portrait mode with USB at bottom
    tft.setRotation(0);
    tft.invertDisplay(true); // Start with inversion ON (most common)

    // Aggressive clear in ALL rotations to remove ghost images
    for (int r = 0; r < 4; r++)
    {
        tft.setRotation(r);
        tft.fillScreen(TFT_BLACK);
    }

    // Set final rotation
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    Serial.println("Display initialized!\n");

    // ========================================
    // WELCOME SCREEN
    // ========================================
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("ESP32 CYD", 120, 100);
    tft.drawString("Hardware Tester", 120, 120);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("v2.0", 120, 145);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Tap to Start", 120, 200);

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("See docs/ for guides", 120, 280);

    waitForTouch();

    // ========================================
    // TEST SEQUENCE
    // ========================================

    // 1. Driver Detection (asks about USB count)
    detectDriver();

    // 2. Color Inversion Test (critical - determines invertDisplay setting)
    testColorInversion();

    // 3. Basic Display Test (colors and patterns)
    testDisplay();

    // 4. RGB LED Test
    testRGBLED();

    // 5. Touch Calibration
    #ifdef TOUCH_MIN_X
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Calibration Found!", 120, 150);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("Using saved values", 120, 175);
        delay(1500);
    #else
        calibrateTouch();
    #endif

    // 6. Memory Test
    testMemory();

    // 7. WiFi Scan
    testWiFi();

    // 8. SD Card Test
    testSD();

    // ========================================
    // FINAL REPORT
    // ========================================
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TC_DATUM);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("ALL TESTS COMPLETE!", 120, 40);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(10, 80);
    tft.println("Results:");
    tft.printf("  Driver: %s\n", driverType.c_str());
    tft.printf("  Invert: %s\n", colorInvertNeeded ? "true" : "false");
    tft.printf("  Touch Cal: %s\n", touchMinX > 0 ? "OK" : "Needed");

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println("");
    tft.println("Check Serial Monitor");
    tft.println("for full config block!");

    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("Touch screen to test", 120, 300);

    // Print configuration to serial
    printConfig();

    Serial.println("\n========================================");
    Serial.println("   Tests complete! Touch to verify.");
    Serial.println("========================================\n");
}

void loop()
{
    static bool screenCleared = false;
    static unsigned long lastTouch = 0;

    // Clear screen once for touch test mode
    if (!screenCleared)
    {
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("TOUCH TEST MODE", 120, 5);

        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("Draw on screen", 120, 25);

        // Draw corner markers for calibration verification
        tft.drawRect(0, 0, 20, 20, TFT_RED);
        tft.drawRect(220, 0, 20, 20, TFT_GREEN);
        tft.drawRect(0, 300, 20, 20, TFT_BLUE);
        tft.drawRect(220, 300, 20, 20, TFT_YELLOW);

        screenCleared = true;
    }

    // Touch handling with visual feedback
    if (touch.touched())
    {
        TS_Point p = touch.getPoint();

        // Map touch to screen coordinates using calibration
        int16_t screenX, screenY;

        if (touchMinX != 0 && touchMaxX != 0)
        {
            // Use calibration data
            screenX = map(p.x, touchMinX, touchMaxX, 0, 240);
            screenY = map(p.y, touchMinY, touchMaxY, 0, 320);
        }
        else
        {
            // Default mapping for uncalibrated
            screenX = map(p.x, 200, 3800, 0, 240);
            screenY = map(p.y, 200, 3800, 0, 320);
        }

        // Constrain to screen
        screenX = constrain(screenX, 0, 239);
        screenY = constrain(screenY, 40, 319);  // Leave header area

        // Draw at touch position
        tft.fillCircle(screenX, screenY, 4, TFT_GREEN);

        // Show raw values in serial (throttled)
        if (millis() - lastTouch > 100)
        {
            Serial.printf("Raw: X=%d Y=%d Z=%d -> Screen: %d,%d\n",
                          p.x, p.y, p.z, screenX, screenY);
            lastTouch = millis();
        }
    }

    delay(10);
}
