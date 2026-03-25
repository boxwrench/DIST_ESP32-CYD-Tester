/*
 * Enhanced Sprite Test Firmware for ESP32 CYD
 *
 * Tests PNG vs RGB565 format comparison and performance
 * Implements all tests from ENHANCED_SPRITE_TEST_PLAN.md
 *
 * Hardware: ESP32-2432S028R (CYD)
 * Display: ILI9341 or ST7789
 * SD Card: Required (FAT32, /sprite_tests/ folder)
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <PNGdec.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

TFT_eSPI tft = TFT_eSPI();
PNG png;

// SD Card pins (CYD standard)
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// Touch pins (for future interactive tests)
#define TOUCH_CS 33

// Screen dimensions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Test sprite dimensions (actual from assets)
#define BLUEGILL_WIDTH 48
#define BLUEGILL_HEIGHT 32
#define CLANKER_WIDTH 40
#define CLANKER_HEIGHT 32
#define BACKGROUND_WIDTH 240
#define BACKGROUND_HEIGHT 240

// ============================================================================
// GLOBAL BUFFERS AND STATE
// ============================================================================

// Sprite buffers
uint16_t *bluegillBuffer = nullptr;
uint16_t *clankerBuffer = nullptr;
uint16_t *backgroundBuffer = nullptr;

// Test state
int currentTest = 0;
bool testsComplete = false;

// Performance tracking
struct TestResult
{
    const char *name;
    float value;
    const char *unit;
};

TestResult results[20];
int resultCount = 0;

// Sprite positions for animation tests
struct Sprite
{
    int16_t x, y;
    int16_t dx, dy;
    bool active;
};

Sprite sprites[25];

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void addResult(const char *name, float value, const char *unit)
{
    if (resultCount < 20)
    {
        results[resultCount].name = name;
        results[resultCount].value = value;
        results[resultCount].unit = unit;
        resultCount++;
    }
}

void displayText(const char *text, int x, int y, uint16_t color = TFT_WHITE, int size = 2)
{
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}

void waitForTouch()
{
    displayText("Tap screen to continue", 10, 280, TFT_YELLOW, 1);
    delay(2000); // Auto-continue after 2 seconds for now
}

void clearScreen()
{
    tft.fillScreen(TFT_BLACK);
}

// ============================================================================
// SD CARD FILE LOADING
// ============================================================================

bool loadRGB565FromSD(const char *filepath, uint16_t *buffer, size_t expectedSize)
{
    File file = SD.open(filepath);
    if (!file)
    {
        Serial.print("Failed to open: ");
        Serial.println(filepath);
        return false;
    }

    size_t fileSize = file.size();
    if (fileSize != expectedSize)
    {
        Serial.print("Size mismatch: ");
        Serial.print(fileSize);
        Serial.print(" vs ");
        Serial.println(expectedSize);
        file.close();
        return false;
    }

    size_t bytesRead = file.read((uint8_t *)buffer, expectedSize);
    file.close();

    Serial.print("Loaded ");
    Serial.print(bytesRead);
    Serial.print(" bytes from ");
    Serial.println(filepath);

    return bytesRead == expectedSize;
}

// File handle for PNG decoder
File *pngFile = nullptr;

// PNG file I/O callbacks
void *myOpen(const char *filename, int32_t *size)
{
    pngFile = new File(SD.open(filename));
    if (pngFile && *pngFile)
    {
        *size = pngFile->size();
        return pngFile;
    }
    return nullptr;
}

void myClose(void *handle)
{
    if (pngFile)
    {
        pngFile->close();
        delete pngFile;
        pngFile = nullptr;
    }
}

int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length)
{
    if (!pngFile)
        return 0;
    return pngFile->read(buffer, length);
}

int32_t mySeek(PNGFILE *handle, int32_t position)
{
    if (!pngFile)
        return 0;
    return pngFile->seek(position);
}

// PNG decoder callback
int pngDraw(PNGDRAW *pDraw)
{
    uint16_t *buffer = (uint16_t *)pDraw->pUser;
    uint16_t *row = buffer + (pDraw->y * pDraw->iWidth);

    // PNGdec can provide pixels in various formats
    // We'll handle the most common ones and composite onto BLACK
    for (int x = 0; x < pDraw->iWidth; x++)
    {
        uint8_t r = 0, g = 0, b = 0, a = 255;
        
        if (pDraw->iBpp == 32) { // RGBA
            r = pDraw->pPixels[x * 4];
            g = pDraw->pPixels[x * 4 + 1];
            b = pDraw->pPixels[x * 4 + 2];
            a = pDraw->pPixels[x * 4 + 3];
        } 
        else if (pDraw->iBpp == 24) { // RGB
            r = pDraw->pPixels[x * 3];
            g = pDraw->pPixels[x * 3 + 1];
            b = pDraw->pPixels[x * 3 + 2];
        } 
        else if (pDraw->iBpp == 8) { // Indexed
            // For indexed, we'd need to look up the palette, but simpler 
            // to just use the first 3 bytes if the library pre-expands it
            // or use the library's built-in helper.
            // For our fish assets (which are RGBA), iBpp will be 32.
            r = pDraw->pPixels[x * 3];
            g = pDraw->pPixels[x * 3 + 1];
            b = pDraw->pPixels[x * 3 + 2];
        }

        // Alpha blending against BLACK background
        // brightness = value * (alpha / 255)
        if (a < 255) {
            r = (uint16_t)r * a / 255;
            g = (uint16_t)g * a / 255;
            b = (uint16_t)b * a / 255;
        }

        // Convert to BGR565 (Hardware native order for CYD)
        uint16_t r5 = (r >> 3) & 0x1F;
        uint16_t g6 = (g >> 2) & 0x3F;
        uint16_t b5 = (b >> 3) & 0x1F;
        
        // BGR565 packing: Bbbbb Gggggg Rrrrr
        row[x] = (b5 << 11) | (g6 << 5) | r5;
    }
    return 1; // Success
}

bool loadPNGFromSD(const char *filepath, uint16_t *buffer, int width, int height)
{
    int rc = png.open(filepath, myOpen, myClose, myRead, mySeek, pngDraw);
    
    if (rc != PNG_SUCCESS)
    {
        Serial.print("PNG open failed: ");
        Serial.println(rc);
        return false;
    }

    rc = png.decode(buffer, 0);
    png.close();

    Serial.print("Loaded PNG: ");
    Serial.println(filepath);

    return rc == PNG_SUCCESS;
}

// ============================================================================
// PART A: BASELINE TESTS
// ============================================================================

void testA1_RGBOrder()
{
    clearScreen();
    displayText("A1: RGB Order Test", 10, 10, TFT_CYAN);

    // Draw pure color bars
    tft.fillRect(10, 50, 60, 60, 0xF800);  // RED
    tft.fillRect(80, 50, 60, 60, 0x07E0);  // GREEN
    tft.fillRect(150, 50, 60, 60, 0x001F); // BLUE

    displayText("R", 35, 120, TFT_WHITE, 2);
    displayText("G", 105, 120, TFT_WHITE, 2);
    displayText("B", 175, 120, TFT_WHITE, 2);

    displayText("Labels match colors?", 10, 150, TFT_WHITE, 1);
    displayText("If not: Toggle TFT_RGB_ORDER", 10, 170, TFT_YELLOW, 1);

    waitForTouch();
}

void testA2_Inversion()
{
    clearScreen();
    displayText("A2: Inversion Test", 10, 10, TFT_CYAN);

    // Draw gradient
    for (int i = 0; i < 200; i++)
    {
        uint8_t gray = map(i, 0, 200, 0, 255);
        uint16_t color = tft.color565(gray, gray, gray);
        tft.drawFastVLine(20 + i, 50, 60, color);
    }

    displayText("BLACK", 20, 120, TFT_WHITE, 2);
    displayText("WHITE", 180, 120, TFT_WHITE, 2);

    displayText("Gradient: Black->White?", 10, 150, TFT_WHITE, 1);
    displayText("If not: Toggle INVERSION_ON", 10, 170, TFT_YELLOW, 1);

    waitForTouch();
}

void testA3_ByteSwap()
{
    clearScreen();
    displayText("A3: Byte Swap Test", 10, 10, TFT_CYAN);

    // Create simple test sprite (10x10, red left half, blue right half)
    const uint16_t testSprite[100] = {
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F,
        0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x001F, 0x001F, 0x001F, 0x001F, 0x001F};

    // Reference with fillRect
    displayText("Reference:", 10, 40, TFT_WHITE, 1);
    tft.fillRect(10, 55, 50, 50, 0xF800); // RED
    tft.fillRect(60, 55, 50, 50, 0x001F); // BLUE

    // Test with swap=false
    displayText("swap=false:", 10, 130, TFT_WHITE, 1);
    tft.setSwapBytes(false);
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            tft.pushImage(10 + x * 10, 145 + y * 10, 10, 10, testSprite);
        }
    }

    // Test with swap=true
    displayText("swap=true:", 10, 220, TFT_WHITE, 1);
    tft.setSwapBytes(true);
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            tft.pushImage(10 + x * 10, 235 + y * 10, 10, 10, testSprite);
        }
    }

    displayText("Which matches reference?", 10, 290, TFT_YELLOW, 1);

    waitForTouch();
}

void testA4_SDvsPNGColors()
{
    clearScreen();
    displayText("A4: SD vs PNG Color Debug", 10, 5, TFT_CYAN);

    // Load bluegill via RGB565 file
    bool rgbLoaded = loadRGB565FromSD("/sprite_tests/fish_bluegill_32x32.rgb565", 
                                       bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    
    // Make a copy of RGB565 data
    uint16_t *rgbCopy = (uint16_t *)malloc(BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    if (rgbCopy && rgbLoaded) {
        memcpy(rgbCopy, bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    }
    
    // Load bluegill via PNG decode
    bool pngLoaded = loadPNGFromSD("/sprite_tests/fish_bluegill_32x32.png", 
                                    bluegillBuffer, BLUEGILL_WIDTH, BLUEGILL_HEIGHT);

    displayText("Reference (fillRect):", 10, 25, TFT_WHITE, 1);
    tft.fillRect(10, 40, 30, 20, TFT_RED);
    tft.fillRect(45, 40, 30, 20, TFT_GREEN);
    tft.fillRect(80, 40, 30, 20, TFT_BLUE);
    tft.fillRect(115, 40, 30, 20, TFT_YELLOW);
    
    // Test RGB565 from SD with swap=true
    displayText("RGB565 swap=true:", 10, 70, TFT_WHITE, 1);
    tft.setSwapBytes(true);
    if (rgbCopy) {
        tft.pushImage(10, 85, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, rgbCopy);
    } else {
        displayText("Load failed!", 10, 85, TFT_RED, 1);
    }
    
    // Test RGB565 from SD with swap=false
    displayText("RGB565 swap=false:", 120, 70, TFT_WHITE, 1);
    tft.setSwapBytes(false);
    if (rgbCopy) {
        tft.pushImage(120, 85, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, rgbCopy);
    }
    
    // Test PNG decoded with swap=true
    displayText("PNG swap=true:", 10, 130, TFT_WHITE, 1);
    tft.setSwapBytes(true);
    if (pngLoaded) {
        tft.pushImage(10, 145, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, bluegillBuffer);
    } else {
        displayText("PNG Load failed!", 10, 145, TFT_RED, 1);
    }
    
    // Test PNG decoded with swap=false
    displayText("PNG swap=false:", 120, 130, TFT_WHITE, 1);
    tft.setSwapBytes(false);
    tft.pushImage(120, 145, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, bluegillBuffer);
    
    // Print first pixel values for debug
    char buf[60];
    if (rgbCopy) {
        sprintf(buf, "RGB565[0]=0x%04X", rgbCopy[0]);
        displayText(buf, 10, 190, TFT_YELLOW, 1);
        Serial.print("RGB565 first pixel: 0x");
        Serial.println(rgbCopy[0], HEX);
    }
    sprintf(buf, "PNG[0]=0x%04X", bluegillBuffer[0]);
    displayText(buf, 10, 205, TFT_YELLOW, 1);
    Serial.print("PNG decoded first pixel: 0x");
    Serial.println(bluegillBuffer[0], HEX);
    
    displayText("Which looks more vibrant?", 10, 230, TFT_GREEN, 1);
    displayText("Check Serial for pixel values", 10, 250, TFT_YELLOW, 1);
    
    // Reset swap to true for subsequent tests
    tft.setSwapBytes(true);
    
    if (rgbCopy) free(rgbCopy);
    
    waitForTouch();
}

void testA5_PureColorPattern()
{
    clearScreen();
    displayText("A5: Pure Color Pattern", 10, 5, TFT_CYAN);
    
    // Allocate buffer for 32x32 test pattern
    uint16_t *patternBuffer = (uint16_t *)malloc(32 * 32 * 2);
    if (!patternBuffer) {
        displayText("Malloc failed!", 10, 40, TFT_RED);
        waitForTouch();
        return;
    }
    
    // Load the 8-color band test pattern
    bool loaded = loadRGB565FromSD("/sprite_tests/test_pattern_32x32.rgb565", 
                                    patternBuffer, 32 * 32 * 2);
    
    if (!loaded) {
        displayText("Pattern not found!", 10, 40, TFT_RED);
        displayText("Copy test_pattern_32x32.rgb565", 10, 60, TFT_YELLOW, 1);
        displayText("to SD /sprite_tests/", 10, 75, TFT_YELLOW, 1);
        free(patternBuffer);
        waitForTouch();
        return;
    }
    
    // Reference colors using fillRect
    displayText("Reference:", 10, 25, TFT_WHITE, 1);
    tft.fillRect(10, 40, 25, 12, TFT_RED);
    tft.fillRect(40, 40, 25, 12, TFT_GREEN);
    tft.fillRect(70, 40, 25, 12, TFT_BLUE);
    tft.fillRect(100, 40, 25, 12, TFT_YELLOW);
    tft.fillRect(10, 55, 25, 12, TFT_CYAN);
    tft.fillRect(40, 55, 25, 12, TFT_MAGENTA);
    tft.fillRect(70, 55, 25, 12, TFT_WHITE);
    tft.fillRect(100, 55, 25, 12, TFT_BLACK);
    
    // Show pattern with swap=true
    displayText("Pattern swap=true:", 10, 80, TFT_WHITE, 1);
    tft.setSwapBytes(true);
    tft.pushImage(10, 95, 32, 32, patternBuffer);
    
    // Show pattern with swap=false
    displayText("Pattern swap=false:", 120, 80, TFT_WHITE, 1);
    tft.setSwapBytes(false);
    tft.pushImage(120, 95, 32, 32, patternBuffer);
    
    // Print first few pixel values
    char buf[60];
    Serial.println("\n=== Test Pattern Pixel Values ===");
    for (int i = 0; i < 8; i++) {
        int y = i * 4;  // Each band is 4 pixels high
        uint16_t pixel = patternBuffer[y * 32];
        sprintf(buf, "Band %d: 0x%04X", i, pixel);
        Serial.println(buf);
    }
    
    displayText("If pattern matches ref:", 10, 145, TFT_GREEN, 1);
    displayText("  -> Byte swap is CORRECT", 10, 160, TFT_GREEN, 1);
    displayText("If colors washed out:", 10, 180, TFT_YELLOW, 1);
    displayText("  -> Check conversion script", 10, 195, TFT_YELLOW, 1);
    
    // Reset
    tft.setSwapBytes(true);
    free(patternBuffer);
    
    waitForTouch();
}

// ============================================================================
// PART B: FORMAT COMPARISON TESTS
// ============================================================================

void testB1_LoadingSpeed()
{
    clearScreen();
    displayText("B1: Loading Speed", 10, 10, TFT_CYAN);

    // Test Bluegill loading
    unsigned long pngStart = micros();
    loadPNGFromSD("/sprite_tests/fish_bluegill_32x32.png", bluegillBuffer, BLUEGILL_WIDTH, BLUEGILL_HEIGHT);
    unsigned long pngLoad = micros() - pngStart;

    unsigned long rgbStart = micros();
    loadRGB565FromSD("/sprite_tests/fish_bluegill_32x32.rgb565", bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    unsigned long rgbLoad = micros() - rgbStart;

    char buf[50];
    sprintf(buf, "PNG: %lu us", pngLoad);
    displayText(buf, 10, 50, TFT_WHITE);

    sprintf(buf, "RGB565: %lu us", rgbLoad);
    displayText(buf, 10, 80, TFT_WHITE);

    displayText(pngLoad < rgbLoad ? "PNG Faster" : "RGB565 Faster", 10, 110, TFT_YELLOW);

    addResult("B1_PNG_Load", pngLoad, "us");
    addResult("B1_RGB_Load", rgbLoad, "us");

    Serial.print("PNG Load: ");
    Serial.print(pngLoad);
    Serial.println(" us");
    Serial.print("RGB565 Load: ");
    Serial.print(rgbLoad);
    Serial.println(" us");

    waitForTouch();
}

void testB2_RenderingSpeed()
{
    clearScreen();
    displayText("B2: Rendering Speed", 10, 10, TFT_CYAN);

    // Load sprite once
    loadRGB565FromSD("/sprite_tests/fish_bluegill_32x32.rgb565", bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);

    // Test rendering speed (10 iterations)
    tft.setSwapBytes(true);
    unsigned long start = micros();
    for (int i = 0; i < 10; i++)
    {
        tft.pushImage(100, 50, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, bluegillBuffer);
    }
    unsigned long renderTime = (micros() - start) / 10;

    char buf[50];
    sprintf(buf, "Avg render: %lu us", renderTime);
    displayText(buf, 10, 50, TFT_WHITE);

    sprintf(buf, "~%.1f FPS max", 1000000.0 / renderTime);
    displayText(buf, 10, 80, TFT_YELLOW);

    addResult("B2_Render_Time", renderTime, "us");

    Serial.print("Render Time: ");
    Serial.print(renderTime);
    Serial.println(" us");

    waitForTouch();
}

void testB3_MemoryUsage()
{
    clearScreen();
    displayText("B3: Memory Usage", 10, 10, TFT_CYAN);

    uint32_t memBefore = ESP.getFreeHeap();

    char buf[50];
    sprintf(buf, "Free: %lu bytes", memBefore);
    displayText(buf, 10, 50, TFT_WHITE);

    // Load background ( large sprite)
    if (backgroundBuffer == nullptr)
    {
        backgroundBuffer = (uint16_t *)malloc(BACKGROUND_WIDTH * BACKGROUND_HEIGHT * 2);
    }
    loadRGB565FromSD("/sprite_tests/background_240x240.rgb565", backgroundBuffer, BACKGROUND_WIDTH * BACKGROUND_HEIGHT * 2);

    uint32_t memAfter = ESP.getFreeHeap();
    uint32_t used = memBefore - memAfter;

    sprintf(buf, "After BG: %lu bytes", memAfter);
    displayText(buf, 10, 80, TFT_WHITE);

    sprintf(buf, "Used: %lu bytes", used);
    displayText(buf, 10, 110, TFT_YELLOW);

    addResult("B3_Memory_Used", used, "bytes");

    Serial.print("Memory Used: ");
    Serial.print(used);
    Serial.println(" bytes");

    waitForTouch();
}

// ============================================================================
// PART C: PERFORMANCE STRESS TESTS
// ============================================================================

void testC1_SpriteFPS()
{
    clearScreen();
    displayText("C1: FPS Stress Test", 10, 10, TFT_CYAN);

    // Load bluegill sprite
    loadRGB565FromSD("/sprite_tests/fish_bluegill_32x32.rgb565", bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);

    int spriteCounts[] = {5, 10, 15, 20, 25};
    tft.setSwapBytes(true);

    for (int countIdx = 0; countIdx < 5; countIdx++)
    {
        int numSprites = spriteCounts[countIdx];

        // Initialize sprite positions
        for (int i = 0; i < numSprites; i++)
        {
            sprites[i].x = random(0, SCREEN_WIDTH - BLUEGILL_WIDTH);
            sprites[i].y = random(0, SCREEN_HEIGHT - BLUEGILL_HEIGHT);
            sprites[i].dx = random(1, 4);
            sprites[i].dy = random(1, 4);
            sprites[i].active = true;
        }

        // Run for 3 seconds
        unsigned long start = millis();
        int frames = 0;

        while (millis() - start < 3000)
        {
            tft.fillScreen(TFT_BLACK);

            // Move and draw sprites
            for (int i = 0; i < numSprites; i++)
            {
                sprites[i].x += sprites[i].dx;
                sprites[i].y += sprites[i].dy;

                // Bounce at edges
                if (sprites[i].x <= 0 || sprites[i].x >= SCREEN_WIDTH - BLUEGILL_WIDTH)
                {
                    sprites[i].dx = -sprites[i].dx;
                }
                if (sprites[i].y <= 0 || sprites[i].y >= SCREEN_HEIGHT - BLUEGILL_HEIGHT)
                {
                    sprites[i].dy = -sprites[i].dy;
                }

                tft.pushImage(sprites[i].x, sprites[i].y, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, bluegillBuffer);
            }

            frames++;
        }

        float fps = frames / 3.0;

        // Display result
        clearScreen();
        displayText("C1: FPS Test", 10, 10, TFT_CYAN);
        char buf[50];
        sprintf(buf, "%d sprites:", numSprites);
        displayText(buf, 10, 50, TFT_WHITE);
        sprintf(buf, "%.1f FPS", fps);
        displayText(buf, 10, 80, TFT_YELLOW, 4);

        char resultName[20];
        sprintf(resultName, "C1_FPS_%d", numSprites);
        addResult(resultName, fps, "FPS");

        Serial.print(numSprites);
        Serial.print(" sprites: ");
        Serial.print(fps);
        Serial.println(" FPS");

        delay(1500);
    }
}

void testC2_BackgroundPlusSprites()
{
    clearScreen();
    displayText("C2: BG + Sprites Test", 10, 10, TFT_CYAN);

    // Load assets
    loadRGB565FromSD("/sprite_tests/background_240x240.rgb565", backgroundBuffer, BACKGROUND_WIDTH * BACKGROUND_HEIGHT * 2);
    loadRGB565FromSD("/sprite_tests/fish_bluegill_32x32.rgb565", bluegillBuffer, BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);

    // Initialize 10 sprites
    for (int i = 0; i < 10; i++)
    {
        sprites[i].x = random(0, BACKGROUND_WIDTH - BLUEGILL_WIDTH);
        sprites[i].y = random(0, BACKGROUND_HEIGHT - BLUEGILL_HEIGHT);
        sprites[i].dx = random(1, 3);
        sprites[i].dy = random(1, 3);
        sprites[i].active = true;
    }

    tft.setSwapBytes(true);
    unsigned long start = millis();
    int frames = 0;

    while (millis() - start < 5000)
    {
        // Draw background
        tft.pushImage(0, 0, BACKGROUND_WIDTH, BACKGROUND_HEIGHT, backgroundBuffer);

        // Move and draw sprites
        for (int i = 0; i < 10; i++)
        {
            sprites[i].x += sprites[i].dx;
            sprites[i].y += sprites[i].dy;

            if (sprites[i].x <= 0 || sprites[i].x >= BACKGROUND_WIDTH - BLUEGILL_WIDTH)
            {
                sprites[i].dx = -sprites[i].dx;
            }
            if (sprites[i].y <= 0 || sprites[i].y >= BACKGROUND_HEIGHT - BLUEGILL_HEIGHT)
            {
                sprites[i].dy = -sprites[i].dy;
            }

            tft.pushImage(sprites[i].x, sprites[i].y, BLUEGILL_WIDTH, BLUEGILL_HEIGHT, bluegillBuffer);
        }

        frames++;
    }

    float fps = frames / 5.0;

    // Display result
    clearScreen();
    displayText("C2: BG + Sprites", 10, 10, TFT_CYAN);
    char buf[50];
    displayText("Background + 10 fish", 10, 50, TFT_WHITE);
    sprintf(buf, "%.1f FPS", fps);
    displayText(buf, 10, 80, TFT_YELLOW, 4);

    addResult("C2_BG_Sprites_FPS", fps, "FPS");

    Serial.print("Background + 10 sprites: ");
    Serial.print(fps);
    Serial.println(" FPS");

    waitForTouch();
}

// ============================================================================
// RESULTS DISPLAY
// ============================================================================

void displayResults()
{
    clearScreen();
    displayText("=== TEST RESULTS ===", 10, 5, TFT_CYAN, 2);

    int y = 30;
    for (int i = 0; i < resultCount && y < 300; i++)
    {
        char buf[80];
        sprintf(buf, "%s: %.1f %s", results[i].name, results[i].value, results[i].unit);
        displayText(buf, 10, y, TFT_WHITE, 1);
        y += 15;
    }

    displayText("See Serial for full output", 10, 305, TFT_YELLOW, 1);

    // Print to serial
    Serial.println("\n===== TEST RESULTS =====");
    for (int i = 0; i < resultCount; i++)
    {
        Serial.print(results[i].name);
        Serial.print(": ");
        Serial.print(results[i].value);
        Serial.print(" ");
        Serial.println(results[i].unit);
    }
    Serial.println("========================\n");
}

// ============================================================================
// SETUP AND MAIN LOOP
// ============================================================================

void setup()
{
    Serial.begin(115200);
    Serial.println("\n\n===== Enhanced Sprite Test Firmware =====");

    // Initialize backlight - try BOTH common CYD pins (matching Bass-Hole)
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    
    // Initialize display
    tft.init();
    
    // Ghost image clear - sweep all rotations (matching Bass-Hole)
    for (int r = 0; r < 4; r++) {
        tft.setRotation(r);
        tft.fillScreen(TFT_BLACK);
    }
    
    // Set final rotation: 3 = Portrait, USB at bottom (verified in Bass-Hole)
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
    // CRITICAL: Enable byte swapping for RGB565 sprites from SD card
    // Without this, colors appear washed out due to endianness mismatch
    tft.setSwapBytes(true);
    
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(TL_DATUM);

    displayText("Initializing...", 10, 10, TFT_CYAN, 2);

    // Initialize SD card
    displayText("SD Card...", 10, 40, TFT_WHITE);
    if (!SD.begin(SD_CS))
    {
        displayText("SD FAILED!", 10, 40, TFT_RED);
        Serial.println("SD Card initialization failed!");
        while (1)
            delay(1000);
    }
    displayText("SD OK", 10, 40, TFT_GREEN);
    Serial.println("SD Card initialized");

    // Allocate sprite buffers (only small ones for now)
    displayText("Allocating buffers...", 10, 70, TFT_WHITE);
    
    uint32_t freeBefore = ESP.getFreeHeap();
    Serial.print("Free heap before: ");
    Serial.println(freeBefore);
    
    bluegillBuffer = (uint16_t *)malloc(BLUEGILL_WIDTH * BLUEGILL_HEIGHT * 2);
    clankerBuffer = (uint16_t *)malloc(CLANKER_WIDTH * CLANKER_HEIGHT * 2);
    // backgroundBuffer allocated on-demand in tests to save memory

    if (!bluegillBuffer || !clankerBuffer)
    {
        displayText("MALLOC FAILED!", 10, 70, TFT_RED);
        Serial.print("Buffer allocation failed! Free heap: ");
        Serial.println(ESP.getFreeHeap());
        while (1)
            delay(1000);
    }
    
    uint32_t freeAfter = ESP.getFreeHeap();
    Serial.print("Free heap after: ");
    Serial.println(freeAfter);
    Serial.print("Used: ");
    Serial.println(freeBefore - freeAfter);
    
    displayText("Buffers OK", 10, 70, TFT_GREEN);

    displayText("Press RESET to start tests", 10, 120, TFT_YELLOW);
    delay(3000);

    Serial.println("Starting tests...\n");
}

void loop()
{
    if (testsComplete)
    {
        delay(1000);
        return;
    }

    switch (currentTest)
    {
    case 0:
        testA1_RGBOrder();
        break;
    case 1:
        testA2_Inversion();
        break;
    case 2:
        testA3_ByteSwap();
        break;
    case 3:
        testA4_SDvsPNGColors();
        break;
    case 4:
        testA5_PureColorPattern();
        break;
    case 5:
        testB1_LoadingSpeed();
        break;
    case 6:
        testB2_RenderingSpeed();
        break;
    case 7:
        testB3_MemoryUsage();
        break;
    case 8:
        testC1_SpriteFPS();
        break;
    case 9:
        testC2_BackgroundPlusSprites();
        break;
    case 10:
        displayResults();
        testsComplete = true;
        break;
    }

    currentTest++;
}
