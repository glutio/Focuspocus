#ifndef __BTHEME_H__
#define __BTHEME_H__
// https://embeddednotepad.com/page/rgb565-color-picker
struct BTheme {
  int16_t border = 1;
  int16_t color = 0xFFFF;
  int16_t background = 0x0000;
  int16_t focusColor = 0x4dda;
  int16_t focusBackground = 0xd689;
  int16_t boundingBoxColor = 0xf800;
  int16_t scrollbarMinThumb = 15;
  int16_t scrollbarSize = 10;
  int16_t buttonRadius = 7;
  int16_t buttonWidth = 50;
  int16_t buttonHeight = 30;
  int16_t buttonPadding = 5;
  int16_t fontSize = 1;
  int16_t fontColor = 0xFFFF;
  int16_t checkboxSize = 15;
  int16_t popupScreenColor = 0x94b2;
  int16_t screenBackground = 0;
};

struct BBWTheme: BTheme {
  BBWTheme() {
    color = 1;
    background = 0;
    focusColor = 0;
    focusBackground = 1;
    boundingBoxColor = 1;
    fontColor = 1;
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