# ESP32-S2 Clicker / Tally Counter Logger

![PlatformIO](https://img.shields.io/badge/PlatformIO-ready-orange)
![ESP32-S2](https://img.shields.io/badge/MCU-ESP32--S2-blue)
![License](https://img.shields.io/badge/License-MIT-green)

Offline-capable ESP32-S2 tally counter with RTC-based timestamping, OLED UI, NeoPixel feedback, buzzer sounds, LittleFS logging, and a built-in Wi-Fi portal for time sync and CSV export.

## Overview

This project is a portable event logging device for scenarios where each button press must be counted reliably and stored with a timestamp. It is especially suitable for nightclub entry counting, attendance tracking, checkpoint logging, and other manual tally workflows.

The device is designed to keep working without internet access. All data is stored locally on LittleFS and can later be downloaded through the built-in access point portal.

## Main Features

- Physical button input with debounce handling
- DS3231 RTC timestamping
- Business-day cutoff logic for late-night operation
- Daily CSV log files
- Monthly summary CSV files
- OLED status screens
- NeoPixel LED animations
- Buzzer feedback
- Wi-Fi SoftAP portal for maintenance and export
- Local-first, offline-capable operation

## Hardware

- ESP32-S2 Mini
- DS3231 RTC module
- SSD1306 128x64 I2C OLED
- WS2812 / NeoPixel LEDs
- Push button
- Small speaker / buzzer

## Project Structure

```text
include/
  AppState.h
  ButtonManager.h
  Config.h
  DisplayManager.h
  Feedback.h
  PortalManager.h
  RtcManager.h
  StorageManager.h
  Utils.h

src/
  AppState.cpp
  ButtonManager.cpp
  DisplayManager.cpp
  Feedback.cpp
  main.cpp
  PortalManager.cpp
  RtcManager.cpp
  StorageManager.cpp
  Utils.cpp
```

## Data Files

Daily logs:

```csv
rtc_datetime,business_date,business_day_name,business_hour,event_type,device_id,unix_time
```

Monthly summary:

```csv
business_date,total_entries
```

## Typical Workflow

1. Power on the device.
2. Press the button for each valid entry/event.
3. The device confirms each press with LED + sound feedback.
4. Events are stored locally as CSV rows.
5. Long-press to open the Wi-Fi portal.
6. Sync RTC time, inspect status, and download logs from a phone.

## Build

This repository is structured for PlatformIO.

```bash
pio run
```

To upload:

```bash
pio run --target upload
```

To monitor serial output:

```bash
pio device monitor
```

## Notes

- The project is intentionally modular to keep RTC logic, storage, display, input handling, and portal code separated.
- Business-day grouping is useful for nightlife and late-shift environments where activity continues after midnight.
- Local CSV export makes the data easy to import into Excel, Google Sheets, or analytics pipelines.

## Suggested GitHub Topics

`esp32` `esp32-s2` `platformio` `tally-counter` `clicker` `rtc` `oled` `littlefs` `csv-logger` `attendance-tracker`

## License

MIT
