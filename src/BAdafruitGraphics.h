#ifndef __BADAFRUITGRAPHICS_H__
#define __BADAFRUITGRAPHICS_H__

#include <Adafruit_GFX.h>
#include "BGraphics.h"

// Adafruit_GFX that forwards to the real display, dropping pixels outside the
// clip. Adafruit has no clip rectangle; its own primitives draw through this.
class BClipGFX: public Adafruit_GFX {
public:
  Adafruit_GFX& g;
  int16_t l, t, r, b;
  BClipGFX(Adafruit_GFX& g);
  void set(int16_t left, int16_t top, int16_t right, int16_t bottom);
  bool fits(int16_t x, int16_t y, int16_t w, int16_t h);
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;
  void startWrite() override;
  void endWrite() override;
};

class BAdafruitGraphics: public BGraphicsApi {
public:
  Adafruit_GFX& g;
  BClipGFX clipped;
  // The display itself when the area fits the clip, else the clipping wrapper.
  Adafruit_GFX& to(int16_t x, int16_t y, int16_t w, int16_t h);
public:
  BAdafruitGraphics(Adafruit_GFX& g);
  virtual int16_t displayWidth();
  virtual int16_t displayHeight();
  virtual void setClip(int16_t left, int16_t top, int16_t right, int16_t bottom);
  virtual void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  virtual void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
  virtual void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);  
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color);
  virtual void drawTextOpaque(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color, uint16_t background);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h);
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h);
  virtual BRect getTextBounds(const char* str, uint8_t size);
  virtual void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  virtual void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  virtual void fillScreen(uint16_t color);
  virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
};

// Draws and measures all text with an Adafruit GFX font (e.g. FreeSans9pt7b from
// <Fonts/FreeSans9pt7b.h>) instead of the built-in 6x8 one; fontSize scales it.
// A separate class so sketches without a font don't pay for it.
class BAdafruitFontGraphics: public BAdafruitGraphics {
public:
  BAdafruitFontGraphics(Adafruit_GFX& g, const GFXfont* font);
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color);
  // GFX fonts draw no background: fill the box, then draw.
  virtual void drawTextOpaque(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color, uint16_t background);
  virtual BRect getTextBounds(const char* str, uint8_t size);
private:
  void measure(const char* str, uint8_t size, int16_t& left, int16_t& top, uint16_t& width, uint16_t& height);
};

#endif
