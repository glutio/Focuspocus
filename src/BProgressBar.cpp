#include "BProgressBar.h"
#include "BFocusManager.h"
#include "BMinMax.h"

BProgressBar::BProgressBar() : minimum(0), maximum(0), value(0) {
  focusable = false;
  orientation = B::horizontal;
}

void BProgressBar::setValue(int16_t val) {
  val = max(minimum, min(maximum, val));
  if (val == value) {
    return;
  }
  if (!parent()) {
    value = val;  // not on a screen yet; drawn on first layout
    return;
  }
  BRect old = progressRect();
  value = val;
  BRect now = progressRect();
  BGraphics g = focusManager().getGraphics(*this);
  focusManager().beginDraw();
  if (orientation == B::horizontal) {
    int16_t from = min(old.width, now.width);
    int16_t to = max(old.width, now.width);
    g.fillRect(now.x + from, now.y, to - from, now.height, now.width > old.width ? actualColor() : actualBackground());
  } else {
    int16_t from = min(old.height, now.height);
    int16_t to = max(old.height, now.height);
    g.fillRect(now.x, now.y + from, now.width, to - from, now.height > old.height ? actualColor() : actualBackground());
  }
  focusManager().endDraw();
}

// The bar is inset by a 1 px border and 1 px padding.
BRect BProgressBar::clientRect() {
  return BRect(2, 2, actualWidth - 4, actualHeight - 4);
}

BRect BProgressBar::progressRect() {
  BRect rt = clientRect();
  long range = (long)maximum - minimum;
  long done = range > 0 ? (long)value - minimum : 0;
  if (orientation == B::horizontal) {
    rt.width = range > 0 ? done * rt.width / range : 0;
  } else {
    rt.height = range > 0 ? done * rt.height / range : 0;
  }
  return rt;
}

void BProgressBar::draw(BGraphics& g) {
  BView::draw(g);
  auto c = actualColor();
  auto b = actualBackground();
  g.fillRect(0, 0, actualWidth, actualHeight, b);
  g.drawRect(0, 0, actualWidth, actualHeight, c);
  BRect rt = progressRect();
  g.fillRect(rt.x, rt.y, rt.width, rt.height, c);
}

void BProgressBar::measure(uint16_t, uint16_t) {
  if (orientation == B::horizontal && !height) {
    actualHeight = focusManager().theme().scrollbarSize;
  } else if (orientation == B::vertical && !width) {
    actualWidth = focusManager().theme().scrollbarSize;
  }
}