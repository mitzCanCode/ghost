// ---------------- GLOBAL NOTIFICATION MARQUEE ----------------
// Change this one string (or call setNotificationText()) to update every
// screen.  The marquee has its own small off-screen buffer and is redrawn
// after a frame refresh, so screen redraws never permanently erase it.

#include <Arduino.h>
#include <string.h>
#include <Adafruit_GFX.h>

#include "display_config.h"
#include "display_frame.h"
#include "display_marquee.h"


char notificationText[NOTIFICATION_MAX_CHARS] = "   MEGA TERMINAL   -   SYSTEM ONLINE   -   BLACK & PINK EDITION   ";


GFXcanvas1 marqueeCanvas(FRAME_W, MARQUEE_MAX_HEIGHT);
int marqueeX = SCREEN_W;
unsigned long lastMarqueeUpdate = 0;

void drawMarqueeBorders() {
  drawFrameRule(MARQUEE_Y - 3, DARK_PINK);
  drawFrameRule(MARQUEE_Y + MARQUEE_H + 3, DARK_PINK);
}

void drawMarquee() {
  marqueeCanvas.fillScreen(0);
  marqueeCanvas.setTextSize(1);
  marqueeCanvas.setTextWrap(false);
  marqueeCanvas.setTextColor(1);
  marqueeCanvas.setCursor(marqueeX - FRAME_X, (MARQUEE_H - 8) / 2);
  marqueeCanvas.print(notificationText);

  tft.drawBitmap(FRAME_X, MARQUEE_Y, marqueeCanvas.getBuffer(),
                 FRAME_W, MARQUEE_H, PINK, BLACK);
}

void setNotificationText(const char* text) {

    if (text == NULL) {
        notificationText[0] = '\0';
        return;
    }

    strncpy(
        notificationText,
        text,
        NOTIFICATION_MAX_CHARS - 1
    );

    notificationText[NOTIFICATION_MAX_CHARS - 1] = '\0';

    marqueeX = SCREEN_W;
    drawMarquee();
}

void updateMarquee() {
  if (millis() - lastMarqueeUpdate < MARQUEE_SPEED) return;
  lastMarqueeUpdate = millis();

  marqueeX -= 2;
  int textWidth = strlen(notificationText) * 6;
  if (marqueeX < -textWidth) marqueeX = SCREEN_W;

  drawMarquee();
}
