#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "display_input.h"

void displayMenuBegin();
void displayMenuUpdate(Button button);
void renderScreen();
bool displayMenuPacketScannerIsActive();
void displayMenuSetPacketScannerAvailable(bool isAvailable);

#endif