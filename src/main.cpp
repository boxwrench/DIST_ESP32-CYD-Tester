#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <SD.h>
#include "CYD_2432S028R.h"

// --- Hardware Definitions ---
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SD_CS 5

// --- Globals ---
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Calibration Data
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
    Serial.println("");
    Serial.println("#define SD_CS     5");
    Serial.println("");
    Serial.println("// --- System Info ---");
    Serial.printf("// Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("// Revision: %d\n", ESP.getChipRevision());
    Serial.printf("// Core Count: %d\n", ESP.getChipCores());
    Serial.printf("// Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println("");
    Serial.println("#endif // CYD_CONFIG_H");
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

    // Init Touch SPI
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(0); // Match TFT rotation usually

    // --- DISPLAY INIT (Robusted) ---
    // Turn on backlight - try both common CYD pins
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);

    tft.init();

    // Portrait mode with USB at bottom
    tft.setRotation(0);
    tft.invertDisplay(true); // Required for correct colors

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
    tft.setTextSize(1); // Standard font size

    Serial.println("Starting Hardware Test...");

    // Intro
    tft.setTextDatum(MC_DATUM);
    tft.drawString("CYD Hardware Test", 120, 140);
    tft.drawString("Tap to Start", 120, 180);
    waitForTouch();

    testDisplay();

    // Check if we already have calibration data from config
    #ifdef TOUCH_MIN_X
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Skipping Calibration", 120, 160);
        tft.drawString("(Config Found)", 120, 180);
        delay(1500);
    #else
        calibrateTouch();
    #endif

    testWiFi();
    testSD();

    // Final Report
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Tests Complete", 120, 100);
    tft.drawString("Check Serial Monitor", 120, 130);
    tft.drawString("for Config Block", 120, 145);

    printConfig();
}

void loop()
{
    // Just display touch coordinates for verification
    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        Serial.printf("X: %d, Y: %d, Z: %d\n", p.x, p.y, p.z);
        tft.fillCircle(120, 250, 10, TFT_GREEN);
    }
    else
    {
        tft.fillCircle(120, 250, 10, TFT_BLACK);
    }
    delay(10);
}
