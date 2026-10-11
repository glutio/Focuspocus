#ifndef __BWIN32GRAPHICS_H__
#define __BWIN32GRAPHICS_H__

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdint.h>
#include "BGraphics.h"
#include "BView.h"

// BGraphicsApi backed by an in-memory 32bpp DIB at display resolution.
// Shapes are rasterized in software so output is deterministic and every
// RGB565 color round-trips exactly through pixel(); text uses GDI with
// Adafruit classic-font metrics (6x8 cell per character, scaled by size).
// Works headless (unit tests) or presented in a window (BWin32Window).
class BWin32Graphics: public BGraphicsApi {
protected:
  int16_t _width;
  int16_t _height;
  HDC _dc;
  HBITMAP _bitmap;
  HGDIOBJ _oldBitmap;
  uint32_t* _bits;
  BITMAPINFO _info;
  uint32_t _version;
  HFONT _fonts[8];
  RECT _clip;

  void hline(int16_t x, int16_t y, int16_t w, uint16_t color);
  void vline(int16_t x, int16_t y, int16_t h, uint16_t color);
  void circleQuadrants(int16_t x0, int16_t y0, int16_t r, uint8_t quadrants, uint16_t color);
  HFONT font(uint8_t size);

public:
  BWin32Graphics(int16_t width, int16_t height);
  ~BWin32Graphics();
  BWin32Graphics(const BWin32Graphics&) = delete;
  BWin32Graphics& operator=(const BWin32Graphics&) = delete;

  // Test/host helpers
  void setPixel(int16_t x, int16_t y, uint16_t color);
  uint16_t pixel(int16_t x, int16_t y);       // RGB565 at (x, y); 0 outside
  uint32_t countPixels(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  uint32_t version() const { return _version; } // bumps on every draw call
  const uint32_t* bits() const { return _bits; }
  const BITMAPINFO& bitmapInfo() const { return _info; }

  static uint32_t toRGB(uint16_t c565);
  static uint16_t to565(uint32_t rgb);

  virtual int16_t displayWidth();
  virtual int16_t displayHeight();
  virtual void setClip(int16_t left, int16_t top, int16_t right, int16_t bottom);
  virtual void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  virtual void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h);
  virtual BRect getTextBounds(const char* str, uint8_t size);
  virtual void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  virtual void fillScreen(uint16_t color);
  virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
};

#endif
