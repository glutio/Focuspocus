#ifndef __BGRAPHICS_H__
#define __BGRAPHICS_H__

#include "Adafruit_GFX.h"

class BRect;

class BGraphics {
protected:
  Adafruit_GFX& _g;
public:
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
public:
  BGraphics(Adafruit_GFX& g);
  void drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, int16_t color);
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, int16_t color);  
  void drawText(const char* str, int16_t x, int16_t y, int8_t size, int16_t color);
  BRect getTextBounds(const char* str, int8_t size);

  friend class BPanel;
};

#endif