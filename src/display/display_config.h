
#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <Arduino.h>

// TFT hardware
#define TFT_CS          10
#define TFT_DC          7
#define TFT_RST         8
#define TFT_BACKLIGHT   9

// Buttons
#define BTN_UP          22
#define BTN_DOWN        23
#define BTN_LEFT        24
#define BTN_RIGHT       25
#define BTN_SELECT      26

// Screen dimensions
#define SCREEN_W        320
#define SCREEN_H        240

// Colors
#define BLACK           0x0000
#define PINK            0xE0B2
#define DARK_PINK       0xE0B2
#define WHITE           0xFFFF

// Frame
#define FRAME_X         5
#define FRAME_W         310

#define STATUS_Y        8
#define STATUS_RULE_Y   22

#define TITLE_Y         31
#define TITLE_RULE_Y    53

// Content
#define CONTENT_X       7
#define CONTENT_Y       61
#define CONTENT_W       306
#define CONTENT_BOTTOM  184

// Marquee
#define MARQUEE_Y       200
#define MARQUEE_H       16
#define MARQUEE_SPEED   20
#define MARQUEE_MAX_HEIGHT 16
#define NOTIFICATION_MAX_CHARS 120

// Footer
#define FOOTER_RULE_Y   222
#define FOOTER_TEXT_Y   228

// Input
#define DEBOUNCE_MS     30

#endif
