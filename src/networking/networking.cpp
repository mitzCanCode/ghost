#include "networking.h"

#include <SPI.h>
#include <Ethernet.h>
#include <string.h>
#include <utility/w5100.h>

#include "../printing/printing.h"
#include "../display/display_packet_view.h"
#include "arp/arp.h"
#include "../../shared.h"

// Kept in this translation unit so there is exactly one definition.
byte frame[MAX_FRAME];
static bool networkingReady = false;
static bool ethernetControllerDetected = false;

void networkingBegin() {
  networkingReady = false;
  ethernetControllerDetected = false;

  // Mega hardware SPI master mode.
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);

  // Disable the SD card, which shares the SPI bus.
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  Ethernet.init(ETH_CS);
  Serial.println("Initializing Ethernet controller...");

  if (!W5100.init()) {
    Serial.println("Ethernet controller not detected!");
    return;
  }

  ethernetControllerDetected = true;
  Serial.println("Ethernet controller detected!");
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    return;
  }

  byte mac[] = {0x02, 0xA1, 0xB2, 0xC3, 0xD4, 0xE5};
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.setMACAddress(mac);
  W5100.execCmdSn(0, Sock_CLOSE);
  W5100.writeSnMR(0, SnMR::MACRAW);
  W5100.execCmdSn(0, Sock_OPEN);
  byte status = W5100.readSnSR(0);
  SPI.endTransaction();

  Serial.print("Socket status: 0x");
  Serial.println(status, HEX);

  if (status != SnSR::MACRAW) {
    Serial.println("MACRAW mode failed!");
    return;
  }

  networkingReady = true;
  Serial.println("Ethernet monitor ready!");
}

bool networkingAvailable() {
  return networkingReady && Ethernet.linkStatus() != LinkOFF;
}

DeviceStatus networkingStatus() {
  if (!ethernetControllerDetected) {
    return DeviceStatus::NotApplicable;
  }

  EthernetLinkStatus link = Ethernet.linkStatus();
  if (link == LinkON) {
    return DeviceStatus::On;
  }
  if (link == LinkOFF) {
    return DeviceStatus::Off;
  }
  return DeviceStatus::NotApplicable;
}

void readRxBuffer(uint16_t ptr, byte *dest, uint16_t len) {
  uint16_t offset = ptr & W5100.SMASK;
  uint16_t address = W5100.RBASE(0) + offset;
  uint16_t first = W5100.SSIZE - offset;

  if (first > len) {
    first = len;
  }

  W5100.read(address, dest, first);

  if (len > first) {
    W5100.read(W5100.RBASE(0), dest + first, len - first);
  }
}

// Decodes Ethernet, VLAN, IPv4, TCP, UDP, ICMP, and ARP frames.
// The input must start at the destination MAC address.
void decodeFrame(const byte *frame, uint16_t length) {
  if (length < 14) {
    return;
  }

  Serial.println();
  Serial.println("--------------- PACKET ---------------");

  Serial.print("Length: ");
  Serial.print(length);
  Serial.println(" bytes");

  Serial.print("Source MAC: ");
  printMAC(frame + 6);
  Serial.println();

  Serial.print("Destination MAC: ");
  printMAC(frame);
  Serial.println();

  uint16_t etherType = read16(frame + 12);
  uint16_t offset = 14;

  // Handle one or more VLAN tags (802.1Q / QinQ).
  while (etherType == 0x8100 || etherType == 0x88A8) {
    if (length < offset + 4) {
      return;
    }

    uint16_t vlanInfo = read16(frame + offset);
    Serial.print("VLAN ID: ");
    Serial.println(vlanInfo & 0x0FFF);

    etherType = read16(frame + offset + 2);
    offset += 4;
  }

  Serial.print("EtherType: 0x");
  Serial.println(etherType, HEX);

  if (etherType == 0x0806) {
    decodeArp(frame + offset, length - offset);
    return;
  }

  if (etherType != 0x0800) {
    Serial.println("Protocol: Other / not decoded");
    return;
  }

  if (length < offset + 20) {
    return;
  }

  byte version = frame[offset] >> 4;
  byte ihl = (frame[offset] & 0x0F) * 4;
  if (version != 4 || ihl < 20 || length < offset + ihl) {
    Serial.println("Invalid IPv4 header");
    return;
  }

  uint16_t totalLength = read16(frame + offset + 2);
  if (totalLength < ihl) {
    return;
  }

  byte protocol = frame[offset + 9];
  Serial.println("Protocol: IPv4");
  Serial.print("Source IP: ");
  printIP(frame + offset + 12);
  Serial.println();
  Serial.print("Destination IP: ");
  printIP(frame + offset + 16);
  Serial.println();
  Serial.print("IP packet length: ");
  Serial.println(totalLength);

  uint16_t transport = offset + ihl;
  uint16_t available = length - transport;
  uint16_t ipPayloadLength = totalLength - ihl;
  if (available > ipPayloadLength) {
    available = ipPayloadLength;
  }

  if (protocol == 6 || protocol == 17) {
    Serial.print("Transport: ");
    Serial.println(protocol == 6 ? "TCP" : "UDP");
    if (available >= 4) {
      Serial.print("Source port: ");
      Serial.println(read16(frame + transport));
      Serial.print("Destination port: ");
      Serial.println(read16(frame + transport + 2));
    }
  } else if (protocol == 1) {
    Serial.println("Transport: ICMP");
    if (available >= 2) {
      Serial.print("ICMP type: ");
      Serial.println(frame[transport]);
      Serial.print("ICMP code: ");
      Serial.println(frame[transport + 1]);
    }
  } else if (protocol == 2) {
    Serial.println("Transport: IGMP");
    if (available >= 2) {
      Serial.print("IGMP Type: 0x");
      Serial.println(frame[transport], HEX);
      Serial.print("IGMP Max Response Time: ");
      Serial.println(frame[transport + 1]);
    }
  } else {
    Serial.print("Transport: Other (IP protocol ");
    Serial.print(protocol);
    Serial.println(")");
  }
}

void networkingPoll() {
  networkingPoll(true);
}

void networkingPoll(bool capturePackets) {
  if (!networkingAvailable()) {
    return;
  }

  byte header[2];

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  uint16_t available = W5100.readSnRX_RSR(0);
  if (available < 2) {
    SPI.endTransaction();
    return;
  }

  uint16_t ptr = W5100.readSnRX_RD(0);
  readRxBuffer(ptr, header, 2);
  uint16_t totalLength = read16(header);

  if (totalLength < 2 || totalLength > W5100.SSIZE) {
    SPI.endTransaction();
    Serial.println("Invalid packet length");
    return;
  }
  if (available < totalLength) {
    SPI.endTransaction();
    return;
  }

  uint16_t frameLength = totalLength - 2;
  if (frameLength > MAX_FRAME) {
    ptr += totalLength;
    W5100.writeSnRX_RD(0, ptr);
    W5100.execCmdSn(0, Sock_RECV);
    SPI.endTransaction();
    return;
  }

  ptr += 2;
  readRxBuffer(ptr, frame, frameLength);
  ptr += frameLength;
  W5100.writeSnRX_RD(0, ptr);
  W5100.execCmdSn(0, Sock_RECV);
  SPI.endTransaction();

  PacketDetails packet = {};
  packet.frameLength = frameLength;
  if (frameLength >= 14) {
    memcpy(packet.destinationMac, frame, sizeof(packet.destinationMac));
    memcpy(packet.sourceMac, frame + 6, sizeof(packet.sourceMac));

    uint16_t etherType = read16(frame + 12);
    uint16_t offset = 14;
    while ((etherType == 0x8100 || etherType == 0x88A8) &&
           frameLength >= offset + 4) {
      if (packet.vlanCount < 2) {
        packet.vlanIds[packet.vlanCount] = read16(frame + offset) & 0x0FFF;
      }
      ++packet.vlanCount;
      packet.hasVlan = true;
      etherType = read16(frame + offset + 2);
      offset += 4;
    }
    packet.etherType = etherType;
    packet.type = PACKET_TYPE_OTHER;

    if (etherType == 0x0806 && frameLength >= offset + 28) {
      const byte *arp = frame + offset;
      packet.type = PACKET_TYPE_ARP;
      packet.hasIp = true;
      packet.hasArp = true;
      packet.arpOperation = read16(arp + 6);
      memcpy(packet.sourceIp, arp + 14, sizeof(packet.sourceIp));
      memcpy(packet.destinationIp, arp + 24, sizeof(packet.destinationIp));
    } else if (etherType == 0x0800 && frameLength >= offset + 20) {
      const byte *ip = frame + offset;
      uint8_t ipHeaderLength = (ip[0] & 0x0F) * 4;
      uint16_t ipLength = read16(ip + 2);

      if ((ip[0] >> 4) == 4 && ipHeaderLength >= 20 &&
          frameLength >= offset + ipHeaderLength && ipLength >= ipHeaderLength) {
        packet.hasIp = true;
        packet.ipLength = ipLength;
        memcpy(packet.sourceIp, ip + 12, sizeof(packet.sourceIp));
        memcpy(packet.destinationIp, ip + 16, sizeof(packet.destinationIp));

        uint8_t protocol = ip[9];
        packet.ipProtocol = protocol;
        uint16_t transportOffset = offset + ipHeaderLength;
        uint16_t available = frameLength - transportOffset;
        uint16_t payloadLength = ipLength - ipHeaderLength;
        if (available > payloadLength) {
          available = payloadLength;
        }

        if (protocol == 6 || protocol == 17) {
          packet.type = protocol == 6 ? PACKET_TYPE_TCP : PACKET_TYPE_UDP;
          if (available >= 4) {
            packet.hasPorts = true;
            packet.sourcePort = read16(frame + transportOffset);
            packet.destinationPort = read16(frame + transportOffset + 2);
          }
        } else if (protocol == 1) {
          packet.type = PACKET_TYPE_ICMP;
          if (available >= 2) {
            packet.hasIcmp = true;
            packet.icmpType = frame[transportOffset];
            packet.icmpCode = frame[transportOffset + 1];
          }
        } else if (protocol == 2) {
          packet.type = PACKET_TYPE_IGMP;
          if (available >= 2) {
            packet.hasIgmp = true;
            packet.igmpType = frame[transportOffset];
            packet.igmpMaxResponse = frame[transportOffset + 1];
          }
        } else {
          packet.type = PACKET_TYPE_IPV4;
        }
      }
    }

    if (capturePackets) {
      packetViewAddPacket(packet);
    }
  }

  if (capturePackets) {
    decodeFrame(frame, frameLength);
  }
}
