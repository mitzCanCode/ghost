#include "display_config.h"
#include "display_frame.h"
#include "../SD/theme/theme.h"

static DeviceStatus wifiStatus = DeviceStatus::NotApplicable;
static DeviceStatus bluetoothStatus = DeviceStatus::NotApplicable;
static DeviceStatus ethernetStatus = DeviceStatus::NotApplicable;
static StorageStatus sdStatus = StorageStatus::NotApplicable;

void drawFrameRule(int y, uint16_t color) {
    tft.drawFastHLine(FRAME_X, y, FRAME_W, color);
}

static void printDeviceStatus(DeviceStatus status) {
    switch (status) {
        case DeviceStatus::On: tft.print("ON"); break;
        case DeviceStatus::Off: tft.print("OFF"); break;
        case DeviceStatus::NotApplicable: tft.print("N/A"); break;
    }
}

static void printStorageStatus(StorageStatus status) {
    switch (status) {
        case StorageStatus::In: tft.print("IN"); break;
        case StorageStatus::Out: tft.print("OUT"); break;
        case StorageStatus::NotApplicable: tft.print("N/A"); break;
    }
}

static void drawStatusBar() {
    tft.fillRect(0, STATUS_Y, SCREEN_W, STATUS_RULE_Y - STATUS_Y, themeBackground());
    tft.setTextSize(1);
    tft.setTextColor(themeSecondary());

    tft.setCursor(22, STATUS_Y);
    tft.print("WIFI:");
    printDeviceStatus(wifiStatus);

    tft.setCursor(104, STATUS_Y);
    tft.print("BL:");
    printDeviceStatus(bluetoothStatus);

    tft.setCursor(177, STATUS_Y);
    tft.print("ETH:");
    printDeviceStatus(ethernetStatus);

    tft.setCursor(256, STATUS_Y);
    tft.print("SD:");
    printStorageStatus(sdStatus);
}

void setStatusBarStatus(
    DeviceStatus wifi,
    DeviceStatus bluetooth,
    DeviceStatus ethernet,
    StorageStatus sd
) {
    if (wifiStatus == wifi && bluetoothStatus == bluetooth &&
        ethernetStatus == ethernet && sdStatus == sd) {
        return;
    }

    wifiStatus = wifi;
    bluetoothStatus = bluetooth;
    ethernetStatus = ethernet;
    sdStatus = sd;
    drawStatusBar();
}

void drawFooter() {
    tft.setTextSize(1);
    tft.setTextColor(themeSecondary());

    tft.setCursor(8, FOOTER_TEXT_Y);
    tft.print("UP/DOWN: MOVE");

    tft.setCursor(130, FOOTER_TEXT_Y);
    tft.print("CENTER: SELECT");

    tft.setCursor(253, FOOTER_TEXT_Y);
    tft.print("LEFT: BACK");
}

void drawSharedFrame(const char* title) {
    tft.fillScreen(themeBackground());

    tft.setTextSize(1);
    tft.setTextColor(themeSecondary());

    drawStatusBar();

    drawFrameRule(STATUS_RULE_Y, themePrimary());

    tft.setTextSize(2);
    tft.setTextColor(themeText());
    tft.setCursor(8, TITLE_Y);
    tft.print(title);

    drawFrameRule(TITLE_RULE_Y, themeSecondary());

    drawMarqueeBorders();

    drawFooter();
}
