# DMA Transfer Deep Dive

DMA (Direct Memory Access) transfers can off‑load pixel pushing to a dedicated DMA engine so that the CPU can prepare the next frame while data is being sent to the display.  On the ESP32 this is supported by the TFT_eSPI library for SPI‑driven displays.  Because the ESP32’s SPI driver is already highly optimised, DMA typically yields at most a 2× improvement when the CPU is able to prepare the next buffer while the previous one is being sent【951023150565736†L921-L927】.

## Enabling DMA

Unlike other options, DMA is not selected at compile time in `User_Setup.h`.  In your sketch you initialise the driver normally (`tft.init()`) and then call `tft.initDMA()` to allocate the DMA engine.  To ensure the chip‑select line stays low during transfers you should call `tft.startWrite()` once after initialisation.  A typical setup from the Bouncy Circles example illustrates the sequence: initialise the display, call `initDMA()`, then create sprite buffers and start writing【755911502728821†L54-L96】.

```cpp
#include <TFT_eSPI.h>
TFT_eSPI tft;

void setup() {
  tft.init();
  tft.initDMA();    // allocate DMA engine
  tft.startWrite(); // keep CS low during DMA
  // ... create image buffers here ...
}
```

No special compile‑time flag is required, but your board must have enough free RAM for the DMA buffer.  A full‑screen 320×240 image at 16‑bit colour consumes 153 600 bytes; the Bouncy Circles demo uses two such buffers (double buffering) which requires about 300 kB of RAM【755911502728821†L0-L22】, so DMA is often unsuitable on boards without PSRAM.

## Buffer ownership and life cycle

A DMA transfer continues after the `pushImageDMA()` call returns.  The library warns that you must not modify or free the source buffer while the DMA engine is reading it【951023150565736†L946-L949】.  A convenient way to avoid this is to allocate the image buffer dynamically (e.g. with `malloc`) and free it only after DMA completion.  Alternatively you can pass a separate double‑buffer pointer when calling `pushImageDMA(x,y,w,h,data,buffer)`; in this mode the function copies the image into the provided buffer before initiating DMA【951023150565736†L965-L978】.  This avoids the original array being modified when byte swapping or clipping is required.

The compiler may optimise away stack‑allocated arrays once a function returns, so avoid declaring the buffer inside a local scope【951023150565736†L937-L943】.  Use heap allocation or static/global variables to ensure the buffer remains valid until the DMA transfer has finished.

## Detecting DMA completion

TFT_eSPI provides two functions to manage DMA progress: `tft.dmaBusy()` returns `true` if a transfer is still in progress, and `tft.dmaWait()` blocks until the current transfer is complete【951023150565736†L980-L999】.  After pushing an image you can poll or wait before re‑using the buffer:

```cpp
tft.pushImageDMA(x, y, w, h, frameBuffer);
while (tft.dmaBusy()) {
  // do other work or yield
}
```

Calling `tft.endWrite()` will also wait for the transfer to complete, so it should not be used until you have finished sending frames; otherwise the waiting defeats the advantage of DMA【951023150565736†L949-L953】.

## Practical example: double‑buffered background

The Bouncy Circles sketch uses two sprite buffers to implement double buffering.  Each frame the next buffer is filled with graphics, then pushed via DMA.  While the DMA transfer is in flight, the CPU begins drawing the next frame into the other buffer.  When the transfer completes, the buffers are swapped.  The following pseudo‑code summarises this technique:

```cpp
uint16_t* frame[2];
bool bufIndex = 0;

void loop() {
  // prepare next frame in frame[bufIndex]
  drawScene(frame[bufIndex], width, height);

  // push current frame via DMA
  tft.pushImageDMA(0, 0, width, height, frame[bufIndex]);

  // flip buffer index and continue drawing next frame while DMA is running
  bufIndex = !bufIndex;
}
```

The example emphasises that DMA can provide good performance when the time to prepare the next buffer is comparable to the transfer time【951023150565736†L921-L927】.  If the CPU finishes drawing long before the DMA completes, the benefit is reduced because you must wait for the transfer to finish before updating the display again.

## Gotchas on ESP32‑WROOM (no PSRAM)

Standard ESP32‑WROOM boards have about 320 kB of free SRAM after allocating Wi‑Fi and system tasks.  A 240×320 buffer at 16‑bit colour uses 153 600 bytes【755911502728821†L0-L22】; doubling it for a second buffer leaves little room for the rest of your program.  You can reduce memory by using smaller sprites or reducing colour depth (8‑bit buffers cut memory in half).  When using DMA, ensure that the buffer stays in internal RAM – PSRAM is not currently supported by TFT_eSPI’s DMA routines.  Test your memory usage with `ESP.getFreeHeap()` before and after allocating the buffers.

## Performance comparison: pushImage vs pushImageDMA

For full‑screen updates the gain from DMA is modest because TFT_eSPI’s normal pixel write routines are already highly optimised.  The library documentation notes that at best DMA can double the rendering performance when the CPU has enough work to overlap with the transfer【951023150565736†L921-L927】.  A typical improvement is 30–50 % for large images, but drawing small sprites with `pushSprite()` or using partial updates often performs just as well without the complexity of DMA.