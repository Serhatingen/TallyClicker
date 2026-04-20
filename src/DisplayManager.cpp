#include "DisplayManager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "AppState.h"
#include "RtcManager.h"
#include "StorageManager.h"
#include "Utils.h"
#include "Feedback.h"

static Adafruit_SSD1306 display(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, &Wire, -1);

void displayInit() {
  Wire.begin(Config::I2C_SDA, Config::I2C_SCL);
  app.oledOk = display.begin(SSD1306_SWITCHCAPVCC, Config::OLED_ADDR);
  if (app.oledOk) {
    display.clearDisplay();
    display.display();
  }
  oledOff();
}

void oledOn() {
  if (!app.oledOk) return;
  display.ssd1306_command(SSD1306_DISPLAYON);
  delay(2);
}

void oledOff() {
  if (!app.oledOk) return;
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void renderLoadingScreen() {
  if (!app.oledOk) return;
  oledOn();
  display.invertDisplay(false);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(18, 12);
  display.print("LOADING");
  display.setTextSize(1);
  display.setCursor(26, 40);
  display.print("Kase Baslatiliyor");
  display.display();
}

void renderOkScreen(uint32_t entryCount, bool invertNow) {
  if (!app.oledOk) return;
  oledOn();
  display.clearDisplay();
  display.invertDisplay(invertNow);
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("OK");

  display.setTextSize(1);
  display.setCursor(42, 4);
  display.print("Gecerli ay:");
  if (rtcTimeReady()) display.print(monthNameTr(getBusinessDateTime(rtc.now()).month()));
  else display.print("RTC ayarsiz");

  display.drawLine(0, 18, 127, 18, SSD1306_WHITE);
  display.setCursor(0, 23);
  display.print("Gunluk giris:");
  display.print(entryCount);

  display.setCursor(0, 34);
  display.print("Aylik toplam:");
  display.print(app.monthlyEntryCount);

  display.setCursor(0, 45);
  display.print("Son basim:");
  display.setCursor(0, 55);
  display.print(app.lastPrintDateTime == "-" ? "-" : app.lastPrintDateTime);
  display.display();
}

void renderPortalScreen(bool invertNow) {
  if (!app.oledOk) return;
  oledOn();
  display.clearDisplay();
  display.invertDisplay(invertNow);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0); display.print("WiFi PORTAL ACIK");
  display.setCursor(0, 11); display.print(app.apName);
  display.setCursor(0, 22); display.print(WiFi.softAPIP().toString());
  display.setCursor(0, 33); display.print(app.portalClientConnected ? "DURUM: BAGLI" : "DURUM: BEKLIYOR");
  display.setCursor(0, 44); display.print("GUNLUK:"); display.print(app.dailyEntryCount);
  display.setCursor(68, 44); display.print("AYLIK:"); display.print(app.monthlyEntryCount);
  display.setCursor(0, 55);
  if (rtcTimeReady()) display.print(formatTime(rtc.now()));
  else display.print("RTC AYARSIZ");
  display.display();
}

void renderDebugScreen(bool invertNow) {
  if (!app.oledOk) return;
  oledOn();
  display.clearDisplay();
  display.invertDisplay(invertNow);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0); display.print("DEBUG "); display.print(app.deviceId);
  display.setCursor(0, 10); display.print("RTC:"); display.print(rtcTimeReady() ? "OK" : "ERR");
  display.setCursor(62, 10); display.print("WiFi:"); display.print(app.apActive ? (app.portalClientConnected ? "BAGLI" : "AP") : "OFF");

  display.setCursor(0, 20);
  if (rtcTimeReady()) display.print(formatDate(rtc.now()));
  else display.print("RTC AYARSIZ");

  display.setCursor(0, 30);
  if (rtcTimeReady()) display.print(formatTime(rtc.now()));
  else display.print("--:--:--");

  display.setCursor(0, 40); display.print(fsUsageText());
  display.setCursor(74, 40); display.print("CLI:"); display.print(app.apActive ? WiFi.softAPgetStationNum() : 0);

  display.setCursor(0, 54);
  if (app.rtcNeedsTimeSync) display.print("RTC SENK GEREK");
  else {
    display.print("SON:");
    if (app.lastPrintDateTime == "-") display.print("-");
    else if (app.lastPrintDateTime.length() >= 8) display.print(app.lastPrintDateTime.substring(app.lastPrintDateTime.length() - 8));
    else display.print(app.lastPrintDateTime);
  }

  display.display();
}

void triggerInvertFlash(unsigned long ms) { app.invertUntilMs = millis() + ms; }

void showDebugScreenNow() {
  app.debugUntilMs = millis() + Config::DEBUG_SCREEN_MS;
  triggerInvertFlash(300);
  soundDebug();
  oledOn();
}

void showLoadingScreen() {
  app.currentScreen = ScreenMode::Loading;
  renderLoadingScreen();
  startupLedAnimation();
  soundStartupHappy();
  delay(Config::STARTUP_LOADING_MS);
}

void updateScreen() {
  bool invertNow = millis() < app.invertUntilMs;

  if (millis() < app.debugUntilMs) {
    app.currentScreen = ScreenMode::Debug;
    renderDebugScreen(invertNow);
    return;
  }
  if (app.apActive) {
    app.currentScreen = ScreenMode::Portal;
    renderPortalScreen(invertNow);
    return;
  }
  if (millis() < app.infoUntilMs) {
    app.currentScreen = ScreenMode::Ok;
    renderOkScreen(app.lastShownEntryCount, invertNow);
    return;
  }

  app.currentScreen = ScreenMode::None;
  if (app.oledOk) {
    display.invertDisplay(false);
    oledOff();
  }
}
