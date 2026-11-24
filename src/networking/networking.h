#ifndef ETHERNET_PROJECT_NETWORKING_H
#define ETHERNET_PROJECT_NETWORKING_H

#include <Arduino.h>

void networkingBegin();
bool networkingAvailable();
void networkingPoll();
void networkingPoll(bool capturePackets);
void readRxBuffer(uint16_t ptr, byte *dest, uint16_t len);
void decodeFrame(const byte *frame, uint16_t length);

#endif  // ETHERNET_PROJECT_NETWORKING_H
