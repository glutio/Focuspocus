#ifndef __BTHEME_H__
#define __BTHEME_H__

#include <stdint.h>

// https://embeddednotepad.com/page/rgb565-color-picker
struct BTheme {
  uint16_t color = 0xFFFF;
  uint16_t background = 0x0000;
  uint16_t focusColor = 0x4dda;
  uint16_t focusBackground = 0xd689;
  uint8_t scrollbarMinThumb = 15;
  uint8_t scrollbarSize = 10;
  uint8_t buttonRadius = 7;
  uint8_t buttonWidth = 50;
  uint8_t buttonHeight = 30;
  uint8_t buttonPadding = 5;
  uint8_t fontSize = 1;
  uint16_t fontColor = 0xFFFF;
  uint8_t checkboxSize = 15;
  uint16_t popupScreenColor = 0x94b2;
  uint16_t screenBackground = 0;
};

struct BBWTheme: BTheme {
  BBWTheme() {
    color = 1;
    background = 0;
    focusColor = 0;
    focusBackground = 1;
    fontColor = 1;
    popupScreenColor = 0;
    scrollbarMinThumb = 8;
    scrollbarSize = 6;
    buttonRadius = 3;
    buttonWidth = 30;
    buttonHeight = 14;
    buttonPadding = 2;
    checkboxSize = 9;
  }
};

struct BWinTheme: BTheme {
public:
  BWinTheme() {
    color = 0;
    background = 0xd69a;
    fontColor = 0;
    focusColor = 0xffff;
    focusBackground = 0x8437;
  }
};

#endif