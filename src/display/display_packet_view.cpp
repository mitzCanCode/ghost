#include <stdio.h>
#include <string.h>

#include <Adafruit_ILI9341.h>

#include "display_config.h"
#include "display_frame.h"
#include "display_input.h"
#include "display_packet_view.h"

namespace {
const uint8_t PACKET_LIST_CAPACITY = 10;
const uint8_t PACKET_VISIBLE_ROWS = 5;
const unsigned long PACKET_VIEW_REFRESH_MS = 250;
const int LIST_HEADER_Y = CONTENT_Y + 12;
const int LIST_FIRST_ROW_Y = CONTENT_Y + 21;
const int LIST_ROW_HEIGHT = 20;

PacketDetails packets[PACKET_LIST_CAPACITY];
uint8_t packetCount = 0;
uint8_t selectedPacket = 0;
uint8_t firstVisiblePacket = 0;
uint8_t displayedPacketCount = 0;
PacketDetails detailPacket;
bool available = false;
bool showingDetails = false;
bool scanning = true;
bool listDirty = false;
bool needsFullRedraw = false;
unsigned long lastListRefresh = 0;

const char* packetTypeName(PacketType type) {
  switch (type) {
    case PACKET_TYPE_ARP: return "ARP";
    case PACKET_TYPE_IPV4: return "IPv4";
    case PACKET_TYPE_TCP: return "TCP";
    case PACKET_TYPE_UDP: return "UDP";
    case PACKET_TYPE_ICMP: return "ICMP";
    case PACKET_TYPE_IGMP: return "IGMP";
    default: return "OTHER";
  }
}

void formatIp(const uint8_t* ip, char* output, size_t outputSize) {
  snprintf(output, outputSize, "%u.%u.%u.%u",
           (unsigned)ip[0], (unsigned)ip[1],
           (unsigned)ip[2], (unsigned)ip[3]);
}

void formatMac(const uint8_t* mac, char* output, size_t outputSize) {
  snprintf(output, outputSize, "%02X:%02X:%02X:%02X:%02X:%02X",
           (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2],
           (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);
}

void drawListRow(uint8_t index, uint8_t visibleIndex) {
  int y = LIST_FIRST_ROW_Y + visibleIndex * LIST_ROW_HEIGHT;
  bool selected = index == selectedPacket;
  PacketDetails& packet = packets[index];

  tft.fillRect(CONTENT_X, y, CONTENT_W, LIST_ROW_HEIGHT - 1,
               selected ? PINK : BLACK);
  tft.setTextSize(1);
  tft.setTextColor(selected ? BLACK : WHITE);
  tft.setCursor(CONTENT_X + 3, y + 6);

  if (packet.hasIp) {
    char sourceIp[16];
    formatIp(packet.sourceIp, sourceIp, sizeof(sourceIp));
    tft.print(sourceIp);
  } else {
    tft.print("n/a");
  }

  tft.setCursor(CONTENT_X + 96, y + 6);
  char sourceMac[18];
  formatMac(packet.sourceMac, sourceMac, sizeof(sourceMac));
  tft.print(sourceMac);

  tft.setCursor(CONTENT_X + 232, y + 6);
  tft.print(packetTypeName(packet.type));

  if (selected) {
    tft.setCursor(CONTENT_X + CONTENT_W - 8, y + 6);
    tft.print(">");
  }
}

void drawCaptureFooter() {
  tft.fillRect(128, FOOTER_TEXT_Y, 100, 8, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(PINK);
  tft.setCursor(130, FOOTER_TEXT_Y);
  tft.print(scanning ? "CENTER: PAUSE" : "CENTER: START");
}

void drawList() {
  tft.setTextSize(1);
  tft.setTextColor(PINK);
  tft.setCursor(CONTENT_X + 3, LIST_HEADER_Y);
  tft.print("IP");
  tft.setCursor(CONTENT_X + 96, LIST_HEADER_Y);
  tft.print("MAC");
  tft.setCursor(CONTENT_X + 232, LIST_HEADER_Y);
  tft.print("TYPE");

  if (packetCount == 0) {
    tft.setTextColor(WHITE);
    tft.setCursor(CONTENT_X + 5, LIST_FIRST_ROW_Y + 8);
    tft.print("Listening for packets...");
    return;
  }

  uint8_t visibleCount = packetCount - firstVisiblePacket;
  if (visibleCount > PACKET_VISIBLE_ROWS) {
    visibleCount = PACKET_VISIBLE_ROWS;
  }
  for (uint8_t i = 0; i < visibleCount; ++i) {
    drawListRow(firstVisiblePacket + i, i);
  }
}

void keepSelectionVisible() {
  if (selectedPacket < firstVisiblePacket) {
    firstVisiblePacket = selectedPacket;
  } else if (selectedPacket >= firstVisiblePacket + PACKET_VISIBLE_ROWS) {
    firstVisiblePacket = selectedPacket - PACKET_VISIBLE_ROWS + 1;
  }
}

void drawDetailRow(uint8_t row, const char* label, const char* value) {
  int y = CONTENT_Y + 12 + row * 12;
  tft.setTextSize(1);
  tft.setTextColor(PINK);
  tft.setCursor(CONTENT_X + 3, y);
  tft.print(label);
  tft.setTextColor(WHITE);
  tft.print(value);
}

void drawDetails() {
  char value[40];
  char ip[16];
  uint8_t row = 0;

  if (detailPacket.hasIp) {
    formatIp(detailPacket.sourceIp, ip, sizeof(ip));
    drawDetailRow(row++, "Source IP: ", ip);
    formatIp(detailPacket.destinationIp, ip, sizeof(ip));
    drawDetailRow(row++, "Destination IP: ", ip);
  } else {
    drawDetailRow(row++, "Source IP: ", "n/a");
    drawDetailRow(row++, "Destination IP: ", "n/a");
  }

  formatMac(detailPacket.sourceMac, value, sizeof(value));
  drawDetailRow(row++, "Source MAC: ", value);
  formatMac(detailPacket.destinationMac, value, sizeof(value));
  drawDetailRow(row++, "Destination MAC: ", value);

  if (detailPacket.type == PACKET_TYPE_IPV4) {
    snprintf(value, sizeof(value), "IPv4 proto %u",
             (unsigned)detailPacket.ipProtocol);
  } else {
    snprintf(value, sizeof(value), "%s", packetTypeName(detailPacket.type));
  }
  if (detailPacket.hasVlan) {
    size_t valueLength = strlen(value);
    if (detailPacket.vlanCount > 2) {
      snprintf(value + valueLength, sizeof(value) - valueLength,
               " V%u/%u+%u", detailPacket.vlanIds[0],
               detailPacket.vlanIds[1],
               (unsigned)(detailPacket.vlanCount - 2));
    } else if (detailPacket.vlanCount > 1) {
      snprintf(value + valueLength, sizeof(value) - valueLength,
               " V%u/%u", detailPacket.vlanIds[0],
               detailPacket.vlanIds[1]);
    } else {
      snprintf(value + valueLength, sizeof(value) - valueLength,
               " V%u", detailPacket.vlanIds[0]);
    }
  }
  drawDetailRow(row++, "Packet type: ", value);

  snprintf(value, sizeof(value), "%u bytes", detailPacket.frameLength);
  drawDetailRow(row++, "Frame length: ", value);

  snprintf(value, sizeof(value), "0x%04X", detailPacket.etherType);
  drawDetailRow(row++, "EtherType: ", value);

  if (detailPacket.type != PACKET_TYPE_ARP && detailPacket.hasIp) {
    snprintf(value, sizeof(value), "%u bytes", detailPacket.ipLength);
    drawDetailRow(row++, "IP length: ", value);
  }

  if (detailPacket.hasPorts) {
    snprintf(value, sizeof(value), "%u -> %u",
             detailPacket.sourcePort, detailPacket.destinationPort);
    drawDetailRow(row++, "Ports: ", value);
  } else if (detailPacket.hasIcmp) {
    snprintf(value, sizeof(value), "%u / %u",
             detailPacket.icmpType, detailPacket.icmpCode);
    drawDetailRow(row++, "ICMP type/code: ", value);
  } else if (detailPacket.hasArp) {
    if (detailPacket.arpOperation == 1) {
      snprintf(value, sizeof(value), "Request");
    } else if (detailPacket.arpOperation == 2) {
      snprintf(value, sizeof(value), "Reply");
    } else {
      snprintf(value, sizeof(value), "%u", detailPacket.arpOperation);
    }
    drawDetailRow(row++, "ARP operation: ", value);
  } else if (detailPacket.hasIgmp) {
    snprintf(value, sizeof(value), "0x%02X / %u",
             (unsigned)detailPacket.igmpType,
             (unsigned)detailPacket.igmpMaxResponse);
    drawDetailRow(row++, "IGMP type / max: ", value);
  }
}
}  // namespace

void packetViewAddPacket(const PacketDetails& packet) {
  bool listWasFull = packetCount == PACKET_LIST_CAPACITY;
  if (listWasFull) {
    memmove(packets, packets + 1,
            sizeof(PacketDetails) * (PACKET_LIST_CAPACITY - 1));
    --packetCount;
    if (firstVisiblePacket > 0) {
      --firstVisiblePacket;
    }
    if (selectedPacket > 0) {
      --selectedPacket;
    }
  }

  packets[packetCount++] = packet;
  if (packetCount == 1) {
    selectedPacket = 0;
    firstVisiblePacket = 0;
  }

  listDirty = true;
  needsFullRedraw = needsFullRedraw || listWasFull;
}

void packetViewRender() {
  drawCaptureFooter();
  if (!available) {
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(CONTENT_X + 5, CONTENT_Y + 12);
    tft.println("Internet not available");
    tft.setTextSize(1);
    tft.setCursor(CONTENT_X + 5, CONTENT_Y + 40);
    tft.println("please connect to a network first.");
    return;
  }

  if (showingDetails) {
    drawDetails();
  } else {
    drawList();
    displayedPacketCount = packetCount;
    listDirty = false;
    needsFullRedraw = false;
  }
}

void packetViewUpdate() {
  if (!available || showingDetails || !listDirty ||
      millis() - lastListRefresh < PACKET_VIEW_REFRESH_MS) {
    return;
  }

  if (needsFullRedraw) {
    drawList();
  } else {
    for (uint8_t i = displayedPacketCount; i < packetCount; ++i) {
      if (i >= firstVisiblePacket &&
          i < firstVisiblePacket + PACKET_VISIBLE_ROWS) {
        drawListRow(i, i - firstVisiblePacket);
      }
    }
  }

  displayedPacketCount = packetCount;
  listDirty = false;
  needsFullRedraw = false;
  lastListRefresh = millis();
}

void packetViewSetAvailable(bool isAvailable) {
  if (available == isAvailable) {
    return;
  }
  available = isAvailable;
  tft.fillRect(CONTENT_X, CONTENT_Y, CONTENT_W,
               CONTENT_BOTTOM - CONTENT_Y, BLACK);
  packetViewRender();
}

bool packetViewIsRunning() {
  return scanning;
}

bool packetViewHandleInput(uint8_t button) {
  if (button == LEFT) {
    if (showingDetails) {
      showingDetails = false;
      tft.fillRect(CONTENT_X, CONTENT_Y, CONTENT_W,
                   CONTENT_BOTTOM - CONTENT_Y, BLACK);
      packetViewRender();
      return true;
    }
    return false;
  }

  if (button == SELECT) {
    scanning = !scanning;
    drawCaptureFooter();
    return true;
  }

  if (!available || packetCount == 0) {
    return true;
  }

  if (showingDetails) {
    return true;
  }

  if (button == UP) {
    selectedPacket = (selectedPacket + packetCount - 1) % packetCount;
    keepSelectionVisible();
    drawList();
    displayedPacketCount = packetCount;
    listDirty = false;
    needsFullRedraw = false;
  } else if (button == DOWN) {
    selectedPacket = (selectedPacket + 1) % packetCount;
    keepSelectionVisible();
    drawList();
    displayedPacketCount = packetCount;
    listDirty = false;
    needsFullRedraw = false;
  } else if (button == RIGHT) {
    detailPacket = packets[selectedPacket];
    showingDetails = true;
    tft.fillRect(CONTENT_X, CONTENT_Y, CONTENT_W,
                 CONTENT_BOTTOM - CONTENT_Y, BLACK);
    drawDetails();
  }

  return true;
}
