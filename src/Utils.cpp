#include "Utils.h"

String formatDate(const DateTime& dt) {
  char buf[11];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
  return String(buf);
}

String formatMonth(const DateTime& dt) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%04d-%02d", dt.year(), dt.month());
  return String(buf);
}

String formatTime(const DateTime& dt) {
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  return String(buf);
}

String formatDateTime(const DateTime& dt) {
  return formatDate(dt) + " " + formatTime(dt);
}

String dayNameTr(uint8_t dow) {
  switch (dow) {
    case 0: return "Pazar";
    case 1: return "Pazartesi";
    case 2: return "Sali";
    case 3: return "Carsamba";
    case 4: return "Persembe";
    case 5: return "Cuma";
    case 6: return "Cumartesi";
    default: return "?";
  }
}

String monthNameTr(uint8_t month) {
  switch (month) {
    case 1: return "Ocak";
    case 2: return "Subat";
    case 3: return "Mart";
    case 4: return "Nisan";
    case 5: return "Mayis";
    case 6: return "Haziran";
    case 7: return "Temmuz";
    case 8: return "Agustos";
    case 9: return "Eylul";
    case 10: return "Ekim";
    case 11: return "Kasim";
    case 12: return "Aralik";
    default: return "?";
  }
}

String csvEscape(String s) {
  s.replace("\"", "\"\"");
  return "\"" + s + "\"";
}

String htmlEscape(String s) {
  s.replace("&", "&amp;");
  s.replace("<", "&lt;");
  s.replace(">", "&gt;");
  s.replace("\"", "&quot;");
  s.replace("'", "&#39;");
  return s;
}

String urlEncode(const String& s) {
  String out;
  char hex[4];
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    bool safe =
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '-' || c == '_' || c == '.' || c == '~';

    if (safe) out += c;
    else {
      snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)c);
      out += hex;
    }
  }
  return out;
}
