#include "BAdafruitGraphics.h"
#include "BView.h"
#include <string.h>

BClipGFX::BClipGFX(Adafruit_GFX& g) : Adafruit_GFX(g.width(), g.height()), g(g), l(0), t(0), r(0), b(0) { }

void BClipGFX::set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
  _width = g.width();
  _height = g.height();
  l = left; t = top; r = right; b = bottom;
}
bool BClipGFX::fits(int16_t x, int16_t y, int16_t w, int16_t h) {
  return x >= l && y >= t && x + w <= r && y + h <= b;
}
void BClipGFX::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x >= l && x < r && y >= t && y < b) g.writePixel(x, y, color);
}
void BClipGFX::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  int16_t x2 = x + w, y2 = y + h;
  if (x < l) x = l;
  if (y < t) y = t;
  if (x2 > r) x2 = r;
  if (y2 > b) y2 = b;
  if (x2 > x && y2 > y) g.writeFillRect(x, y, x2 - x, y2 - y, color);
}
void BClipGFX::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { writeFillRect(x, y, w, 1, color); }
void BClipGFX::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { writeFillRect(x, y, 1, h, color); }
void BClipGFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  g.startWrite();
  writeFillRect(x, y, w, h, color);
  g.endWrite();
}
void BClipGFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { fillRect(x, y, w, 1, color); }
void BClipGFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { fillRect(x, y, 1, h, color); }
void BClipGFX::startWrite() { g.startWrite(); }
void BClipGFX::endWrite() { g.endWrite(); }

BAdafruitGraphics::BAdafruitGraphics(Adafruit_GFX& g) : g(g), clipped(g) { }

Adafruit_GFX& BAdafruitGraphics::to(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (clipped.fits(x, y, w, h)) return g;
  return clipped;
}

int16_t BAdafruitGraphics::displayWidth() { 
  return g.width();
}
int16_t BAdafruitGraphics::displayHeight() {
  return g.height();
}
void BAdafruitGraphics::setClip(int16_t left, int16_t top, int16_t right, int16_t bottom) {
  clipped.set(left, top, right, bottom);
}

void BAdafruitGraphics::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  to(x, y, width, height).drawRect(x, y, width, height, color);
}

void BAdafruitGraphics::fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  to(x, y, width, height).fillRect(x, y, width, height, color);
}

void BAdafruitGraphics::drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  to(x, y, width, height).drawRoundRect(x, y, width, height, radius, color);
}

void BAdafruitGraphics::fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  to(x, y, width, height).fillRoundRect(x, y, width, height, radius, color);
}

// Adafruit draws glyphs pixel by pixel anyway, so text always goes through the clip.
void BAdafruitGraphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  clipped.setTextWrap(false);
  clipped.setCursor(x, y);
  clipped.setTextColor(color);
  clipped.setTextSize(size);
  clipped.print(str);
}
// The classic font: a 6x8 cell per character, scaled by size.
BRect BAdafruitGraphics::getTextBounds(const char* str, uint8_t size) {
  BRect rt;
  if (size < 1) size = 1;
  if (*str) {
    rt.width = strlen(str) * 6 * size;
    rt.height = 8 * size;
  }
  return rt;
}

void BAdafruitGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h) {
  to(x, y, w, h).drawRGBBitmap(x, y, bitmap, w, h);
}

void BAdafruitGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h) {
  to(x, y, w, h).drawRGBBitmap(x, y, bitmap, mask, w, h);
}

void BAdafruitGraphics::drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  to(centerX - radius, centerY - radius, 2 * radius + 1, 2 * radius + 1).drawCircle(centerX, centerY, radius, color);
}

void BAdafruitGraphics::fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  to(centerX - radius, centerY - radius, 2 * radius + 1, 2 * radius + 1).fillCircle(centerX, centerY, radius, color);
}

void BAdafruitGraphics::drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
    // Every other line: a few fast lines instead of a pixel-by-pixel dither.
    Adafruit_GFX& g = to(x, y, width, height);
    g.startWrite();
    for (int16_t j = y + (y & 1); j < y + height; j += 2) {
        g.writeFastHLine(x, j, width, color);
    }
    g.endWrite();
}

void BAdafruitGraphics::fillScreen(uint16_t color) {
  clipped.fillScreen(color);
}

void BAdafruitGraphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  to(x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, abs(x2 - x1) + 1, abs(y2 - y1) + 1).drawLine(x1, y1, x2, y2, color);
}


BAdafruitFontGraphics::BAdafruitFontGraphics(Adafruit_GFX& g, const GFXfont* font) : BAdafruitGraphics(g) {
  clipped.setFont(font);
}

// Text boxes get the height of "Ay" so labels line up whatever their letters.
void BAdafruitFontGraphics::measure(const char* str, uint8_t size, int16_t& left, int16_t& top, uint16_t& width, uint16_t& height) {
  int16_t y1;
  uint16_t w;
  clipped.setTextWrap(false);
  clipped.setTextSize(size);
  clipped.getTextBounds("Ay", 0, 0, &left, &top, &w, &height);
  clipped.getTextBounds(str, 0, 0, &left, &y1, &width, &w);
}

// GFX fonts draw from the baseline; x, y here is the top left of the text box.
void BAdafruitFontGraphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  int16_t left, top;
  uint16_t w, h;
  measure(str, size, left, top, w, h);
  BAdafruitGraphics::drawText(str, x - left, y - top, size, color);
}

BRect BAdafruitFontGraphics::getTextBounds(const char* str, uint8_t size) {
  BRect rt;
  int16_t left, top;
  uint16_t w, h;
  measure(str, size, left, top, w, h);
  if (*str) {
    rt.width = w;
    rt.height = h;
  }
  return rt;
}
