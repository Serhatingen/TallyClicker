#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "AppState.h"
#include "RtcManager.h"
#include "StorageManager.h"
#include "Feedback.h"
#include "DisplayManager.h"
#include "PortalManager.h"
#include "ButtonManager.h"

static void initIdentity() {
  uint64_t mac = ESP.getEfuseMac();
  uint32_t shortMac = (uint32_t)(mac & 0xFFFFFFULL);
  app.deviceId = "S2-" + String(shortMac, HEX);
  app.deviceId.toUpperCase();
  app.apName = "Kase-" + app.deviceId;
}

void setup() {
  Serial.begin(115200);
  delay(300);

  initIdentity();
  buttonInit();
  feedbackInit();
  displayInit();
  storageInit();
  rtcInit();
  portalInit();

  if (app.fsOk && rtcTimeReady()) {
    ensureHeaderIfNeeded(getDailyLogPathForNow(rtc.now()),
      "rtc_datetime,business_date,business_day_name,business_hour,event_type,device_id,unix_time");
    ensureHeaderIfNeeded(getMonthlySummaryPathForNow(rtc.now()),
      "business_date,total_entries");
  }

  syncDailyCountersFromRtc();
  app.lastShownEntryCount = app.dailyEntryCount;

  showLoadingScreen();

  if (app.rtcNeedsTimeSync) {
    app.debugUntilMs = millis() + Config::DEBUG_SCREEN_MS;
    flashError();
    soundDebug();
  }
}

void loop() {
  if (rtcTimeReady()) {
    countTodayEntries();
    countCurrentMonthEntries();
  }

  if (app.apActive) {
    handlePortalLoop();
    updatePortalButtonLogic();
    updateScreen();

    if ((long)(millis() - app.debugUntilMs) < 0) clearPixels();
    else if (app.portalClientConnected) portalConnectedLedPulse();
    else portalIdleLedBreath();

    delay(2);
    return;
  }

  updateActiveModeButtonLogic();
  updateScreen();

  if ((long)(millis() - app.debugUntilMs) < 0) clearPixels();
  else if ((long)(millis() - app.infoUntilMs) < 0) okScreenGlowAnimation();
  else idleReadyAnimation();

  if (!app.apActive && (long)(millis() - app.infoUntilMs) >= 0 && (long)(millis() - app.debugUntilMs) >= 0 && app.currentScreen == ScreenMode::None) {
    oledOff();
  }

  delay(1);
}
