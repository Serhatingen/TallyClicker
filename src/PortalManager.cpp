#include "PortalManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include "Config.h"
#include "AppState.h"
#include "RtcManager.h"
#include "StorageManager.h"
#include "DisplayManager.h"
#include "Feedback.h"
#include "Utils.h"

static WebServer web(80);
static DNSServer dns;

void touchPortalActivity() { app.portalLastActivityAt = millis(); }

static void setupRoutes() {
  web.on("/sync-time", HTTP_GET, []() {
    touchPortalActivity();
    if (!web.hasArg("y") || !web.hasArg("mo") || !web.hasArg("day") ||
        !web.hasArg("h") || !web.hasArg("mi") || !web.hasArg("s")) {
      web.send(400, "application/json", "{\"ok\":false,\"msg\":\"eksik parametre\"}");
      return;
    }

    bool ok = adjustRtcFromLocalParts(
      web.arg("y").toInt(), web.arg("mo").toInt(), web.arg("day").toInt(),
      web.arg("h").toInt(), web.arg("mi").toInt(), web.arg("s").toInt()
    );

    if (ok) {
      syncDailyCountersFromRtc();
      app.lastShownEntryCount = app.dailyEntryCount;
      triggerInvertFlash(400);
      soundConnected();
      web.send(200, "application/json", "{\"ok\":true}");
    } else {
      web.send(400, "application/json", "{\"ok\":false,\"msg\":\"gecersiz zaman\"}");
    }
  });

  web.on("/", HTTP_GET, []() {
    touchPortalActivity();
    if (rtcTimeReady()) {
      countTodayEntries();
      countCurrentMonthEntries();
    }

    bool rtcValid = rtcTimeReady();
    String nowStr = rtcValid ? formatDateTime(rtc.now()) : "AYARSIZ";
    String currentDaily = rtcValid ? getDailyLogPathForNow(rtc.now()) : "RTC ayarsiz";
    String currentMonth = rtcValid ? getMonthlySummaryPathForNow(rtc.now()) : "RTC ayarsiz";

    String html;
    html += "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<title>Kase Logger</title><style>";
    html += "body{font-family:Arial;background:#111;color:#fff;margin:0;padding:16px}";
    html += ".box{max-width:1000px;margin:auto}.card{background:#1b1b1b;border:1px solid #333;border-radius:14px;padding:16px;margin-bottom:14px}";
    html += "table{width:100%;border-collapse:collapse}th,td{padding:10px;border-bottom:1px solid #333;text-align:left}";
    html += "a{color:#8ecbff;text-decoration:none}button{padding:10px 14px;border:none;border-radius:10px;background:#1f7aff;color:#fff;font-weight:bold;cursor:pointer}";
    html += ".danger{background:#a32626}.ok{color:#77e58b}.bad{color:#ff8686}</style></head><body><div class='box'>";

    html += "<div class='card'><h2>Kase Logger</h2>";
    html += "<p><b>Device ID:</b> " + htmlEscape(app.deviceId) + "</p>";
    html += "<p><b>SSID:</b> " + htmlEscape(app.apName) + "</p>";
    html += "<p><b>IP:</b> " + WiFi.softAPIP().toString() + "</p>";
    html += "<p><b>RTC:</b> <span class='" + String(rtcValid ? "ok" : "bad") + "'>" + String(rtcValid ? "OK" : "AYARSIZ") + "</span></p>";
    html += "<p><b>Saat:</b> " + htmlEscape(nowStr) + "</p>";
    html += "<p><b>Bugunku Giris:</b> " + String(app.dailyEntryCount) + "</p>";
    html += "<p><b>Bu Ay Giris:</b> " + String(app.monthlyEntryCount) + "</p>";
    html += "<p><b>Son Basim:</b> " + htmlEscape(app.lastPrintDateTime) + "</p>";
    html += "<p><b>FS Doluluk:</b> " + htmlEscape(fsUsageText()) + "</p>";
    html += "<p><b>Aktif Gunluk:</b> " + htmlEscape(currentDaily) + "</p>";
    html += "<p><b>Aktif Aylik:</b> " + htmlEscape(currentMonth) + "</p>";
    if (!rtcValid) html += "<p class='bad'><b>Uyari:</b> RTC saati gecersiz veya enerji kaybi yasamis. Telefonden saat esitle.</p>";
    html += "<p><button onclick='syncTimeNow()'>Saati telefonumdan esitle</button></p>";
    html += "<p><a href='/log-now'>Test log yaz</a></p>";
    html += "<p><a href='/download-all-logs'>Tum loglari tek dosyada indir</a></p></div>";

    html += "<div class='card'><h3>Aylik Dosyalar</h3><table><thead><tr><th>Ay</th><th>Toplam</th><th>Ozet</th><th>Toplu</th></tr></thead><tbody>";
    html += buildMonthlyRows();
    html += "</tbody></table></div>";

    html += "<div class='card'><h3>Gunluk Dosyalar</h3><table><thead><tr><th>Tarih</th><th>Giris</th><th>Indir</th></tr></thead><tbody>";
    html += buildDailyRows();
    html += "</tbody></table></div>";

    html += "<div class='card'><form action='/delete-all' method='POST' onsubmit='return confirm(\"Tum loglar silinsin mi?\")'>";
    html += "<button class='danger' type='submit'>Tum loglari sil</button></form></div>";

    html += "<script>async function syncTimeNow(){const n=new Date();";
    html += "const url=`/sync-time?y=${n.getFullYear()}&mo=${n.getMonth()+1}&day=${n.getDate()}&h=${n.getHours()}&mi=${n.getMinutes()}&s=${n.getSeconds()}`;";
    html += "try{const r=await fetch(url,{cache:'no-store'});const j=await r.json();if(j.ok){location.reload();}else{alert('Saat senkronu basarisiz');}}catch(e){alert('Saat senkronu basarisiz');}}</script>";
    html += "</div></body></html>";

    web.send(200, "text/html; charset=utf-8", html);
  });

  web.on("/download", HTTP_GET, []() {
    touchPortalActivity();
    if (!web.hasArg("file")) { web.send(400, "text/plain", "file gerekli"); return; }
    String path = normalizePath(web.arg("file"));
    if (path.isEmpty()) { web.send(400, "text/plain", "gecersiz dosya"); return; }
    if (!LittleFS.exists(path)) { web.send(404, "text/plain", "dosya yok"); return; }
    File f = LittleFS.open(path, FILE_READ);
    if (!f) { web.send(500, "text/plain", "dosya acilamadi"); return; }
    web.sendHeader("Content-Disposition", "attachment; filename=\"" + path.substring(1) + "\"");
    web.streamFile(f, "text/csv");
    f.close();
  });

  web.on("/download-month-bulk", HTTP_GET, []() {
    touchPortalActivity();
    if (!web.hasArg("month")) { web.send(400, "text/plain", "month gerekli"); return; }
    String monthStr = web.arg("month");

    if (monthStr.length() != 7 || monthStr.charAt(4) != '-') {
      web.send(400, "text/plain", "gecersiz ay parametresi");
      return;
    }
    for (int i = 0; i < 7; i++) {
      if (i == 4) continue;
      if (!isDigit(monthStr.charAt(i))) {
        web.send(400, "text/plain", "gecersiz ay parametresi");
        return;
      }
    }

    if (!app.fsOk) { web.send(503, "text/plain", "dosya sistemi hazir degil"); return; }

    web.sendHeader("Content-Disposition", "attachment; filename=\"bulk_" + monthStr + ".csv\"");
    web.setContentLength(CONTENT_LENGTH_UNKNOWN);
    web.send(200, "text/csv", "");
    web.sendContent("source_file,rtc_datetime,business_date,business_day_name,business_hour,event_type,device_id,unix_time\n");

    File root = LittleFS.open("/");
    if (root) {
      File file = root.openNextFile();
      while (file) {
        String name = file.name();
        if (!file.isDirectory() && name.startsWith("/day_") && getMonthFromDailyPath(name) == monthStr) {
          File f = LittleFS.open(name, FILE_READ);
          if (f) {
            bool firstLine = true;
            while (f.available()) {
              String line = f.readStringUntil('\n');
              line.trim();
              if (line.isEmpty()) continue;
              if (firstLine) { firstLine = false; continue; }
              web.sendContent(name.substring(1) + "," + line + "\n");
            }
            f.close();
          }
        }
        file = root.openNextFile();
      }
      root.close();
    }
    web.sendContent("");
  });

  web.on("/download-all-logs", HTTP_GET, []() {
    touchPortalActivity();
    if (!app.fsOk) { web.send(503, "text/plain", "dosya sistemi hazir degil"); return; }

    web.sendHeader("Content-Disposition", "attachment; filename=\"all_logs.csv\"");
    web.setContentLength(CONTENT_LENGTH_UNKNOWN);
    web.send(200, "text/csv", "");
    web.sendContent("file_name,content\n");

    File root = LittleFS.open("/");
    if (root) {
      File file = root.openNextFile();
      while (file) {
        String name = file.name();
        if (!file.isDirectory() && (name.startsWith("/day_") || name.startsWith("/month_"))) {
          File f = LittleFS.open(name, FILE_READ);
          if (f) {
            while (f.available()) {
              String line = f.readStringUntil('\n');
              line.trim();
              if (!line.isEmpty()) web.sendContent(csvEscape(name) + "," + csvEscape(line) + "\n");
            }
            f.close();
          }
        }
        file = root.openNextFile();
      }
      root.close();
    }
    web.sendContent("");
  });

  web.on("/delete-all", HTTP_POST, []() {
    touchPortalActivity();
    deleteAllLogs();
    web.sendHeader("Location", "/", true);
    web.send(302, "text/plain", "");
  });

  web.on("/log-now", HTTP_GET, []() {
    touchPortalActivity();
    if (appendEvent("manual_test")) {
      markLastPrintNow();
      app.lastShownEntryCount = countTodayEntries();
      app.infoUntilMs = millis() + Config::INFO_SCREEN_MS;
      triggerInvertFlash(250);
      soundClick();
    }
    web.sendHeader("Location", "/", true);
    web.send(302, "text/plain", "");
  });

  web.onNotFound([]() {
    touchPortalActivity();
    web.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    web.send(302, "text/plain", "");
  });
}

void portalInit() { setupRoutes(); }

void stopPortal() {
  if (!app.apActive) return;
  dns.stop();
  web.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  app.apActive = false;
  app.portalClientConnected = false;
  app.lastPortalClientConnected = false;
  oledOff();
  clearPixels();
}

void startPortal() {
  if (app.apActive) return;

  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  delay(120);
  WiFi.mode(WIFI_OFF);
  delay(120);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  delay(120);

  if (!WiFi.softAP(app.apName.c_str(), Config::AP_PASSWORD, 1, 0, 4)) {
    flashError();
    soundDebug();
    app.debugUntilMs = millis() + 3000;
    return;
  }

  delay(150);
  dns.start(53, "*", WiFi.softAPIP());
  web.begin();

  app.apActive = true;
  app.portalClientConnected = false;
  app.lastPortalClientConnected = false;
  touchPortalActivity();

  flashPortal();
  triggerInvertFlash(180);
  oledOn();
}

void updatePortalClientStatus() {
  if (!app.apActive) return;
  if (millis() - app.lastClientCheckMs < Config::CLIENT_CHECK_MS) return;

  app.lastClientCheckMs = millis();
  app.portalClientConnected = (WiFi.softAPgetStationNum() > 0);

  if (app.portalClientConnected != app.lastPortalClientConnected) {
    app.lastPortalClientConnected = app.portalClientConnected;
    triggerInvertFlash(400);
    soundConnected();
  }
}

void handlePortalLoop() {
  if (!app.apActive) return;

  dns.processNextRequest();
  web.handleClient();
  updatePortalClientStatus();

  unsigned long interval = app.portalClientConnected ? Config::HEARTBEAT_CONNECTED_MS : Config::HEARTBEAT_IDLE_MS;
  if (millis() - app.lastHeartbeatMs >= interval) {
    app.lastHeartbeatMs = millis();
    if (app.portalClientConnected) soundHeartbeatConnected();
    else soundHeartbeatIdle();
  }

  if (millis() - app.portalLastActivityAt >= Config::PORTAL_AUTO_OFF_MS) {
    stopPortal();
  }
}
