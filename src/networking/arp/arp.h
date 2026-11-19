#ifndef ETHERNET_PROJECT_ARP_H
#define ETHERNET_PROJECT_ARP_H

#include <Arduino.h>
#include <Ethernet.h>

// Decode an ARP payload (the Ethernet/VLAN header is not included).
void decodeArp(const byte *arp, uint16_t length);

// This controls only how long scanDevice waits. It does not modify the W5500's
// global retry registers, so established TCP/UDP sockets keep their settings.
#ifndef ARP_SCAN_TIMEOUT_MS
#define ARP_SCAN_TIMEOUT_MS 1000UL
#endif

// Resolves one on-link IPv4 address through the W5500's automatic ARP process.
// Returns false when the reserved socket is busy, the address is off-link, or
// no ARP resolution succeeds before ARP_SCAN_TIMEOUT_MS.
bool scanDevice(IPAddress targetIP, byte *macAddress);

// Scans every usable address in the configured local subnet, up to the safety
// limit below. Output is written as an IP/MAC table to Serial.
#ifndef ARP_SCAN_MAX_HOSTS
#define ARP_SCAN_MAX_HOSTS 254UL
#endif

void scanLocalNetwork();

#endif  // ETHERNET_PROJECT_ARP_H
