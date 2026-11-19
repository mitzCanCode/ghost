#include "ping.h"

#include <SPI.h>
#include <utility/w5100.h>

namespace {
const byte PING_SOCKET = 1;
const uint16_t PING_SIZE = 40;
const uint16_t PACKET_INFO_SIZE = 6;
uint16_t pingSequence = 0;

uint16_t icmpChecksum(const byte *data, uint16_t length) {
  uint32_t sum = 0;
  for (uint16_t i = 0; i + 1 < length; i += 2) {
    sum += (static_cast<uint16_t>(data[i]) << 8) | data[i + 1];
  }
  if (length & 1) {
    sum += static_cast<uint16_t>(data[length - 1]) << 8;
  }
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  return static_cast<uint16_t>(~sum);
}

void readSocket1Buffer(uint16_t ptr, byte *dest, uint16_t len) {
  uint16_t offset = ptr & W5100.SMASK;
  uint16_t first = W5100.SSIZE - offset;
  if (first > len) first = len;
  W5100.read(W5100.RBASE(PING_SOCKET) + offset, dest, first);
  if (len > first) {
    W5100.read(W5100.RBASE(PING_SOCKET), dest + first, len - first);
  }
}
}  // namespace

void pingIP(IPAddress target, uint8_t packetsToSend, uint16_t timeoutMs) {
  byte packet[PING_SIZE];
  byte rx[64];
  uint8_t received = 0;
  uint8_t sent = 0;

  Serial.println();
  Serial.print("PING ");
  Serial.println(target);

  SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
  if (W5100.readSnSR(PING_SOCKET) != SnSR::IPRAW) {
    W5100.execCmdSn(PING_SOCKET, Sock_CLOSE);
    W5100.writeSnMR(PING_SOCKET, SnMR::IPRAW);
    W5100.writeSnPROTO(PING_SOCKET, IPPROTO::ICMP);
    W5100.writeSnTTL(PING_SOCKET, 64);
    W5100.execCmdSn(PING_SOCKET, Sock_OPEN);
  }
  bool socketReady = W5100.readSnSR(PING_SOCKET) == SnSR::IPRAW;
  SPI.endTransaction();
  if (!socketReady) {
    Serial.println("Could not open ping socket.");
    return;
  }

  for (byte attempt = 0; attempt < packetsToSend; ++attempt) {
    uint16_t sequence = ++pingSequence;
    packet[0] = 8; packet[1] = 0; packet[2] = 0; packet[3] = 0;
    packet[4] = 0x12; packet[5] = 0x34;
    packet[6] = sequence >> 8; packet[7] = sequence & 0xFF;
    for (byte i = 8; i < PING_SIZE; ++i) packet[i] = i;
    uint16_t checksum = icmpChecksum(packet, PING_SIZE);
    packet[2] = checksum >> 8; packet[3] = checksum & 0xFF;

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    byte destination[4] = {target[0], target[1], target[2], target[3]};
    W5100.writeSnDIPR(PING_SOCKET, destination);
    uint16_t freeSize = W5100.readSnTX_FSR(PING_SOCKET);
    if (freeSize < PING_SIZE) {
      SPI.endTransaction();
      Serial.println("Ping transmit buffer full.");
      continue;
    }
    uint16_t txPtr = W5100.readSnTX_WR(PING_SOCKET);
    uint16_t txOffset = txPtr & W5100.SMASK;
    uint16_t first = W5100.SSIZE - txOffset;
    if (first > PING_SIZE) first = PING_SIZE;
    W5100.write(W5100.SBASE(PING_SOCKET) + txOffset, packet, first);
    if (PING_SIZE > first) {
      W5100.write(W5100.SBASE(PING_SOCKET), packet + first, PING_SIZE - first);
    }
    txPtr += PING_SIZE;
    W5100.writeSnTX_WR(PING_SOCKET, txPtr);
    W5100.writeSnIR(PING_SOCKET, SnIR::SEND_OK | SnIR::TIMEOUT);
    W5100.execCmdSn(PING_SOCKET, Sock_SEND);
    SPI.endTransaction();
    ++sent;

    unsigned long startTime = millis();
    bool replyFound = false;
    while (millis() - startTime < timeoutMs) {
      SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
      uint16_t available = W5100.readSnRX_RSR(PING_SOCKET);
      if (available < PACKET_INFO_SIZE) {
        SPI.endTransaction();
        delay(2);
        continue;
      }
      uint16_t rxPtr = W5100.readSnRX_RD(PING_SOCKET);
      byte info[PACKET_INFO_SIZE];
      readSocket1Buffer(rxPtr, info, PACKET_INFO_SIZE);
      uint16_t dataLength = (static_cast<uint16_t>(info[4]) << 8) | info[5];
      if (dataLength == 0 || dataLength > available - PACKET_INFO_SIZE) {
        SPI.endTransaction();
        delay(2);
        continue;
      }
      uint16_t readLength = dataLength > sizeof(rx) ? sizeof(rx) : dataLength;
      readSocket1Buffer(rxPtr + PACKET_INFO_SIZE, rx, readLength);
      rxPtr += PACKET_INFO_SIZE + dataLength;
      W5100.writeSnRX_RD(PING_SOCKET, rxPtr);
      W5100.execCmdSn(PING_SOCKET, Sock_RECV);
      SPI.endTransaction();

      if (dataLength >= 8 && rx[0] == 0 && rx[1] == 0 &&
          rx[4] == 0x12 && rx[5] == 0x34 &&
          rx[6] == static_cast<byte>(sequence >> 8) &&
          rx[7] == static_cast<byte>(sequence & 0xFF)) {
        Serial.print("Reply from "); Serial.print(target);
        Serial.print(": time="); Serial.print(millis() - startTime);
        Serial.println(" ms");
        ++received;
        replyFound = true;
        break;
      }
      delay(1);
    }
    if (!replyFound) {
      Serial.print("Request timed out for ");
      Serial.println(target);
    }
    if (attempt + 1 < packetsToSend) delay(500);
  }

  Serial.println();
  Serial.println("===== PING STATISTICS =====");
  Serial.print("Packets sent: "); Serial.println(sent);
  Serial.print("Packets received: "); Serial.println(received);
  Serial.print("Packets lost: "); Serial.println(sent - received);
  if (sent > 0) {
    Serial.print("Packet loss: ");
    Serial.print((static_cast<float>(sent - received) * 100.0) / sent);
    Serial.println("%");
  }
  Serial.println("===========================");
}
