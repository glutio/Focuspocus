#include <Arduino.h>
#include "BGraphics.h"
#include "BView.h"

BGraphics::BGraphics(Adafruit_GFX& g) : x(0), y(0), width(g.width()), height(g.height()), _g(g) {  }  

void BGraphics::drawRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  _g.drawRect(this->x + x, this->y + y, width, height, color);
}

void BGraphics::fillRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  _g.fillRect(this->x + x, this->y + y, width, height, color);
}

void BGraphics::drawRoundRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius, int16_t color) {
  _g.drawRoundRect(this->x + x, this->y + y, width, height, radius, color);
}

void BGraphics::fillRoundRect(int16_t x, int16_t y, int16_t width, int16_t height, int16_t radius, int16_t color) {
  _g.fillRoundRect(this->x + x, this->y + y, width, height, radius, color);
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

void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {
  _g.drawRGBBitmap(this->x + x, this->y + y, bitmap, w, h);
}

void BGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) {
  _g.drawRGBBitmap(this->x + x, this->y + y, bitmap, mask, w, h);
}

//#define PI 3.14159265

void BGraphics::drawArc(int centerX, int centerY, int radius, int startAngle, int endAngle, uint16_t color) {
    // Convert the start and end angles to radians
    double startRad = startAngle * PI / 180.0;
    double endRad = endAngle * PI / 180.0;
    
    int lastX = -1;
    int lastY = -1;

    for (double theta = startRad; theta <= endRad; theta += 0.01) {  // increment by small value for smooth curve
        int x = centerX + radius * cos(theta);
        int y = centerY + radius * sin(theta);

        if (x != lastX || y != lastY) {
            _g.writePixel(this->x + x, this->y + y, color);
            lastX = x;
            lastY = y;
        }
    }
}

void BGraphics::drawContour(int16_t x, int16_t y, uint8_t mask[], int16_t width, int16_t height, int16_t color) {
  _g.startWrite();
  int16_t bw = (width + 7) / 8;
  for (int16_t i = 0; i < height; i++) {
    int16_t leftMost = -1, rightMost = -1;
    for (int16_t j = 0; j < width; j++) {      
      auto b = pgm_read_byte(&mask[i * bw + j / 8]);
      auto bit = b & (0x80 >> (j % 8));
      if (bit) {
          leftMost = j;
          break;
      }
    }
      
    for (int16_t j = width - 1; j >= 0; j--) {      
      auto b = pgm_read_byte(&mask[i * bw + j / 8]);
      auto bit = b & (0x80 >> (j % 8));
      if (bit) {
          rightMost = j;
          break;
      }
    }
   
    if (leftMost >=0) {
      _g.writePixel(this->x + x + leftMost, this->y + y + i, color);
    }
    if (rightMost >=0) {
      _g.writePixel(this->x + x + rightMost, this->y + y + i, color);
    }
  }

  for (int16_t i = 0; i < width; i++) {
    int16_t topMost = -1, bottomMost = -1;
    for (int16_t j = 0; j < height; j++) {
      auto b = pgm_read_byte(&mask[j * bw + i / 8]);
      auto bit = b & (0x80 >> (i % 8));
      if (topMost < 0 && bit) {
          topMost = j;
          break;
      }
    }
    for (int16_t j = height - 1; j >= 0; j--) {
      auto b = pgm_read_byte(&mask[j * bw + i / 8]);
      auto bit = b & (0x80 >> (i % 8));
      if (bit) {
        bottomMost = j;
        break;
      }
    }

    if (topMost >=0) {
      _g.writePixel(this->x +x + i, this->y + y + topMost, color);
    }
    if (bottomMost >=0) {
      _g.writePixel(this->x + x + i, this->y + y + bottomMost, color);
    }
  }
  _g.endWrite();
}

void BGraphics::drawCircle(int centerX, int centerY, int radius, uint16_t color) {
  _g.drawCircle(this->x + centerX, this->y + centerY, radius, color);
}

void BGraphics::fillCircle(int centerX, int centerY, int radius, uint16_t color) {
  _g.fillCircle(this->x + centerX, this->y + centerY, radius, color);
}
