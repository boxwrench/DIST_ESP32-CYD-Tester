# Performance Optimization for Boss Battles and Bullet‑Hell Moments

Intense “boss battle” scenes with many sprites, bullets and particles can push the ESP32 to its limits.  To maintain smooth frame rates (≥ 20 FPS) during these moments, optimize entity management, collision detection, particle effects and monitor performance.  This document outlines patterns and techniques tailored to CYD games.

## 1. Managing many simultaneous sprites

### Object pooling

Creating and destroying objects frequently (e.g., bullets) can cause heap fragmentation and incur dynamic allocation overhead.  Use an **object pool**: pre‑allocate a fixed number of objects and reuse them【217222634938005†L12-L35】.  Each object has an `active` flag; when you need a new bullet you reuse an inactive object instead of allocating a new one.  When a bullet leaves the screen or hits a target, mark it inactive and return it to the pool.

Example bullet pool structure:

```cpp
struct Bullet {
  bool active;
  int16_t x, y;
  int16_t vx, vy;
};

constexpr size_t MAX_BULLETS = 50;
Bullet bullets[MAX_BULLETS];

void spawnBullet(int16_t x, int16_t y, int16_t vx, int16_t vy) {
  for (auto &b : bullets) {
    if (!b.active) {
      b.active = true;
      b.x = x; b.y = y;
      b.vx = vx; b.vy = vy;
      break;
    }
  }
}

void updateBullets() {
  for (auto &b : bullets) {
    if (!b.active) continue;
    b.x += b.vx;
    b.y += b.vy;
    if (b.x < 0 || b.x > 320 || b.y < 0 || b.y > 240) b.active = false;
  }
}
```

### Fixed‑size arrays vs dynamic containers

Use fixed arrays or `std::array` to store sprites and bullets.  Avoid `std::vector::push_back()` and `std::vector::erase()` in the inner loop.  Iterate over the array each frame and skip inactive entries.

## 2. Collision detection optimization

### Spatial partitioning

When you have many bullets and enemies, naive O(n²) collision checks (testing every bullet against every enemy) become expensive.  Use spatial partitioning:

* **Uniform grid (spatial hash):** Divide the playfield into cells (e.g., 32×32 pixels).  For each entity, compute which cell(s) it occupies and insert it into that cell’s list.  Only test collisions within the same or neighbouring cells.
* **Quadtree:** Recursively subdivide the screen into quadrants containing entities.  Traverse the quadtree to find potential collisions.  Quadtree implementation is more complex but scales well with many entities.

### Bounding box vs pixel‑perfect

Start with **axis‑aligned bounding box (AABB)** checks, which are cheap (`if rectangles overlap`).  Perform more expensive pixel‑perfect collision checks only when bounding boxes overlap.  For circular sprites, compare squared distances instead of computing square roots.

### Collision layers/masks

Assign each entity a collision layer or mask.  Bullets only need to check collisions against enemies, not other bullets.  This reduces the number of pairs to test.

## 3. Particle effects on constrained hardware

Simple particle systems can add polish without overwhelming the ESP32.  Tips:

* **Particle pool:** Use a fixed‑size pool similar to bullets.  Each particle has position, velocity, lifespan and colour index.  Update position and decrement lifespan each frame; deactivate when expired.
* **Limited particles:** Determine a maximum number of particles (e.g., 100) and avoid spawning more when the pool is full.
* **Reuse sprites:** Instead of drawing each particle with `drawPixel()`, group small particles into one sprite (e.g., 16×16) and update its pixels directly, then push the sprite.
* **Simplify effects:** Use simple animations like fading colours or shrinking dots.  Avoid per‑particle alpha blending.

## 4. Performance monitoring

### Measuring frame time and FPS

Insert timing code in your main loop to measure how long each frame takes:

```cpp
uint32_t lastFrame = micros();

void loop() {
  uint32_t now = micros();
  uint32_t frameTime = now - lastFrame;
  lastFrame = now;
  float fps = 1e6f / frameTime;
  // Optionally display fps on screen
  updateGame();
  renderFrame();
}
```

If FPS drops below your target (e.g., 20 FPS), consider reducing the number of particles or bullet updates.

### Profiling bottlenecks

Time individual sections of the update loop (input, logic, collision, rendering) by toggling a GPIO or measuring with microsecond timers.  This identifies which part of the code consumes the most time.

## 5. Graceful degradation

When the game becomes too busy, degrade gracefully to maintain responsiveness:

* **Reduce particle count:** Skip spawning new particles or deactivate non‑essential effects when FPS drops below a threshold.
* **Sprite level of detail (LOD):** Use smaller or simpler sprites when there are many on screen.  For example, draw fish as coloured circles instead of full bitmaps during explosions.
* **Dynamic quality adjustment:** Lower the rendering resolution or limit the number of bullets per frame when performance dips.  Restore full detail when the scene calms down.

## Summary

Optimizing boss battles on the ESP32 CYD involves reusing objects through pooling【217222634938005†L12-L35】, accelerating collision detection with spatial partitioning and AABB checks, limiting particle effects and monitoring FPS.  Implement fall‑back behaviours when performance drops to keep the game playable.  With these patterns you can sustain 20 FPS or higher even during bullet‑hell moments.
