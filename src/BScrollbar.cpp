#include "BFocusManager.h"
#include "BScrollbar.h"

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

bool BScrollbar::thumbHitTest(BMouseInputEvent& event) {
  BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
  BRect rt = clientRect();
  if (orientation == B::horizontal) {
    return pt.x >= _thumbPos && pt.x <= _thumbPos + _thumbSize && pt.y >= rt.y && pt.y <= rt.y + rt.height;
  } else {
    return pt.y >= _thumbPos && pt.y <= _thumbPos + _thumbSize && pt.x >= rt.x && pt.x <= rt.x + rt.width;
  }
}

BRect BScrollbar::clientRect() {
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
  hideThumb(g);
  _thumbPos = pos;
  showThumb(g);
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
      if (thumbHitTest(event)) {
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
  hideThumb(g);
  updateThumb();
  showThumb(g);
  focusManager().endDraw();
}

void BScrollbar::hideThumb(BGraphics& g) {
  BRect rt = clientRect();
  auto b = (isFocused()) ? focusManager().theme().focusBackground : viewBackground(*this);  
  auto radius = focusManager().theme().buttonRadius;
  if (orientation == B::horizontal) {
    g.fillRoundRect(_thumbPos, rt.y, _thumbSize, rt.height, radius, b);
  } else {
    g.fillRoundRect(rt.x, _thumbPos, rt.width, _thumbSize, radius, b);
  }
}

void BScrollbar::showThumb(BGraphics& g) {
  BRect rt = clientRect();
  auto c = (isFocused()) ? focusManager().theme().focusColor : viewColor(*this);
  auto radius = focusManager().theme().buttonRadius;
  if (orientation == B::horizontal) {
    g.fillRoundRect(_thumbPos, rt.y, _thumbSize, rt.height, radius, c);
  } else {
    g.fillRoundRect(rt.x, _thumbPos, rt.width, _thumbSize, radius, c);
  }
}


void BScrollbar::draw(BGraphics& g) {
  BView::draw(g);
  auto c = (isFocused()) ? focusManager().theme().focusColor : viewColor(*this);
  auto b = (isFocused()) ? focusManager().theme().focusBackground : viewBackground(*this);
  auto radius = focusManager().theme().buttonRadius;
  g.fillRoundRect(0, 0, actualWidth, actualHeight, radius, b);
  g.drawRoundRect(0, 0, actualWidth, actualHeight, radius, c);
  BRect rt = clientRect();
  if (orientation == B::horizontal) {
    g.fillRoundRect(_thumbPos, rt.y, _thumbSize, rt.height, radius, c);
  } else {    
    g.fillRoundRect(rt.x, _thumbPos, rt.width, _thumbSize, radius, c);
  }
}

void BScrollbar::layout() {
  updateThumb();
  clearDirtyLayout();
  dirty();
}

void BScrollbar::updateThumb() {
  if (maximum - minimum != 0) {
    auto range = abs(maximum - minimum);
    auto rt = clientRect();
    auto minThumb = focusManager().theme().scrollbarMinThumb;
    int16_t scrollArea;
    if (orientation == B::horizontal) {    
      _thumbSize = min(rt.width, max(minThumb, step * 1.0 / range * rt.width));
      scrollArea = rt.width - _thumbSize;
      _thumbPos = rt.x;     
    } else {
      _thumbSize = min(rt.height, max(minThumb, step * 1.0 / range * rt.height));
      scrollArea = rt.height - _thumbSize;
      _thumbPos = rt.y;
    }
    
    _thumbPos += labs((long)value - minimum) * scrollArea / range;
  } else {
    _thumbPos = 0;
    _thumbSize = 0;
  }
}

void BScrollbar::measure(uint16_t availableWidth, uint16_t availableHeight) {
  if (orientation == B::horizontal && !height) {
    actualHeight = focusManager().theme().scrollbarSize;
  } else if (orientation == B::vertical && !width) {
    actualWidth = focusManager().theme().scrollbarSize;
  }
}
