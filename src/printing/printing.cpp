#include "printing.h"

void printMAC(const byte *mac) {
  for (byte i = 0; i < 6; ++i) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
}

void printIP(const byte *ip) {
  for (byte i = 0; i < 4; ++i) {
    Serial.print(ip[i]);
    if (i < 3) {
      Serial.print(".");
    }
  }
}

uint16_t read16(const byte *p) {
  return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}
