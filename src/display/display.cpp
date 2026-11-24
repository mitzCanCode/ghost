#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "display.h"
#include "display_config.h"
#include "display_frame.h"
#include "display_input.h"
#include "display_menu.h"
#include "display_marquee.h"
#include "display_packet_view.h"
#include "../networking/networking.h"
#include "../SD/sd_manager.h"

static const unsigned long STATUS_SCAN_INTERVAL_MS = 5000;

// Single TFT instance
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// Startup logo
static const char* logo[] = {
    "             ('-. .-.               .-')    .-') _",
    "            ( OO )  /              ( OO ). (  OO) )",
    "  ,----.    ,--. ,--. .-'),-----. (_)---\\_)/     '._",
    " '  .-./-') |  | |  |( OO'  .-.  '/    _ | |'--...__)",
    " |  |_( O- )|   .|  |/   |  | |  |\\  :` `. '--.  .--'",
    " |  | .--, \\|       |\\_) |  |\\|  | '..`''.)   |  |",
    "(|  | '. (_/|  .-.  |  \\ |  | |  |.-._)   \\   |  |",
    " |  '--'  | |  | |  |   `'  '-'  '\\       /   |  |",
    "  `------'  `--' `--'     `-----'  `-----'    `--'"
};

void showStartupLogo() {

    const int lineCount = sizeof(logo) / sizeof(logo[0]);
    const int lineHeight = 12;

    int startY = (SCREEN_H - lineCount * lineHeight) / 2;

    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    tft.setTextColor(PINK);
    tft.setFont();

    for (int i = 0; i < lineCount; i++) {
        tft.setCursor(0, startY + i * lineHeight);
        tft.println(logo[i]);
    }

    delay(2000);

    tft.fillScreen(BLACK);
}

void displayBegin() {

    pinMode(TFT_BACKLIGHT, OUTPUT);
    digitalWrite(TFT_BACKLIGHT, HIGH);

    tft.begin(60000000);
    tft.setRotation(1);

    showStartupLogo();

    displayInputBegin();
    displayMenuBegin();
}

static void updateHardwareStatuses() {
    static unsigned long lastScan = 0;
    static bool hasScanned = false;
    unsigned long now = millis();

    if (hasScanned && now - lastScan < STATUS_SCAN_INTERVAL_MS) {
        return;
    }

    hasScanned = true;
    lastScan = now;
    sdManagerRefresh();

    setStatusBarStatus(
        DeviceStatus::NotApplicable,
        DeviceStatus::NotApplicable,
        networkingStatus(),
        sdManagerStatus()
    );
}

void displayUpdate() {

    displayInputUpdate();
    updateMarquee();
    updateHardwareStatuses();

    bool scannerActive = displayMenuPacketScannerIsActive();
    bool available = networkingAvailable();
    if (scannerActive) {
        displayMenuSetPacketScannerAvailable(available);
    }
    if (available) {
        networkingPoll(scannerActive && packetViewIsRunning());
    }
    if (scannerActive) {
        packetViewUpdate();
    }
}