# Dual‑Core Task Architecture

The ESP32 contains two Xtensa LX6 processors (core 0 and core 1) that can run tasks independently.  By default the Arduino framework uses only core 1 for `setup()` and `loop()`【256771706044844†L67-L139】.  Pinning tasks to specific cores allows you to distribute work, improving responsiveness and avoiding contention.

## Choosing cores

The general guideline is to dedicate core 1 to time‑critical tasks such as display rendering and heavy computations, and use core 0 for peripheral handling and background work (touch input, sensor polling, audio)【256771706044844†L67-L139】.  You can query the current core with `xPortGetCoreID()`【256771706044844†L71-L93】.  When using Wi‑Fi or Bluetooth, those subsystems run on a dedicated FreeRTOS task pinned to core 0, so avoid assigning too many high‑priority tasks to that core.

## Creating pinned tasks

FreeRTOS includes `xTaskCreatePinnedToCore()` for creating tasks on a specific core.  The function signature is:

```c
BaseType_t xTaskCreatePinnedToCore(
  TaskFunction_t pvTaskCode,
  const char * const pcName,
  const uint32_t usStackDepth,
  void * const pvParameters,
  UBaseType_t uxPriority,
  TaskHandle_t * const pvCreatedTask,
  const BaseType_t xCoreID );
```

The last parameter specifies the core: `0` for core 0, `1` for core 1, or `tskNO_AFFINITY` to allow either core【453632800343659†L210-L228】.  Tasks pinned to a core will only run on that core, while unpinned tasks may migrate between cores.

When creating tasks from within the Arduino environment you should specify the stack size in bytes (not words), set an appropriate priority (higher numbers pre‑empt lower), and pass a handle if you need to control the task later【453632800343659†L234-L344】.

Example:

```cpp
void renderTask(void *pvParameters) {
  for (;;) {
    drawFrame();    // render graphics
    vTaskDelay(1);
  }
}

void touchTask(void *pvParameters) {
  for (;;) {
    readTouch();    // sample touch controller
    xQueueSend(touchQueue, &touchData, portMAX_DELAY);
    vTaskDelay(1);
  }
}

void setup() {
  xQueueCreate(10, sizeof(TouchData));
  xTaskCreatePinnedToCore(renderTask, "Render", 4096, nullptr, 2, nullptr, 1); // core 1
  xTaskCreatePinnedToCore(touchTask, "Touch", 2048, nullptr, 1, nullptr, 0);   // core 0
}
```

## Inter‑core communication

Tasks running on separate cores must exchange data safely.  FreeRTOS queues provide a thread‑safe, FIFO mechanism to send data from one core to another【649971739621624†L58-L70】.  The sending task calls `xQueueSend()`, while the receiving task calls `xQueueReceive()`; the queue automatically handles context switches and blocking.  Semaphores are useful to signal events, and mutexes protect shared resources such as file systems or I2C buses【649971739621624†L58-L70】.

For example, a touch task pinned to core 0 can enqueue touch coordinates, and the render task pinned to core 1 dequeues and processes them during its update cycle.

## Pattern: touch/audio on core 0, rendering on core 1

On a typical CYD game the main loop draws graphics and updates animations.  Touch input should be read frequently and without blocking rendering, so place the touch reading in a low‑priority task on core 0.  If your game plays sound effects or streams audio, the audio decoding and mixing task can also run on core 0 to avoid interfering with drawing.  The render task on core 1 reads the latest input events from a queue and updates the game state.

## Avoiding deadlocks and race conditions

Always avoid blocking calls that hold a resource for a long time.  Tasks pinned to different cores may still compete for the SPI bus or flash memory.  Use mutexes when accessing shared hardware; release them as soon as possible.  Do not call `delay()` inside critical sections; use `vTaskDelay()` or `xTaskNotifyWait()` to yield.  Avoid reading or writing global variables from multiple tasks without protection; instead pass data via queues or protect with a mutex.

## Example: touch queue processed by render task

```cpp
QueueHandle_t touchQueue;

void renderTask(void *pvParameters) {
  TouchData t;
  for (;;) {
    // process all pending touch events
    while (xQueueReceive(touchQueue, &t, 0) == pdTRUE) {
      handleTouchEvent(t);
    }
    drawFrame();
    vTaskDelay(1);
  }
}

void setup() {
  touchQueue = xQueueCreate(5, sizeof(TouchData));
  xTaskCreatePinnedToCore(renderTask, "Render", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(touchTask, "Touch", 2048, nullptr, 1, nullptr, 0);
}
```

By separating input handling and rendering across the two cores, you can achieve smooth graphics while maintaining responsive controls.