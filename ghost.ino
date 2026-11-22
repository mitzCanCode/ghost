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




// #include <SPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ILI9341.h>

// #include "src/display/display.h"

// #define TFT_CS   10
// #define TFT_DC   7
// #define TFT_RST  8

// Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// // Create display object
// Display display(tft);

// void setup() {

//     Serial.begin(9600);

//     pinMode(9, OUTPUT);
//     digitalWrite(9, HIGH);

//     // Initialize display
//     display.begin();

//     // Print ASCII art
//     display.printAsciiArt(0x0010, 5);
// }

// void loop() {

// }



#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <string.h>

// ---------------- HARDWARE ----------------

#define TFT_CS   10
#define TFT_DC    7
#define TFT_RST   8
#define TFT_BACKLIGHT 9

#define BTN_UP      22
#define BTN_DOWN    23
#define BTN_LEFT    24
#define BTN_RIGHT   25
#define BTN_SELECT  26

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

#define SCREEN_W 320
#define SCREEN_H 240

// ---------------- COLORS ----------------

#define BLACK       0x0000
#define PINK        0xE0B2  // Bright, vibrant pink
#define DARK_PINK   0xE0B2  // Deeper pink
#define WHITE       0xFFFF

// ---------------- SHARED LAYOUT ----------------
// Every screen uses these coordinates.  Content renderers must only draw in
// CONTENT_X..(CONTENT_X + CONTENT_W) and CONTENT_Y..CONTENT_BOTTOM.

const int FRAME_X = 5;
const int FRAME_W = 310;

const int STATUS_Y = 8;
const int STATUS_RULE_Y = 22;

const int TITLE_Y = 31;
const int TITLE_RULE_Y = 53;

// Menu area
const int CONTENT_X = 7;
const int CONTENT_Y = 61;
const int CONTENT_W = 306;
const int CONTENT_BOTTOM = 184;

// Marquee section
const int MARQUEE_Y = 200;
const int MARQUEE_H = 16;

// Footer
const int FOOTER_RULE_Y = 222;
const int FOOTER_TEXT_Y = 228;

// ---------------- BUTTON INPUT ----------------

#define DEBOUNCE_MS 30

const uint8_t buttonPins[] = {
  BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT
};

enum Button { UP, DOWN, LEFT, RIGHT, SELECT, NONE };

Button readButton() {
  static bool lastState[5] = { HIGH, HIGH, HIGH, HIGH, HIGH };
  static unsigned long lastTime[5] = { 0, 0, 0, 0, 0 };

  for (int i = 0; i < 5; i++) {
    bool current = digitalRead(buttonPins[i]);

    if (current == LOW && lastState[i] == HIGH &&
        millis() - lastTime[i] > DEBOUNCE_MS) {
      lastState[i] = LOW;
      lastTime[i] = millis();
      return (Button)i;
    }

    if (current == HIGH) {
      lastState[i] = HIGH;
    }
  }

  return NONE;
}

// ---------------- Other ----------------

void drawFrameRule(int y, uint16_t color) {
  tft.drawFastHLine(FRAME_X, y, FRAME_W, color);
}

// ---------------- GLOBAL NOTIFICATION MARQUEE ----------------
// Change this one string (or call setNotificationText()) to update every
// screen.  The marquee has its own small off-screen buffer and is redrawn
// after a frame refresh, so screen redraws never permanently erase it.

#define MARQUEE_SPEED 20
#define MARQUEE_MAX_HEIGHT 16
#define NOTIFICATION_MAX_CHARS 120

char notificationText[NOTIFICATION_MAX_CHARS] =
  "   MEGA TERMINAL   -   SYSTEM ONLINE   -   BLACK & PINK EDITION   ";

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

// ---------------- SHARED SCREEN FRAME ----------------

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

  // Status bar
  tft.setTextSize(1);
  tft.setTextColor(PINK);
  tft.setCursor(8, STATUS_Y);
  tft.print("ghost.");

  tft.fillCircle(265, STATUS_Y + 3, 3, PINK);

  tft.setCursor(275, STATUS_Y);
  tft.print("ONLINE");

  // Status divider
  drawFrameRule(STATUS_RULE_Y, PINK);

  // Title
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(8, TITLE_Y);
  tft.print(title);

  // Title divider
  drawFrameRule(TITLE_RULE_Y, DARK_PINK);

  // Marquee borders
  drawMarqueeBorders();

  // Footer
  drawFooter();
}

// ---------------- REUSABLE CONTENT RENDERERS ----------------
// Add a table, list, chart, or panel later by creating another renderer with
// this signature.  It will automatically live inside the same shared frame.




typedef void (*ContentRenderer)();

struct MenuItem {
  const char* label;
  bool navigation;
};

struct MenuContent {
  const MenuItem* items;
  uint8_t count;
};

int selectedItem = 0;
const MenuContent* activeMenu = NULL;


void drawMenuItem(uint8_t index) {
  if (activeMenu == NULL || index >= activeMenu->count) return;

  const int itemHeight = 22;
  const int itemGap = 2;

  int y = CONTENT_Y + index * (itemHeight + itemGap);

  if (y + itemHeight > CONTENT_BOTTOM) return;

  bool selected = (index == selectedItem);

  const MenuItem& item = activeMenu->items[index];

  // Clear and draw selection background.
  tft.fillRect(
    CONTENT_X, y,
    CONTENT_W, itemHeight,
    selected ? PINK : BLACK
  );

  tft.setTextSize(2);
  tft.setTextColor(selected ? BLACK : WHITE);

  // Draw item name without a selection prefix.
  tft.setCursor(15, y + 3);
  tft.print(item.label);

  // Push navigation arrow to the far right.
  if (item.navigation) {
    tft.setCursor(CONTENT_X + CONTENT_W - 16, y + 3);
    tft.print(">");
  }
}

void renderMenuContent() {
  if (activeMenu == NULL) return;

  for (uint8_t i = 0; i < activeMenu->count; i++) {
    drawMenuItem(i);
  }
}

void updateMenuSelection(int previousSelection) {
  drawMenuItem(previousSelection);
  drawMenuItem(selectedItem);
}

// ---------------- APPLICATION SCREENS ----------------

enum Screen { MAIN_MENU, SETTINGS, TOOLS, SYSTEM_INFO, ABOUT };

struct ScreenDefinition {
  const char* title;
  const MenuContent* menu;
  ContentRenderer renderContent;
};

const MenuItem mainItems[] = {
  { "System Information", true },
  { "Settings", true },
  { "Tools", true },
  { "About", true }
};

const MenuItem settingsItems[] = {
  { "Display Settings", false },
  { "Back", false }
};

const MenuItem toolsItems[] = {
  { "Diagnostics", false },
  { "Back", false }
};

const MenuItem infoItems[] = {
  { "Arduino Mega 2560", false },
  { "ILI9341 Display", false },
  { "240 x 320 TFT", false },
  { "SPI Interface", false },
  { "Back", false }
};

const MenuItem aboutItems[] = {
  { "MEGA TERMINAL", false },
  { "ILI9341 UI Framework", false },
  { "Version 0.1", false },
  { "Black and Pink", false },
  { "Back", false }
};

const MenuContent mainMenu = { mainItems, 4 };
const MenuContent settingsMenu = { settingsItems, 2 };
const MenuContent toolsMenu = { toolsItems, 2 };
const MenuContent infoMenu = { infoItems, 5 };
const MenuContent aboutMenu = { aboutItems, 5 };

const ScreenDefinition screens[] = {
  { "MAIN MENU",   &mainMenu,     renderMenuContent },
  { "SETTINGS",    &settingsMenu, renderMenuContent },
  { "TOOLS",       &toolsMenu,    renderMenuContent },
  { "SYSTEM INFO", &infoMenu,     renderMenuContent },
  { "ABOUT",       &aboutMenu,    renderMenuContent }
};

Screen currentScreen = MAIN_MENU;

void renderScreen() {
  const ScreenDefinition& screen = screens[currentScreen];
  activeMenu = screen.menu;

  if (activeMenu != NULL && activeMenu->count > 0) {
    selectedItem = constrain(
        selectedItem, 0, (int)activeMenu->count - 1
    );
  }

  drawSharedFrame(screen.title);
  screen.renderContent();
  drawMarquee();
}

// ---------------- NAVIGATION ----------------

void goBack() {
  if (currentScreen == MAIN_MENU) return;
  currentScreen = MAIN_MENU;
  selectedItem = 0;
  renderScreen();
}

void activateItem() {
  switch (currentScreen) {
    case MAIN_MENU:
      switch (selectedItem) {
        case 0: currentScreen = SYSTEM_INFO; break;
        case 1: currentScreen = SETTINGS;    break;
        case 2: currentScreen = TOOLS;       break;
        case 3: currentScreen = ABOUT;       break;
      }
      selectedItem = 0;
      break;

    case SETTINGS:
      if (selectedItem == 1) { goBack(); return; }
      // Display Settings placeholder.
      break;

    case TOOLS:
      if (selectedItem == 1) { goBack(); return; }
      // Diagnostics placeholder.
      break;

    case SYSTEM_INFO:
      if (selectedItem == 4) { goBack(); return; }
      break;

    case ABOUT:
      if (selectedItem == 4) { goBack(); return; }
      break;
  }

  renderScreen();
}


void handleInput(Button button) {
  if (button == NONE) return;

  const MenuContent* menu = screens[currentScreen].menu;
  uint8_t itemCount = menu == NULL ? 0 : menu->count;

  if (button == LEFT) {
    goBack();
    return;
  }

  if (button == UP && itemCount > 0) {
    int previousSelection = selectedItem;

    selectedItem = (selectedItem + itemCount - 1) % itemCount;

    updateMenuSelection(previousSelection);
  }

  if (button == DOWN && itemCount > 0) {
    int previousSelection = selectedItem;

    selectedItem = (selectedItem + 1) % itemCount;

    updateMenuSelection(previousSelection);
  }

  if ((button == SELECT || button == RIGHT) && itemCount > 0) {
    activateItem();
  }
}

// ---------------- SETUP / LOOP ----------------

void setup() {
  Serial.begin(9600);

  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);

  for (uint8_t i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  tft.begin(60000000);
  tft.setRotation(1);  // Same landscape rotation as the original sketch.

  currentScreen = MAIN_MENU;
  selectedItem = 0;
  renderScreen();
}

void loop() {
  handleInput(readButton());
  updateMarquee();
}
