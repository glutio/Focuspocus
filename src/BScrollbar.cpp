#include <Arduino.h>
#include "BFocusManager.h"
#include "BScrollbar.h"

BScrollbar::BScrollbar() 
  : minimum(0), maximum(0), value(0), _thumbPos(0), _thumbSize(0), orientation(B::horizontal), step(1) {    
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
  rt.x = margin.left + padding + border;
  rt.y = margin.top + padding + border;
  rt.width = actualWidth - (padding + border) * 2; 
  rt.height = actualHeight - (padding + border) * 2;
  return rt;
}

void BScrollbar::moveThumb(BMouseInputEvent& event) {
  int val;
  int16_t pos;
  auto rt = clientRect();
  if (orientation == B::horizontal) {
    auto scrollArea = rt.width - _thumbSize;
    pos = min(rt.x + scrollArea, max(_thumbPos + (event.x - _oldX), rt.x));
    float normalizedThumbPos = pos * 1.0 / scrollArea;
    val = (normalizedThumbPos * (maximum - minimum)) + minimum;
    if (pos == rt.x) {
      val = minimum;
    } else if (x == rt.x + scrollArea) {
      val = maximum;
    }
  } else {
    auto scrollArea = rt.height - _thumbSize;
    pos = min(rt.y + scrollArea, max(_thumbPos + (event.y - _oldY), rt.y));
    float normalizedThumbPos = pos * 1.0 / scrollArea;
    val = (normalizedThumbPos * (maximum - minimum)) + minimum;
    if (pos == rt.y) {
      val = minimum;
    } else if (x == rt.y + scrollArea) {
      val = maximum;
    }
  }

  if (value != val) {
    value = val;
    onChange(this, value);
  }
  BGraphics g = focusManager().getGraphics(*this);
  focusManager().beginDraw();
  hideThumb(g);
  _thumbPos = pos;
  showThumb(g);
  focusManager().endDraw();
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
      else { 
        auto pt = focusManager().mapScreenToView(*this, event.x, event.y);
        BKeyboardInputEvent kbdEvent;
        kbdEvent.type = BInputEvent::evKeyDown;
        if (orientation == B::horizontal) {
          if (pt.x < _thumbPos) {
            kbdEvent.code = BKeyboard::kbLeft;
          }
          if (pt.x > _thumbPos + _thumbSize) {
            kbdEvent.code = BKeyboard::kbRight;
          }
        }
        else {
          if (pt.y < _thumbPos) {
            kbdEvent.code = BKeyboard::kbUp;
          }
          if (pt.y > _thumbPos + _thumbSize) {
            kbdEvent.code = BKeyboard::kbDown;
          }
        }
        handleKeyboard(kbdEvent);
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
  switch (event.type) {
    case BInputEvent::evKeyDown: {
      if ((orientation == B::horizontal && (event.code == BKeyboard::kbLeft || event.code == BKeyboard::kbRight)) ||
          (orientation == B::vertical   && (event.code == BKeyboard::kbUp   || event.code == BKeyboard::kbDown))) 
      {
        int16_t val;
        switch (event.code) {
          case BKeyboard::kbLeft: 
            val = max(minimum, value - step);
            break;
          case BKeyboard::kbRight:
            val = min(maximum, value + step);
            break;
          case BKeyboard::kbUp: 
            val = max(minimum, value - step);
            break;
          case BKeyboard::kbDown:
            val = min(maximum, value + step);
            break;
        }
        if (value != val) {
          value = val;
          onChange(this, value);
        }
        BGraphics g = focusManager().getGraphics(*this);
        focusManager().beginDraw();
        hideThumb(g);
        layout();
        showThumb(g);
        focusManager().endDraw();
      }
    }
    break;
  }
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
    
    _thumbPos += abs((int32_t)value - minimum) * scrollArea / range;
  } else {
    _thumbPos = 0;
    _thumbSize = 0;    
  }  
  clearDirtyLayout();
  dirty();
}

void BScrollbar::measure(uint16_t availableWidth, uint16_t availableHeight) {
  if (orientation == B::horizontal && !height) {
    actualHeight = focusManager().theme().scrollbarSize;
  } else if (orientation == B::vertical && !width) {
    actualWidth = focusManager().theme().scrollbarSize;
  }
}
