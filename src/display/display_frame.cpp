#include "display_config.h"
#include "display_frame.h"

void drawFrameRule(int y, uint16_t color) {
    tft.drawFastHLine(FRAME_X, y, FRAME_W, color);
}

void drawFooter() {
    tft.setTextSize(1);
    tft.setTextColor(PINK);

    tft.setCursor(8, FOOTER_TEXT_Y);
    tft.print("UP/DOWN: MOVE");

    tft.setCursor(130, FOOTER_TEXT_Y);
    tft.print("CENTER: SELECT");

    tft.setCursor(253, FOOTER_TEXT_Y);
    tft.print("LEFT: BACK");
}

void drawSharedFrame(const char* title) {
    tft.fillScreen(BLACK);

    tft.setTextSize(1);
    tft.setTextColor(PINK);

    tft.setCursor(8, STATUS_Y);
    tft.print("ghost.");

    tft.fillCircle(265, STATUS_Y + 3, 3, PINK);

    tft.setCursor(275, STATUS_Y);
    tft.print("ONLINE");

    drawFrameRule(STATUS_RULE_Y, PINK);

    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(8, TITLE_Y);
    tft.print(title);

    drawFrameRule(TITLE_RULE_Y, DARK_PINK);

    drawMarqueeBorders();

    drawFooter();
}
