# SD Card Streaming

Large assets such as high‑resolution images, background music or video cannot always fit in the ESP32’s internal memory.  Streaming data from the SD card allows you to load content on demand.  However, the limited SPI and memory bandwidth on the ESP32 requires careful buffering.

## ESP32 I/O capabilities

An analysis of the ESP32’s I/O shows that it has two user‑accessible SPI buses capable of running at up to 80 MHz, a 1‑bit or 4‑bit SD bus for SD cards, an I2S peripheral for audio, and around 100 kB of RAM available for buffers【670236192496900†L40-L50】.  Dual‑core boards can divide SD reading and display updates across different cores, which improves performance【670236192496900†L40-L50】.  However, there is insufficient RAM to hold two full 320×240 16‑bit frame buffers simultaneously and the ESP32 does not have the processing power to decode MP4 video【670236192496900†L54-L59】.

## Buffered image streaming

When displaying a large image stored on an SD card, read it row by row or in small chunks into a buffer, then write that buffer to the display.  This avoids loading the entire image into RAM.  A typical workflow:

1. Open the file on the SD card (e.g. BMP or raw RGB565).
2. Seek to the start of the image data.
3. For each row:
   - Read `rowWidth` pixels (or a smaller block if memory is limited) into a buffer.
   - Call `tft.pushPixels()` to write the block to the display at the appropriate position.
4. Close the file.

For BMP files the rows are stored bottom‑to‑top and padded to 4‑byte boundaries, so adjust your reading accordingly.

Example code:

```cpp
File f = SD.open("/background.bmp");
uint16_t buffer[64]; // 64‑pixel line buffer
for (int y=0; y<height; y++) {
  // read one row into buffer in 4 passes of 64 pixels
  for (int x=0; x<width; x += 64) {
    f.read(buffer, 64 * 2);        // read 64 pixels (RGB565)
    tft.setAddrWindow(x, height-1-y, 64, 1);
    tft.pushPixels(buffer, 64);
  }
}
f.close();
```

By using a small buffer (64–128 pixels) you minimise RAM usage and allow other tasks to run while reading the next block.

## Double buffering and multitasking

To reduce tearing and maintain a smooth frame rate, you can allocate two line buffers and alternate between them.  While the display writes one buffer, the SD reading task fills the other.  On a dual‑core ESP32, pin the SD reading task to core 0 and the rendering task to core 1【670236192496900†L40-L50】.  Use a FreeRTOS queue or semaphore to signal when a buffer is ready to display.

```cpp
SemaphoreHandle_t bufReady;
volatile int activeBuf = 0;
uint16_t lineBuf[2][128];

void sdTask(void *pv) {
  for (;;) {
    int nextBuf = activeBuf ^ 1;
    // read next block into lineBuf[nextBuf]
    // ...
    xSemaphoreGive(bufReady);
    vTaskDelay(1);
  }
}

void renderTask(void *pv) {
  for (;;) {
    if (xSemaphoreTake(bufReady, portMAX_DELAY) == pdTRUE) {
      // display lineBuf[activeBuf]
      activeBuf ^= 1;
    }
  }
}
```

This pattern overlaps I/O with rendering and improves throughput.

## Streaming audio/video

Audio streaming uses a similar concept: read small chunks of the file into a ring buffer, decode if necessary (e.g. WAV to PCM), then write to the DAC or I2S.  For video (e.g. Motion JPEG) decode one frame at a time and display it while reading the next.  The TinyTV projects and other examples often use custom container formats and pre‑decode frames offline to reduce load【894430183822501†L26-L37】.  Because there isn’t enough memory to hold multiple full frames, you must process frames sequentially and accept lower resolutions or frame rates.

## Considerations

- File system: Use the `SDFat` library or `SPIFFS` for faster file access; format the card with FAT32.
- SPI speed: Set the SPI bus frequency for SD card and display as high as reliable (e.g. 40 MHz); use separate SPI buses if available.
- Avoid blocking the main loop while waiting for SD reads; schedule reads via tasks.
- For large assets, pre‑convert images to raw RGB565 to avoid costly decoding during streaming.

By streaming assets from the SD card with careful buffering and task scheduling, you can include high‑resolution backgrounds or long music tracks in your CYD projects without exhausting memory.