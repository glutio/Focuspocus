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
: x(0), y(0), width(0), height(0), 
  minWidth(0), minHeight(0), maxWidth(-1), maxHeight(-1), 
  actualWidth(0), actualHeight(0),
  _isDirty(true), focusable(true) {}

bool BView::hitTest(int16_t ptX, int16_t ptY) {
  auto hit = ptX >= 0 && ptX <= actualWidth &&
             ptY >= 0 && ptY <= actualHeight;
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
  focusManager().dirty();
}

bool BView::isDirty() {
  return _isDirty;
}

void BView::clearDirty() {
  _isDirty = false;
}

void BView::dirtyLayout() {
  _needsLayout = true;
  focusManager().dirtyLayout();

  if (!width || !height) {
    BPanel* p = parent();
    while(p) {
      p->_needsLayout = true;
      p = p->parent();
    }
  }
}

bool BView::isDirtyLayout() {
  return _needsLayout;
}

void BView::clearDirtyLayout() {
  _needsLayout = false;
}

void BView::focus() {
  if (focusable) {
    focusManager().focus(*this);
  }
}

bool BView::isFocused() {
  return focusManager().focusedView() == this;
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

void BView::measure(uint16_t availableWidth,uint16_t availableHeight) {
}

void BView::layout() {
  clearDirtyLayout();
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
      focusManager().captureMouse(*this);        
      focus();
      _isDown = true;        
      if (_animate) {
        dirty();
      }
      break;
    }
    case BInputEvent::evMouseUp:
    {
      focusManager().releaseMouse(*this);
      _isDown = false;
      if (_animate) {
        dirty();
      }
      BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
      bool hit = hitTest(pt.x, pt.y);
      if (hit) {
        onClick(this, hit);
      }     
      break;
    }
    case BInputEvent::evMouseMove:
    {
      if (focusManager().capturingView() == this) {
        BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
        bool hit = hitTest(pt.x, pt.y);      
        if (hit != _isDown) {
          _isDown = hit;
          if (_animate) {
            dirty();
          }
        }
      }
      break;
    }
  }
}

void BButton::measure(uint16_t availableHeight, uint16_t availableWidth) {
  if (!width || !height) {
    int16_t themeWidth = focusManager().theme().buttonWidth;
    auto themeHeight = focusManager().theme().buttonHeight;
    if (text) {
      BRect rt = focusManager().getGraphics().getTextBounds(text, viewFontSize(*this));
      if (!width) {
        actualWidth = rt.width + focusManager().theme().buttonPadding * 2;
      }
      if (!height) {
        actualHeight = rt.height + focusManager().theme().buttonPadding * 2;
      }
    }
    if (!width && actualWidth < themeWidth) {
      actualWidth = themeWidth;
    }
    if (!height && actualHeight < themeHeight) {
      actualHeight = themeHeight;
    }
    if (!width) {
      actualWidth = max(minWidth, min(maxWidth, actualWidth));
    }
    if (!height) {
      actualHeight = max(minHeight, min(maxHeight, actualHeight)); 
    }
  }
}

void BButton::draw(BGraphics& g) {
  BView::draw(g);
  auto parentBackground = viewBackground(*parent());
  auto radius = focusManager().theme().buttonRadius;

  if (_isDown) {
    g.drawRoundRect(0, 0, actualWidth, actualHeight, radius, parentBackground);
    g.drawRoundRect(1, 1, actualWidth - 2, actualHeight - 2, radius, parentBackground);
    g.fillRoundRect(2, 2, actualWidth - 4, actualHeight - 4, radius, focusManager().theme().focusBackground);
  }
  else {
    auto c = viewColor(*this);
    auto b = viewBackground(*this);
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
  auto rt = g.getTextBounds(text, viewFontSize(*this));
  int16_t c = (isFocused()) ? focusManager().theme().focusColor : viewFontColor(*this);
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
    ++i;
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
    ++i;
    return i;
}

BPanel::ReverseIterator BPanel::rend() {
    return ReverseIterator(*this, -1);
}

void BPanel::add(BView* view) {
  _children.Add(view);
}

void BPanel::remove(BView* view) {
  _children.Remove(view);
}

void BPanel::draw(BGraphics& g) {
  g.fillRect(border, border, g.width - border * 2, g.height - border * 2, viewBackground(*this));
  if (border) {
    g.drawRect(0, 0, g.width, g.height, viewColor(*this));
  }
  BView::draw(g);  
}

int16_t BPanel::indexOf(BView& view) {
  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i] == &view) {
      return (int16_t)i;
    }
  }
  return -1;
}

uint16_t BPanel::applyMinMax(uint16_t val, uint16_t minimum, uint16_t maximum) {    
  if (val < minimum) {
    return minimum;
  } else if (val > maximum) {
    return maximum;
  }
  return val;
}

void BPanel::setParent(BPanel& parent) {
  _focusManager = parent._focusManager;
  BView::setParent(parent);
}

void BPanel::setViewParent(BView& view) {
  view.setParent(*this);
}

BFocusManager& BPanel::focusManager() {
  return *_focusManager;
}

void BPanel::touchView(BView& view) {
  view._isDirty = true;
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->_needsLayout = true;
  }
}

void BPanel::layout() {
  for(BView& view: *this) {
    setViewParent(view);
    touchView(view);
    if (view.width > 0) {
      view.actualWidth = view.width;
    }
    if (view.height > 0) {
      view.actualHeight = view.height;
    }
    view.layout();
  }
  clearDirtyLayout();
  dirty();
}

void BPanel::measure(uint16_t availableWidth, uint16_t availableHeight) {  
  int16_t minX = INT16_MAX, minY = INT16_MAX;
  int16_t maxX = 0, maxY = 0;
  for(BView& view : *this) {
    setViewParent(view);      
    view.measure(availableWidth - paddingWidth() - marginWidth(view), availableHeight - paddingHeight() - marginHeight(view));
    if (width == 0 || height == 0) {
      minX = min(view.x, minX);
      minY = min(view.y, minY);
      maxX = max(view.x + view.actualWidth + marginWidth(view), maxX);
      maxY = max(view.y + view.actualHeight + marginHeight(view), maxY);
    }
    if (width == 0) {
      actualWidth = max(minWidth, min(maxWidth, maxX - minX));      
    }
    if (height == 0) {
      actualHeight = max(minHeight, min(maxHeight, maxY - minY));
    }
  }
}

int16_t BPanel::clientWidth() {
  return actualWidth - paddingWidth();
};

int16_t BPanel::clientHeight() {
  return actualHeight - paddingWidth();
}

uint16_t BPanel::childrenCount() {
  uint16_t ct = 0;
  for(BView& view : *this) {
    ++ct;
  }
  return ct;
}

void BStackPanel::measure(uint16_t availableWidth, uint16_t availableHeight) {
  int16_t finalSize;
  int16_t len = 0;
  int16_t w = 0, h = 0;
  // uint16_t hmax = 0, wmax = 0;
  for(BView& view : *this) {
    setViewParent(view);
    view.measure(availableWidth - paddingWidth() - marginWidth(view), availableHeight - paddingHeight() - marginHeight(view));
    if (!width || !height) {
      if (viewWidth(view) > 0 && viewHeight(view) > 0) {
        ++len;
      }
      if (!width && viewWidth(view) > 0) {
        if (orientation == B::horizontal) {
          w += viewWidth(view) + marginWidth(view);
          //hmax = max(hmax, view.actualHeight + marginHeight(view));
        } else {
          w = max(w, viewWidth(view) + marginWidth(view));
          //hmax += view.actualHeight + marginHeight(view);
        }
      }
      if (!height && viewHeight(view) > 0) {
        if (orientation == B::horizontal) {
          h = max(h, viewHeight(view) + marginHeight(view));
          //wmax += view.actualWidth + marginWidth(view);
        } else {
          h += viewHeight(view) + marginHeight(view);
         // wmax = max(wmax, view.actualWidth + marginWidth(view));
        }
      }
    }
  }

  if (!width) {
    if (orientation == B::horizontal) {
      actualWidth = w + spacing * (len - 1) + paddingWidth();
    } else {
      actualWidth = w + paddingWidth();
    }
    actualWidth = max(minWidth, min(maxWidth, actualWidth));
  }
  if (!height) {
    if (orientation == B::horizontal) {
      actualHeight = h + paddingHeight();
    } else {
      actualHeight = h + spacing * (len - 1) + paddingHeight();
    }
    actualHeight = max(minHeight, min(maxHeight, actualHeight));
  }
}

void BStackPanel::layout() {
  if (orientation == B::horizontal) {
    layoutHorizontal();
  }
  else {
    layoutVertical();
  }
  clearDirtyLayout();
  dirty();
}

void BStackPanel::layoutVertical() {
  int16_t len = 0;
  int16_t totalFactor = 0;
  int16_t fixedSize = 0;

  for(BView& view : *this) {
    touchView(view);
    if (view.height < 0) {
      totalFactor += abs(view.height);
      view.actualHeight = -1;
    } else {
      fixedSize += viewHeight(view) + marginHeight(view);
      view.actualHeight = viewHeight(view);
    }

    if (view.width < 0) {
      view.actualWidth = applyMinMax(clientWidth() - marginWidth(view), view.minWidth, view.maxWidth);
    }
    else if (view.width > 0) {
      view.actualWidth = view.width;
    }

    if (view.actualHeight && view.actualWidth) {
      ++len;
    }
  }

  if (!len) {
    return;
  }

  int16_t totalSize = clientHeight() - (spacing * (len - 1));
  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  for(int i = 0; i < 10 * _children.Length(); ++i) {
    bool converged = true;
    for(BView& view: *this) {
      if (view.height < 0 && view.actualHeight == -1) {
        uint16_t proposedSize = abs(view.height) * unitSize - marginHeight(view);
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minHeight, view.maxHeight);
        if (proposedSize != minmaxSize) {
          view.actualHeight = minmaxSize;
          fixedSize += minmaxSize + marginHeight(view);
          totalFactor -= abs(view.height);
          unitSize = (totalSize - fixedSize) / totalFactor;
          converged = false;
        }
      }
    }
    if (converged) {
      break;
    }
  }

  totalSize = 0;
  for(BView& view : *this) {
    if (view.height < 0 && view.actualHeight == -1) {
      view.actualHeight = abs(view.height) * unitSize - marginHeight(view);
    }
    totalSize += view.actualHeight + marginHeight(view);
  }

  int16_t startPos = 0;
  if (verticalAlignment == B::bottom) {
    startPos = clientHeight() - totalSize - ((len - 1) * spacing);
  } else if (verticalAlignment == B::center) {
    startPos = (clientHeight() - totalSize - ((len -1) * spacing)) /2;
  }

  for(BView& view : *this) {
    view.y = startPos;
    startPos += view.actualHeight + marginHeight(view) + spacing;    

    if (horizontalAlignment == B::right) {
      view.x = clientWidth() - view.actualWidth - marginWidth(view);
    } else if (horizontalAlignment == B::center) {
      view.x = (clientWidth() - view.actualWidth - marginWidth(view)) / 2;
    }

    view.layout();
  }
}

void BStackPanel::layoutHorizontal() {
  // compute size available for controls
  int16_t len = 0;
  int16_t totalFactor = 0; // sum of proportion factors
  int16_t fixedSize = 0; // sum of sizes of fixed-size controls

  for(BView& view : *this) {
    touchView(view);
    if (view.width < 0) {
      totalFactor += abs(view.width);
      view.actualWidth = -1; // mark for auto layout     
    } else {
      fixedSize += viewWidth(view) + marginWidth(view);
      view.actualWidth = viewWidth(view); // fixed size
    }

    if (view.height < 0) {
      // occupy entire area in this dimension
      view.actualHeight = applyMinMax(clientHeight() - marginHeight(view), view.minHeight, view.maxHeight);
    }
    else if (view.height > 0) {
      view.actualHeight = view.height;
    }

    if (view.actualHeight && view.actualWidth) {
      ++len;
    }
  }
  
  if (!len) {
    return;
  }

  int16_t totalSize = clientWidth() - (spacing * (len - 1));
  // unit of proportion
  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  // autosize controls, apply minmax, repeat until all minmax conditions are met, limit number of attempts
  for(int i = 0; i < 10 * _children.Length(); ++i) {
    bool converged = true;
    for(BView& view : *this) {
      if (view.width < 0 && view.actualWidth == -1) { // if marked for auto sizing
        uint16_t proposedSize = abs(view.width) * unitSize - marginWidth(view);
        uint16_t minmaxSize = applyMinMax(proposedSize, view.minWidth, view.maxWidth);
        if (proposedSize != minmaxSize) { // if constrained and has a fixed size now
          view.actualWidth = minmaxSize;
          fixedSize += minmaxSize + marginWidth(view);
          totalFactor -= abs(view.width);
          // recompute unit size based on available space
          unitSize = (totalSize - fixedSize) / totalFactor;
          converged = false; // reposition auto-sized controls
        }
      }
    }
    if (converged) {
      break;
    }
  }

  totalSize = 0;
  for(BView& view : *this) {
    if (view.width < 0 && view.actualWidth == -1) {
      // position unconstrained controls
      view.actualWidth = abs(view.width) * unitSize - marginWidth(view);
    }
    totalSize += view.actualWidth + marginWidth(view);
  }

  // compute render origin based on alignment
  int16_t startPos = 0;
  if (horizontalAlignment == B::right) {
    startPos = clientWidth() - totalSize - ((len - 1) * spacing);
  } else if (horizontalAlignment == B::center) {
    startPos = (clientWidth() - totalSize - ((len - 1) * spacing)) / 2;
  }

  // position controls starting from origin
  for(BView& view : *this) {
    view.x = startPos;
    startPos += view.actualWidth + marginWidth(view) + spacing;    

    // position controls in the other dimension    
    if (verticalAlignment == B::bottom) {
      view.y = clientHeight() - view.actualHeight - marginHeight(view);
    } else if (verticalAlignment == B::center) {
      view.y = (clientHeight() - view.actualHeight - marginHeight(view)) /2;
    }

    view.layout();
  }
}
