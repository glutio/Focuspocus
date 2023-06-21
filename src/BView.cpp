#include <Arduino.h>
#include "BView.h"
#include "BFocusManager.h"

bool BView::showBoundingBox(false);

void BView::handleEvent(BInputEvent& event) {
  if (event.type & BInputEvent::evMouse) {
    onMouse(this, (BMouseInputEvent&)event);
  }
}

BView::BView() 
: x(0), y(0), width(-1), height(-1), 
  minWidth(-1), minHeight(-1), maxWidth(-1), maxHeight(-1), 
  actualWidth(0), actualHeight(0),
  _isDirty(true), focusable(true) {}

bool BView::hitTest(int16_t ptX, int16_t ptY) {
  auto hit = ptX >= margin.left && ptX <= margin.left + actualWidth &&
             ptY >= margin.top && ptY <= margin.top + actualHeight;
  return hit;
}

void BView::setParent(BPanel& parent) {
  if (_parent != &parent) {
    auto old = _parent;
    _parent = &parent;
    parentChanged(old);
  }
}

BPanel* BView::parent() {
  return _parent;
}

void BView::dirty() {
  _isDirty = true;
  if (_parent) {
    _parent->focusManager().dirty();
  }
}

bool BView::isDirty() {
  return _isDirty;
}

void BView::clearDirty() {
  _isDirty = false;
}

void BView::focus() {
  if (focusable) {
    focusManager().focus(*this);
  }
}

bool BView::isFocused() {
  return focusManager().focusedControl() == this;
}

BFocusManager& BView::focusManager() {
  BPanel* panel = asPanel();
  if (panel) {
    return panel->focusManager();
  }

  if (_parent) {
    return _parent->focusManager();
  }
}

void BView::parentChanged(BPanel* oldParent) {
}

void BView::measure(int16_t availableWidth, int16_t availableHeight) {
}

void BView::layout() {
}

void BView::draw(BGraphics& g) {
  if (BView::showBoundingBox) {
    g.drawRect(-margin.left, -margin.top, g.width + margin.left + margin.right, g.height + margin.top + margin.bottom, focusManager().theme().boundingBoxColor);
  }
}

/* 
  BControl
// */
void BControl::handleEvent(BInputEvent& event) {
  BView::handleEvent(event);
  if (event.type == BInputEvent::evCommand) {
    BCommandInputEvent& cmdEvent = (BCommandInputEvent&)event;
    switch(cmdEvent.command) {
      case BCommandInputEvent::cmFocus:
      case BCommandInputEvent::cmBlur: {
        dirty();
        BFocusInputEvent& cmd = (BFocusInputEvent&)event;
        onFocus(this, cmd);
        break;
      }
    }
  }
}

BFontAware::BFontAware() : fontSize(1), fontColor(0xFFFF) {
}

BColorAware::BColorAware() : color(0xFFFF), background(0) {
}

BButton::BButton() : _isDown(false), _animate(true) {
}

void BButton::handleEvent(BInputEvent& event) {
  BControl::handleEvent(event);
  if (event.type & BInputEvent::evMouse)
  {
    handleMouse((BMouseInputEvent&)event);
  } else if (event.type & BInputEvent::evKeyboard) {
    handleKeyboard((BKeyboardInputEvent&)event);
  } 
}

void BButton::handleKeyboard(BKeyboardInputEvent& event) {
    switch (event.type) {
    case BInputEvent::evKeyDown: {
      if (event.code == BKeyboard::kbEnter) {
        if (!_isDown) {
          _isDown = true;      
          if (_animate) {
            dirty();
          }
        }
      }
      break;
    }
    case BInputEvent::evKeyUp: {
      if (event.code == BKeyboard::kbEnter) {
        if (_isDown) {
          _isDown = false;
          if (_animate) {
            dirty();
          }
          onClick(this, true);
        }
      }
      break;
    }
  }
}

void BButton::handleMouse(BMouseInputEvent& event) {
  switch (event.type) {
    case BInputEvent::evMouseDown: 
    {     
      focus();
      if (!_isDown) {
        _isDown = true;        
        if (_animate) {
          dirty();
        }
        focusManager().captureMouse(*this);        
      }
      break;
    }
    case BInputEvent::evMouseUp:
    {
      focusManager().releaseMouse(*this);
      auto x = event.x; auto y = event.y;
      if(_isDown)
      {
        _isDown = false;
        if (_animate) {
          dirty();
        }
      }
      BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
      bool isDown = hitTest(pt.x, pt.y);
      onClick(this, isDown);
      break;
    }
    case BInputEvent::evMouseMove:
    {
      if (event.buttonDown && focusManager().capturingView() == this) {
        BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
        auto isDown = hitTest(pt.x, pt.y);
        if (isDown != _isDown) {
          _isDown = isDown;
          if (_animate) {
            dirty();
          }
        }
      }
      break;
    }
  }
}

void BButton::draw(BGraphics& g) {
  BView::draw(g);
  auto parentBackground = parent()->background;
  auto radius = focusManager().theme().buttonRadius;

  if (_isDown) {
    g.drawRoundRect(0, 0, actualWidth, actualHeight, radius, parentBackground);
    g.drawRoundRect(1, 1, actualWidth - 2, actualHeight - 2, radius, parentBackground);
    g.fillRoundRect(2, 2, actualWidth - 4, actualHeight - 4, radius, focusManager().theme().focusBackground);
  }
  else {
    auto c = color;
    auto b = background;
    if (isFocused()) {
      c = focusManager().theme().focusColor;
      b = focusManager().theme().focusBackground;
    } 

    g.drawRoundRect(0, 0, actualWidth, actualHeight, radius, c);
    g.fillRoundRect(1, 1, actualWidth - 2, actualHeight - 2, radius, b);
  }
  drawContent(g);
}

void BButton::drawContent(BGraphics& g) {
  auto rt = g.getTextBounds(text, fontSize);
  int16_t c = (isFocused()) ? focusManager().theme().focusColor : fontColor;
  g.drawText(text, (g.width - rt.width)/2, (g.height - rt.height)/2, fontSize, c);
}

/*
  BPanel
*/
BPanel::Iterator::Iterator(BPanel& panel, unsigned index) : panel(panel), index(index) {}

bool BPanel::Iterator::operator!=(const Iterator& other) const {
    return index != other.index;
}

BView& BPanel::Iterator::operator*() {
    return *(panel._children[index]);
}

BPanel::Iterator& BPanel::Iterator::operator++() {
    do {
        ++index;
    } while(index < (int)panel._children.Length() && panel._children[index] == nullptr);
    return *this;
}

BPanel::ReverseIterator::ReverseIterator(BPanel& panel, unsigned index) : panel(panel), index(index) {}

bool BPanel::ReverseIterator::operator!=(const ReverseIterator& other) const {
    return index != other.index;
}

BView& BPanel::ReverseIterator::operator*() {
    return *(panel._children[index]);
}

BPanel::ReverseIterator& BPanel::ReverseIterator::operator++() {
    do {
        --index;
    } while(index >= 0 && panel._children[index] == nullptr);
    return *this;
}

BPanel::Iterator BPanel::begin() {
    if (!_children.Length()) {
      return end();
    }

    auto i = Iterator(*this, -1);
    i++;
    return i;
}

BPanel::Iterator BPanel::end() {
    return Iterator(*this, _children.Length());
}

BPanel::ReverseIterator BPanel::rbegin() {
    if (!_children.Length()) {
      return rend();
    }
    
    auto i = ReverseIterator(*this, _children.Length() - 1);
    i++;
    return i;
}

BPanel::ReverseIterator BPanel::rend() {
    return ReverseIterator(*this, -1);
}

BPanel::BPanel(BView* children[], unsigned count, unsigned capacity) 
  : _children(children, count, capacity), _needsLayout(true) {
  focusable = false;
}

void BPanel::add(BView* view) {
  _children.Add(view);
}

void BPanel::remove(BView* view) {
  _children.Remove(view);
}

void BPanel::draw(BGraphics& g) {
  BView::draw(g);  
  g.fillRect(1, 1, g.width-2, g.height-2, background);
  g.drawRect(0, 0, g.width, g.height, color);
  for(BView& v : *this) {
    v.dirty();
  }
}

int BPanel::indexOf(BView& view) {
  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i] == &view) {
      return (int)i;
    }
  }
  return -1;
}

int16_t BPanel::applyMinMax(int16_t val, int16_t minimum, int16_t maximum) {    
  if (minimum >= 0 && val < minimum) {
    return minimum;
  } else if (maximum >=0 && val > maximum) {
    return maximum;
  }
  return val;
}

void BPanel::setParent(BPanel& parent) {
  _focusManager = parent._focusManager;
  BView::setParent(parent);
}

void BPanel::dirtyLayout() {
  dirty();
  _needsLayout = true;
  if (_focusManager) {
    _focusManager->dirtyLayout();
  }
}

bool BPanel::isDirtyLayout() {
  return _needsLayout;
}

void BPanel::clearDirtyLayout() {
  _needsLayout = false;
}

BFocusManager& BPanel::focusManager() {
  return *_focusManager;
}

void BPanel::touchView(BView& view) {
  view.setParent(*this);
  view.dirty();
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->_focusManager = _focusManager;
    panel->dirtyLayout();
  }
}

void BPanel::layout() {
  for(BView& view: *this) {
    touchView(view);
    measure(actualWidth, actualHeight);
    view.actualWidth = view.width;
    view.actualHeight = view.height;
    view.layout();
  }
}

int16_t BPanel::clientWidth() {
  return actualWidth - padding.left - padding.right;
};

int16_t BPanel::clientHeight() {
  return actualHeight - padding.top - padding.bottom;
}

void BStackPanel::layout() {
  if (orientation == horizontal) {
    layoutHorizontal();
  }
  else {
    layoutVertical();
  }
}

void BStackPanel::layoutVertical() {
  int16_t totalSize = clientHeight() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0;
  int16_t fixedSize = 0;

  for(BView& view : *this) {
    touchView(view);
    view.measure(clientWidth(), totalSize - fixedSize);
    if (viewHeight(view) < 0) {
      totalFactor += abs(viewHeight(view));
      view.actualHeight = -1;      
    } else {
      fixedSize += viewHeight(view) + marginHeight(view);
      view.actualHeight = viewHeight(view);
    }

    if (viewWidth(view) < 0) {
      view.actualWidth = applyMinMax(clientWidth() - marginWidth(view), view.minWidth, view.maxWidth);
    }
    else {
      view.actualWidth = viewWidth(view);
    }
  }

  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  for(int i = 0; i < 10 * _children.Length(); ++i) {
    bool converged = true;
    for(BView& view: *this) {
      if (viewHeight(view) < 0 && view.actualHeight < 0) {
        uint16_t proposedSize = abs(viewHeight(view)) * unitSize - marginHeight(view);
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minHeight, view.maxHeight);
        if (proposedSize != minmaxSize) {
          view.actualHeight = minmaxSize;
          fixedSize += minmaxSize + marginHeight(view);
          totalFactor -= abs(viewHeight(view));
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
  for(BView& view : *this) {
    if (viewHeight(view) < 0 && view.actualHeight < 0) {
      view.actualHeight = abs(viewHeight(view)) * unitSize - marginHeight(view);
    }
    totalSize += view.actualHeight + marginHeight(view);
  }

  int16_t startPos = 0;
  if (verticalAlignment == bottom) {
    startPos = clientHeight() - totalSize - ((_children.Length() - 1) * spacing);
  } else if (verticalAlignment == center) {
    startPos = (clientHeight() - totalSize - ((_children.Length() -1) * spacing)) /2;
  }

  for(BView& view : *this) {
    view.y = startPos;
    startPos += view.actualHeight + marginHeight(view) + spacing;    

    if (horizontalAlignment == right) {
      view.x = clientWidth() - view.actualWidth - marginWidth(view);
    } else if (horizontalAlignment == center) {
      view.x = (clientWidth() - view.actualWidth - marginWidth(view)) /2;
    }

    view.layout();
  }
}

void BStackPanel::layoutHorizontal() {
  // compute size available for controls
  int16_t totalSize = clientWidth() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0; // sum of proportion factors
  int16_t fixedSize = 0; // sum of sizes of fixed-size controls

  for(BView& view : *this) {
    touchView(view);
    view.measure(totalSize - fixedSize, clientHeight());
    if (viewWidth(view) < 0) {
      totalFactor += abs(viewWidth(view));
      view.actualWidth = -1; // mark for auto layout     
    } else {
      fixedSize += viewWidth(view) + marginWidth(view);
      view.actualWidth = viewWidth(view); // fixed size
    }

    if (viewHeight(view) < 0) {
      // occupy entire area in this dimension
      view.actualHeight = applyMinMax(clientHeight() - marginHeight(view), view.minHeight, view.maxHeight);
    }
    else {
      view.actualHeight = viewHeight(view);
    }
  }

  // unit of proportion
  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  // autosize controls, apply minmax, repeat until all minmax conditions are met, limit number of attempts
  for(int i = 0; i < 10 * _children.Length(); ++i) {
    bool converged = true;
    for(BView& view : *this) {
      if (viewWidth(view) < 0 && view.actualWidth < 0) { // if marked for auto sizing
        uint16_t proposedSize = abs(viewWidth(view)) * unitSize - marginWidth(view);
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minWidth, view.maxWidth);
        if (proposedSize != minmaxSize) { // if constrained and has a fixed size now
          view.actualWidth = minmaxSize;
          fixedSize += minmaxSize + marginWidth(view);
          totalFactor -= abs(viewWidth(view));
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
  for(BView& view : *this) {
    if (viewWidth(view) < 0 && view.actualWidth < 0) {
      // position unconstrained controls
      view.actualWidth = abs(viewWidth(view)) * unitSize - marginWidth(view);
    }
    totalSize += view.actualWidth + marginWidth(view);
  }

  // compute render origin based on alignment
  int16_t startPos = 0;
  if (horizontalAlignment == right) {
    startPos = clientWidth() - totalSize - ((_children.Length() - 1) * spacing);
  } else if (horizontalAlignment == center) {
    startPos = (clientWidth() - totalSize - ((_children.Length() -1) * spacing)) / 2;
  }

  // position controls starting from origin
  for(BView& view : *this) {
    view.x = startPos;
    startPos += view.actualWidth + marginWidth(view) + spacing;    

    // position controls in the other dimension    
    if (verticalAlignment == bottom) {
      view.y = clientHeight() - view.actualHeight - marginHeight(view);
    } else if (verticalAlignment == center) {
      view.y = (clientHeight() - view.actualHeight - marginHeight(view)) /2;
    }

    view.layout();
  }
}
