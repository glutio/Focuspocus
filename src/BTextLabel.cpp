#include <Arduino.h>
#include "BTextLabel.h"
#include "BFocusManager.h"

BTextLabel::BTextLabel() : horizontalAlignment(left), verticalAlignment(top), text(0) {
  focusable = false;
}

void BTextLabel::measure(int16_t availableWidth, int16_t availableHeight) {
  if (!width || !height) {
    BGraphics g = focusManager().getGraphics();
    BRect rt = g.getTextBounds(text, viewFontSize(*this));
    if (!width) {
      actualWidth = min(rt.width, availableWidth);
    }
    if (!height) {
      actualHeight = min(rt.height, availableHeight);
    }
  }
}

void BTextLabel::draw(BGraphics& g) {
  BView::draw(g);
  BRect rt = g.getTextBounds(text, viewFontSize(*this));
  int16_t x = 0;
  int16_t y = 0;
  if (horizontalAlignment == right) {    
    x = g.width - rt.width;  
  } else if (horizontalAlignment == center) {
    x = (g.width - rt.width) / 2;
  }
  if (verticalAlignment == bottom) {    
    y = g.height - rt.height;  
  } else if (verticalAlignment == center) {
    y = (g.height - rt.height) / 2;
  }
  g.fillRect(1, 1, g.width - 2, g.height - 2, viewBackground(*this));
  g.drawText(text, x, y, viewFontSize(*this), viewFontColor(*this));
}