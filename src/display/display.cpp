
#include "display.h"

// ASCII ART
const char* const Display::asciiArtLines[] = {
  "             ('-. .-.               .-')    .-') _",
  "            ( OO )  /              ( OO ). (  OO) )",
  "  ,----.    ,--. ,--. .-'),-----. (_)---\\_)/     '._ ",
  " '  .-./-') |  | |  |( OO'  .-.  '/    _ | |'--...__)",
  " |  |_( O- )|   .|  |/   |  | |  |\\  :` `. '--.  .--'",
  " |  | .--, \\|       |\\_) |  |\\|  | '..`''.)   |  |",
  "(|  | '. (_/|  .-.  |  \\ |  | |  |.-._)   \\   |  |",
  " |  '--'  | |  | |  |   `'  '-'  '\\       /   |  |",
  "  `------'  `--' `--'     `-----'  `-----'    `--'"
};

const int Display::lineCount =
    sizeof(Display::asciiArtLines) / sizeof(Display::asciiArtLines[0]);

// Constructor
Display::Display(Adafruit_ILI9341 &display)
    : tft(display) {
}

// Initialize display
void Display::begin() {
    tft.begin();
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK);
}

// Print ASCII art
void Display::printAsciiArt(uint16_t textColor, int characterDelay) {

    tft.setFont();
    tft.setTextSize(1);
    tft.setTextColor(textColor);

    const int lineHeight = 12;

    // Center vertically
    int startY = (240 - (lineCount * lineHeight)) / 2;

    for (int i = 0; i < lineCount; i++) {

        tft.setCursor(0, startY + i * lineHeight);

        for (int j = 0; asciiArtLines[i][j] != '\0'; j++) {
            tft.write(asciiArtLines[i][j]);
            delay(characterDelay); // Delay between characters
        }
    }

}
