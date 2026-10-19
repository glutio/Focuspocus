// BGauge: a dial with a needle. Built only on the public API, so it doubles as
// a template for custom widgets.
// A semicircle dial with ticks and a needle; minimum is on the left.
// How a custom view works:
//   measure() picks a default size, draw() paints the whole view,
//   setValue() repaints just the needle via focusManager().getGraphics().
// No floats: the needle angle uses an integer sine approximation.
#ifndef __BGAUGE_H__
#define __BGAUGE_H__

#include "BView.h"
#include "BFocusManager.h"

class BGauge: public BView {
public:
  int16_t minimum;
  int16_t maximum;
  int16_t value;
  BNullable<uint16_t> needleColor;  // default: theme focusColor

  BGauge() : minimum(0), maximum(100), value(0) {
    focusable = false;
  }

  // Clamps to [minimum, maximum] and redraws only the needle.
  void setValue(int16_t val) {
    val = val < minimum ? minimum : val > maximum ? maximum : val;
    if (val == value) {
      return;
    }
    if (!parent()) {
      value = val;  // not on a screen yet; drawn on first layout
      return;
    }
    BGraphics g = focusManager().getGraphics(*this);
    focusManager().beginDraw();
    drawNeedle(g, actualBackground());
    value = val;
    drawNeedle(g, needle());
    focusManager().endDraw();
  }

  virtual void measure(uint16_t, uint16_t) {
    if (!width) {
      actualWidth = 120;
    } else if (width > 0) {
      actualWidth = width;
    }
    if (!height) {
      actualHeight = actualWidth / 2 + 4;  // a stretched width is known after layout
    }
  }

  virtual void draw(BGraphics& g) {
    BView::draw(g);
    uint16_t c = actualColor();
    g.fillRect(0, 0, actualWidth, actualHeight, actualBackground());
    int16_t r = radius();
    for (int16_t a = 0; a < 180; a += 6) {  // the arc, as short lines
      g.drawLine(px(a, r), py(a, r), px(a + 6, r), py(a + 6, r), c);
    }
    for (int16_t a = 0; a <= 180; a += 18) {  // 11 ticks
      g.drawLine(px(a, r - 6), py(a, r - 6), px(a, r), py(a, r), c);
    }
    drawNeedle(g, needle());
  }

  // Integer sine for whole degrees in [-180, 180], scaled by 1024
  // (Bhaskara I approximation, error < 0.2%).
  static int16_t sin1024(int16_t deg) {
    if (deg < 0) {
      return -sin1024(-deg);
    }
    long p = (long)deg * (180 - deg);
    return (int16_t)(4096L * p / (40500L - p));
  }

private:
  uint16_t needle() {
    return needleColor.hasValue() ? (uint16_t)needleColor : focusManager().theme().focusColor;
  }

  int16_t radius() {
    int16_t r = actualWidth / 2 < actualHeight - 4 ? actualWidth / 2 : actualHeight - 4;
    return r - 1;
  }
  int16_t centerX() { return actualWidth / 2; }
  int16_t centerY() { return actualHeight - 4; }

  // A point at `deg` (0 = right, 90 = up, 180 = left) and distance `r` from the center.
  int16_t px(int16_t deg, int16_t r) { return centerX() + (long)r * sin1024(90 - deg) / 1024; }
  int16_t py(int16_t deg, int16_t r) { return centerY() - (long)r * sin1024(deg) / 1024; }

  int16_t angle() {
    long range = (long)maximum - minimum;
    return range > 0 ? 180 - (int16_t)(180L * (value - minimum) / range) : 180;
  }

  void drawNeedle(BGraphics& g, uint16_t color) {
    int16_t a = angle();
    int16_t r = radius() - 9;  // stays clear of the ticks
    g.drawLine(centerX(), centerY(), px(a, r), py(a, r), color);
    g.fillCircle(centerX(), centerY(), 3, needle());  // hub
  }
};

#endif
