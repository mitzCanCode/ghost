
#include <Arduino.h>
#include <string.h>

#include "display_config.h"
#include "display_frame.h"
#include "display_input.h"
#include "display_marquee.h"
#include "display_menu.h"

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
        case 3: currentScreen = ABOUT;        break;
      }
      selectedItem = 0;
      break;

    case SETTINGS:
      if (selectedItem == 1) { goBack(); return; }
      break;

    case TOOLS:
      if (selectedItem == 1) { goBack(); return; }
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

void displayMenuUpdate(Button button) {
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

void displayMenuBegin() {
  currentScreen = MAIN_MENU;
  selectedItem = 0;
  renderScreen();
}
