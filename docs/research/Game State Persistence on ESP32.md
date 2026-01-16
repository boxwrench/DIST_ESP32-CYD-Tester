# Game State Persistence on ESP32

Players expect games to save progress, unlocks and high scores.  On the ESP32 CYD (no PSRAM) you must choose a storage method that balances reliability, wear‑leveling, capacity and complexity.  This document compares **Preferences (EEPROM emulation)**, **SD card**, and **SPIFFS/LittleFS**, and suggests strategies for designing robust save systems.

## 1. Preferences library (EEPROM/NVS)

The ESP32’s **Preferences** library provides a simple key‑value store backed by the non‑volatile storage (NVS) subsystem.  It is intended as a drop‑in replacement for Arduino’s EEPROM.  The documentation notes that Preferences is best for storing many small values, such as configuration options; for larger data you should use a file system【375433897840499†L56-L65】.  Key features:

* **Namespaces:** Each game can have its own namespace (e.g., `"gameSave"`) to avoid collisions with other modules【375433897840499†L69-L107】.
* **Data types:** Supports `uint8_t`, `int32_t`, `float`, `bool`, and strings up to 1984 bytes.
* **Wear‑leveling:** NVS spreads writes across flash pages to avoid wearing out a single sector.
* **Size limits:** A default ESP32 partition allocates 20 kB for Preferences【612032560485883†L112-L114】; check your partition table to verify available space.

**Usage example:**

```cpp
#include <Preferences.h>

Preferences prefs;

// Save game data
void saveGame(uint32_t coins, uint8_t level) {
  prefs.begin("gameSave", false);
  prefs.putUInt("coins", coins);
  prefs.putUChar("level", level);
  prefs.end();
}

// Load game data (return defaults if not present)
void loadGame(uint32_t &coins, uint8_t &level) {
  prefs.begin("gameSave", true);
  coins = prefs.getUInt("coins", 0);
  level = prefs.getUChar("level", 1);
  prefs.end();
}
```

**Pros:**

* Simple API; suitable for small save data (scores, settings).
* Built‑in wear‑leveling; reliable across power failures.
* No file system overhead.

**Cons:**

* Limited total space (~20 kB).  Not ideal for large save files or assets.
* Data is opaque; cannot store complex structures directly without serialization.

## 2. SD card saves

Many CYD boards include an SD card slot.  Saving to SD offers large storage capacity and easy data transfer to/from a PC.

* **File formats:** You can store save data in binary (`.sav`), JSON or a custom format.  Binary formats are compact and fast; JSON is human‑readable but uses more space.
* **Corruption protection:** Always flush (`file.flush()`) after writing and close the file.  To protect against corruption, write to a temporary file then rename it when complete.  Maintain multiple save slots or backups.
* **Save/load timing:** Reading and writing to SD via SPI is slower than Preferences and may conflict with TFT drawing.  Buffer writes to minimise latency, or perform saves during less intensive parts of the game.

**Pros:**
* Virtually unlimited storage for levels, replays and user‑created content.
* Easy to migrate saves by swapping SD cards.

**Cons:**
* Requires SD card hardware and file system management.
* Less reliable if the card is removed or power is lost during writes.
* Slower access compared to NVS.

## 3. SPIFFS/LittleFS

**SPIFFS** (SPI Flash File System) and **LittleFS** are file systems that reside on the ESP32’s internal flash.  They provide a hierarchical file store with wear‑leveling and recovery.  The ESP32 default partition allocates ~1.5 MB for SPIFFS【612032560485883†L112-L114】.  Articles note that SPIFFS is better than EEPROM for storing web assets and log data【612032560485883†L49-L53】, and it distributes wear across flash sectors【612032560485883†L131-L133】.

* **Pros:** More space than Preferences; integrated wear‑leveling; simple file API.  Suitable for storing multiple save files or large structures like inventories.
* **Cons:** Slower than RAM; file open/write operations may block.  The flash lifetime still depends on write cycles.

## 4. Designing save data

When designing your save format, consider:

1. **What to save:** Persist player coins, unlocked fish, positions, inventory, level progress, high scores, and settings.
2. **Versioning:** Include a version number in the save file.  When loading, detect old versions and migrate data to new structures.
3. **Checksums/validation:** Add a checksum (CRC32) at the end of the save file to detect corruption.  Validate before loading to avoid crashes.
4. **Auto‑save strategies:** Auto‑save at safe points (e.g., after completing a level) rather than every frame.  For high‑score tables, save only when a new high score is achieved.  Provide a manual save option.

## 5. Code example: simple save system with corruption protection

This example writes a binary save file with a header, payload and CRC32 checksum to SPIFFS:

```cpp
#include <SPIFFS.h>
#include <CRC32.h>

struct GameState {
  uint32_t coins;
  uint8_t  level;
  uint8_t  unlockedFish;
};

bool saveToFile(const GameState &state) {
  File f = SPIFFS.open("/save.dat.tmp", FILE_WRITE);
  if (!f) return false;
  // Write header
  f.write("BASS", 4);
  // Write payload
  f.write((uint8_t*)&state, sizeof(state));
  // Compute CRC32
  CRC32 crc;
  crc.update("BASS", 4);
  crc.update((uint8_t*)&state, sizeof(state));
  uint32_t checksum = crc.finalize();
  f.write((uint8_t*)&checksum, sizeof(checksum));
  f.flush();
  f.close();
  // Rename to final file
  SPIFFS.remove("/save.dat");
  return SPIFFS.rename("/save.dat.tmp", "/save.dat");
}

bool loadFromFile(GameState &state) {
  File f = SPIFFS.open("/save.dat", FILE_READ);
  if (!f) return false;
  char header[4];
  if (f.readBytes(header, 4) != 4 || strncmp(header, "BASS", 4) != 0) return false;
  if (f.readBytes((char*)&state, sizeof(state)) != sizeof(state)) return false;
  uint32_t storedCrc;
  f.readBytes((char*)&storedCrc, sizeof(storedCrc));
  CRC32 crc;
  crc.update(header, 4);
  crc.update((uint8_t*)&state, sizeof(state));
  return storedCrc == crc.finalize();
}
```

## Summary

Choose your save method based on data size and reliability requirements.  Use the Preferences library for small settings or counters; its NVS storage automatically handles wear‑leveling【375433897840499†L56-L65】.  For larger save data or multiple slots, use SPIFFS/LittleFS or an SD card.  Design your save format with versioning and checksums to handle updates and corruption gracefully.  Proper save strategies ensure that players retain progress in your fish tank game.
