#include "Feedback.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "AppState.h"

static Adafruit_NeoPixel pixels(Config::LED_COUNT, Config::LED_PIN, NEO_GRB + NEO_KHZ800);

static void setAllPixels(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < Config::LED_COUNT; i++) pixels.setPixelColor(i, pixels.Color(r, g, b));
  pixels.show();
}

static void beepTone(int freq, int durationMs) {
  ledcWriteTone(Config::SPEAKER_PIN, freq);
  delay(durationMs);
  ledcWriteTone(Config::SPEAKER_PIN, 0);
  delay(4);
}

void feedbackInit() {
  ledcAttach(Config::SPEAKER_PIN, 2000, Config::AUDIO_RES_BITS);
  ledcWriteTone(Config::SPEAKER_PIN, 0);

  pixels.begin();
  pixels.setBrightness(24);
  clearPixels();
}

void clearPixels() {
  for (int i = 0; i < Config::LED_COUNT; i++) pixels.setPixelColor(i, 0);
  pixels.show();
}

void startupLedAnimation() {
  clearPixels();
  pixels.setPixelColor(1, pixels.Color(0, 0, 10));
  pixels.setPixelColor(2, pixels.Color(0, 0, 10));
  pixels.show();
  delay(60);
  clearPixels();
  pixels.setPixelColor(0, pixels.Color(0, 0, 6));
  pixels.setPixelColor(1, pixels.Color(0, 10, 18));
  pixels.setPixelColor(2, pixels.Color(0, 10, 18));
  pixels.setPixelColor(3, pixels.Color(0, 0, 6));
  pixels.show();
  delay(70);
  clearPixels();
}

void flashSuccess() {
  clearPixels();
  pixels.setPixelColor(0, pixels.Color(0, 8, 0));
  pixels.show();
  delay(10);
  pixels.setPixelColor(1, pixels.Color(0, 16, 0));
  pixels.show();
  delay(10);
  pixels.setPixelColor(2, pixels.Color(0, 24, 0));
  pixels.show();
  delay(10);
  pixels.setPixelColor(3, pixels.Color(8, 32, 8));
  pixels.show();
  delay(18);
  clearPixels();
}

void flashError() {
  setAllPixels(24, 0, 0);
  delay(45);
  clearPixels();
  delay(25);
  setAllPixels(12, 0, 0);
  delay(35);
  clearPixels();
}

void flashPortal() {
  clearPixels();
  pixels.setPixelColor(0, pixels.Color(0, 0, 8));
  pixels.setPixelColor(3, pixels.Color(0, 0, 8));
  pixels.show();
  delay(35);
  pixels.setPixelColor(1, pixels.Color(0, 8, 18));
  pixels.setPixelColor(2, pixels.Color(0, 8, 18));
  pixels.show();
  delay(45);
  clearPixels();
}

void portalIdleLedBreath() {
  static unsigned long lastStep = 0;
  static int level = 1;
  static int delta = 1;
  if (millis() - lastStep < 80) return;
  lastStep = millis();
  level += delta;
  if (level >= 10) delta = -1;
  if (level <= 1) delta = 1;
  pixels.setPixelColor(0, pixels.Color(0, 0, level));
  pixels.setPixelColor(1, pixels.Color(0, 0, level + 2));
  pixels.setPixelColor(2, pixels.Color(0, 0, level + 2));
  pixels.setPixelColor(3, pixels.Color(0, 0, level));
  pixels.show();
}

void portalConnectedLedPulse() {
  static unsigned long lastStep = 0;
  static bool phase = false;
  if (millis() - lastStep < 180) return;
  lastStep = millis();
  phase = !phase;
  if (phase) {
    pixels.setPixelColor(0, pixels.Color(0, 10, 0));
    pixels.setPixelColor(1, pixels.Color(8, 16, 0));
    pixels.setPixelColor(2, pixels.Color(8, 16, 0));
    pixels.setPixelColor(3, pixels.Color(0, 10, 0));
  } else {
    pixels.setPixelColor(0, pixels.Color(0, 2, 0));
    pixels.setPixelColor(1, pixels.Color(2, 5, 0));
    pixels.setPixelColor(2, pixels.Color(2, 5, 0));
    pixels.setPixelColor(3, pixels.Color(0, 2, 0));
  }
  pixels.show();
}

void idleReadyAnimation() {
  if (millis() - app.lastAnimMs < 90) return;
  app.lastAnimMs = millis();
  clearPixels();
  switch (app.idleAnimStep) {
    case 0:
      pixels.setPixelColor(0, pixels.Color(0, 0, 3));
      pixels.setPixelColor(3, pixels.Color(0, 0, 3));
      break;
    case 1:
      pixels.setPixelColor(1, pixels.Color(0, 5, 8));
      pixels.setPixelColor(2, pixels.Color(0, 5, 8));
      break;
    case 2:
      pixels.setPixelColor(0, pixels.Color(0, 3, 1));
      pixels.setPixelColor(1, pixels.Color(0, 8, 3));
      pixels.setPixelColor(2, pixels.Color(0, 8, 3));
      pixels.setPixelColor(3, pixels.Color(0, 3, 1));
      break;
  }
  pixels.show();
  app.idleAnimStep = (app.idleAnimStep + 1) & 0x03;
}

void okScreenGlowAnimation() {
  if (millis() - app.lastAnimMs < 120) return;
  app.lastAnimMs = millis();
  uint8_t a = app.okAnimPhase % 6;
  uint8_t lv = (a < 3) ? (4 + a * 4) : (16 - (a - 3) * 4);
  pixels.setPixelColor(0, pixels.Color(0, lv, 0));
  pixels.setPixelColor(1, pixels.Color(0, lv + 4, 0));
  pixels.setPixelColor(2, pixels.Color(0, lv + 4, 0));
  pixels.setPixelColor(3, pixels.Color(0, lv, 0));
  pixels.show();
  app.okAnimPhase = (app.okAnimPhase + 1) % 6;
}

void soundClick() {
  beepTone(1047, 28);
  delay(6);
  beepTone(1319, 34);
}

void soundStartupHappy() {
  beepTone(580, 60); delay(12);
  beepTone(775, 55); delay(12);
  beepTone(1068, 70); delay(12);
  beepTone(1175, 55); delay(12);
  beepTone(1568, 70); delay(12);
  beepTone(1175, 55); delay(12);
  beepTone(1568, 70);
}

void soundConnected() {
  beepTone(988, 45); delay(8);
  beepTone(1319, 45); delay(8);
  beepTone(1760, 55);
}

void soundHeartbeatIdle() { beepTone(110, 14); }

void soundHeartbeatConnected() {
  beepTone(988, 18); delay(10);
  beepTone(1319, 20);
}

void soundDebug() {
  beepTone(740, 40); delay(10);
  beepTone(880, 45); delay(10);
  beepTone(988, 55);
}
