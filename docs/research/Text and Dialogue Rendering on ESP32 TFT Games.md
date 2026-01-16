# Text and Dialogue Rendering on ESP32 TFT Games

Displaying dialogue and UI text efficiently is essential for games on the ESP32 Cheap Yellow Display.  TFT\_eSPI offers multiple font options with different memory footprints and rendering speeds.  This document covers font selection, drawing techniques, dialogue box patterns, memory‑efficient text storage and localization considerations.

## 1. Font options in TFT\_eSPI

### Built‑in bitmap fonts

TFT\_eSPI includes several fixed‑size bitmap fonts (GLCD Font 1, Font2, …).  These are compiled into your program and reside in flash.  They support ASCII characters only and require negligible RAM.  The library notes that using built‑in fonts is fast and they can be enabled or disabled via `User_Setup_Select.h` to save flash【708024030522094†L624-L686】.

### Smooth (anti‑aliased) fonts

The library also supports **smooth fonts**, which are proportional and anti‑aliased.  Smooth fonts are stored in external `.vlw` files in SPIFFS or converted to C arrays in flash.  They enable rendering of any Unicode character, but each size of a font is a separate file【708024030522094†L624-L686】.  Rendering smooth fonts requires more CPU and reads from flash, but produces high‑quality text.  You can render text with or without background; the library automatically scales glyph bitmaps and handles kerning.

### Custom fonts

You can convert TTF or OTF fonts to the `.vlw` format using `Processing` scripts provided in the TFT\_eSPI repository.  Store the `.vlw` files in SPIFFS and load them at runtime with `tft.loadFont()`.

### Memory usage per font

* **Built‑in fonts:** Use negligible RAM; stored in program flash.
* **Smooth fonts:** Each glyph is stored as pixel data.  A 16‑pt font may require several kilobytes per character set.  Store fonts in SPIFFS to avoid using heap.

## 2. Text rendering performance

### drawString() speed

`tft.drawString()` draws a null‑terminated string at a given position.  Using built‑in fonts this is very fast.  With smooth fonts the library caches glyphs and draws anti‑aliased pixels; this is slower but still suitable for short labels.

### Character‑by‑character vs full string

For typewriter effects (revealing characters one by one), drawing each character individually is acceptable because strings are short.  For full paragraphs, call `drawString()` once to reduce overhead.

### Text with background

When drawing over a complex background, first fill the text area with a solid rectangle or push a sprite containing the text.  `drawString()` can render text with transparent background when using smooth fonts; ensure the sprite’s transparent color is set correctly.

## 3. Dialogue box systems

### Speech bubble rendering

Use a semi‑transparent rectangle or rounded rectangle as the dialogue box.  Draw the bubble in a sprite, draw the text inside, then push the sprite to the display.  This prevents flicker and simplifies dirty rectangle management.

### Typewriter effect

Create a pointer (`currentIndex`) into your dialogue string.  Each frame or at a fixed interval (e.g., 50 ms) increment the pointer and redraw the portion of the string up to `currentIndex`.  Use `drawString()` with built‑in fonts or draw each character individually for smooth fonts.

```cpp
String line = "Ty Knotts: Welcome to the tank!";
uint32_t lastType = 0;
size_t index = 0;

void updateDialogue() {
  uint32_t now = millis();
  if (index < line.length() && now - lastType >= 50) {
    index++;
    lastType = now;
    // Clear and redraw dialogue box sprite
    dialogueSprite.fillSprite(TFT_DARKGREY);
    dialogueSprite.drawString(line.substring(0, index), 4, 4, 2);
    dialogueSprite.pushSprite(10, 200);
  }
}
```

### Text wrapping

Implement a simple text wrapper that splits a string into lines based on maximum pixel width.  `tft.textWidth()` returns the width of a string for the current font.  When adding a word would exceed the dialogue box width, move to the next line.

## 4. Memory‑efficient text storage

* **PROGMEM string tables:** Store constant strings (menu labels, dialogue) in flash using `PROGMEM`.  This frees RAM for dynamic data.
* **String compression:** For large amounts of dialogue, compress strings (e.g., run‑length encoding or dictionary compression) and decompress on the fly.  Keep a lookup table of common words and replace them with short tokens.
* **Procedural text generation:** Generate text at runtime using templates and variables (e.g., “You caught {n} coins!”) to reduce stored strings.

## 5. Localization considerations

Games may need to support multiple languages:

* **UTF‑8 support:** Smooth fonts support Unicode if the glyphs are available in the `.vlw` file.  Built‑in fonts cover only ASCII and cannot display accented characters.
* **Variable string length:** Different languages require different space; design UI elements to adapt to longer or shorter strings.  Use text wrapping functions to avoid truncation.
* **Font coverage:** Ensure your custom fonts include glyphs for all target languages (Latin extended, Cyrillic, etc.).  This increases font file size, so consider separate font files per language.

## Summary

Efficient text rendering on the ESP32 involves choosing appropriate fonts, minimising draw calls and managing memory.  Use built‑in bitmap fonts for fast labels and menus, and smooth fonts from SPIFFS for high‑quality dialogue.  Draw text into sprites to avoid flicker, implement typewriter effects by revealing characters gradually, and wrap text based on pixel width.  Store strings in PROGMEM or compressed form and plan for localization by selecting fonts with adequate glyph coverage【708024030522094†L624-L686】.
