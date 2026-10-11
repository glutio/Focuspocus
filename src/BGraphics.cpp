#include "BGraphics.h"
#include "BView.h"

BGraphics::BGraphics(BGraphicsApi& api)
  : api(api), x(0), y(0), width(api.displayWidth()), height(api.displayHeight()), visible(true),
    clipLeft(0), clipTop(0), clipRight(width), clipBottom(height) {
}

void BGraphics::clip(int16_t x, int16_t y, int16_t width, int16_t height) {
  x += this->x;
  y += this->y;
  if (clipLeft < x) clipLeft = x;
  if (clipTop < y) clipTop = y;
  if (clipRight > x + width) clipRight = x + width;
  if (clipBottom > y + height) clipBottom = y + height;
}

bool BGraphics::inside() {
  return x >= clipLeft && y >= clipTop && x + width <= clipRight && y + height <= clipBottom;
}
int16_t BGraphics::displayWidth() {
  return api.displayWidth();
}
int16_t BGraphics::displayHeight() {
  return api.displayHeight();
}
void BGraphics::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  if (!visible) return;
  api.drawRect(this->x + x, this->y + y, width, height, color);
};
void BGraphics::fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  if (!visible) return;
  api.fillRect(this->x + x, this->y + y, width, height, color);
}
void BGraphics::drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  if (!visible) return;
  api.drawRoundRect(this->x + x, this->y + y, width, height, radius, color);
}
void BGraphics::fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  if (!visible) return;
  api.fillRoundRect(this->x + x, this->y + y, width, height, radius, color);
}
void BGraphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  if (!visible || !str) return;
  api.drawText(str, this->x + x, this->y + y, size, color);
}
void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t width, uint16_t height) {
  if (!visible) return;
  api.drawBitmap(this->x + x, this->y + y, bitmap, width, height);
}
void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t width, uint16_t height) {
  if (!visible) return;
  api.drawBitmap(this->x + x, this->y + y, bitmap, mask, width, height);
}
BRect BGraphics::getTextBounds(const char* str, uint8_t size) {
  if (!str) return BRect();
  return api.getTextBounds(str, size);
}
void BGraphics::drawContour(int16_t x, int16_t y, const uint8_t mask[], uint16_t width, uint16_t height, uint16_t color) {
  if (!visible) return;
  api.drawContour(this->x + x, this->y + y, mask, width, height, color);
}
void BGraphics::drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  if (!visible) return;
  api.drawCircle(this->x + centerX, this->y + centerY, radius, color);
}
void BGraphics::fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  if (!visible) return;
  api.fillCircle(this->x + centerX, this->y + centerY, radius, color);
}
void BGraphics::drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  if (!visible) return;
  api.drawShade(this->x + x, this->y + y, width, height, color);
};
void BGraphics::fillScreen(uint16_t color) {
  if (!visible) return;
  api.fillScreen(color);
}
void BGraphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  if (!visible) return;
  api.drawLine(this->x + x1, this->y + y1, this->x + x2, this->y + y2, color);
}
