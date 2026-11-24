#ifndef DISPLAY_FRAME_H
#define DISPLAY_FRAME_H

#include <Adafruit_ILI9341.h>
#include "../../shared.h"

extern Adafruit_ILI9341 tft;

void drawFrameRule(int y, uint16_t color);
void drawFooter();
void drawSharedFrame(const char* title);
void drawMarqueeBorders();
void setStatusBarStatus(
    DeviceStatus wifi,
    DeviceStatus bluetooth,
    DeviceStatus ethernet,
    StorageStatus sd
);

#endif
