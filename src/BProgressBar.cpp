#include <Arduino.h>
#include "BProgressBar.h"
#include "BFocusManager.h"

BProgressBar::BProgressBar() : minimum(0), maximum(0), value(0) {
  focusable = false;
  orientation = B::horizontal;
}

void BProgressBar::onTimerTick(BFocusManager* fm, bool) {
  static unsigned long ms = millis();
  static int dir = 1;
  if (millis() - ms > 500) {
    if (value == maximum || value == minimum) dir*=-1;
    setValue(value + dir);
    ms = millis();
  }
}

void BProgressBar::setValue(int16_t val) {
  BGraphics g = focusManager().getGraphics(*this);
  BRect rtold = progressRect();
  auto radius = focusManager().theme().buttonRadius;
  value = max(minimum, min(maximum, val));
  BRect rtnew = progressRect();
  if (orientation == B::horizontal) {
    if (rtold.width > rtnew.width) {
      g.fillRect(rtnew.x + rtnew.width, rtnew.y, rtold.width - rtnew.width, rtold.height, viewBackground(*this));
    }
    else {
      g.fillRect(rtold.x + rtold.width, rtold.y, rtnew.width - rtold.width, rtold.height, viewColor(*this));
    }
  }
}

void BProgressBar::parentChanged(BPanel* old) {
  focusManager().onTimerTick += BFocusManager::TimerTickEvent(this, &BProgressBar::onTimerTick);
}

BRect BProgressBar::clientRect() {
  BRect rt;
  auto border = 1;
  auto padding = 1;
  rt.x = margin.left + padding + border;
  rt.y = margin.top + padding + border;
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
      rtProgress.width = abs((int32_t)value - minimum) * rt.width / range;
      rtProgress.height = rt.height;
    } else {
      rtProgress.height = abs((int32_t)value - minimum) * rt.height / range;
      rtProgress.width = rt.width;
    }
  }
  return rtProgress;
}

void BProgressBar::draw(BGraphics& g) {
  BView::draw(g);
  auto c = viewColor(*this);
  auto b = viewBackground(*this);
  auto radius = focusManager().theme().buttonRadius;
  g.fillRect(0, 0, actualWidth, actualHeight, b);
  g.drawRect(0, 0, actualWidth, actualHeight, c);
  BRect rt = progressRect();
  g.fillRect(rt.x, rt.y, rt.width, rt.height, c);
}

void BProgressBar::measure(uint16_t availableWidth, uint16_t availableHeight) {
  if (orientation == B::horizontal && !height) {
    actualHeight = focusManager().theme().scrollbarSize;
  } else if (orientation == B::vertical && !width) {
    actualWidth = focusManager().theme().scrollbarSize;
  }
}