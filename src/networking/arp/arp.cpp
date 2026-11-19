#include "arp.h"

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include "../../printing/printing.h"

namespace {
// Socket 7 is reserved exclusively for this short-lived ARP probe. Do not use
// it for an EthernetClient, EthernetServer, or EthernetUDP elsewhere.
const byte ARP_SCAN_SOCKET = 7;
const uint16_t ARP_SCAN_SOURCE_PORT = 49152;
const uint16_t ARP_SCAN_DESTINATION_PORT = 9;  // UDP discard service

bool isOnLink(IPAddress target) {
  IPAddress local = Ethernet.localIP();
  IPAddress mask = Ethernet.subnetMask();
  for (byte i = 0; i < 4; ++i) {
    if ((local[i] & mask[i]) != (target[i] & mask[i])) {
      return false;
    }
  }
  return !(target == local);
}

bool isUsableUnicastMac(const byte *mac) {
  bool allZero = true;
  bool allFF = true;
  for (byte i = 0; i < 6; ++i) {
    allZero = allZero && mac[i] == 0;
    allFF = allFF && mac[i] == 0xFF;
  }
  return !allZero && !allFF && (mac[0] & 0x01) == 0;
}

uint32_t ipToUint32(IPAddress ip) {
  return (static_cast<uint32_t>(ip[0]) << 24) |
         (static_cast<uint32_t>(ip[1]) << 16) |
         (static_cast<uint32_t>(ip[2]) << 8) |
         ip[3];
}

IPAddress uint32ToIp(uint32_t value) {
  return IPAddress(value >> 24, value >> 16, value >> 8, value);
}

void closeScanSocket() {
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  W5100.execCmdSn(ARP_SCAN_SOCKET, Sock_CLOSE);
  SPI.endTransaction();
}
}  // namespace

void decodeArp(const byte *arp, uint16_t length) {
  if (length < 28) {
    return;
  }

  uint16_t opcode = read16(arp + 6);
  Serial.println("Protocol: ARP");
  Serial.print("Operation: ");
  if (opcode == 1) {
    Serial.println("Request");
  } else if (opcode == 2) {
    Serial.println("Reply");
  } else {
    Serial.print("Other (");
    Serial.print(opcode);
    Serial.println(")");
  }

  Serial.print("Sender IP: ");
  printIP(arp + 14);
  Serial.println();
  Serial.print("Target IP: ");
  printIP(arp + 24);
  Serial.println();
}

bool scanDevice(IPAddress targetIP, byte *macAddress) {
  if (macAddress == NULL || !isOnLink(targetIP)) {
    return false;
  }

  // Never close or reconfigure a socket that another part of the sketch owns.
  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  if (W5100.readSnSR(ARP_SCAN_SOCKET) != SnSR::CLOSED) {
    SPI.endTransaction();
    return false;
  }

  // A fresh UDP socket has no destination MAC. SEND therefore runs W5500's
  // automatic ARP exchange before the one-byte UDP datagram is transmitted.
  W5100.writeSnPORT(ARP_SCAN_SOCKET, ARP_SCAN_SOURCE_PORT);
  W5100.writeSnMR(ARP_SCAN_SOCKET, SnMR::UDP);
  W5100.execCmdSn(ARP_SCAN_SOCKET, Sock_OPEN);
  if (W5100.readSnSR(ARP_SCAN_SOCKET) != SnSR::UDP) {
    SPI.endTransaction();
    closeScanSocket();
    return false;
  }

  byte destination[4] = {
    targetIP[0], targetIP[1], targetIP[2], targetIP[3]
  };
  W5100.writeSnDIPR(ARP_SCAN_SOCKET, destination);
  W5100.writeSnDPORT(ARP_SCAN_SOCKET, ARP_SCAN_DESTINATION_PORT);

  uint16_t txPtr = W5100.readSnTX_WR(ARP_SCAN_SOCKET);
  byte probe = 0;
  W5100.write(W5100.SBASE(ARP_SCAN_SOCKET) +
                  (txPtr & W5100.SMASK),
              probe);
  W5100.writeSnTX_WR(ARP_SCAN_SOCKET, txPtr + 1);
  W5100.writeSnIR(ARP_SCAN_SOCKET, SnIR::SEND_OK | SnIR::TIMEOUT);
  W5100.execCmdSn(ARP_SCAN_SOCKET, Sock_SEND);
  SPI.endTransaction();

  unsigned long started = millis();
  while (millis() - started < ARP_SCAN_TIMEOUT_MS) {
    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    byte interrupt = W5100.readSnIR(ARP_SCAN_SOCKET);

    if (interrupt & SnIR::SEND_OK) {
      W5100.readSnDHAR(ARP_SCAN_SOCKET, macAddress);
      W5100.writeSnIR(ARP_SCAN_SOCKET, SnIR::SEND_OK);
      SPI.endTransaction();
      closeScanSocket();
      return isUsableUnicastMac(macAddress);
    }

    if (interrupt & SnIR::TIMEOUT) {
      W5100.writeSnIR(ARP_SCAN_SOCKET, SnIR::TIMEOUT);
      SPI.endTransaction();
      closeScanSocket();
      return false;
    }

    SPI.endTransaction();
    delay(1);
  }

  // This stops only the scanner's reserved socket; all other sockets survive.
  closeScanSocket();
  return false;
}

void scanLocalNetwork() {
  IPAddress local = Ethernet.localIP();
  IPAddress mask = Ethernet.subnetMask();
  uint32_t localValue = ipToUint32(local);
  uint32_t maskValue = ipToUint32(mask);
  uint32_t network = localValue & maskValue;
  uint32_t broadcast = network | ~maskValue;

  // A /31 or /32 has no conventional usable host range for this scanner.
  if (broadcast <= network + 1) {
    Serial.println("ARP scan: subnet has no usable host addresses.");
    return;
  }

  uint32_t hostCount = broadcast - network - 1;
  if (hostCount > ARP_SCAN_MAX_HOSTS) {
    Serial.print("ARP scan refused: subnet has ");
    Serial.print(hostCount);
    Serial.print(" hosts; ARP_SCAN_MAX_HOSTS is ");
    Serial.println(ARP_SCAN_MAX_HOSTS);
    return;
  }

  Serial.println();
  Serial.println("========== ARP SCAN ==========");
  Serial.print("Subnet: ");
  Serial.print(local);
  Serial.print(" / ");
  Serial.print(mask);
  Serial.println();
  Serial.println("IP address       MAC address");
  Serial.println("---------------  -----------------");

  uint16_t found = 0;
  byte mac[6];
  for (uint32_t address = network + 1; address < broadcast; ++address) {
    if (address == localValue) {
      continue;
    }

    IPAddress target = uint32ToIp(address);
    if (scanDevice(target, mac)) {
      Serial.print(target);
      Serial.print("  ");
      printMAC(mac);
      Serial.println();
      ++found;
    }
  }

  Serial.println("-------------------------------");
  Serial.print("Hosts found: ");
  Serial.println(found);
  Serial.println("================================");
}
