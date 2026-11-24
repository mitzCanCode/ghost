
#include "display_config.h"
#include "display_input.h"
#include "display_menu.h"

const uint8_t buttonPins[] = {
  BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT
};

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

void displayInputBegin() {
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void displayInputUpdate() {
  Button button = readButton();
if (button != NONE) {
        switch (button) {
            case UP:
                Serial.println("UP button pressed");
                break;

            case DOWN:
                Serial.println("DOWN button pressed");
                break;

            case LEFT:
                Serial.println("LEFT button pressed");
                break;

            case RIGHT:
                Serial.println("RIGHT button pressed");
                break;

            case SELECT:
                Serial.println("SELECT button pressed");
                break;

            default:
                Serial.println("Unknown button");
                break;
        }

        displayMenuUpdate(button);
    }
}