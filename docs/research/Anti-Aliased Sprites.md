# Anti‑Aliased Sprites

Standard sprites in TFT_eSPI use binary transparency: a pixel is either fully opaque or fully transparent.  Anti‑aliased sprites use alpha blending to create smooth edges and semi‑transparent pixels.  The TFT_eSPI library includes functions such as `alphaBlend()` for per‑pixel blending【334763912322719†L450-L456】.

## Alpha blending in TFT_eSPI

The header defines `uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)`, where `alpha` ranges from 0 (100 % background) to 255 (100 % foreground)【951023150565736†L897-L904】.  The implementation splits each 16‑bit RGB565 colour into red, green and blue components, blends them proportionally, and recombines them【694668361900523†L5504-L5513】.  An overloaded version accepts a `dither` parameter to reduce colour banding【951023150565736†L902-L908】.  There is also `alphaBlend24()` for 24‑bit colours【951023150565736†L906-L908】.

In a GitHub issue the author suggests using a single small sprite for all digits and performing row‑by‑row shading with `alphaBlend()`【334763912322719†L450-L456】.  When blending in a sprite you can read the existing pixel colour using `spr.readPixel(x,y)`【334763912322719†L529-L531】, compute the blended colour and write it back.

## Manual anti‑aliased overlay

To overlay an anti‑aliased sprite on a background:

1. Draw or store your sprite with an alpha channel.  Create an array of structures containing the 16‑bit foreground colour and an 8‑bit alpha for each pixel.
2. For each pixel:
   - Read the background pixel from the frame buffer or use `spr.readPixel(x,y)`.
   - Compute the blended colour: `uint16_t c = tft.alphaBlend(alpha, fg, bg);`.
   - Write the colour into the sprite or directly to the display.

Example:

```cpp
for (int j=0; j<spriteH; j++) {
  for (int i=0; i<spriteW; i++) {
    uint8_t alpha = alphaArray[j * spriteW + i];
    if (alpha == 0) continue;        // fully transparent
    uint16_t fg = fgArray[j * spriteW + i];
    uint16_t bg = spr.readPixel(i, j);
    uint16_t blended = tft.alphaBlend(alpha, fg, bg);
    spr.drawPixel(i, j, blended);
  }
}
spr.pushSprite(x, y, 0);  // draw with index 0 as transparent if using 8‑bit sprite
```

This code reads each background pixel and blends it with the foreground colour.  When pushing to the display, specify the transparent index to avoid drawing unused pixels.

## Pre‑blended sprites

For performance, pre‑compute the blending between your sprite and a known background colour.  If the background is a solid colour, you can apply `alphaBlend()` offline and store the result as a 16‑bit array.  For dynamic backgrounds this approach is not possible; instead restrict anti‑aliased elements to small sprites and perform blending only where necessary.

## Performance considerations

Reading and writing each pixel for alpha blending is slower than copying opaque pixels.  The example discussion notes that reading pixels with `readPixel()`, blending and writing them back can be a bottleneck when moving many digits【334763912322719†L586-L597】.  To mitigate this:

- Keep anti‑aliased sprites small.
- Use look‑up tables for alpha values to avoid divisions.
- Blend only the edges and leave the interior opaque.
- Consider using 8‑bit indexed sprites with precomputed transparency for simple cases.

Anti‑aliased fonts are already supported by TFT_eSPI’s smooth font system; the library renders glyphs with anti‑aliasing into a 16‑bit sprite and then pushes them【951023150565736†L897-L904】.  Borrowing this strategy for sprites can improve visual quality without sacrificing too much performance.