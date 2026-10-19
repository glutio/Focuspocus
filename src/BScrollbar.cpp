#include "BFocusManager.h"
#include "BScrollbar.h"
#include "BMinMax.h"

BScrollbar::BScrollbar() 
  : _grabOffset(0), _thumbPos(0), _thumbSize(0), minimum(0), maximum(0), value(0), step(1), orientation(B::horizontal) {    
}

void BScrollbar::handleEvent(BInputEvent& event) {
  BControl::handleEvent(event);
  if (event.type & BInputEvent::evMouse) {
    handleMouse((BMouseInputEvent&)event);
  } else if (event.type & BInputEvent::evKeyboard) {
    handleKeyboard((BKeyboardInputEvent&)event);
  }
}

// Thumb and track are inset by a 1 px border and 1 px padding.
BRect BScrollbar::clientRect() {
  return BRect(2, 2, actualWidth - 4, actualHeight - 4);
}

BRect BScrollbar::thumbRect() {
  BRect rt = clientRect();
  if (orientation == B::horizontal) {
    return BRect(_thumbPos, rt.y, _thumbSize, rt.height);
  }
  return BRect(rt.x, _thumbPos, rt.width, _thumbSize);
}

void BScrollbar::fillThumb(BGraphics& g, uint16_t color) {
  BRect rt = thumbRect();
  g.fillRoundRect(rt.x, rt.y, rt.width, rt.height, focusManager().theme().buttonRadius, color);
}

uint16_t BScrollbar::thumbColor() {
  return isFocused() ? focusManager().theme().focusColor : actualColor();
}

uint16_t BScrollbar::trackColor() {
  return isFocused() ? focusManager().theme().focusBackground : actualBackground();
}
// Thumb follows the pointer, keeping the point where it was grabbed.
void BScrollbar::moveThumb(BMouseInputEvent& event) {
  BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
  BRect rt = clientRect();
  bool horizontal = orientation == B::horizontal;
  int16_t start = horizontal ? rt.x : rt.y;
  int16_t area = (horizontal ? rt.width : rt.height) - _thumbSize;
  int16_t pos = (horizontal ? pt.x : pt.y) - _grabOffset;
  pos = max(start, min((int16_t)(start + area), pos));

  int16_t val = minimum;
  if (area > 0) {
    long range = (long)maximum - minimum;
    val = minimum + ((long)(pos - start) * range + area / 2) / area;
  }

  BGraphics g = focusManager().getGraphics(*this);
  focusManager().beginDraw();
  fillThumb(g, trackColor());
  _thumbPos = pos;
  fillThumb(g, thumbColor());
  focusManager().endDraw();

  if (value != val) {
    value = val;
    raiseOnChange(value);
  }
}

void BScrollbar::handleMouse(BMouseInputEvent& event) {
  switch(event.type) {
    case BInputEvent::evMouseDown: {
      focus();
      BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
      int16_t p = (orientation == B::horizontal) ? pt.x : pt.y;
      if (thumbRect().contains(pt.x, pt.y)) {
        focusManager().captureMouse(*this);
        _grabOffset = p - _thumbPos;
      } else if (p < _thumbPos) {
        stepBy(-step);
      } else if (p > _thumbPos + _thumbSize) {
        stepBy(step);
      }
      break;
    }
    case BInputEvent::evMouseMove:
    case BInputEvent::evMouseUp: {
      if (focusManager().capturingView() == this) {
        moveThumb(event);
        if (event.type == BInputEvent::evMouseUp) {
          focusManager().releaseMouse(*this);
        }
      }
      break;
    }
    default:
      break;
  }
}

void BScrollbar::handleKeyboard(BKeyboardInputEvent& event) {
  if (event.type != BInputEvent::evKeyDown) {
    return;
  }
  bool horizontal = orientation == B::horizontal;
  if (event.code == (horizontal ? BKeyboard::kbLeft : BKeyboard::kbUp)) {
    stepBy(-step);
  } else if (event.code == (horizontal ? BKeyboard::kbRight : BKeyboard::kbDown)) {
    stepBy(step);
  }
}

void BScrollbar::stepBy(int16_t delta) {
  int16_t old = value;
  long val = (long)value + delta;
  setValue((int16_t)max((long)minimum, min((long)maximum, val)));
  if (value != old) {
    raiseOnChange(value);
  }
}

void BScrollbar::setValue(int16_t val) {
  val = max(minimum, min(maximum, val));
  if (val == value) {
    return;
  }
  value = val;
  if (!parent()) {
    return;  // not on a screen yet; drawn on first layout
  }
  BGraphics g = focusManager().getGraphics(*this);
  focusManager().beginDraw();
  fillThumb(g, trackColor());
  updateThumb();
  fillThumb(g, thumbColor());
  focusManager().endDraw();
}

void BScrollbar::draw(BGraphics& g) {
  BView::draw(g);
  auto radius = focusManager().theme().buttonRadius;
  g.fillRoundRect(0, 0, actualWidth, actualHeight, radius, trackColor());
  g.drawRoundRect(0, 0, actualWidth, actualHeight, radius, thumbColor());
  fillThumb(g, thumbColor());
}
void BScrollbar::layout() {
  updateThumb();
  clearDirtyLayout();
  dirty();
}

void BScrollbar::updateThumb() {
  long range = (long)maximum - minimum;
  if (range <= 0) {
    _thumbPos = 0;
    _thumbSize = 0;
    return;
  }
  BRect rt = clientRect();
  bool horizontal = orientation == B::horizontal;
  int16_t length = horizontal ? rt.width : rt.height;
  long size = max((long)focusManager().theme().scrollbarMinThumb, (long)step * length / range);
  _thumbSize = (int16_t)min((long)length, size);
  _thumbPos = (horizontal ? rt.x : rt.y) + ((long)value - minimum) * (length - _thumbSize) / range;
}
void BScrollbar::measure(uint16_t, uint16_t) {
  if (orientation == B::horizontal && !height) {
    actualHeight = focusManager().theme().scrollbarSize;
  } else if (orientation == B::vertical && !width) {
    actualWidth = focusManager().theme().scrollbarSize;
  }
}
