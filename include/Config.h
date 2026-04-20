#pragma once
#include <Arduino.h>

namespace Config {
  constexpr int I2C_SDA = 7;
  constexpr int I2C_SCL = 6;
  constexpr int BUTTON_PIN = 3;
  constexpr int LED_PIN = 4;
  constexpr int LED_COUNT = 4;
  constexpr int SPEAKER_PIN = 5;

  constexpr int SCREEN_WIDTH = 128;
  constexpr int SCREEN_HEIGHT = 64;
  constexpr uint8_t OLED_ADDR = 0x3C;
  constexpr int AUDIO_RES_BITS = 8;

  constexpr int CLUB_DAY_CUTOFF_HOUR = 0;

  constexpr unsigned long BUTTON_DEBOUNCE_MS = 18;
  constexpr unsigned long LONG_PRESS_MS = 3000;
  constexpr unsigned long DEBUG_PRESS_MS = 10000;
  constexpr unsigned long INFO_SCREEN_MS = 2200;
  constexpr unsigned long DEBUG_SCREEN_MS = 5000;
  constexpr unsigned long PORTAL_AUTO_OFF_MS = 120000;
  constexpr unsigned long STARTUP_LOADING_MS = 1800;
  constexpr unsigned long HEARTBEAT_IDLE_MS = 3000;
  constexpr unsigned long HEARTBEAT_CONNECTED_MS = 1000;
  constexpr unsigned long CLIENT_CHECK_MS = 500;

  constexpr const char* AP_PASSWORD = "clicker123";
}
