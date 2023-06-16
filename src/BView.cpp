#include <Arduino.h>
#include "BView.h"
#include "BFocusManager.h"

void BView::handleEvent(BInputEvent& event) {
  if (event.type & BInputEvent::evMouse) {
    onMouse(this, (BMouseInputEvent&)event);
  }
}

BView::BView() 
: x(0), y(0), width(-1), height(-1), minWidth(-1), minHeight(-1), maxWidth(-1), maxHeight(-1), _isDirty(true) {}

void BView::applyOffset(int16_t& x, int16_t& y, int8_t sign) {
  x += sign * this->x;
  y += sign * this->y;
}

void BView::applyOffset(int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += this->x;
  y += this->y;
  width = actualWidth() + margin.left + margin.right;
  height = actualHeight() + margin.top + margin.bottom;
}

void BView::applyMargins(int16_t& x, int16_t& y, int8_t sign) {
  x += sign * margin.left;
  y += sign * margin.top;
}

void BView::applyMargins(int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += margin.left;
  y += margin.top;
  width -= margin.left + margin.right;
  height -= margin.top + margin.bottom;
}

BRect BView::boundingBox() {
  BRect rt;
  rt.x = x;
  rt.y = y;
  rt.width = actualWidth() + margin.right + margin.left;
  rt.height = actualWidth() + margin.bottom + margin.top;
}

bool BView::hitTest(uint16_t ptX, uint16_t ptY) {
  auto hit = ptX >= x + margin.left && ptX <= x + margin.left + _actualWidth &&
             ptY >= y + margin.top && ptY <= y + margin.top + _actualHeight;
  return hit;
}

int16_t BView::actualWidth() {
  return _actualWidth;
}

int16_t BView::actualHeight() {
  return _actualHeight;
}

BPanel* BView::parent() {
  return _parent;
}

void BView::dirty() {
  _isDirty = true;
  BFocusManager::dirty();
}

bool BView::isDirty() {
  return _isDirty;
}

void BView::clearDirty() {
  _isDirty = false;
}

BControl::BControl() : color(0xFFFF) {};

void BControl::focus() {
  BFocusManager::focus(*this);
}

bool BControl::isFocused() {
  return BFocusManager::focusedControl() == this;
}

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

BButton::BButton() : _isDown(false), _capture(false) {
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
      if (!_isDown) {
        _isDown = true;      
        dirty();
      }
      break;
    }
    case BInputEvent::evKeyUp: {
      _isDown = false;
      dirty();
      break;
    }
  }
}

void BButton::handleMouse(BMouseInputEvent& event) {
  switch (event.type) {
    case BInputEvent::evMouseDown: 
    {
      if (!_isDown) {
        _isDown = true;        
        focus();
        dirty();
        BFocusManager::captureMouse(*this);        
      }
      break;
    }
    case BInputEvent::evMouseUp:
    {
      BFocusManager::releaseMouse(*this);
      auto x = event.x; auto y = event.y;
      BPoint pt = BFocusManager::mapScreenToView(*this, event.x, event.y);
      if(_isDown)
      {
        _isDown = false;
        dirty();
      }
      bool isDown = hitTest(pt.x + x, pt.y + y);
      onClick(this, isDown);
      break;
    }
    case BInputEvent::evMouseMove:
    {
      if (event.buttonDown && BFocusManager::capturingView() == this) {
        BPoint pt = BFocusManager::mapScreenToView(*this, event.x, event.y);
        auto isDown = hitTest(pt.x + x, pt.y + y);
        if (isDown != _isDown) {
          _isDown = isDown;
          dirty();
        }
      }
      break;
    }
  }
}

void BButton::draw(BGraphics& g) {
  BGraphics border(g);
  border.x-=margin.left;
  border.y-=margin.top;
  border.width +=margin.left+margin.right;
  border.height += margin.top+margin.bottom;
  border.drawRect(0, 0, border.width, border.height, 0x00FF);
  //g.fillRect(x, y, actualWidth(), actualHeight(), 0x0000);
  auto c = isFocused() ? 0xFF20 : color;
  if (_isDown) {
    g.drawRect(0, 0, actualWidth(), actualHeight(), 0);
    g.drawRect(1, 1, actualWidth()-2, actualHeight()-2, 0);
//    g.fillRect(0, 0, actualWidth(), actualHeight(), 0);
    g.fillRect(2, 2, actualWidth()-4, actualHeight()-4, c);
  }
  else {
//    g.fillRect(2, 2, actualWidth()-4, actualHeight()-4, 0);
    g.fillRect(0, 0, actualWidth(), actualHeight(), c);
  }
  drawContent(g);
}

void BButton::drawContent(BGraphics& g) {
  auto rt = g.getTextBounds(text, fontSize);  
  g.drawText(text, (g.width - rt.width)/2, (g.height - rt.height)/2, fontSize, fontColor);
}

/*
  BPanel
*/
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
  graphics.fillRect(1, 1, graphics.width-2, graphics.height-2, 0);
  graphics.drawRect(0, 0, graphics.width, graphics.height, 0xFFFF);
  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i]) {
      _children[i]->dirty();
    }
  }
}

int16_t BPanel::indexOf(BView& view) {
  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i] == &view) {
      return (int16_t)i;
    }
  }
  return -1;
}

void BPanel::applyPadding(int16_t& x, int16_t& y, int8_t sign) {
  x += sign * padding.left;
  y += sign * padding.top;
}

void BPanel::applyPadding(int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += padding.left;
  y += padding.top;
  width -= padding.left + padding.right;
  height -= padding.top + padding.bottom;
}

int16_t BPanel::applyMinMax(int16_t val, int16_t minimum, int16_t maximum) {    
  if (minimum >= 0 && val < minimum) {
    return minimum;
  } else if (maximum >=0 && val > maximum) {
    return maximum;
  }
  return val;
}

void BPanel::dirtyLayout() {
  dirty();
  _needsLayout = true;
  BFocusManager::dirtyLayout();
}

bool BPanel::isDirtyLayout() {
  return _needsLayout;
}

void BPanel::touchView(BView& view) {
  view._parent = this;
  view.dirty();
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->dirtyLayout();
  }
}

void BPanel::layout() {
  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i]) {
      BView& view = *_children[i];
      touchView(view);
      view._actualWidth = view.width;
      view._actualHeight = view.height;
    }
  }
  _needsLayout = false;
}

int16_t BPanel::clientWidth() {
  return actualWidth() - padding.left - padding.right;
};

int16_t BPanel::clientHeight() {
  return actualHeight() - padding.top - padding.bottom;
}

void BStackPanel::layout() {
  if (orientation == horizontal) {
    layoutHorizontal();
  }
  else {
    layoutVertical();
  }
  _needsLayout = false;
}

void BStackPanel::layoutVertical() {
  int16_t totalSize = clientHeight() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0;
  int16_t fixedSize = 0;

  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i]) {
      BView& view = *_children[i];
      touchView(view);
      if (view.height < 0) {
        totalFactor += abs(view.height);
        view._actualHeight = -1;      
      } else {
        fixedSize += view.height + marginHeight(view);
        view._actualHeight = view.height;
      }

      if (view.width < 0) {
        view._actualWidth = applyMinMax(clientWidth() - marginWidth(view), view.minWidth, view.maxWidth);
      }
      else {
        view._actualWidth = view.width;
      }
    }
  }

  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  int ct = 10 * _children.Length();
  while(ct-- > 0) {
    bool converged = true;
    for(unsigned i = 0; i < _children.Length(); ++i) {
      if (_children[i]) {
        BView& view = *_children[i];
        if (view.height < 0 && view._actualHeight < 0) {
          uint16_t proposedSize = abs(view.height) * unitSize - marginHeight(view);
          uint16_t minmaxSize = applyMinMax(proposedSize, view.minHeight, view.maxHeight);
          if (proposedSize != minmaxSize) {
            view._actualHeight = minmaxSize;
            fixedSize += minmaxSize + marginHeight(view);
            totalFactor -= abs(view.height);
            converged = false;
          }
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
    if (_children[i]) {
      BView& view = *_children[i];
      if (view.height < 0 && view._actualHeight < 0) {
        view._actualHeight = abs(view.height) * unitSize - marginHeight(view);
      }
      totalSize += view._actualHeight + marginHeight(view);
    }
  }

  int16_t startPos = 0;
  if (verticalAlignment == bottom) {
    startPos = clientHeight() - totalSize - ((_children.Length() - 1) * spacing);
  } else if (verticalAlignment == center) {
    startPos = (clientHeight() - totalSize - ((_children.Length() -1) * spacing)) /2;
  }

  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i]) {
      BView& view = *_children[i];  
      view.y = startPos;

      startPos += view._actualHeight + marginHeight(view) + spacing;    

      if (horizontalAlignment == right) {
        view.x = clientWidth() - view._actualWidth - marginWidth(view);
      } else if (horizontalAlignment == center) {
        view.x = (clientWidth() - view._actualWidth - marginWidth(view)) /2;
      }
    }
  }
}

void BStackPanel::layoutHorizontal() {
  // compute size available for controls
  int16_t totalSize = clientWidth() - (spacing * (_children.Length() - 1));
  int16_t totalFactor = 0; // sum of proportion factors
  int16_t fixedSize = 0; // sum of sizes of fixed-size controls

  for(unsigned i = 0; i < _children.Length(); ++i) {
    if (_children[i]) {
      BView& view = *_children[i];
      touchView(view);

      if (view.width < 0) {
        totalFactor += abs(view.width);
        view._actualWidth = -1; // mark for auto layout     
      } else {
        fixedSize += view.width + marginWidth(view);
        view._actualWidth = view.width; // fixed size
      }

      if (view.height < 0) {
        // occupy entire area in this dimension
        view._actualHeight = applyMinMax(clientHeight() - marginHeight(view), view.minHeight, view.maxHeight);
      }
      else {
        view._actualHeight = view.height;
      }
    }
  }

  // unit of proportion
  int16_t unitSize = (totalSize - fixedSize) / totalFactor;
  int ct = 10 * _children.Length();  // limit the number of attempts to converge
  while(ct-- > 0) { // autosize controls, apply minmax, repeat until all minmax conditions are met
    bool converged = true;
    for(unsigned i = 0; i < _children.Length(); ++i) {
      if (_children[i]) {
        BView& view = *_children[i];
        if (view.width < 0 && view._actualWidth < 0) { // if marked for auto sizing
          uint16_t proposedSize = abs(view.width) * unitSize - marginWidth(view);
          uint16_t minmaxSize = applyMinMax(proposedSize, view.minWidth, view.maxWidth);
          if (proposedSize != minmaxSize) { // if constrained and has a fixed size now
            view._actualWidth = minmaxSize;
            fixedSize += minmaxSize + marginWidth(view);
            totalFactor -= abs(view.width);
            converged = false; // reposition auto-sized controls
          }
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
    if (_children[i]) {
      BView& view = *_children[i];
      if (view.width < 0 && view._actualWidth < 0) {
        // position unconstrained controls
        view._actualWidth = abs(view.width) * unitSize - marginWidth(view);
      }
      totalSize += view._actualWidth + marginWidth(view);
    }
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
    if (_children[i]) {
      BView& view = *_children[i];  
      view.x = startPos;
      startPos += view._actualWidth + marginWidth(view) + spacing;    

      // position controls in the other dimension    
      if (verticalAlignment == bottom) {
        view.y = clientHeight() - view._actualHeight - marginHeight(view);
      } else if (verticalAlignment == center) {
        view.y = (clientHeight() - view._actualHeight - marginHeight(view)) /2;
      }
    }
  }
}
