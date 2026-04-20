#pragma once
#include <RTClib.h>

extern RTC_DS3231 rtc;

bool rtcInit();
bool isRtcTimeValid(const DateTime& t);
bool rtcTimeReady();
DateTime getBusinessDateTime(const DateTime& nowLocal);
int getBusinessHour(const DateTime& nowLocal);
bool adjustRtcFromLocalParts(int year, int month, int day, int hour, int minute, int second);
void markLastPrintNow();
