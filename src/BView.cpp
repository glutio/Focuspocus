#include <Arduino.h>
#include "BView.h"
#include "BFocusManager.h"

void BView::handleEvent(BInputEvent& event) {
  if (event.type & BInputEvent::evMouse) {
    onMouse(this, (BMouseInputEvent&)event);
  }
}

BView::BView() : x(0), y(0), width(-1), height(-1), minWidth(-1), minHeight(-1), maxWidth(-1), maxHeight(-1), _isDirty(true) {}

bool BView::hitTest(uint16_t ptX, uint16_t ptY) {
  return ptX >= x && ptX <= x + _actualWidth && ptY >= y && ptY <= y + _actualHeight;
}

int16_t BView::actualWidth() {
  return _actualWidth;
}

int16_t BView::actualHeight() {
  return _actualHeight;
}

void BView::dirty() {
  _isDirty = true;
  BFocusManager::dirty();
}

void BControl::focus() {
  BFocusEventArgs args;
  args.blurred = BFocusManager::focus(*this);
  if (args.blurred != this) {
    args.focused = this;
    if (args.blurred) {
      args.blurred->onFocus(this, args);
    }
    onFocus(this, args);
  }
}

bool BControl::isFocused() {
  return BFocusManager::focusedElement() == this;
}

BFontAware::BFontAware() : fontSize(1), fontColor(0xFFFF) {
}

BButton::BButton() : _isDown(false), _capture(false) {
  onMouse += MouseEvent(this, &BButton::handleMouse);
  onFocus += FocusEvent(this, &BButton::handleFocus);
}

void BButton::handleFocus(BControl* sender, BFocusEventArgs& args) {
  dirty();
}

void BButton::handleMouse(BView* sender, BMouseInputEvent& event) {
  switch (event.type) {
    case BInputEvent::evMouseDown: 
    {
      if (!_isDown) {
        _isDown = true;
        BFocusManager::captureMouse(*this);
        onClick(this, true);
        focus();
        dirty();
      }
      break;
    }
    case BInputEvent::evMouseUp:
    {
      BFocusManager::releaseMouse(*this);
      BPoint pt = BFocusManager::mapScreenToView(event.x, event.y, *this);
      bool hit = hitTest(pt.x, pt.y);
      onClick(this, hit);
      _isDown = false;
      dirty();
      break;
    }
    case BInputEvent::evMouseMove:
    {
      if (event.buttonDown && BFocusManager::capturingView() == this) {
        BPoint pt = BFocusManager::mapScreenToView(event.x, event.y, *this);
        _isDown = hitTest(pt.x, pt.y);
        dirty();
      }
      break;
    }
  }
}

void BButton::draw(BGraphics& g) {
  //g.fillRect(x, y, actualWidth(), actualHeight(), 0x0000);
  auto c = isFocused() ? 0xFF20 : color;
  if (_isDown) {
    g.drawRect(x, y, actualWidth(), actualHeight(), 0);
    g.drawRect(x+2, y+2, actualWidth()-4, actualHeight()-4, c);
  }
  else {
    g.drawRect(x+2, y+2, actualWidth()-4, actualHeight()-4, 0);
    g.drawRect(x, y, actualWidth(), actualHeight(), c);
  }
  drawContent(g);
}

void BButton::drawContent(BGraphics& g) {
  auto rt = g.getTextBounds(text, fontSize);  
  g.drawText(text, x + (actualWidth() - rt.width)/2, y + (actualHeight() - rt.height)/2, fontSize, fontColor);
}

BPanel::BPanel(BView* children[], unsigned count, unsigned capacity) 
  : _children(children, count, capacity), _needsLayout(true) {
}

void BPanel::add(BView* view) {
  _children.Add(view);
}

void BPanel::remove(BView* view) {
  _children.Remove(view);
}

void BPanel::draw(BGraphics& graphics) {
  BGraphics g(graphics);
  applyOffset(g.x, g.y, g.width, g.height);    
  // Serial.print(g.x); Serial.print(" ");
  // Serial.print(g.y); Serial.print(" ");
  // Serial.print(g.width); Serial.print(" ");
  // Serial.print(g.height); Serial.println();
  
  // Serial.print(x); Serial.print(" ");
  // Serial.print(y); Serial.print(" ");
  // Serial.print(actualWidth()); Serial.print(" ");
  // Serial.print(actualHeight()); Serial.println();
  
  graphics.drawRect(x, y, _actualWidth, _actualHeight, 0xFFFF);
  for(auto i = 0; i < _children.Length(); ++i) {
    _children[i]->draw(g);
  }
}

void BPanel::applyOffset(int16_t& x, int16_t& y) {
  x += this->x + padding.left;
  y += this->y + padding.top;
}

void BPanel::applyOffset(int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += this->x + padding.left;
  y += this->y + padding.top;
  width = actualWidth() - (padding.right + padding.left);
  height = actualHeight() - (padding.top + padding.bottom);
}

int16_t BPanel::applyMinMax(int16_t val, int16_t minimum, int16_t maximum) {    
  if (minimum >= 0 && val < minimum) {
    return minimum;
  } else if (maximum >=0 && val > maximum) {
    return maximum;
  }
  return val;
}

void BPanel::layout() {
  Serial.print("layout1");
  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];
    view._actualWidth = view.width;
    view._actualHeight = view.height;
    
    BPanel* panel = view.asPanel();
    if (panel) {
      panel->layout();
    }
  }
}

int16_t BPanel::clientWidth() {
  return _actualWidth - padding.left - padding.right;
};

int16_t BPanel::clientHeight() {
  return _actualHeight - padding.top - padding.bottom;
}

void BStackPanel::layout() {
  if (orientation == horizontal) {
    layoutHorizontal();
  }
  else {
    layoutVertical();
  }

  for(unsigned i = 0; i < _children.Length(); ++i) {
    BPanel* panel = _children[i]->asPanel();
    if (panel) {
      panel->layout();
    }
  }
}

void BStackPanel::layoutVertical() {
  int16_t totalSize = clientHeight() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0;

  int16_t fixedSize = 0;
  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];
    if (view.height < 0) {
      totalFactor += abs(view.height);
      view._actualHeight = -1;      
    } else {
      fixedSize += view.height;
      view._actualHeight = view.height;
    }

    if (view.width < 0) {
      view._actualWidth = applyMinMax(clientWidth(), view.minWidth, view.maxWidth);
    }
    else {
      view._actualWidth = view.width;
    }
  }

  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  int ct = 10 * _children.Length();
  while(ct-- > 0) {
    bool converged = true;
    for(unsigned i = 0; i < _children.Length(); ++i) {
      BView& view = *_children[i];
      if (view.height < 0 && view._actualHeight < 0) {
        uint16_t proposedSize = abs(view.width) * unitSize;
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minWidth, view.maxWidth);
        if (proposedSize != minmaxSize) {
          view._actualHeight = minmaxSize;
          fixedSize += minmaxSize;
          totalFactor -= abs(view.height);
          converged = false;
        }
      }
    }    
    if (converged) {
      break;
    }
    unitSize = (totalSize - fixedSize) / totalFactor;
  }

  totalSize = 0;
  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];
    if (view.height < 0 && view._actualHeight < 0) {
      view._actualHeight = abs(view.height) * unitSize;
    }
    totalSize += view._actualHeight;
  }

  int16_t startPos = 0;
  if (verticalAlignment == bottom) {
    startPos = clientHeight() - totalSize - ((_children.Length() - 1) * spacing);
  } else if (verticalAlignment == center) {
    startPos = (clientHeight() - totalSize - ((_children.Length() -1) * spacing)) /2;
  }

  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];  
    view.y = startPos;
    startPos += view._actualHeight + spacing;    

    if (horizontalAlignment == right) {
      view.x = clientWidth() - view._actualWidth;
    } else if (horizontalAlignment == center) {
      view.x = (clientWidth() - view._actualWidth) /2;
    }
  }
}

void BStackPanel::layoutHorizontal() {
  // compute size available for controls
  int16_t totalSize = clientWidth() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0; // sum of proportion factors
  int16_t fixedSize = 0; // sum of sizes of fixed-size controls

  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];
    if (view.width < 0) {
      totalFactor += abs(view.width);
      view._actualWidth = -1; // mark for auto layout     
    } else {
      fixedSize += view.width;
      view._actualWidth = view.width; // fixed size
    }

    if (view.height < 0) {
      // occupy entire area in this dimension
      view._actualHeight = applyMinMax(clientHeight(), view.minHeight, view.maxHeight);
    }
    else {
      view._actualHeight = view.height;
    }
  }

  // unit of proportion
  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  int ct = 10 * _children.Length();  // limit the number of attempts to converge
  while(ct-- > 0) { // autosize controls, apply minmax, repeat until all minmax conditions are met
    bool converged = true;
    for(unsigned i = 0; i < _children.Length(); ++i) {
      BView& view = *_children[i];
      if (view.width < 0 && view._actualWidth < 0) { // if marked for auto sizing
        uint16_t proposedSize = abs(view.width) * unitSize;
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minWidth, view.maxWidth);
        if (proposedSize != minmaxSize) { // if constrained and has a fixed size now
          view._actualWidth = minmaxSize;
          fixedSize += minmaxSize;
          totalFactor -= abs(view.width);
          converged = false; // reposition auto-sized controls
        }
      }
    }    
    if (converged) {
      break;
    }
    // recompute unit size based on available space
    unitSize = (totalSize - fixedSize) / totalFactor;
  }

  totalSize = 0;
  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];
    if (view.width < 0 && view._actualWidth < 0) {
      // position unconstrained controls
      view._actualWidth = abs(view.width) * unitSize;
    }
    totalSize += view._actualWidth;
  }

  // compute render origin based on alignment
  int16_t startPos = 0;
  if (horizontalAlignment == right) {
    startPos = clientWidth() - totalSize - ((_children.Length() - 1) * spacing);
  } else if (horizontalAlignment == center) {
    startPos = (clientWidth() - totalSize - ((_children.Length() -1) * spacing)) / 2;
  }

  // position controls starting from origin
  for(unsigned i = 0; i < _children.Length(); ++i) {
    BView& view = *_children[i];  
    view.x = startPos;
    startPos += view._actualWidth + spacing;    

    // position controls in the other dimension    
    if (verticalAlignment == bottom) {
      view.y = clientHeight() - view._actualHeight;
    } else if (verticalAlignment == center) {
      view.y = (clientHeight() - view._actualHeight) /2;
    }
  }
}

