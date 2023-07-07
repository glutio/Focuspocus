#include <Arduino.h>
#include "BFocusManager.h"
#include "BView.h"

void BFocusManager::applyOffset(BView& view, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * view.x;
  y += sign * view.y;
}

void BFocusManager::applyOffset(BView& view, BGraphics& g) {
  g.x += view.x;
  g.y += view.y;

  g.width = view.actualWidth + view.margin.left + view.margin.right;
  g.height = view.actualHeight + view.margin.top + view.margin.bottom;
}

void BFocusManager::applyMargins(BView& view, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * view.margin.left;
  y += sign * view.margin.top;
}

void BFocusManager::applyMargins(BView& view, BGraphics& g) {
  g.x += view.margin.left;
  g.y += view.margin.top;
  g.width -= view.margin.left + view.margin.right;
  g.height -= view.margin.top + view.margin.bottom;
}

void BFocusManager::applyPadding(BPanel& panel, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * (panel.padding.left + panel.border);
  y += sign * (panel.padding.top + panel.border);
}

void BFocusManager::applyPadding(BPanel& panel, BGraphics& g) {
  g.x += panel.padding.left + panel.border;
  g.y += panel.padding.top + panel.border;
  g.width -= panel.padding.left + panel.padding.right + panel.border * 2;
  g.height -= panel.padding.top + panel.padding.bottom + panel.border * 2;
}

bool BFocusManager::findViewHelper(BView& view, int16_t x, int16_t y, BView*& target) {  
  if (view.hitTest(x - view.x - view.margin.left, y - view.y - view.margin.top)) {
    target = &view;
    BPanel* panel = view.asPanel();
    if (panel) {
      applyOffset(*panel, x, y, -1);
      applyMargins(*panel, x, y, -1);
      applyPadding(*panel, x, y, -1);
      for (BView& v : *panel) {
        if (findViewHelper(v, x, y, target)) {
          return true;
        }
      }
    }
    return true;
  }
  return false;
}

void BFocusManager::mapScreenToViewHelper(BView& view, int16_t& x, int16_t& y) {
  applyMargins(view, x, y, -1);
  applyOffset(view, x, y, -1);
  BPanel* parent = view.parent();
  while(parent) {
    BPanel& panel = *parent;
    applyOffset(panel, x, y, -1);
    applyMargins(panel, x, y, -1);
    applyPadding(panel, x, y, -1);
    parent = panel.parent();
  }
}

void BFocusManager::mapViewToScreenHelper(BView& view, int16_t& x, int16_t& y) {
  applyMargins(view, x, y);
  applyOffset(view, x, y);
  BPanel* parent = view.parent();
  while(parent) {
    BPanel& panel = *parent;
    applyOffset(panel, x, y);
    applyMargins(panel, x, y);
    applyPadding(panel, x, y);
    parent = panel.parent();
  }
}

BRootView* BFocusManager::top() {
  if (_stack.Length()) {
    return _stack[_stack.Length() - 1];
  }
  return nullptr;
}

BPanel* BFocusManager::root() {
  auto t = top();
  if (t) {
    return &t->root;
  }
  return nullptr;
}

void BFocusManager::layoutRoot() {
  auto r = root();
  if (r) {
    BPanel& panel = *r;
    if (panel.width < 0) {
      panel.actualWidth = _g.width - panel.margin.left - panel.margin.right;
      panel.actualWidth = panel.applyMinMax(panel.actualWidth, panel.minWidth, panel.maxWidth);
    }
    panel.x = (_g.width - panel.actualWidth - panel.margin.left - panel.margin.right) / 2;
    
    if (panel.height < 0) {
      panel.actualHeight = _g.height - panel.margin.top - panel.margin.bottom;
      panel.actualHeight = panel.applyMinMax(panel.actualHeight, panel.minHeight, panel.maxHeight);
    }
    panel.y = (_g.height - panel.actualHeight - panel.margin.top - panel.margin.bottom) / 2;

    _isDirty = true;
  }
}

void BFocusManager::handleEvent(BInputEvent& event) {  
  if (_needsRootLayout) {
    return;
  }

  BView* view;
  if (event.type & BInputEvent::evMouse) {
    if (_capture) {
      view = _capture;
    }
    else {
      BMouseInputEvent& mouseEvent = (BMouseInputEvent&)event;
      view = findView(mouseEvent);
    }
  }
  else {
    view = focusedView();
  }
  if (view) {
    view->handleEvent(event);
  }
}

void BFocusManager::broadcastCommandHelper(BView& view, BCommandInputEvent& event) {
  view.handleEvent(event);

  BPanel* panel = view.asPanel();
  if (panel) {
    for(BView& v : *panel) {
      broadcastCommandHelper(v, event);
    }
  }
}

void BFocusManager::broadcastCommand(BCommandInputEvent& event) {
  auto r = root();
  if (r) {
    broadcastCommandHelper(*r, event);
  }
}

BTheme& BFocusManager::theme() {
  return *_theme;
}
  
BView* BFocusManager::focus(BView& view) {  
  if (top() && focusedView() != &view)
  {
    auto blurred = focusedView();  
    auto focused = &view;
    
    top()->focused = focused;

    BFocusInputEvent focusEvent;
    focusEvent.blurred = blurred;
    focusEvent.focused = focused;

    if (blurred) {
      focusEvent.command = BCommandInputEvent::cmBlur;
      blurred->handleEvent(focusEvent);
    }

    focusEvent.command = BCommandInputEvent::cmFocus; 
    focused->handleEvent(focusEvent);
    
    return blurred;
  }

  return &view;
}

BView* BFocusManager::focusedView() {
  auto t = top();  
  if (t) {
    return t->focused;
  }
  return nullptr;
}


BView* BFocusManager::focusNextHelper(BPanel& panel, int16_t tabIndex) {
  for(auto i = BPanel::Iterator(panel, tabIndex); i != panel.end(); ++i) {    
    BPanel* ctlPanel = (*i).asPanel();
    if (ctlPanel && !ctlPanel->focusable) {
      auto ctl = focusNextHelper(*ctlPanel, 0);
      if (ctl) {
        return ctl;
      }
    }

    if ((*i).focusable) {
      return &(*i);
    }
  }

  return nullptr;
}

BView* BFocusManager::focusNextHelper(BView& view) {
  int16_t tabIndex;
  auto parent = view.parent();
  BView* pview = &view;
  while (parent) {      
    BPanel* panel = parent->asPanel();
    if (panel) {
      tabIndex = panel->indexOf(*pview);
      auto ctl = focusNextHelper(*panel, tabIndex + 1);
      if (ctl) {
        return ctl;
      }        
    }
    pview = parent;
    parent = parent->parent();
  }

  return nullptr;
}


BView* BFocusManager::focusPrevHelper(BPanel& panel, int16_t tabIndex) {
  for(auto i = BPanel::ReverseIterator(panel, tabIndex); i != panel.rend(); ++i) {
    BPanel* ctlPanel = (*i).asPanel();
    if (ctlPanel && !ctlPanel->focusable) {            
      auto ctl = focusPrevHelper(*ctlPanel, ctlPanel->_children.Length() - 1);
      if (ctl) {
        return ctl;
      }
    }

    if ((*i).focusable) {
      return &(*i);
    }
  }
  return nullptr;
}

BView* BFocusManager::focusPrevHelper(BView& view) {
  int16_t tabIndex;
  auto parent = view.parent();
  BView* pview = &view;
  while (parent) {          
    BPanel* panel = parent->asPanel();
    if (panel) {
      tabIndex = panel->indexOf(*pview);
      auto ctl = focusPrevHelper(*panel, tabIndex - 1);
      if (ctl) {
        return ctl;
      }        
    }
    pview = parent;
    parent = parent->parent();
  }

  return nullptr;
}

BView* BFocusManager::focusFirstHelper(BView& view) {
  BPanel* panel = view.asPanel();
  if (panel && !panel->focusable) {
    for(BView& v : *panel) {
      auto control = focusFirstHelper(v);
      if (control) {
        return control;
      }
    }
  }

  if (view.focusable) {
    return &view;
  }
  return nullptr;
}

void BFocusManager::focusLastHelper(BView& view, BView*& control) {
  if (view.focusable) {
    control = &view;
  }

  BPanel* panel = view.asPanel();
  if (panel && !panel->focusable) {
    for(BView& v : *panel) {
      focusLastHelper(v, control);
    }
  }
}

BView* BFocusManager::focusFirst() {
  auto r = root();  
  if (r) {
    auto first = focusFirstHelper(*r);
    if (first) {
      focus(*first);
      return first;
    }
  }
  return nullptr;
}

BView* BFocusManager::focusLast() {
  auto r = root();  
  if (r) {
    BView* last = nullptr;
    focusLastHelper(*r, last);
    if (last) {
      focus(*last);
      return last;
    }
  }
  return nullptr;
}

BView* BFocusManager::focusNext() {
  if (top()) {
    if (!focusedView()) {
      return focusFirst();
    }

    auto next = focusNextHelper(*focusedView());
    if (next) {
      focus(*next);
      return next;
    }
    
    return focusFirst();
  }

  return nullptr;
}

BView* BFocusManager::focusPrev() {
  if (top()) {
    if (!focusedView()) {
      return focusLast();
    }

    auto prev = focusPrevHelper(*focusedView());
    if (prev) {
      focus(*prev);
      return prev;
    }
    return focusLast();
  }

  return nullptr;
}

BPoint BFocusManager::mapScreenToView(BView& view, int16_t x, int16_t y) {
  BPoint pt;
  pt.x = x;
  pt.y = y;
  mapScreenToViewHelper(view, pt.x, pt.y);
  return pt;
}

BPoint BFocusManager::mapViewToScreen(BView& view, int16_t x, int16_t y) {
  BPoint pt;
  pt.x = x;
  pt.y = y;
  mapViewToScreenHelper(view, pt.x, pt.y);
  return pt;
}

BGraphics BFocusManager::getGraphics() {
  return _g;
}

BGraphics BFocusManager::getGraphics(BView& view) {
  BGraphics g(_g);
  g.width = view.actualWidth;
  g.height = view.actualHeight;
  mapViewToScreenHelper(view, g.x, g.y);
  return g;
}

BView* BFocusManager::findView(BMouseInputEvent& event) {
  BView* target = nullptr;
  auto r = root();
  if (r) {
    findViewHelper(*r, event.x, event.y, target);
  }
  return target;
}

void BFocusManager::drawPass(BView& view, BGraphics& g) {
  BGraphics gv(g);
  applyOffset(view, gv);
  applyMargins(view, gv);
  if (view.isDirty()) {  
    view.draw(gv);
  }

  BPanel* panel = view.asPanel();
  if (panel) {
    applyPadding(*panel, gv);
    for(BView& v : *panel) {
      if (panel->isDirty()) {
        v.dirty();
      }
      drawPass(v, gv);
    }
  }
  view.clearDirty();
}

void BFocusManager::measurePass(BView& view, uint16_t availableWidth, uint16_t availableHeight) {
  if (view.isDirtyLayout()) {    
      view.measure(availableWidth, availableHeight);
  }
  else {
    BPanel* panel = view.asPanel();
    if (panel) {
      for(BView& child : *panel) {
        measurePass(child, panel->clientWidth(), panel->clientHeight());
      }
    }
  }
}

void BFocusManager::layoutPass(BView& view) {  
  if (view.isDirtyLayout()) {
    view.layout();
  } 
  else {
    BPanel* panel = view.asPanel();
    if (panel) {
      for(BView& child : *panel) {
        layoutPass(child);
      }
    }
  }
}

void BFocusManager::setupPass(BPanel& panel) {
  for(BView& child: panel) {
    child.setParent(panel);
    BPanel* p = child.asPanel();
    if (p) {
      setupPass(*p);
    }
  }
}

void BFocusManager::loop() {
  auto r = root();
  if (r) {
    BPanel& panel = *r;
    if (_needsRootLayout) {
      if (panel._focusManager != this) {
        panel._focusManager = this;
        panel.parentChanged(nullptr);
      }
      panel.dirtyLayout();
      _needsRootLayout = false;
    }
    
    for(auto i = 0; i < 20 && _needsLayout; ++i) {
      _needsLayout = false;
      setupPass(panel);
      auto availableWidth = _g.width - panel.margin.left - panel.margin.right;
      auto availableHeight = _g.height - panel.margin.top - panel.margin.bottom;      
      measurePass(panel, availableWidth, availableHeight);      
      if (panel.isDirtyLayout()) {
        layoutRoot();
      }
      layoutPass(panel);
    }

    if (_isDirty) {
      beginDraw();
      drawPass(panel, _g);
      _isDirty = false;
      endDraw();
    }

    auto ms = millis();
    if (ms - _msTimer >= 16) {
      _msTimer = ms;
      onTimerTick(this, true);
    }
  }
}

void BFocusManager::beginDraw() {
  raiseOnBeforeRender(true);
}
void BFocusManager::endDraw() {
  raiseOnAfterRender(true);
}


void BFocusManager::captureMouse(BView& view) {
  _capture = &view;
}

void BFocusManager::releaseMouse(BView& view) {
  if (_capture == &view) {
    _capture = nullptr;
  }
}

BView* BFocusManager::capturingView() {
  return _capture;
}

void BFocusManager::dirty() {
  _isDirty = true;
}

void BFocusManager::dirtyLayout() {
  _needsLayout = true;
}

void BFocusManager::push(BRootView& root) {
  _stack.Add(&root);
  _needsRootLayout = true;
}

void BFocusManager::pop() {
  if (_stack.Length()) {
    _stack.Remove(_stack.Length() - 1);    
  }
  _needsRootLayout = true;
}
