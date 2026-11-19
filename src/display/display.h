
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

class Display {
public:

    // Constructor
    Display(Adafruit_ILI9341 &display);

    // Initialize TFT
    void begin();

    // Print ASCII art
    void printAsciiArt(
        uint16_t textColor = 0x0010,
        int characterDelay = 5
    );

private:

    Adafruit_ILI9341 &tft;

    static const char* const asciiArtLines[];
    static const int lineCount;
};

#endif
