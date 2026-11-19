#ifndef ETHERNET_PROJECT_PING_H
#define ETHERNET_PROJECT_PING_H

#include <Arduino.h>
#include <Ethernet.h>

void pingIP(IPAddress target, uint8_t packetsToSend, uint16_t timeoutMs = 1000);

#endif  // ETHERNET_PROJECT_PING_H
