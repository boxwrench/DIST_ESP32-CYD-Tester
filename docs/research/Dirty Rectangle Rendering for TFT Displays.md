# Dirty Rectangle Rendering for TFT Displays

When rendering games on resource‑constrained systems like the ESP32 CYD, redrawing the entire screen every frame is often unnecessary and wasteful.  The **dirty rectangle** technique redraws only parts of the screen that have changed, reducing SPI bandwidth and CPU load.  This document explains the algorithm and practical implementation strategies for TFT displays (ILI9341 or ST7789).

## 1. The dirty rectangle algorithm

The core idea is to **track regions of the screen that changed** between frames and update only those areas.  A *dirty rectangle* is defined by its coordinates `(x, y, width, height)`.  During the game loop, you:

1. Determine which objects moved or changed state.
2. Compute the bounding box of each changed object.
3. Merge overlapping rectangles to reduce the number of draw calls.
4. For each resulting rectangle, save the background (if needed), render the new content, and update the display.

An overview from the ScummVM blog describes dirty rectangles as storing draw calls from the current and previous frames, comparing them to determine which parts of the screen changed, and then redrawing only those parts【450373212300481†L19-L34】.  This approach significantly reduces the workload when most of the scene is static.

### Tracking changed regions

* **Per‑sprite tracking:** Each sprite stores its previous and current positions.  When the sprite moves, it marks both the old and new bounding boxes as dirty.
* **Grid‑based invalidation:** Divide the screen into a grid (e.g., 32×32 pixels).  When any object covers a grid cell, mark that cell as dirty.  The grid approach simplifies merging but can over‑invalidate when objects cross cell boundaries.
* **Bounding box union:** Compute the union of bounding boxes for all moving sprites.  For small numbers of sprites, this may be simpler than per‑cell tracking.

### Merging rectangles

Merging dirty rectangles reduces the number of calls to `pushImage()` or `pushSprite()`.  When two rectangles overlap or are adjacent, combine them into a single larger rectangle.  However, merging too aggressively may redraw more pixels than necessary; pick a heuristic (e.g., merge only if the overlap area exceeds a threshold).

### Full redraw vs partial redraw

If the number or size of dirty rectangles becomes large (e.g., during explosions), it may be faster to redraw the entire screen.  Implement logic to measure the total dirty area: if it exceeds a threshold (e.g., 25 % of the screen), perform a full redraw instead of partial updates.

## 2. Implementation strategies

### Background restoration

Before drawing a sprite at a new position you must restore the background in the old position.  There are several approaches:

* **Store full background in memory:** Keep an off‑screen copy of the background image (e.g., a 240×240 16‑bit buffer).  When a sprite moves, copy the corresponding region from this buffer back to the screen before drawing the new sprite.  This requires significant RAM (≈115 kB for 240×240 16‑bit) but ensures perfect restoration.

* **Tile‑based restoration:** Store the background as small tiles (e.g., 32×32) in PROGMEM.  When a sprite vacates an area, redraw only the tiles intersecting the region.  This reduces RAM usage at the cost of more `pushImage()` calls.

* **Layered rendering:** Render moving sprites on a separate sprite or framebuffer and composite it over the static background each frame.  This uses more CPU but eliminates the need to restore the background separately.

### Per‑sprite tracking

Maintain for each sprite:

* `oldRect` – bounding rectangle of previous frame.
* `newRect` – bounding rectangle of current frame.

If `oldRect` differs from `newRect`, mark both rectangles as dirty.  After all sprites are processed, merge the dirty rectangles and redraw them.

### Grid‑based invalidation

Divide the display into equal‑sized cells (e.g., 16×16 or 32×32).  Each cell has a dirty flag.  When a sprite moves, mark all cells it occupies (before and after move) as dirty.  During the redraw phase, iterate over dirty cells, restore the background and draw any sprites covering the cell.  This approach simplifies tracking but may redraw slightly more area than necessary when sprites partially overlap cells.

## 3. Performance trade‑offs

* **Overdraw vs complexity:** Drawing large dirty rectangles may waste cycles by redrawing pixels that didn’t change.  However, calculating many small rectangles and switching SPI windows can be slower due to overhead.  Merge rectangles judiciously.
* **Memory vs CPU:** Storing a full background enables fast restoration but consumes a large portion of RAM.  Tile‑based methods use less RAM but require more CPU to redraw multiple tiles.  Choose based on available RAM and performance budget.
* **When dirty rectangles hurt:** If almost every pixel changes each frame (e.g., screen‑wide particle effects), the overhead of tracking and merging rectangles outweighs any benefits; simply redraw the whole screen.

## 4. Game loop pattern with dirty rectangles (pseudo‑code)

```cpp
// Assume `sprites` is a list of game objects with x, y, width, height
std::vector<Rect> dirtyRects;

void updateGame() {
  dirtyRects.clear();
  // 1. Update positions and mark dirty regions
  for (auto &s : sprites) {
    Rect oldRect = s.prevRect();
    s.update();                      // updates position/state
    Rect newRect = s.currentRect();
    if (!oldRect.equals(newRect)) {
      dirtyRects.push_back(oldRect);
      dirtyRects.push_back(newRect);
    }
  }
  // 2. Merge overlapping rectangles (simple example)
  mergeRects(dirtyRects);
  // 3. For each dirty rectangle
  for (auto &r : dirtyRects) {
    restoreBackground(r);           // copy from tile or background buffer
    drawSpritesInRect(r);           // redraw any sprites covering r
  }
  tft.endWrite();                   // finish SPI transaction if batching
}
```

Implementations vary: you could maintain a sprite “draw order”, use multiple layers, or integrate dirty rectangles with a sprite batching system.  The key is to minimise SPI transfers while keeping the code maintainable.

## Summary

Dirty rectangle rendering can significantly improve performance on ESP32 TFT displays by redrawing only the parts of the screen that change.  Track bounding boxes for moving sprites, merge overlapping regions and decide when a full screen redraw is more efficient.  Combine this technique with memory management and sprite batching to achieve smooth gameplay on the CYD.
