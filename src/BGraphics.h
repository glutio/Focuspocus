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
  virtual void drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  virtual void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  virtual void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, int16_t color);
  virtual void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, int16_t color);  
  virtual void drawText(const char* str, int16_t x, int16_t y, int8_t size, int16_t color);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
  virtual BRect getTextBounds(const char* str, int8_t size);
  virtual void drawContour(int16_t x, int16_t y, uint8_t mask[], int16_t width, int16_t height, int16_t color);
  virtual void drawArc(int centerX, int centerY, int radius, int startAngle, int endAngle, uint16_t color);
  virtual void drawCircle(int centerX, int centerY, int radius, uint16_t color);
  virtual void fillCircle(int centerX, int centerY, int radius, uint16_t color);
  friend class BPanel;
};

#endif