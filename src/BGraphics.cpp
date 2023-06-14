#include <Arduino.h>
#include "BGraphics.h"
#include "BView.h"

BGraphics::BGraphics(Adafruit_GFX& g) : x(0), y(0), width(g.width()), height(g.height()), _g(g) {  }  

void BGraphics::drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  _g.drawRect(this->x + x, this->y + y, width, height, color);
};

void BGraphics::fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  _g.fillRect(this->x + x, this->y + y, width, height, color);
}

void BGraphics::drawText(const char* str, int16_t x, int16_t y, int8_t size, int16_t color) {
  _g.setCursor(this->x + x, this->y + y);
  _g.setTextColor(color);
  _g.setTextSize(size);
  _g.print(str);
}

BRect BGraphics::getTextBounds(const char* str, int8_t size) {
  BRect rt;
  _g.setTextSize(size);
  _g.getTextBounds(str, 0, 0, &rt.x, &rt.y, &rt.width, &rt.height);
  return rt;
}