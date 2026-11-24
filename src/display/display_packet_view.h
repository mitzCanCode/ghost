#ifndef DISPLAY_PACKET_VIEW_H
#define DISPLAY_PACKET_VIEW_H

#include <Arduino.h>

enum PacketType {
  PACKET_TYPE_OTHER,
  PACKET_TYPE_ARP,
  PACKET_TYPE_IPV4,
  PACKET_TYPE_TCP,
  PACKET_TYPE_UDP,
  PACKET_TYPE_ICMP,
  PACKET_TYPE_IGMP
};

struct PacketDetails {
  uint8_t sourceMac[6];
  uint8_t destinationMac[6];
  uint8_t sourceIp[4];
  uint8_t destinationIp[4];
  uint16_t vlanIds[2];
  uint16_t frameLength;
  uint16_t etherType;
  uint16_t ipLength;
  uint16_t sourcePort;
  uint16_t destinationPort;
  uint16_t arpOperation;
  uint8_t icmpType;
  uint8_t icmpCode;
  uint8_t vlanCount;
  uint8_t ipProtocol;
  uint8_t igmpType;
  uint8_t igmpMaxResponse;
  PacketType type;
  bool hasIp;
  bool hasPorts;
  bool hasArp;
  bool hasIcmp;
  bool hasVlan;
  bool hasIgmp;
};

void packetViewAddPacket(const PacketDetails& packet);
void packetViewRender();
void packetViewUpdate();
void packetViewSetAvailable(bool available);
bool packetViewIsRunning();
bool packetViewHandleInput(uint8_t button);

#endif
