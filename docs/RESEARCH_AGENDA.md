# CYD Development Research Agenda

This document tracks research topics for ESP32 CYD development. Each topic includes a discrete research prompt that can be used to generate detailed documentation.

**Primary Project:** Bass-Hole (fishing game)
**Secondary Use:** General CYD development patterns

---

## Research Status

| # | Topic | Priority | Status | Generated Filename |
|---|-------|----------|--------|-------------------|
| 1 | Sprite Performance | P1 | ✅ Complete | `TFT_eSPI Sprite Performance on ESP32 (ILI9341).md` |
| 2 | Memory Management | P1 | ✅ Complete | `Memory Management for Sprite-Based Games on ESP32.md` |
| 3 | Dirty Rectangle Optimization | P2 | ✅ Complete | `Dirty Rectangle Rendering for TFT Displays.md` |
| 4 | Audio Output | P2 | ✅ Complete | `Audio Output Options for ESP32 CYD Projects.md` |
| 5 | Animation Frame Management | P1 | ✅ Complete | `Sprite Animation Systems for ESP32 Games.md` |
| 6 | Touch Input During Rendering | P2 | ✅ Complete | `Handling Touch Input During Heavy Rendering on ESP32.md` |
| 7 | Game State Persistence | P2 | ✅ Complete | `Game State Persistence on ESP32.md` |
| 8 | Boss Battle Performance | P3 | ✅ Complete | `Performance Optimization for Boss Battles and Bullet-Hell Moments.md` |
| 9 | Text/Dialogue Rendering | P3 | ✅ Complete | `Text and Dialogue Rendering on ESP32 TFT Games.md` |
| 10 | Sensor Integration | P3 | ✅ Complete | `Sensor Integration for ESP32 CYD Projects.md` |
| 11 | API/Cloud Connectivity | P3 | ✅ Complete | `Cloud Connectivity and API Integration for ESP32 CYD Projects.md` |
| 12 | Additional Input Methods | P3 | ✅ Complete | `Additional Input Methods for ESP32 CYD Projects.md` |
| 13 | Output Options | P3 | ✅ Complete | `Output Options for ESP32 CYD Projects Beyond the Display.md` |

**Priority Key:** P1 = Blocking current work, P2 = Next phase, P3 = Future/nice-to-have

> [!NOTE]
> **Research Complete (2026-01-16):** All 13 topics have been researched and documented.
> Generated filenames differ from originally expected names but content matches each topic.
> **Next Step:** Hardware validation required to apply these findings.

---

## Category: Graphics & Rendering

### 1. TFT_eSPI Sprite Performance

**Why it matters:** Bass-Hole uses slow pixel-by-pixel rendering. Need to understand the fastest methods.

**Research Prompt:**
```
TFT_eSPI sprite rendering performance on ESP32 with ILI9341. Compare these methods:
1. drawPixel() in loops (current Bass-Hole approach)
2. pushImage() with transparency color parameter
3. TFT_eSprite with pushSprite()
4. pushImageDMA() if available

For each method, explain:
- How transparency is handled
- Memory requirements
- Approximate performance (pixels/second or FPS for 32x32 sprite)
- Code example
- Limitations and gotchas

Focus on ESP32 without PSRAM (standard CYD boards).
```

**Output file:** `SPRITE_PERFORMANCE.md`

---

### 3. Dirty Rectangle Optimization

**Why it matters:** Bass-Hole uses this but implementation may not be optimal.

**Research Prompt:**
```
Dirty rectangle rendering optimization for TFT displays (ILI9341/ST7789) on ESP32. Explain:

1. The dirty rectangle algorithm
   - Tracking changed regions
   - Merging overlapping rectangles
   - When to use full redraw vs partial

2. Implementation strategies:
   - Per-sprite tracking
   - Grid-based invalidation
   - Bounding box union

3. Background restoration approaches:
   - Store full background in memory (current Bass-Hole approach)
   - Tile-based restoration
   - Layered rendering

4. Performance tradeoffs:
   - Overdraw costs
   - Memory vs CPU tradeoffs
   - When dirty rectangles hurt more than help

5. Code patterns for game loops with dirty rectangles

Focus on practical implementation for fish tank style games with moving sprites over static background.
```

**Output file:** `DIRTY_RECTANGLES.md`

---

### 5. Animation Frame Management

**Why it matters:** Fish will need swimming animations, bosses will have attack animations.

**Research Prompt:**
```
Sprite animation systems for ESP32 game development. Cover:

1. Frame-based animation fundamentals:
   - Sprite sheet layouts (horizontal strip vs grid)
   - Frame timing (fixed vs variable delta time)
   - Animation state machines

2. Memory-efficient approaches:
   - Single sprite buffer with frame switching
   - Preloaded vs streamed frames
   - Compressed animation data

3. Multiple animated entities:
   - Managing many fish with independent animation states
   - Animation pooling
   - Staggered frame updates to distribute load

4. Implementation patterns:
   - Animation struct/class design
   - Frame timing with millis()
   - Blending animations (idle -> swim -> eat)

5. Code example: Simple animation system for ESP32 with TFT_eSPI

Focus on memory-constrained systems without PSRAM.
```

**Output file:** `ANIMATION_SYSTEMS.md`

---

### 9. Text/Dialogue Rendering

**Why it matters:** Ty Knotts dialogue system needs efficient text display.

**Research Prompt:**
```
Text rendering optimization for ESP32 TFT games. Cover:

1. TFT_eSPI font options:
   - Built-in fonts (GLCD, Font2, etc.)
   - Smooth fonts
   - Custom fonts
   - Memory usage per font

2. Text rendering performance:
   - drawString() speed
   - Character-by-character vs full string
   - Text with background (flicker-free)

3. Dialogue box systems:
   - Speech bubble rendering
   - Typewriter effect (character reveal)
   - Text wrapping

4. Memory-efficient text storage:
   - PROGMEM string tables
   - String compression
   - Procedural text generation

5. Localization considerations:
   - UTF-8 support
   - Variable string lengths
   - Font coverage

Include code example for dialogue box with typewriter effect.
```

**Output file:** `TEXT_RENDERING.md`

---

## Category: Memory & Performance

### 2. ESP32 Memory Management for Games

**Why it matters:** Bass-Hole has 240x240 background + multiple fish sprites. Need to understand limits.

**Research Prompt:**
```
Memory management for sprite-based games on ESP32 (no PSRAM). Cover:

1. PROGMEM vs heap for sprite storage
   - When to use each
   - How to read from PROGMEM efficiently
   - Maximum practical sizes

2. Heap fragmentation
   - How it happens with dynamic sprite allocation
   - Prevention strategies
   - Monitoring tools (ESP.getFreeHeap, ESP.getMaxAllocHeap)

3. Practical limits for CYD games:
   - Maximum background sprite size
   - Maximum number of simultaneous sprites
   - Rule of thumb calculations

4. Memory-saving techniques:
   - Palette-based sprites (4-bit, 8-bit) vs RGB565
   - Sprite sheets vs individual sprites
   - Tile-based backgrounds

Include code examples for ESP32 Arduino framework.
```

**Output file:** `MEMORY_MANAGEMENT.md`

---

### 8. Boss Battle Performance

**Why it matters:** Bosses have complex sprites, attack patterns, particles - potential performance cliff.

**Research Prompt:**
```
Performance optimization for "bullet hell" style moments in ESP32 games. Cover:

1. Many simultaneous sprites:
   - Object pooling patterns
   - Fixed-size arrays vs dynamic allocation
   - Active/inactive entity management

2. Collision detection optimization:
   - Spatial partitioning (grid-based)
   - Bounding box vs pixel-perfect
   - Collision layers/masks

3. Particle effects on constrained hardware:
   - Simple particle systems
   - Maximum practical particle count
   - Particle pooling

4. Performance monitoring:
   - FPS measurement and display
   - Finding bottlenecks
   - Acceptable frame drops

5. Graceful degradation:
   - Reducing particle count under load
   - Sprite LOD (level of detail)
   - Dynamic quality adjustment

Focus on maintaining 20+ FPS during intense moments on ESP32 with ILI9341.
```

**Output file:** `PERFORMANCE_OPTIMIZATION.md`

---

## Category: Input

### 6. Touch Input During Heavy Rendering

**Why it matters:** Game needs responsive tap-to-feed and tap-to-collect while rendering 15 fish.

**Research Prompt:**
```
Touch input handling in ESP32 games with continuous rendering. Cover:

1. Touch polling vs interrupts:
   - XPT2046 IRQ pin usage
   - Polling frequency requirements
   - Debouncing touch events

2. Input during render loop:
   - Where to sample touch in game loop
   - Buffering touch events
   - Missing touches during long renders

3. Touch responsiveness targets:
   - Human perception thresholds
   - Acceptable latency for tap games
   - Testing touch latency

4. Dual-core solutions:
   - Touch handling on Core 0
   - Rendering on Core 1
   - FreeRTOS task communication

5. Practical patterns for CYD touch games:
   - Touch queue implementation
   - Gesture detection (tap vs hold vs drag)
   - Touch calibration at runtime

Include code examples using XPT2046_Touchscreen library.
```

**Output file:** `TOUCH_INPUT.md`

---

### 12. Additional Input Methods

**Why it matters:** Expand CYD beyond touchscreen - physical controls, external devices.

**Research Prompt:**
```
Additional input methods for ESP32 CYD projects. Cover each option with wiring, libraries, and code examples:

1. Physical buttons:
   - GPIO button wiring (pull-up vs pull-down)
   - Debouncing techniques
   - Button libraries (OneButton, Bounce2)
   - Multi-button handling

2. Rotary encoders:
   - Wiring diagram
   - Reading rotation and button press
   - ESP32 encoder libraries
   - Use cases (menu navigation, value adjustment)

3. USB keyboard input:
   - USB Host capability on ESP32
   - Required hardware (USB Host Shield or ESP32-S2/S3)
   - Libraries for HID input
   - Limitations on standard ESP32

4. Bluetooth keyboard/gamepad:
   - ESP32 Bluetooth HID host
   - Pairing process
   - Supported devices
   - Latency considerations

5. IR remote control:
   - IR receiver wiring (VS1838B)
   - IRremote library
   - Learning remote codes
   - Use cases

6. Gesture sensors:
   - APDS-9960 (wave gestures)
   - PAJ7620 (more gestures)
   - Wiring and libraries

7. Joystick/analog input:
   - Analog joystick wiring
   - ADC reading on ESP32
   - Calibration and dead zones

For each, note:
- Additional hardware required
- Available GPIO pins on CYD
- Power requirements
- Practical applications

Focus on inputs that work with standard ESP32-2432S028R CYD boards.
```

**Output file:** `INPUT_METHODS.md`

---

## Category: Output

### 4. Audio Output

**Why it matters:** Phase 6 mentions "Sound effects (ESP32 buzzer - 8-bit style)"

**Research Prompt:**
```
Audio output options for ESP32 CYD (Cheap Yellow Display) boards. Cover:

1. Built-in audio capabilities:
   - Does CYD have a speaker/buzzer? Which pin?
   - DAC output options (GPIO25, GPIO26)
   - I2S for external speakers

2. Simple sound generation:
   - tone() function limitations on ESP32
   - ledcWriteTone() for PWM audio
   - Playing simple 8-bit style sound effects

3. Playing WAV/audio files:
   - From SD card
   - From PROGMEM
   - Libraries (ESP8266Audio, etc.)

4. Audio while rendering:
   - Does audio interrupt display updates?
   - Using second core for audio
   - DMA considerations

5. Practical recommendations for retro game sound effects on CYD

Include wiring diagrams if external components needed.
```

**Output file:** `AUDIO_OUTPUT.md`

---

### 13. Output Options (Printer, Email, etc.)

**Why it matters:** Extend CYD projects beyond the screen - physical output, notifications, data export.

**Research Prompt:**
```
Output options for ESP32 CYD projects beyond the display. Cover each with practical implementation:

1. Thermal printer output:
   - Compatible thermal printers (Adafruit, generic 58mm)
   - Wiring (TTL serial)
   - Libraries (Adafruit_Thermal)
   - Printing text, graphics, barcodes
   - Paper and power requirements
   - Use cases: receipts, labels, game scores, tickets

2. Email notifications:
   - ESP32 email libraries (ESP_Mail_Client)
   - SMTP server configuration
   - Gmail app passwords
   - Sending plain text vs HTML
   - Attachments (screenshots?)
   - Rate limiting and best practices

3. SMS/text notifications:
   - Twilio API integration
   - Alternative services
   - Cost considerations
   - Use cases: alerts, game events

4. Push notifications:
   - Firebase Cloud Messaging (FCM)
   - Pushover, Pushbullet
   - IFTTT webhooks
   - Setting up mobile notifications

5. Data logging/export:
   - SD card CSV logging
   - Google Sheets API integration
   - MQTT to data platforms
   - InfluxDB/Grafana dashboards

6. Physical outputs:
   - LED strips (WS2812B/NeoPixel)
   - Servo motors
   - Relay control
   - Haptic feedback (vibration motors)

7. Screen capture/sharing:
   - Capturing TFT to BMP
   - Uploading screenshots
   - Streaming display over network

For each output type, include:
- Required hardware/services
- Libraries and setup
- Code example
- Practical use cases for games/projects
- Costs (if applicable)

Focus on what's achievable with standard ESP32 CYD hardware plus minimal additions.
```

**Output file:** `OUTPUT_OPTIONS.md`

---

## Category: Storage

### 7. Game State Persistence

**Why it matters:** Need to save coins, unlocked fish, progress between sessions.

**Research Prompt:**
```
Game save systems for ESP32. Compare these storage options:

1. EEPROM / Preferences library:
   - Wear leveling concerns
   - Size limits
   - Read/write speed
   - Best practices for game saves

2. SD card saves:
   - File formats (binary vs JSON vs custom)
   - Corruption protection
   - Save/load timing

3. SPIFFS/LittleFS:
   - Pros/cons vs SD card
   - Partition sizing
   - File management

4. Save data design:
   - What to save (game state struct)
   - Versioning saves for updates
   - Checksum/validation
   - Auto-save strategies

5. Code example: Simple save system with corruption protection

Focus on reliability - players hate losing progress.
```

**Output file:** `GAME_SAVES.md`

---

## Category: Connectivity

### 11. API/Cloud Connectivity

**Why it matters:** Make CYD projects smarter with online features - leaderboards, remote data, AI integration.

**Research Prompt:**
```
Cloud connectivity and API integration for ESP32 CYD projects. Cover:

1. WiFi fundamentals on ESP32:
   - Connecting to networks
   - Handling disconnections gracefully
   - WiFiManager for easy setup
   - Static IP vs DHCP
   - Connection during gameplay (background task)

2. HTTP/REST API consumption:
   - HTTPClient library
   - GET and POST requests
   - JSON parsing (ArduinoJson)
   - Error handling and timeouts
   - Rate limiting

3. Practical API integrations:

   a. Weather data:
      - OpenWeatherMap API
      - Displaying weather on CYD
      - Using weather in games (fish behavior changes with real weather?)

   b. Time/NTP:
      - Getting accurate time
      - Time-based game events
      - Daily rewards systems

   c. Leaderboards:
      - Simple backend options (Firebase, Supabase)
      - Submitting scores
      - Fetching top scores
      - Anti-cheat considerations

   d. Remote configuration:
      - Fetching game settings from server
      - A/B testing
      - Kill switch for broken versions

   e. AI/LLM integration:
      - Calling OpenAI/Anthropic APIs
      - Generating dynamic dialogue
      - Response time considerations
      - Token costs
      - Caching responses

4. Real-time connections:
   - WebSockets on ESP32
   - MQTT for IoT patterns
   - Server-sent events

5. Security considerations:
   - API key storage (not in code!)
   - HTTPS on ESP32
   - Certificate handling

6. Offline-first design:
   - Caching API responses
   - Queue actions when offline
   - Sync when connection restored

Include code examples for:
- Fetching JSON from an API
- Posting a high score
- Simple AI text generation call
```

**Output file:** `CLOUD_CONNECTIVITY.md`

---

## Category: Sensors

### 10. Sensor Integration

**Why it matters:** Make CYD projects interactive with the physical world - environmental awareness, motion, proximity.

**Research Prompt:**
```
Sensor integration for ESP32 CYD projects. Cover each sensor type with wiring, libraries, and game/project applications:

1. Environmental sensors:

   a. Temperature/Humidity (DHT22, BME280):
      - Wiring diagram
      - Libraries
      - Reading values
      - Game ideas: fish behavior changes with room temp

   b. Light sensor (LDR, BH1750):
      - Analog vs digital options
      - Auto-brightness for display
      - Day/night game modes

   c. Air quality (MQ-series, CCS811):
      - What they measure
      - Warm-up requirements
      - Display air quality on CYD

2. Motion/Position sensors:

   a. Accelerometer/Gyroscope (MPU6050, LSM6DS3):
      - Wiring (I2C)
      - Reading orientation
      - Gesture detection (shake, tilt)
      - Game control via tilting CYD

   b. PIR motion sensor:
      - Detecting presence
      - Wake-on-motion
      - Security/alert applications

   c. Ultrasonic distance (HC-SR04):
      - Wiring
      - Distance measurement
      - Proximity-based interactions

3. Biometric sensors:

   a. Heart rate (MAX30102):
      - Wiring and placement
      - Libraries
      - Game ideas: difficulty scales with heart rate

   b. Fingerprint (R307):
      - User identification
      - Saving per-user game progress

4. RFID/NFC:
   - RC522 RFID reader
   - Reading tags/cards
   - Game ideas: physical cards unlock fish species
   - Amiibo-style collectibles

5. Sound sensors:
   - Microphone modules (MAX9814)
   - Sound level detection
   - Clap detection
   - Voice-activated features

6. GPS (NEO-6M):
   - Location-based features
   - Outdoor applications
   - Power consumption

For each sensor, include:
- Wiring diagram for CYD (available pins)
- Required libraries
- Basic code example
- Creative applications for games
- Power considerations
- I2C address conflicts to watch for

Note which GPIO pins are available on CYD after display and touch are connected.
```

**Output file:** `SENSOR_INTEGRATION.md`

---

## Research Output Structure

When research is complete, save documents to:

```
DIST_ESP32-CYD-Tester/
└── docs/
    └── research/
        ├── SPRITE_PERFORMANCE.md
        ├── MEMORY_MANAGEMENT.md
        ├── DIRTY_RECTANGLES.md
        ├── AUDIO_OUTPUT.md
        ├── ANIMATION_SYSTEMS.md
        ├── TOUCH_INPUT.md
        ├── GAME_SAVES.md
        ├── PERFORMANCE_OPTIMIZATION.md
        ├── TEXT_RENDERING.md
        ├── SENSOR_INTEGRATION.md
        ├── CLOUD_CONNECTIVITY.md
        ├── INPUT_METHODS.md
        └── OUTPUT_OPTIONS.md
```

---

## How to Use This Document

1. **Pick a topic** based on current priority
2. **Copy the research prompt** to your AI research tool
3. **Generate the documentation**
4. **Save to the research folder** with the specified filename
5. **Update the status table** at the top of this document
6. **Apply findings** to Bass-Hole or tester project as needed

---

## Additional Research Topics (Phase 2)

These questions emerged from the initial research and warrant follow-up investigation.

| # | Topic | Priority | Status | Generated Filename |
|---|-------|----------|--------|-------------------|
| 14 | DMA Transfer Deep Dive | P2 | ✅ Complete | `DMA Transfer Deep Dive.md` |
| 15 | 8-bit Indexed Color Sprites | P1 | ✅ Complete | `8-bit Indexed Color Sprites.md` |
| 16 | Dual-Core Task Architecture | P2 | ✅ Complete | `Dual-Core Task Architecture.md` |
| 17 | Sprite Sheet Packing | P2 | ✅ Complete | `Sprite Sheet Packing.md` |
| 18 | LVGL Integration | P3 | ✅ Complete | `LVGL Integration.md` |
| 19 | Real-Time Audio Mixing | P3 | ✅ Complete | `Real-Time Audio Mixing.md` |
| 20 | SD Card Streaming | P2 | ✅ Complete | `SD Card Streaming.md` |
| 21 | Anti-Aliased Sprites | P3 | ✅ Complete | `Anti-Aliased Sprites.md` |
| 22 | Wokwi Community Gems | P1 | ✅ Complete | `WOKWI_GEMS.md` |

> [!NOTE]
> **Phase 2 Research Complete (2026-01-16):** All 8 follow-up topics have been researched.

### 14. DMA Transfer Deep Dive

**Research Prompt:**
```
Detailed implementation guide for pushImageDMA() on ESP32 with TFT_eSPI. Cover:
1. Enabling DMA in User_Setup.h
2. Buffer ownership during transfer (when is it safe to modify?)
3. Detecting DMA completion
4. Practical example: background rendering with DMA
5. Gotchas on ESP32-WROOM (no PSRAM) boards
6. Performance comparison: pushImage vs pushImageDMA
```

**Output file:** `DMA_TRANSFER_GUIDE.md`

---

### 15. 8-bit Indexed Color Sprites

**Research Prompt:**
```
Converting and rendering 8-bit indexed color sprites on ESP32 with TFT_eSPI. Cover:
1. Palette design (16-color for fish, 256-color for backgrounds)
2. Converting PNG to indexed C arrays (tool recommendations)
3. Runtime color lookup during rendering
4. Memory savings calculation
5. Performance impact of lookup vs raw RGB565
6. Code example: AnimatedSprite class with 8-bit frames
```

**Output file:** `INDEXED_COLOR_SPRITES.md`

---

### 16. Dual-Core Task Architecture

**Research Prompt:**
```
FreeRTOS dual-core architecture for ESP32 games. Cover:
1. When to use Core 0 vs Core 1
2. Creating tasks pinned to specific cores
3. Safe communication between cores (queues, mutexes)
4. Pattern: Touch/audio on Core 0, rendering on Core 1
5. Avoiding deadlocks and race conditions
6. Code example: Touch input queue processed by render task
```

**Output file:** `DUAL_CORE_ARCHITECTURE.md`

---

### 17. Sprite Sheet Packing

**Research Prompt:**
```
Sprite sheet creation and usage for ESP32 games. Cover:
1. Texture atlas creation tools (TexturePacker alternatives)
2. Optimal sheet sizes for ESP32 memory
3. UV coordinate extraction at runtime
4. Sprite sheet data structures in C
5. Code example: Animation player using packed sheet
```

**Output file:** `SPRITE_SHEETS.md`

---

## Conflicts & Alternative Approaches

These are tensions between the new research findings and existing project decisions. **All are hypothetical until hardware testing validates the research.**

### Conflict 1: Pixel-by-Pixel vs pushImage for Transparency

**Current Approach (PHASE2_FINDINGS.md):**
> "Pixel-by-pixel gives consistent results across different displays. TFT_eSPI's pushImage() with transparency was unreliable."

**Research Finding (Sprite Performance doc):**
> "pushImage() with transparency color parameter is MUCH faster than drawPixel() loops."

**Possible Angles:**
1. **Stick with pixel-by-pixel** - If pushImage transparency truly is unreliable on our board
2. **Revisit pushImage** - The "unreliability" may have been caused by byte swap mismatch, not pushImage itself
3. **Hybrid approach** - Use pushImage for opaque sprites, pixel-by-pixel only when needed
4. **TFT_eSprite** - Create sprite in RAM, push entire sprite (handles transparency differently)

**Resolution:** Run SPRITE_TEST_PLAN.md Test 4 (Transparency) with correctly configured swap bytes. The "unreliability" may have been a symptom of the byte swap bug, not intrinsic to pushImage.

---

### Conflict 2: invertDisplay vs Pre-Inverted Colors

**Current Approach (config.h):**
```cpp
// All colors pre-inverted via XOR 0xFFFF
#define COLOR_BLACK  0xFFFF
#define COLOR_WHITE  0x0000
```

**Research Finding:**
> "There are THREE separate settings: RGB Order, Byte Swap, Inversion. These are often conflated."

**Possible Angles:**
1. **Keep pre-inverted colors** - Works, but confusing and error-prone
2. **Fix root cause** - Use correct inversion setting in platformio.ini, use normal color values
3. **Centralize conversion** - Create macro that XORs at compile time, clearly documented

**Resolution:** Run SPRITE_TEST_PLAN.md Test 2 (Inversion). If `TFT_INVERSION_ON` gives correct colors for BOTH sprites AND primitives, remove all the XOR workarounds.

---

### Conflict 3: Manual Byte Swap vs setSwapBytes()

**Current Approach (gfxDrawSpriteTransparent):**
```cpp
// Manual byte swap in rendering loop
uint16_t swappedPixel = ((pixel >> 8) | (pixel << 8));
```

**Research Finding (DEVLOG.md):**
> "setSwapBytes() affects pushImage/pushSprite but NOT drawPixel. Bass-Hole does manual swap in gfxDrawSpriteTransparent() but NOT in gfxDrawSprite() - this is the bug."

**Possible Angles:**
1. **Apply manual swap everywhere** - Consistent but defeats purpose of setSwapBytes
2. **Use setSwapBytes(true) globally** - Then REMOVE manual swap, use pushImage
3. **Generate sprites with correct byte order** - No swap needed at all

**Resolution:** Determine correct byte order via SPRITE_TEST_PLAN.md Test 3, then refactor to use ONE consistent approach across all rendering paths.

---

### Conflict 4: EEPROM vs Preferences vs SD Card

**Current Approach (DEVELOPMENT_STRATEGY.md):**
> "Save/load game state (EEPROM)" and "Save/load game state to SD card"

**Research Finding (Game Saves doc):**
> "EEPROM has wear leveling concerns. Preferences library is better. SD card has corruption risk but more space."

**Possible Angles:**
1. **SD card primary** - Already implemented, good for large saves
2. **Preferences for settings** - Small, fast, wear-leveled for options
3. **Hybrid** - Preferences for quick state, SD for full saves
4. **LittleFS** - Internal flash filesystem, no SD required

**Resolution:** Review actual save data size needs. For Bass-Hole (coins, fish, progress) the data is likely small enough for Preferences. SD could be fallback or for exports.

---

### Conflict 5: DMA Memory Requirements (NEW from Phase 2)

**Current Approach:**
- Using pushImage for sprite rendering
- Full 240x240 background stored in PROGMEM

**Research Finding (DMA Transfer Deep Dive):**
> "A full-screen 320×240 buffer at 16-bit consumes 153,600 bytes. DMA double-buffering requires ~300kB of RAM - unsuitable on boards without PSRAM."

**Possible Angles:**
1. **Skip DMA** - Standard pushImage is already fast, DMA adds complexity
2. **Partial DMA** - Use DMA for background only, not sprites
3. **Reduce buffer size** - Use 8-bit indexed backgrounds to halve memory
4. **Single buffer DMA** - Accept tearing risk, avoid double-buffer memory cost

**Resolution:** DMA is likely NOT worthwhile for Bass-Hole given memory constraints. Standard rendering is already 30+ FPS.

---

### Conflict 6: 16-bit vs 8-bit Sprites (NEW from Phase 2)

**Current Approach (PHASE2_FINDINGS.md):**
> All sprites stored as `const uint16_t[]` in PROGMEM (RGB565)

**Research Finding (8-bit Indexed Color Sprites):**
> "8-bit sprites use half the memory. A 240×240 16-bit background uses 115KB; 8-bit uses 57KB."

**Possible Angles:**
1. **Stay 16-bit** - Current workflow works, no conversion needed
2. **Convert backgrounds to 8-bit** - Biggest memory savings, backgrounds don't need full color
3. **Convert fish sprites to 8-bit** - Each fish uses 2KB now → 1KB with indexed
4. **Full 8-bit pipeline** - Everything indexed, shared palettes per category

**Resolution:** Fish have ~10 colors, backgrounds have more variety. Consider 8-bit fish sprites with 16-color palettes. This would require:
- New conversion tool (`img2code_indexed.py`)
- Palette arrays per sprite category
- TFT_eSprite with `setColorDepth(8)`

---

### Conflict 7: LVGL vs Raw TFT_eSPI (NEW from Phase 2)

**Current Approach:**
- Raw TFT_eSPI for everything (sprites, UI, text)

**Research Finding (LVGL Integration):**
> "LVGL is well suited for complex widgets and polished UI but raw TFT_eSPI is preferable for maximum frame rates in games."

**Resolution:** LVGL is NOT recommended for Bass-Hole. The game's UI is simple (coins, buttons) and doesn't need LVGL's widget system. Stay with raw TFT_eSPI for performance.

---

## Research Summary

| Phase | Topics | Status |
|-------|--------|--------|
| Phase 1 | Topics 1-13 (Core CYD development) | ✅ Complete |
| Phase 2 | Topics 14-21 (Advanced techniques) | ✅ Complete |
| Phase 3 | TBD based on hardware testing | Pending hardware |

**Total Research Documents:** 21

---

## Notes

- Research prompts are designed to be self-contained
- Each prompt asks for code examples - essential for implementation
- Focus is on practical, working solutions over theoretical
- ESP32 without PSRAM is the target (standard CYD boards)
- **All planned research is now complete** - future topics should emerge from hardware testing

---

*Last updated: 2026-01-16*
