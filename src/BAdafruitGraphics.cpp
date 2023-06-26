#include "BAdafruitGraphics.h"
#include "BView.h"

/*
  Adafruit Graphics
*/
BAdafruitGraphics::BAdafruitGraphics(Adafruit_GFX& g) : g(g) { }

int16_t BAdafruitGraphics::displayWidth() { 
  return g.width();
}
int16_t BAdafruitGraphics::displayHeight() {
  return g.height();
}

void BAdafruitGraphics::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  g.drawRect(x, y, width, height, color);
}

void BAdafruitGraphics::fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  g.fillRect(x, y, width, height, color);
}

void BAdafruitGraphics::drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  g.drawRoundRect(x, y, width, height, radius, color);
}

void BAdafruitGraphics::fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  g.fillRoundRect(x, y, width, height, radius, color);
}

void BAdafruitGraphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  g.setCursor(x, y);
  g.setTextColor(color);
  g.setTextSize(size);
  g.print(str);
}

BRect BAdafruitGraphics::getTextBounds(const char* str, uint8_t size) {
  BRect rt;
  g.setTextSize(size);
  g.getTextBounds(str, 0, 0, &rt.x, &rt.y, &rt.width, &rt.height);
  return rt;
}

void BAdafruitGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h) {
  g.drawRGBBitmap(x, y, bitmap, w, h);
}

void BAdafruitGraphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h) {
  g.drawRGBBitmap(x, y, bitmap, mask, w, h);
}

void BAdafruitGraphics::drawArc(int16_t centerX, int16_t centerY, uint16_t radius, int16_t startAngle, int16_t endAngle, uint16_t color) {
    // Convert the start and end angles to radians
    double startRad = startAngle * PI / 180.0;
    double endRad = endAngle * PI / 180.0;
    
    int lastX = -1;
    int lastY = -1;

    for (double theta = startRad; theta <= endRad; theta += 0.01) {  // increment by small value for smooth curve
        int x = centerX + radius * cos(theta);
        int y = centerY + radius * sin(theta);

        if (x != lastX || y != lastY) {
            g.writePixel(x, y, color);
            lastX = x;
            lastY = y;
        }
    }
}

void BAdafruitGraphics::drawContour(int16_t x, int16_t y, uint8_t mask[], uint16_t width, uint16_t height, uint16_t color) {
  g.startWrite();
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
      g.writePixel(x + leftMost, y + i, color);
    }
    if (rightMost >=0) {
      g.writePixel(x + rightMost, y + i, color);
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
      g.writePixel(x + i, y + topMost, color);
    }
    if (bottomMost >=0) {
      g.writePixel(x + i, y + bottomMost, color);
    }
  }
  g.endWrite();
}

void BAdafruitGraphics::drawCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  g.drawCircle(centerX, centerY, radius, color);
}

void BAdafruitGraphics::fillCircle(int16_t centerX, int16_t centerY, uint16_t radius, uint16_t color) {
  g.fillCircle(centerX, centerY, radius, color);
}

void BAdafruitGraphics::drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
    g.startWrite();
    // Draw a dithered pattern to simulate shading    
    for (int16_t j = y; j < y + height; j++) {
        for (int16_t i = x; i < x + width; i++) {
            // Create a checkerboard pattern by drawing a pixel
            // if the sum of the coordinates is even
            if ((i + j) % 2 == 0) {
                g.writePixel(i, j, color);
            }
        }
    }
    g.endWrite();
}

void BAdafruitGraphics::fillScreen(uint16_t color) {
  g.fillScreen(color);
}

void BAdafruitGraphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  g.drawLine(x1, y1, x2, y2, color);
}