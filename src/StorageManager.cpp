#include "StorageManager.h"
#include <LittleFS.h>
#include "AppState.h"
#include "RtcManager.h"
#include "Utils.h"

bool storageInit() {
  app.fsOk = LittleFS.begin(true);
  return app.fsOk;
}

float fsUsagePercent() {
  if (!app.fsOk) return 0.0f;
  size_t total = LittleFS.totalBytes();
  if (total == 0) return 0.0f;
  size_t used = LittleFS.usedBytes();
  return (100.0f * (float)used) / (float)total;
}

String fsUsageText() {
  if (!app.fsOk) return "FS:YOK";
  char buf[20];
  snprintf(buf, sizeof(buf), "FS:%0.1f%%", fsUsagePercent());
  return String(buf);
}

String getDailyLogPathForNow(const DateTime& nowLocal) {
  DateTime business = getBusinessDateTime(nowLocal);
  return "/day_" + formatDate(business) + ".csv";
}

String getMonthlySummaryPathForNow(const DateTime& nowLocal) {
  DateTime business = getBusinessDateTime(nowLocal);
  return "/month_" + formatMonth(business) + "_summary.csv";
}

String getMonthFromDailyPath(const String& path) {
  if (!path.startsWith("/day_") || path.length() < 15) return "";
  return path.substring(5, 12);
}

String getDateFromDailyPath(const String& path) {
  if (!path.startsWith("/day_") || path.length() < 15) return "";
  return path.substring(5, 15);
}

String normalizePath(String path) {
  path.trim();
  if (!path.startsWith("/")) path = "/" + path;
  if (path.startsWith("/day_") && path.endsWith(".csv")) return path;
  if (path.startsWith("/month_") && path.endsWith("_summary.csv")) return path;
  return "";
}

void ensureHeaderIfNeeded(const String& path, const String& headerLine) {
  if (!app.fsOk) return;
  if (!LittleFS.exists(path)) {
    File f = LittleFS.open(path, FILE_WRITE);
    if (!f) return;
    f.println(headerLine);
    f.close();
  }
}

bool appendLine(const String& path, const String& line) {
  if (!app.fsOk) return false;
  File f = LittleFS.open(path, FILE_APPEND);
  if (!f) return false;
  f.println(line);
  f.close();
  return true;
}

uint32_t countEventsInFile(const String& path) {
  if (!app.fsOk || !LittleFS.exists(path)) return 0;
  File f = LittleFS.open(path, FILE_READ);
  if (!f) return 0;

  uint32_t count = 0;
  bool firstLine = true;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;
    if (firstLine) {
      firstLine = false;
      continue;
    }
    count++;
  }
  f.close();
  return count;
}

uint32_t countMonthEntriesByMonthString(const String& monthStr) {
  if (!app.fsOk) return 0;
  File root = LittleFS.open("/");
  if (!root) return 0;

  uint32_t total = 0;
  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    if (!file.isDirectory() && name.startsWith("/day_") && getMonthFromDailyPath(name) == monthStr) {
      total += countEventsInFile(name);
    }
    file = root.openNextFile();
  }
  root.close();
  return total;
}

uint32_t countMonthEntriesFromSummaryFile(const String& monthlyPath) {
  if (!app.fsOk || !LittleFS.exists(monthlyPath)) return 0;
  File f = LittleFS.open(monthlyPath, FILE_READ);
  if (!f) return 0;

  uint32_t total = 0;
  bool firstLine = true;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;
    if (firstLine) {
      firstLine = false;
      continue;
    }
    int comma = line.indexOf(',');
    if (comma < 0) continue;
    total += (uint32_t) line.substring(comma + 1).toInt();
  }
  f.close();
  return total;
}

void syncDailyCountersFromRtc() {
  if (!rtcTimeReady()) {
    app.dailyEntryCount = 0;
    app.monthlyEntryCount = 0;
    app.currentBusinessDate = "";
    app.currentBusinessMonth = "";
    return;
  }

  DateTime nowLocal = rtc.now();
  DateTime business = getBusinessDateTime(nowLocal);
  app.currentBusinessDate = formatDate(business);
  app.currentBusinessMonth = formatMonth(business);

  String dailyPath = getDailyLogPathForNow(nowLocal);
  String monthlyPath = getMonthlySummaryPathForNow(nowLocal);

  app.dailyEntryCount = countEventsInFile(dailyPath);
  app.monthlyEntryCount = countMonthEntriesFromSummaryFile(monthlyPath);

  if (app.monthlyEntryCount == 0 && LittleFS.exists(dailyPath)) {
    app.monthlyEntryCount = countMonthEntriesByMonthString(app.currentBusinessMonth);
  }
}

uint32_t countTodayEntries() {
  if (!rtcTimeReady()) return 0;
  DateTime business = getBusinessDateTime(rtc.now());
  String d = formatDate(business);
  String m = formatMonth(business);
  if (d != app.currentBusinessDate || m != app.currentBusinessMonth) syncDailyCountersFromRtc();
  return app.dailyEntryCount;
}

uint32_t countCurrentMonthEntries() {
  if (!rtcTimeReady()) return 0;
  DateTime business = getBusinessDateTime(rtc.now());
  String d = formatDate(business);
  String m = formatMonth(business);
  if (d != app.currentBusinessDate || m != app.currentBusinessMonth) syncDailyCountersFromRtc();
  return app.monthlyEntryCount;
}

bool rebuildMonthlySummary(const String& businessMonth, const String& monthlyPath, const String& currentDay) {
  if (!app.fsOk) return false;

  static const char* tmpPath = "/~tmp_monthly.csv";

  if (LittleFS.exists(tmpPath)) LittleFS.remove(tmpPath);

  File mf = LittleFS.open(tmpPath, FILE_WRITE);
  if (!mf) return false;
  mf.println("business_date,total_entries");

  File root = LittleFS.open("/");
  if (!root) {
    mf.close();
    LittleFS.remove(tmpPath);
    return false;
  }

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    if (!file.isDirectory() && name.startsWith("/day_") && getMonthFromDailyPath(name) == businessMonth) {
      String d = getDateFromDailyPath(name);
      uint32_t countForDay = (d == currentDay) ? app.dailyEntryCount : countEventsInFile(name);
      mf.println(d + "," + String(countForDay));
    }
    file = root.openNextFile();
  }
  root.close();
  mf.close();

  LittleFS.remove(monthlyPath);
  if (!LittleFS.rename(tmpPath, monthlyPath)) {
    LittleFS.remove(tmpPath);
    return false;
  }

  app.monthlyEntryCount = countMonthEntriesFromSummaryFile(monthlyPath);
  return true;
}

bool appendEvent(const char* eventType) {
  if (!app.fsOk || !rtcTimeReady()) return false;

  DateTime nowLocal = rtc.now();
  DateTime business = getBusinessDateTime(nowLocal);
  String businessDate = formatDate(business);
  String businessMonth = formatMonth(business);

  if (businessDate != app.currentBusinessDate || businessMonth != app.currentBusinessMonth) syncDailyCountersFromRtc();

  String dailyPath = getDailyLogPathForNow(nowLocal);
  String monthlyPath = getMonthlySummaryPathForNow(nowLocal);

  ensureHeaderIfNeeded(dailyPath, "rtc_datetime,business_date,business_day_name,business_hour,event_type,device_id,unix_time");
  ensureHeaderIfNeeded(monthlyPath, "business_date,total_entries");

  String line;
  line += csvEscape(formatDateTime(nowLocal));
  line += "," + csvEscape(businessDate);
  line += "," + csvEscape(dayNameTr(business.dayOfTheWeek()));
  line += "," + String(getBusinessHour(nowLocal));
  line += "," + csvEscape(String(eventType));
  line += "," + csvEscape(app.deviceId);
  line += "," + String((uint32_t) nowLocal.unixtime());

  if (!appendLine(dailyPath, line)) return false;

  app.dailyEntryCount++;
  app.currentBusinessDate = businessDate;
  app.currentBusinessMonth = businessMonth;
  app.lastShownEntryCount = app.dailyEntryCount;

  return rebuildMonthlySummary(businessMonth, monthlyPath, businessDate);
}

String buildDailyRows() {
  if (!app.fsOk) return "<tr><td colspan='3'>LittleFS yok</td></tr>";

  String rows;
  File root = LittleFS.open("/");
  if (!root) return "<tr><td colspan='3'>Klasor acilamadi</td></tr>";

  File file = root.openNextFile();
  bool found = false;
  while (file) {
    String name = file.name();
    if (!file.isDirectory() && name.startsWith("/day_")) {
      found = true;
      rows += "<tr><td>" + htmlEscape(name.substring(5, 15)) + "</td>";
      rows += "<td>" + String(countEventsInFile(name)) + "</td>";
      rows += "<td><a href='/download?file=" + urlEncode(name) + "'>Indir</a></td></tr>";
    }
    file = root.openNextFile();
  }
  root.close();

  if (!found) rows = "<tr><td colspan='3'>Gunluk log yok</td></tr>";
  return rows;
}

String buildMonthlyRows() {
  if (!app.fsOk) return "<tr><td colspan='4'>LittleFS yok</td></tr>";

  String rows;
  File root = LittleFS.open("/");
  if (!root) return "<tr><td colspan='4'>Klasor acilamadi</td></tr>";

  File file = root.openNextFile();
  bool found = false;
  while (file) {
    String name = file.name();
    if (!file.isDirectory() && name.startsWith("/month_") && name.endsWith("_summary.csv")) {
      found = true;
      String monthStr = name.substring(7, 14);
      rows += "<tr><td>" + htmlEscape(monthStr) + "</td>";
      rows += "<td>" + String(countMonthEntriesFromSummaryFile(name)) + "</td>";
      rows += "<td><a href='/download?file=" + urlEncode(name) + "'>Ozet CSV</a></td>";
      rows += "<td><a href='/download-month-bulk?month=" + urlEncode(monthStr) + "'>Toplu Indir</a></td></tr>";
    }
    file = root.openNextFile();
  }
  root.close();

  if (!found) rows = "<tr><td colspan='4'>Aylik log yok</td></tr>";
  return rows;
}

bool deleteAllLogs() {
  if (!app.fsOk) return false;

  String pathsToDelete;
  File root = LittleFS.open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      String name = file.name();
      if (!file.isDirectory()) pathsToDelete += name + "\n";
      file = root.openNextFile();
    }
    root.close();
  }

  int start = 0;
  while (start < (int)pathsToDelete.length()) {
    int end = pathsToDelete.indexOf('\n', start);
    if (end < 0) end = pathsToDelete.length();
    String name = pathsToDelete.substring(start, end);
    name.trim();
    if (!name.isEmpty()) LittleFS.remove(name);
    start = end + 1;
  }

  syncDailyCountersFromRtc();
  app.lastShownEntryCount = app.dailyEntryCount;
  app.lastPrintDateTime = "-";
  return true;
}
