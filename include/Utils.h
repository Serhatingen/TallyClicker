#pragma once
#include <Arduino.h>
#include <RTClib.h>

String formatDate(const DateTime& dt);
String formatMonth(const DateTime& dt);
String formatTime(const DateTime& dt);
String formatDateTime(const DateTime& dt);
String dayNameTr(uint8_t dow);
String monthNameTr(uint8_t month);
String csvEscape(String s);
String htmlEscape(String s);
String urlEncode(const String& s);
