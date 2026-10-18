#include "BTextLabel.h"
#include "BFocusManager.h"
#include "BMinMax.h"

BTextLabel::BTextLabel() : horizontalAlignment(B::left), verticalAlignment(B::top), text(0) {
  focusable = false;
}

void BTextLabel::setText(const char* value) {
  text = value;
  if (parent()) {
    dirtyLayout();  // also redraws
  }
}

void BTextLabel::measure(uint16_t availableWidth, uint16_t availableHeight) {
  BView::measure(availableWidth, availableHeight);
  if (width <= 0 || height <= 0) {
    BRect rt = focusManager().getTextBounds(text, viewFontSize(*this));
    if (width <= 0) {
      actualWidth = min((uint16_t)rt.width, availableWidth);
    }
    if (height <= 0) {
      actualHeight = min((uint16_t)rt.height, availableHeight);
    }
  }
}

void BTextLabel::draw(BGraphics& g) {
  BRect rt = g.getTextBounds(text, viewFontSize(*this));
  int16_t x = 0;
  int16_t y = 0;
  if (horizontalAlignment == B::right) {    
    x = g.width - rt.width;  
  } else if (horizontalAlignment == B::center) {
    x = (g.width - rt.width) / 2;
  }
  if (verticalAlignment == B::bottom) {    
    y = g.height - rt.height;  
  } else if (verticalAlignment == B::center) {
    y = (g.height - rt.height) / 2;
  }
  // Fill only around the text; the text paints its own box, so it never flashes.
  uint16_t bg = viewBackground(*this);
  int16_t l = max(x, (int16_t)0), t = max(y, (int16_t)0);
  int16_t r = min((int16_t)(x + rt.width), g.width), b = min((int16_t)(y + rt.height), g.height);
  if (r <= l || b <= t) {
    g.fillRect(0, 0, g.width, g.height, bg);
  } else {
    if (t > 0) g.fillRect(0, 0, g.width, t, bg);
    if (b < g.height) g.fillRect(0, b, g.width, g.height - b, bg);
    if (l > 0) g.fillRect(0, t, l, b - t, bg);
    if (r < g.width) g.fillRect(r, t, g.width - r, b - t, bg);
  }
  BView::draw(g);
  g.drawText(text, x, y, viewFontSize(*this), viewFontColor(*this), bg);
}