#include "ButtonManager.h"
#include <Arduino.h>
#include "Config.h"
#include "AppState.h"
#include "StorageManager.h"
#include "RtcManager.h"
#include "Feedback.h"
#include "DisplayManager.h"
#include "PortalManager.h"

void buttonInit() {
  pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
}

void registerValidClick() {
  if (appendEvent("valid")) {
    markLastPrintNow();
    flashSuccess();
    soundClick();
    app.infoUntilMs = millis() + Config::INFO_SCREEN_MS;
    triggerInvertFlash(180);
    oledOn();
  } else {
    flashError();
  }
}

void updatePortalButtonLogic() {
  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long lastDebounceMs = 0;
  static unsigned long pressedAt = 0;
  static bool longPressHandled = false;
  static bool debugPressHandled = false;

  bool reading = digitalRead(Config::BUTTON_PIN);

  if (reading != lastReading) {
    lastDebounceMs = millis();
    lastReading = reading;
  }

  if (millis() - lastDebounceMs > Config::BUTTON_DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) {
        pressedAt = millis();
        longPressHandled = false;
        debugPressHandled = false;
      } else if (!longPressHandled && !debugPressHandled) {
        touchPortalActivity();
      }
    }
  }

  if (stableState == LOW && !debugPressHandled && millis() - pressedAt >= Config::DEBUG_PRESS_MS) {
    debugPressHandled = true;
    longPressHandled = true;
    showDebugScreenNow();
  } else if (stableState == LOW && !longPressHandled && millis() - pressedAt >= Config::LONG_PRESS_MS) {
    longPressHandled = true;
    stopPortal();
    flashPortal();
  }
}

void updateActiveModeButtonLogic() {
  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long lastDebounceMs = 0;
  static unsigned long pressedAt = 0;
  static bool longPressHandled = false;
  static bool debugPressHandled = false;

  bool reading = digitalRead(Config::BUTTON_PIN);

  if (reading != lastReading) {
    lastDebounceMs = millis();
    lastReading = reading;
  }

  if (millis() - lastDebounceMs > Config::BUTTON_DEBOUNCE_MS) {
    if (reading != stableState) {
      stableState = reading;
      if (stableState == LOW) {
        pressedAt = millis();
        longPressHandled = false;
        debugPressHandled = false;
      } else if (!longPressHandled && !debugPressHandled) {
        registerValidClick();
      }
    }
  }

  if (stableState == LOW && !debugPressHandled && millis() - pressedAt >= Config::DEBUG_PRESS_MS) {
    debugPressHandled = true;
    longPressHandled = true;
    showDebugScreenNow();
  } else if (stableState == LOW && !longPressHandled && millis() - pressedAt >= Config::LONG_PRESS_MS) {
    longPressHandled = true;
    startPortal();
  }
}
