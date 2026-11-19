// #include "src/networking/networking.h"
// #include "src/networking/ping/ping.h"
// #include "src/networking/arp/arp.h"

// void setup() {
//   Serial.begin(19200);
//   networkingBegin();

//   Serial.println("Testing pinging functionality");
//   pingIP(IPAddress(8, 8, 8, 8), 5);
//   pingIP(IPAddress(192, 168, 3, 1), 10);

//   scanLocalNetwork();
//   delay(10000); // Sleep 10 seconds to make sure everything okay in testing before continuing
// }

// void loop() {
//   networkingPoll();
// }




#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "src/display/display.h"

#define TFT_CS   10
#define TFT_DC   7
#define TFT_RST  8

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// Create display object
Display display(tft);

void setup() {

    Serial.begin(9600);

    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);

    // Initialize display
    display.begin();

    // Print ASCII art
    display.printAsciiArt(0x0010, 5);
}

void loop() {

}
