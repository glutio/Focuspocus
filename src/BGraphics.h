#ifndef __BGRAPHICS_H__
#define __BGRAPHICS_H__

#include <stdint.h>

class BRect;

class BGraphicsApi {
public:
  virtual int16_t displayWidth() = 0;
  virtual int16_t displayHeight() = 0;
  // Area the following calls may draw in (screen coordinates, right and bottom
  // exclusive); pixels outside it must be left untouched, fillScreen included.
  virtual void setClip(int16_t left, int16_t top, int16_t right, int16_t bottom) = 0;
  virtual void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) = 0;
  virtual void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) = 0;
  virtual void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) = 0;
  virtual void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) = 0;  
  virtual void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) = 0;
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h) = 0;
  virtual void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h) = 0;
  virtual BRect getTextBounds(const char* str, uint8_t size) = 0;
  virtual void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) = 0;
  virtual void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) = 0;
  virtual void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) = 0;
  virtual void fillScreen(uint16_t color) = 0;
  virtual void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) = 0;
};

// What views draw with: forwards to a BGraphicsApi (the display), offset to
// the view's position and clipped to its parents. A plain wrapper, not a
// BGraphicsApi itself, so it has no vtable and calls are direct.
class BGraphics {
public:
  BGraphicsApi& api;
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  // When false, all drawing calls are ignored (e.g. the view is covered by
  // another screen). Set by BFocusManager::getGraphics(view).
  bool visible;
  // Screen area of the parent panels' client areas (right and bottom exclusive).
  // Drawing is clipped to it (by the display driver, see BGraphicsApi::setClip).
  int16_t clipLeft;
  int16_t clipTop;
  int16_t clipRight;
  int16_t clipBottom;
private:
  bool ready();
public:
  BGraphics(BGraphicsApi& api);
  // Narrows the clip to a rectangle given relative to this graphics' origin.
  void clip(int16_t x, int16_t y, int16_t width, int16_t height);
  // True if any part of the x, y, width, height area is inside the clip.
  bool overlaps();
  int16_t displayWidth();
  int16_t displayHeight();
  void drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
  void drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color);
  void fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color);
  void drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color);
  void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t width, uint16_t height);
  void drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t width, uint16_t height);
  BRect getTextBounds(const char* str, uint8_t size);
  void drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  void fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color);
  void drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color);
  void fillScreen(uint16_t color);
  void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
};

#endif
