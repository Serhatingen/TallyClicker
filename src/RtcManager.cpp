#include "RtcManager.h"
#include "AppState.h"
#include "Config.h"
#include "Utils.h"

RTC_DS3231 rtc;

bool rtcInit() {
  app.rtcOk = rtc.begin();
  if (!app.rtcOk) {
    app.rtcNeedsTimeSync = true;
    app.lastPrintDateTime = "RTC yok";
    return false;
  }

  DateTime now = rtc.now();
  if (rtc.lostPower() || !isRtcTimeValid(now)) {
    app.rtcNeedsTimeSync = true;
    app.lastPrintDateTime = "RTC ayarsiz";
    return false;
  }

  app.rtcNeedsTimeSync = false;
  return true;
}

bool isRtcTimeValid(const DateTime& t) {
  if (t.year() < 2024 || t.year() > 2099) return false;
  if (t.month() < 1 || t.month() > 12) return false;
  if (t.day() < 1 || t.day() > 31) return false;
  if (t.hour() > 23) return false;
  if (t.minute() > 59) return false;
  if (t.second() > 59) return false;
  return true;
}

bool rtcTimeReady() {
  if (!app.rtcOk) return false;
  if (app.rtcNeedsTimeSync) return false;
  return isRtcTimeValid(rtc.now());
}

DateTime getBusinessDateTime(const DateTime& nowLocal) {
  return nowLocal;
}

int getBusinessHour(const DateTime& nowLocal) {
  return nowLocal.hour();
}

bool adjustRtcFromLocalParts(int year, int month, int day, int hour, int minute, int second) {
  if (!app.rtcOk) return false;
  DateTime dt(year, month, day, hour, minute, second);
  if (!isRtcTimeValid(dt)) return false;
  rtc.adjust(dt);
  app.rtcNeedsTimeSync = false;
  return true;
}

void markLastPrintNow() {
  if (rtcTimeReady()) app.lastPrintDateTime = formatDateTime(rtc.now());
  else app.lastPrintDateTime = "RTC ayarsiz";
}
