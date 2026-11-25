#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

#define SD_CS 4
#define ETH_CS 10

struct ThemeConfig {
    uint32_t primary = 0xFF1493;
    uint32_t secondary = 0xFF69B4;
    bool isDark = true;
};

extern ThemeConfig theme;

inline uint16_t rgb888To565(uint32_t rgb) {
    uint8_t red = (rgb >> 16) & 0xFF;
    uint8_t green = (rgb >> 8) & 0xFF;
    uint8_t blue = rgb & 0xFF;
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

inline uint16_t themePrimary() {
    return rgb888To565(theme.primary);
}

inline uint16_t themeSecondary() {
    return rgb888To565(theme.secondary);
}

inline uint16_t themeBackground() {
    return theme.isDark ? 0x0000 : 0xFFFF;
}

inline uint16_t themeText() {
    return theme.isDark ? 0xFFFF : 0x0000;
}

uint8_t themeCount();
const char* themeIdAt(uint8_t index);
const char* themeLabelAt(uint8_t index);
uint8_t selectedThemeIndex();
bool loadTheme();
bool selectTheme(const char* id);