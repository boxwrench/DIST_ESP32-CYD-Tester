/*
 * Color Diagnostic Firmware for ESP32 CYD (Overhauled)
 * 
 * Fixes:
 * 1. Persistent Gamma Status display
 * 2. Proper layout for 320x240 (Rotation 3)
 * 3. Graceful handling of BG memory limits
 * 4. Faster cycling for comparison
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

TFT_eSPI tft = TFT_eSPI();

#define SD_CS 5
#define BLUEGILL_WIDTH 48
#define BLUEGILL_HEIGHT 32
#define BACKGROUND_WIDTH 240
#define BACKGROUND_HEIGHT 240

uint16_t *spriteBuffer = nullptr;
size_t bufferSize = BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2;

// Gamma State
uint8_t gammaCurves[] = {0x01, 0x02, 0x04, 0x08};
int currentGammaIdx = 0;

// ============================================================================
// UTILITIES
// ============================================================================

void displayText(const char *text, int x, int y, uint16_t color = TFT_WHITE, int size = 1) {
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}

void setGamma(uint8_t curve) {
    tft.writecommand(0x26); // Gamma Set
    tft.writedata(curve);
    Serial.print("Gamma Curve set to: 0x");
    Serial.println(curve, HEX);
}

void showStatusOverlay() {
    char buf[64];
    sprintf(buf, "GAMMA: 0x%02X | RAM: %d KB", gammaCurves[currentGammaIdx], ESP.getFreeHeap() / 1024);
    
    // Draw background for status to ensure readability
    tft.fillRect(0, 225, 320, 15, TFT_NAVY);
    displayText(buf, 10, 228, TFT_YELLOW, 1);
}

bool loadRGB565(const char *path, size_t size) {
    File file = SD.open(path);
    if (!file) {
        Serial.print("Failed to open: ");
        Serial.println(path);
        return false;
    }
    
    if (file.size() != size) {
        Serial.print("Size mismatch for ");
        Serial.print(path);
        file.close();
        return false;
    }
    
    file.read((uint8_t*)spriteBuffer, size);
    file.close();
    Serial.print("Loaded: ");
    Serial.println(path);
    return true;
}

// ============================================================================
// TESTS
// ============================================================================

void showReferenceColors() {
    tft.fillScreen(TFT_BLACK);
    displayText("1. REFERENCE COLORS", 10, 10, TFT_CYAN, 2);
    
    // R, G, B, Y Blocks
    int blockW = 60;
    int blockH = 60;
    int startY = 50;

    tft.fillRect(10, startY, blockW, blockH, TFT_RED);
    displayText("RED", 10, startY + blockH + 5, TFT_RED, 1);
    
    tft.fillRect(80, startY, blockW, blockH, TFT_GREEN);
    displayText("GREEN", 80, startY + blockH + 5, TFT_GREEN, 1);
    
    tft.fillRect(150, startY, blockW, blockH, TFT_BLUE);
    displayText("BLUE", 150, startY + blockH + 5, TFT_BLUE, 1);
    
    tft.fillRect(220, startY, blockW, blockH, TFT_YELLOW);
    displayText("YELLOW", 220, startY + blockH + 5, TFT_YELLOW, 1);
    
    displayText("Goal: Deep Red, Vibrant Green, Sharp Blue", 10, 150, TFT_WHITE, 1);
    displayText("Washed out? Check Gamma or TFT_RGB_ORDER", 10, 165, TFT_ORANGE, 1);

    showStatusOverlay();
    delay(4000); 
}

void testFile(const char *filename, const char *label, int y_pos, bool swap) {
    bufferSize = BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2;
    bool loaded = loadRGB565(filename, bufferSize);
    
    tft.setSwapBytes(swap);
    
    char buf[64];
    sprintf(buf, "%s (swap=%s)", label, swap ? "ON" : "OFF");
    displayText(buf, 10, y_pos, TFT_WHITE, 1);
    
    if (loaded) {
        // Draw copies across the row
        for(int i=0; i<5; i++) {
            tft.pushImage(10 + (60 * i), y_pos + 12, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, spriteBuffer);
        }
    } else {
        displayText("FILE NOT FOUND", 100, y_pos, TFT_RED, 1);
    }
}

void testBackground(const char *filename, const char *label, bool swap) {
    tft.fillScreen(TFT_BLACK);
    displayText(label, 10, 10, TFT_WHITE, 2);
    
    size_t bgSize = BACKGROUND_WIDTH * BACKGROUND_HEIGHT * 2;
    
    // Attempt realloc
    void* newBuf = realloc(spriteBuffer, bgSize);
    if (!newBuf) {
        displayText("BG MEMORY FAIL (SKIP)", 40, 100, TFT_RED, 2);
        delay(1500);
        return;
    }
    spriteBuffer = (uint16_t*)newBuf;

    if (loadRGB565(filename, bgSize)) {
        tft.setSwapBytes(swap);
        tft.pushImage(40, 40, BACKGROUND_WIDTH, BACKGROUND_HEIGHT, spriteBuffer);
        
        char buf[32];
        sprintf(buf, "Swap: %s", swap ? "TRUE" : "FALSE");
        displayText(buf, 10, 25, TFT_YELLOW, 1);
    } else {
        displayText("FILE MISSING", 50, 120, TFT_RED, 2);
    }
    
    showStatusOverlay();
    delay(4000);
    
    // Restore small buffer to be safe
    spriteBuffer = (uint16_t*)realloc(spriteBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
}

void runSpriteSequence() {
    tft.fillScreen(TFT_BLACK);
    displayText("2. SPRITE COMPARISON", 10, 10, TFT_CYAN, 2);
    
    // Adjusted Y positions to fit in 240 height
    testFile("/sprite_tests/fish_bluegill_EXTERNAL.rgb565", "EXT", 40, true);
    testFile("/sprite_tests/fish_bluegill_EXTERNAL.rgb565", "EXT", 75, false);
    testFile("/sprite_tests/fish_bluegill_CURRENT.rgb565", "CUR", 110, true);
    testFile("/sprite_tests/fish_bluegill_CURRENT.rgb565", "CUR", 145, false);
    testFile("/sprite_tests/fish_bluegill_VERIFIED.rgb565", "VER", 185, true);
    
    showStatusOverlay();
    delay(5000);
    
    // Only one background test per cycle to save time/memory
    testBackground("/sprite_tests/background_EXTERNAL.rgb565", "BG TEST", true);
}

// ============================================================================
// MAIN
// ============================================================================

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n===== Color Diagnostic Firmware Overhaul =====");
    
    // Backlight
    pinMode(21, OUTPUT); digitalWrite(21, HIGH);
    pinMode(27, OUTPUT); digitalWrite(27, HIGH);
    
    // Display
    tft.init();
    tft.setRotation(1); // 320x240 landscape (USB port down)
    tft.invertDisplay(true); // Required for correct colors on this board
    tft.fillScreen(TFT_BLACK);
    
    // SD Card
    if (!SD.begin(SD_CS)) {
        displayText("SD FAILED!", 10, 100, TFT_RED, 3);
        while(1);
    }
    
    // Initial Buffer
    spriteBuffer = (uint16_t*)malloc(BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    
    displayText("Starting sequence...", 10, 100, TFT_GREEN, 2);
    delay(1000);
}

void loop() {
    // Apply Gamma
    setGamma(gammaCurves[currentGammaIdx]);
    
    showReferenceColors();
    runSpriteSequence();

    // Next Gamma
    currentGammaIdx = (currentGammaIdx + 1) % 4;
}
