# Cloud Connectivity and API Integration for ESP32 CYD Projects

Connecting your fish tank game to the internet enables features such as weather‑driven events, leaderboards, time synchronization and AI‑powered dialogue.  The ESP32 has built‑in Wi‑Fi and supports HTTP, WebSockets and MQTT.  This document covers networking fundamentals, practical API integrations, security considerations and offline‑first design.

## 1. Wi‑Fi fundamentals on ESP32

The ESP32 can operate in **station** (STA), **access point** (AP) or **dual** mode.  In STA mode it connects to a Wi‑Fi network; in AP mode it hosts its own network.  The API allows scanning for available networks, connecting to one and monitoring connection state【758173930586198†L77-L104】.  A typical connection sequence in Arduino looks like this:

```cpp
#include <WiFi.h>

void connectWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
```

Use the **WiFiManager** library to present a configuration portal if credentials are absent.  Handle disconnections gracefully by checking `WiFi.status()` regularly and attempting to reconnect.

### Static IP vs DHCP

By default, the ESP32 uses DHCP to obtain an IP address.  For applications that provide local services or require port forwarding, assign a static IP using `WiFi.config()`.

### Connection during gameplay

Maintain the connection in the background while rendering.  Running network code on one core and rendering on the other prevents stuttering.  Limit the frequency of network calls to avoid blocking the main loop.

## 2. HTTP/REST API consumption

### HTTPClient library

The Arduino `HTTPClient` class simplifies HTTP requests.  Example usage from a how‑to: create a `WiFiClient` object, connect to a server, and then use `HTTPClient::GET()` or `POST()`【164849748904786†L31-L52】.  For JSON parsing, use the `ArduinoJson` library.

```cpp
#include <HTTPClient.h>
#include <ArduinoJson.h>

void fetchWeather(const char* url) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());
    float temperature = doc["main"]["temp"];
    // use temperature in game
  }
  http.end();
}
```

Handle errors and timeouts gracefully; set a reasonable timeout using `http.setConnectTimeout(ms)` and `http.setTimeout(ms)`.

### Rate limiting

Many APIs limit the number of requests per minute.  Cache responses and avoid repeated calls when data changes slowly (e.g., weather).  Exponential backoff helps when a server is unavailable.

## 3. Practical API integrations

### a. Weather data

Use services like OpenWeatherMap to fetch current weather or forecasts.  Display weather icons or adjust fish behaviour (e.g., more active when it’s sunny).  Store the API key in NVS or in a separate file outside version control.

### b. Time/NTP

Synchronize the real‑time clock using Network Time Protocol (NTP).  Libraries like `NTPClient` connect to pool.ntp.org and update the ESP32 time.  Use the current date to trigger daily rewards or seasonal events.

### c. Leaderboards

Implement a simple REST backend (e.g., Firebase Functions, Supabase) that accepts HTTP POST requests to submit scores and GET requests to retrieve the top N scores.  Send JSON data with player name and score.  Example POST:

```cpp
DynamicJsonDocument doc(128);
doc["name"] = "Ty";
doc["score"] = 1234;
String payload;
serializeJson(doc, payload);

HTTPClient http;
http.begin("https://example.com/leaderboard");
http.addHeader("Content-Type", "application/json");
http.POST(payload);
```

### d. Remote configuration

Store game parameters (difficulty, spawn rates) on a server.  At startup, fetch a JSON config and adjust gameplay accordingly.  This allows you to fine‑tune the game remotely or enable/disable features (e.g., holiday events).  Cache the config locally so the game runs offline.

### e. AI/LLM integration

You can call cloud‑based AI APIs (e.g., OpenAI, Anthropic) to generate dynamic dialogue or fish names.  However, these APIs are rate‑limited and may have latency of several hundred milliseconds.  Cache responses and consider using simpler Markov chains for offline alternatives.  Also be mindful of token costs and bandwidth.

## 4. Real‑time connections

### WebSockets

Use the `WebSocketsClient` library to open a persistent connection to a server.  WebSockets are useful for real‑time leaderboards or multiplayer interactions.  Handle reconnects and keep messages small.

### MQTT

MQTT is a lightweight publish/subscribe protocol used in IoT.  Libraries like `PubSubClient` allow the ESP32 to subscribe to topics and receive updates instantly.  Use MQTT for remote control commands or for sending sensor data to a dashboard.

### Server‑sent events

Some REST APIs support server‑sent events (SSE) to push updates.  The ESPAsyncWebServer library includes SSE support.  However, WebSockets or MQTT are more widely used on ESP32.

## 5. Security considerations

* **API key storage:** Never hard‑code API keys in your code; store them in NVS or an external file.  For OTA updates, you can embed the key encrypted and decrypt at runtime.
* **HTTPS:** Use HTTPS to encrypt API calls.  The ESP32’s WiFi library supports TLS.  You must provide the server’s root certificate or enable `INSECURE_SSL` for testing (not recommended for production).
* **Certificate handling:** If using HTTPS, store certificates in PROGMEM or SPIFFS.  Certificates expire; plan to update them when needed.
* **Rate limiting and abuse prevention:** Validate score submissions on the server and include anti‑cheat measures (e.g., HMAC signatures).

## 6. Offline‑first design

Games should continue running without internet.  Strategies:

* **Cache API responses:** Store last known weather, leaderboard or config data in SPIFFS or NVS.  Use cached values when offline and refresh when a connection becomes available.
* **Queue actions:** When offline, queue score submissions or configuration changes.  When the connection is restored, replay queued actions.
* **Graceful degradation:** Disable online features (leaderboards, live events) when offline.  Provide a local high score table stored in NVS.

## Summary

The ESP32’s built‑in Wi‑Fi and HTTP libraries make cloud integration straightforward.  Connect to Wi‑Fi in station mode【758173930586198†L77-L104】, use `HTTPClient` to fetch JSON【164849748904786†L31-L52】, and parse it with ArduinoJson.  Integrate weather, time, leaderboards and remote configuration into your game while respecting API rate limits and storing keys securely.  For real‑time interaction, use WebSockets or MQTT.  Design your game to work offline by caching data and queuing actions, ensuring a smooth experience regardless of network connectivity.
