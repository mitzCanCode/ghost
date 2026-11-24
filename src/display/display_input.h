#ifndef DISPLAY_INPUT_H
#define DISPLAY_INPUT_H

#define DEBOUNCE_MS 30

enum Button {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    NONE
};

void displayInputBegin();
void displayInputUpdate();

#endif