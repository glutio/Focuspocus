#include <Arduino.h>
#include "BFocusManager.h"
#include "BScrollbar.h"

BScrollbar::BScrollbar() 
  : minimum(0), maximum(0), value(0), _thumbX(0), _thumbY(0), _thumbSize(0), orientation(horizontal) {
}

int16_t BScrollbar::pad() {
  return focusManager().theme().scrollbarPadding;
}

void BScrollbar::handleEvent(BInputEvent& event) {
  BControl::handleEvent(event);
  if (event.type & BInputEvent::evMouse) {
    handleMouse((BMouseInputEvent&)event);
  } else if (event.type & BInputEvent::evKeyboard) {
    handleKeyboard((BKeyboardInputEvent&)event);
  }
}

bool BScrollbar::thumbHitTest(BMouseInputEvent& event) {
  BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
  if (orientation == horizontal) {
    return pt.x >= _thumbX && pt.x <= _thumbX + _thumbSize && pt.y >= _thumbY && pt.y <= _thumbY + actualHeight - pad() * 2;
  }
}

void BScrollbar::moveThumb(BMouseInputEvent& event) {
  int val;
  int16_t x = _thumbX;
  int16_t y = _thumbY;
  if (orientation == horizontal) {
    auto scrollArea = actualWidth - _thumbSize - pad() * 2 - 2;
    auto minX = margin.left + 1 + pad();
    auto maxX = minX + scrollArea;
    x = min(maxX, max(_thumbX + (event.x - _oldX), minX));
    float normalizedThumbPos = x * 1.0 / scrollArea;
    val = (normalizedThumbPos * (maximum - minimum)) + minimum;
    if (x == minX) {
      val = minimum;
    } else if (x == maxX) {
      val = maximum;
    }
  } else {
  }
  Serial.println(val);
  redrawThumb(x, y);
  value = val;
  _thumbX = x;
  _thumbY = y;
  onChange(this, value);
}

void BScrollbar::handleMouse(BMouseInputEvent& event) {
  switch(event.type) {
    case BInputEvent::evMouseDown: {
      focus();
      if (thumbHitTest(event)) {
        focusManager().captureMouse(*this);
        _oldX = event.x;
        _oldY = event.y;
      }
      break;
    }
    case BInputEvent::evMouseUp: {
      focusManager().releaseMouse(*this);
    }
    case BInputEvent::evMouseMove: {      
        if (focusManager().capturingView() == this) {
          moveThumb(event);
        _oldX = event.x;
        _oldY = event.y;
        }
    }
  }
}

void BScrollbar::handleKeyboard(BKeyboardInputEvent& event) {
}

void BScrollbar::redrawThumb(int16_t x, int16_t y) {
  BGraphics g = focusManager().getGraphics(*this);
  auto c = (isFocused()) ? focusManager().theme().focusColor : color;
  auto b = (isFocused()) ? focusManager().theme().focusBackground : background;  
  g.fillRect(_thumbX, _thumbY, _thumbSize, actualHeight - pad() * 2 - 2, b);
  g.fillRect(x, y, _thumbSize, actualHeight - pad() * 2 - 2, c);
}

void BScrollbar::draw(BGraphics& g) {
  auto c = (isFocused()) ? focusManager().theme().focusColor : color;
  auto b = (isFocused()) ? focusManager().theme().focusBackground : background;
  g.fillRect(0, 0, actualWidth, actualHeight, b);
  g.drawRect(0, 0, actualWidth, actualHeight, c);
  g.fillRect(_thumbX, _thumbY, _thumbSize, actualHeight - pad() * 2 - 2, c);
}

void BScrollbar::layout() {
   if (maximum - minimum != 0) {
    if (orientation == horizontal) {
      auto range = abs(maximum - minimum);
      auto scrollWidth = actualWidth - pad() * 2 - 2;
      _thumbSize = min(actualWidth, max(25, 1.0 / range * scrollWidth));
      _thumbX = margin.left + 1 + pad() + (abs((uint32_t)value - minimum) * (scrollWidth - _thumbSize)) / range;
      _thumbY = margin.top + 1 + pad();
      Serial.println(_thumbX);
    }
  }
}
