#ifndef __BTHEME_H__
#define __BTHEME_H__

struct BTheme {
  int16_t border = 1;
  int16_t color = 0xFFFF;
  int16_t background = 0x0000;
  int16_t focusColor = 0x4dda;
  int16_t focusBackground = 0xd689;
  int16_t boundingBoxColor = 0xf800;
  int16_t scrollbarMinThumb = 15;
  int16_t buttonRadius = 7;
  int16_t fontSize = 1;
  int16_t fontColor = 0xFFFF;
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



#endif