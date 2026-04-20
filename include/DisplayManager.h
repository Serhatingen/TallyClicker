#pragma once

void displayInit();
void oledOn();
void oledOff();
void renderLoadingScreen();
void renderOkScreen(uint32_t entryCount, bool invertNow);
void renderPortalScreen(bool invertNow);
void renderDebugScreen(bool invertNow);
void triggerInvertFlash(unsigned long ms);
void showDebugScreenNow();
void showLoadingScreen();
void updateScreen();
