#ifndef ETHERNET_PROJECT_PRINTING_H
#define ETHERNET_PROJECT_PRINTING_H

#include <Arduino.h>

void printMAC(const byte *mac);
void printIP(const byte *ip);
uint16_t read16(const byte *p);

#endif  // ETHERNET_PROJECT_PRINTING_H
