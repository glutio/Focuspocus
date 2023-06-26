#ifndef __BGRAPHICS_H__
#define __BGRAPHICS_H__

#include "Adafruit_GFX.h"

class BRect;

class BGraphicsApi {
public:
  virtual int16_t displayWidth() = 0;
  virtual int16_t displayHeight() = 0;
  virtual void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) = 0;
  virtual void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) = 0;
  virtual void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) = 0;
  virtual void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) = 0;  
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) = 0;
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h) = 0;
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h) = 0;
  virtual BRect getTextBounds(const char* str, uint8_t size) = 0;
  virtual void drawContour(int16_t x, int16_t y, uint8_t mask[], uint16_t width, uint16_t height, uint16_t color) = 0;
  virtual void drawArc(int16_t centerX, int16_t centerY, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color) = 0;
  virtual void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) = 0;
  virtual void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) = 0;
  virtual void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) = 0;
  virtual void fillScreen(uint16_t color) = 0;
  virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) = 0;
};

class BGraphics: public BGraphicsApi {
public:
  BGraphicsApi& api;
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
public:
  BGraphics(BGraphicsApi& api);
  virtual int16_t displayWidth();
  virtual int16_t displayHeight();
  virtual void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color);
  virtual void fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color);
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t width, uint16_t height);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t width, uint16_t height);
  virtual BRect getTextBounds(const char* str, uint8_t size);
  virtual void drawContour(int16_t x, int16_t y, uint8_t mask[], uint16_t width, uint16_t height, uint16_t color);
  virtual void drawArc(int16_t centerX, int16_t centerY, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color);
  virtual void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  virtual void fillScreen(uint16_t color);
  virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
};

#endif