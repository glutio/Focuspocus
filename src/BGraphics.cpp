#include <Arduino.h>
#include "BGraphics.h"
#include "BView.h"

BGraphics::BGraphics(BGraphicsApi& api) 
  : api(api), x(0), y(0), width(api.displayWidth()), height(api.displayHeight()) { 
}
int16_t BGraphics::displayWidth() {
  return api.displayWidth();
}
int16_t BGraphics::displayHeight() {
  return api.displayHeight();
}
void BGraphics::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  api.drawRect(this->x + x, this->y + y, width, height, color);
};
void BGraphics::fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  api.fillRect(this->x + x, this->y + y, width, height, color);
}
void BGraphics::drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  api.drawRoundRect(this->x + x, this->y + y, width, height, radius, color);
}
void BGraphics::fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  api.fillRoundRect(this->x + x, this->y + y, width, height, radius, color);
}
void BGraphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  api.drawText(str, this->x + x, this->y + y, size, color);
}
void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t width, uint16_t height) {
  api.drawBitmap(this->x + x, this->y + y, bitmap, width, height);
}
void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t width, uint16_t height) {
  api.drawBitmap(this->x + x, this->y + y, bitmap, mask, width, height);
}
BRect BGraphics::getTextBounds(const char* str, uint8_t size) {
  api.getTextBounds(str, size);
}
void BGraphics::drawContour(int16_t x, int16_t y, uint8_t mask[], uint16_t width, uint16_t height, uint16_t color) {
  api.drawContour(this->x + x, this->y + y, mask, width, height, color);
}
void BGraphics::drawArc(int16_t centerX, int16_t centerY, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color) {
  api.drawArc(this->x + centerX, this->y + centerY, radius, startAngle, endAngle, color);
}
void BGraphics::drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  api.drawCircle(this->x + centerX, this->y + centerY, radius, color);
}
void BGraphics::fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  api.fillCircle(this->x + centerX, this->y + centerY, radius, color);
}
void BGraphics::drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  api.drawShade(this->x + x, this->y + y, width, height, color);
};
void BGraphics::fillScreen(uint16_t color) {
  api.fillScreen(color);
}
void BGraphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  api.drawLine(this->x + x1, this->y + y1, this->x + x2, this->y + y2, color);
}