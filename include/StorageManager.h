#pragma once
#include <Arduino.h>

bool storageInit();
float fsUsagePercent();
String fsUsageText();

String getDailyLogPathForNow(const DateTime& nowLocal);
String getMonthlySummaryPathForNow(const DateTime& nowLocal);
String getMonthFromDailyPath(const String& path);
String getDateFromDailyPath(const String& path);
String normalizePath(String path);

void ensureHeaderIfNeeded(const String& path, const String& headerLine);
bool appendLine(const String& path, const String& line);
uint32_t countEventsInFile(const String& path);
uint32_t countMonthEntriesByMonthString(const String& monthStr);
uint32_t countMonthEntriesFromSummaryFile(const String& monthlyPath);

void syncDailyCountersFromRtc();
uint32_t countTodayEntries();
uint32_t countCurrentMonthEntries();
bool rebuildMonthlySummary(const String& businessMonth, const String& monthlyPath, const String& currentDay);
bool appendEvent(const char* eventType);

String buildDailyRows();
String buildMonthlyRows();
bool deleteAllLogs();
