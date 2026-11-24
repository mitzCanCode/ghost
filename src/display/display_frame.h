#ifndef DISPLAY_FRAME_H
#define DISPLAY_FRAME_H

#include <Adafruit_ILI9341.h>

extern Adafruit_ILI9341 tft;

void drawFrameRule(int y, uint16_t color);
void drawFooter();
void drawSharedFrame(const char* title);
void drawMarqueeBorders();

#endif
