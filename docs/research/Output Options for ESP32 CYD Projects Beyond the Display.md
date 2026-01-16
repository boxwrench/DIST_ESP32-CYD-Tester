# Output Options for ESP32 CYD Projects Beyond the Display

In addition to showing graphics on the TFT, Cheap Yellow Display projects can produce physical and digital outputs such as printed scores, email or SMS notifications, light effects and data logging.  This document explores a range of options, summarising required hardware, libraries, code examples and use cases.

## 1. Thermal printer output

### Hardware

Small **thermal receipt printers** (e.g., Adafruit Mini Thermal Printer, generic 58 mm printers) communicate over TTL serial at 19200 bps.  They require a 5 V supply capable of delivering 1–2 A and a separate 5 V regulator because printing uses a heating element.

### Wiring

Connect the printer’s **RX** to an ESP32 UART TX pin (e.g., GPIO1 or Serial2 TX) through a logic level converter if necessary.  Ground and 5 V must be shared.  Do not connect the printer’s TX if you do not need to read data.

### Libraries

* **Adafruit_Thermal**: A library for controlling mini thermal printers.  It supports printing text, barcodes and simple bitmaps.

### Code example

```cpp
#include <Adafruit_Thermal.h>
#include <HardwareSerial.h>

HardwareSerial mySerial(1);
Adafruit_Thermal printer(&mySerial);

void setup() {
  mySerial.begin(19200, SERIAL_8N1, /*RX*/ 33, /*TX*/ 27);
  printer.begin();
  printer.println("Bass-Hole High Score: 12345");
  printer.feed(2);
}
```

### Use cases

* Print scores, tickets or receipts after completing a level.
* Generate labels for fish species collected.
* Print QR codes or barcodes for achievements.

### Paper and power requirements

Thermal printers use special heat‑sensitive paper (58 mm wide).  They draw peak currents (> 1 A) when printing dark areas; use a dedicated 5 V supply and thick wires.

## 2. Email notifications

### Libraries

* **ESP_Mail_Client**: Sends emails via SMTP.  Supports HTML and plain‑text messages and attachments.

### SMTP setup

* Obtain credentials for an SMTP server (e.g., Gmail, Outlook).  For Gmail, enable “app passwords” and use TLS.
* Configure `SMTPData` with server, port, sender address, password and recipient.

### Code example

```cpp
#include <ESP_Mail_Client.h>

SMTPSession smtp;
void sendEmail(const String &subject, const String &message) {
  SMTP_Message msg;
  msg.sender.name = "Bass-Hole";
  msg.sender.email = "youraddress@gmail.com";
  msg.subject = subject;
  msg.text.content = message;
  msg.addRecipient("Player", "player@example.com");
  smtp.connect("smtp.gmail.com", 465, "youraddress@gmail.com", "app_password");
  smtp.sendMail(&msg);
  smtp.closeSession();
}
```

### Use cases

* Notify players when they achieve a new high score or unlock a rare fish.
* Send daily summaries of aquarium status.

### Considerations

* Email sending may block for several seconds; perform in a separate task or during non‑critical sections.
* Respect email providers’ sending limits and anti‑spam policies.

## 3. SMS/text notifications

### Services

* **Twilio**, **Nexmo** or other SMS gateways allow sending SMS via REST API.  You need an account and pay per message.

### Implementation

Use HTTPS with `HTTPClient` to send a POST request to the service’s API endpoint.  Include authentication tokens in the header or URL.

### Cost considerations

SMS messages have a per‑message cost and may incur monthly fees.  Use sparingly for important alerts.

### Use cases

* Alert players when their fish have died or when it’s time to feed them.

## 4. Push notifications

### Options

* **Firebase Cloud Messaging (FCM):** Google’s cross‑platform push notification service.  Requires registering your app and obtaining a server key.
* **Pushover, Pushbullet:** Third‑party services that send notifications to mobile apps.  Provide simple HTTP APIs.
* **IFTTT webhooks:** Trigger actions on other platforms (e.g., Telegram, Slack) via webhooks.

### Setup

Implement server logic (e.g., on a cloud function) that receives events from the ESP32 and sends push notifications via the chosen service.  Alternatively, call the service directly from the ESP32 if the API is simple.

### Use cases

* Notify players about daily rewards or when an in‑game event starts.
* Send reminders to check on the aquarium.

## 5. Data logging/export

### SD card CSV logging

Write game statistics, sensor data or debug information to a CSV file on the SD card.  Use `SD.open()` to append lines.  Players can remove the SD card and load the file into a spreadsheet for analysis.

### Google Sheets API

Use the Google Sheets API to append rows to a spreadsheet.  Authenticate with OAuth 2.0 and send HTTP requests via `HTTPClient`.  This requires a server component or service account credentials, which may be complex on the ESP32.

### MQTT to data platforms

Publish game metrics to an MQTT broker and visualise them on dashboards (e.g., Node‑RED, InfluxDB with Grafana).  This approach is lightweight and real‑time.

### InfluxDB/Grafana dashboards

Send time‑series data (FPS, memory usage, sensor readings) to a backend and display it on Grafana dashboards.  Suitable for development monitoring.

## 6. Physical outputs

### LED strips (WS2812B/NeoPixel)

* **Hardware:** Addressable RGB LEDs controlled by a single digital pin.  Use a level shifter if power is 5 V.
* **Library:** `Adafruit_NeoPixel` or `FastLED`.
* **Use cases:** Visual effects synchronized with game events (e.g., aquarium lights change colour when a boss appears).

### Servo motors

* **Hardware:** Standard 9 g or 3 g servo motors controlled via PWM (e.g., using the LEDC peripheral).  Requires 5 V supply.
* **Use cases:** Move physical objects in response to game actions (e.g., open a treasure chest when you unlock it in the game).

### Relay control

* **Hardware:** Relay modules allow switching mains voltage devices (lamps, pumps).  Use opto‑isolated modules and follow electrical safety practices.
* **Use cases:** Turn on a fountain or aquarium bubbler when a certain fish appears.

### Haptic feedback (vibration motors)

* **Hardware:** Small coin vibration motors driven via a transistor from a GPIO pin.
* **Use cases:** Provide tactile feedback when tapping the screen or capturing a fish.

## 7. Screen capture/sharing

### Capturing TFT to BMP

Implement a function that reads pixel data from the display’s framebuffer or from your sprite buffers and writes a BMP file to SD card.  For a 320×240 RGB565 image, write a BMP header and then the pixel data in BGR565 order.  This allows players to share screenshots.

### Uploading screenshots

Use HTTP POST with multipart/form‑data to upload the BMP to a web server or image hosting API.  Alternatively, send the file as an email attachment.

### Streaming display over network

Send compressed frame data over a WebSocket or UDP to stream the display in real time.  Due to bandwidth limits, compress frames (e.g., JPEG) and reduce resolution or frame rate.

## Summary

The ESP32 CYD can output information and effects through many channels beyond its built‑in screen.  Thermal printers let you print scores and QR codes; email and SMS services provide asynchronous notifications; push notifications integrate with mobile ecosystems; data logging exports gameplay metrics; and physical outputs like LEDs, servos and relays create tangible interactions.  Choose output methods that fit your game’s theme, budget and complexity, and remember to handle power and safety considerations when controlling external hardware.
