#include "theme.h"

#include <stdio.h>
#include <string.h>

namespace {
struct ThemeDefinition {
    const char* id;
    const char* label;
    uint32_t primary;
    uint32_t secondary;
    bool isDark;
};

const ThemeDefinition THEMES[] = {
    {"matrix", "Matrix", 0x00FF41, 0x008F11, true},
    {"cyberpunk", "Cyberpunk", 0xFF00FF, 0x00FFFF, true},
    {"purple", "Purple", 0x8000FF, 0x8000FF, true},
    {"red", "Red", 0xFF0000, 0xFF0000, true},
    {"blue", "Blue", 0x0000FF, 0x0000FF, true},
    {"cyan", "Cyan", 0x00FFFF, 0x00FFFF, true},
    {"green", "Green", 0x00FF00, 0x00FF00, true},
    {"yellow", "Yellow", 0xFFFF00, 0xFFFF00, true},
    {"pink", "Pink", 0xFF1493, 0xFF69B4, true},
    {"black-white", "Black and White", 0xFFFFFF, 0xFFFFFF, true},
    {"white-black", "White and Black", 0x000000, 0x000000, false}
};

const uint8_t THEME_COUNT = sizeof(THEMES) / sizeof(THEMES[0]);
const char THEME_FILE[] = "themes.jsn";
const char CONFIG_FILE[] = "config.jsn";
const uint8_t DEFAULT_THEME_INDEX = 8;

void applyTheme(uint8_t index) {
    theme.primary = THEMES[index].primary;
    theme.secondary = THEMES[index].secondary;
    theme.isDark = THEMES[index].isDark;
}

void resetTheme() {
    applyTheme(DEFAULT_THEME_INDEX);
}

void formatColor(uint32_t color, char* output, size_t outputSize) {
    snprintf(output, outputSize, "#%06lX", (unsigned long)(color & 0xFFFFFF));
}

bool createThemesFile() {
    JsonDocument doc;
    JsonArray themes = doc["themes"].to<JsonArray>();

    for (uint8_t i = 0; i < THEME_COUNT; ++i) {
        JsonObject entry = themes.add<JsonObject>();
        char primary[8];
        char secondary[8];
        formatColor(THEMES[i].primary, primary, sizeof(primary));
        formatColor(THEMES[i].secondary, secondary, sizeof(secondary));

        entry["id"] = THEMES[i].id;
        entry["name"] = THEMES[i].label;
        entry["primary"] = primary;
        entry["secondary"] = secondary;
        entry["isDark"] = THEMES[i].isDark;
    }

    if (SD.exists(THEME_FILE) && !SD.remove(THEME_FILE)) return false;
    File file = SD.open(THEME_FILE, FILE_WRITE);
    if (!file) return false;
    bool success = serializeJsonPretty(doc, file) > 0;
    file.close();
    if (!success) SD.remove(THEME_FILE);
    return success;
}

bool writeConfig() {
    JsonDocument doc;
    doc["primary"] = theme.primary;
    doc["secondary"] = theme.secondary;
    doc["isDark"] = theme.isDark;

    if (SD.exists(CONFIG_FILE) && !SD.remove(CONFIG_FILE)) return false;
    File file = SD.open(CONFIG_FILE, FILE_WRITE);
    if (!file) return false;
    bool success = serializeJsonPretty(doc, file) > 0;
    file.close();
    return success;
}

bool loadConfig() {
    File file = SD.open(CONFIG_FILE, FILE_READ);
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error ||
        !doc["primary"].is<uint32_t>() ||
        !doc["secondary"].is<uint32_t>() ||
        !doc["isDark"].is<bool>()) {
        return false;
    }

    uint32_t primary = doc["primary"].as<uint32_t>();
    uint32_t secondary = doc["secondary"].as<uint32_t>();
    if (primary > 0xFFFFFF || secondary > 0xFFFFFF) return false;

    theme.primary = primary;
    theme.secondary = secondary;
    theme.isDark = doc["isDark"].as<bool>();
    return true;
}
}  // namespace

ThemeConfig theme;

uint8_t themeCount() {
    return THEME_COUNT;
}

const char* themeIdAt(uint8_t index) {
    return index < THEME_COUNT ? THEMES[index].id : NULL;
}

const char* themeLabelAt(uint8_t index) {
    return index < THEME_COUNT ? THEMES[index].label : NULL;
}

uint8_t selectedThemeIndex() {
    for (uint8_t i = 0; i < THEME_COUNT; ++i) {
        if (theme.primary == THEMES[i].primary &&
            theme.secondary == THEMES[i].secondary &&
            theme.isDark == THEMES[i].isDark) {
            return i;
        }
    }
    return DEFAULT_THEME_INDEX;
}

bool loadTheme() {
    digitalWrite(ETH_CS, HIGH);
    resetTheme();

    if (!SD.exists(THEME_FILE) && !createThemesFile()) {
        SD.remove(CONFIG_FILE);
        resetTheme();
        return false;
    }

    if (!SD.exists(CONFIG_FILE)) {
        if (writeConfig()) return true;
        SD.remove(CONFIG_FILE);
        resetTheme();
        return false;
    }

    if (loadConfig()) return true;

    SD.remove(CONFIG_FILE);
    resetTheme();
    return false;
}

bool selectTheme(const char* id) {
    if (id == NULL) return false;

    for (uint8_t i = 0; i < THEME_COUNT; ++i) {
        if (strcmp(THEMES[i].id, id) != 0) continue;

        digitalWrite(ETH_CS, HIGH);
        applyTheme(i);
        if (writeConfig()) return true;

        SD.remove(CONFIG_FILE);
        resetTheme();
        return false;
    }
    return false;
}
