#include "BProgressBar.h"
#include "BFocusManager.h"

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
    g.fillRect(now.x + from, now.y, to - from, now.height, now.width > old.width ? viewColor(*this) : viewBackground(*this));
  } else {
    int16_t from = min(old.height, now.height);
    int16_t to = max(old.height, now.height);
    g.fillRect(now.x, now.y + from, now.width, to - from, now.height > old.height ? viewColor(*this) : viewBackground(*this));
  }
  focusManager().endDraw();
}

BRect BProgressBar::clientRect() {
  BRect rt;
  auto border = 1;
  auto padding = 1;
  // Graphics and mapped points already exclude the margin.
  rt.x = padding + border;
  rt.y = padding + border;
  rt.width = actualWidth - (padding + border) * 2; 
  rt.height = actualHeight - (padding + border) * 2;
  return rt;
}

BRect BProgressBar::progressRect() {
  BRect rtProgress;
  BRect rt = clientRect();
  rtProgress.x = rt.x;
  rtProgress.y = rt.y;
  if (maximum - minimum != 0) {
    auto range = abs(maximum - minimum);
    if (orientation == B::horizontal) {    
      rtProgress.width = labs((long)value - minimum) * rt.width / range;
      rtProgress.height = rt.height;
    } else {
      rtProgress.height = labs((long)value - minimum) * rt.height / range;
      rtProgress.width = rt.width;
    }
  }
  return rtProgress;
}

void BProgressBar::draw(BGraphics& g) {
  BView::draw(g);
  auto c = viewColor(*this);
  auto b = viewBackground(*this);
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