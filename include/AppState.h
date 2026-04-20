#pragma once
#include <Arduino.h>

enum class ScreenMode {
  None,
  Loading,
  Ok,
  Portal,
  Debug
};

struct AppState {
  bool fsOk = false;
  bool rtcOk = false;
  bool oledOk = false;
  bool apActive = false;
  bool rtcNeedsTimeSync = false;

  bool portalClientConnected = false;
  bool lastPortalClientConnected = false;

  unsigned long portalLastActivityAt = 0;
  unsigned long infoUntilMs = 0;
  unsigned long debugUntilMs = 0;
  unsigned long invertUntilMs = 0;
  unsigned long lastHeartbeatMs = 0;
  unsigned long lastClientCheckMs = 0;
  unsigned long lastAnimMs = 0;

  uint32_t dailyEntryCount = 0;
  uint32_t monthlyEntryCount = 0;
  uint32_t lastShownEntryCount = 0;

  String currentBusinessDate = "";
  String currentBusinessMonth = "";
  String lastPrintDateTime = "-";
  String deviceId;
  String apName;

  uint8_t idleAnimStep = 0;
  uint8_t okAnimPhase = 0;

  ScreenMode currentScreen = ScreenMode::None;
};

extern AppState app;
