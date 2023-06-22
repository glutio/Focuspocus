#include <Arduino.h>
#include "BFocusManager.h"
#include "BView.h"

void BFocusManager::applyOffset(BView& view, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * view.x;
  y += sign * view.y;
}

void BFocusManager::applyOffset(BView& view, int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += view.x;
  y += view.y;

  width = view.actualWidth + view.margin.left + view.margin.right;
  height = view.actualHeight + view.margin.top + view.margin.bottom;
}

void BFocusManager::applyMargins(BView& view, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * view.margin.left;
  y += sign * view.margin.top;
}

void BFocusManager::applyMargins(BView& view, int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += view.margin.left;
  y += view.margin.top;
  width -= view.margin.left + view.margin.right;
  height -= view.margin.top + view.margin.bottom;
}

void BFocusManager::applyPadding(BPanel& panel, int16_t& x, int16_t& y, int8_t sign) {
  x += sign * (panel.padding.left + panel.border);
  y += sign * (panel.padding.top + panel.border);
}

void BFocusManager::applyPadding(BPanel& panel, int16_t& x, int16_t& y, int16_t& width, int16_t& height) {
  x += panel.padding.left + panel.border;
  y += panel.padding.top + panel.border;
  width -= panel.padding.left + panel.padding.right + panel.border * 2;
  height -= panel.padding.top + panel.padding.bottom + panel.border * 2;
}

bool BFocusManager::findViewHelper(BView& view, int16_t x, int16_t y, BView*& target) {  
  if (view.hitTest(x - view.x, y - view.y)) {
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
  x -= view.x;
  y -= view.y; 
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
  x += view.x;
  y += view.y; 
  BPanel* parent = view.parent();
  while(parent) {
    BPanel& panel = *parent;
    applyOffset(panel, x, y);
    applyMargins(panel, x, y);
    applyPadding(panel, x, y);
    parent = panel.parent();
  }
}

BPanel* BFocusManager::root() {
  if (_stack.Length()) {
    return _stack[_stack.Length() - 1];
  }

  return nullptr;
}

void BFocusManager::touchTree(BView& view) {
  view.dirty();
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->dirtyLayout();
    for(BView& v : *panel) {      
      touchTree(v);
    }
  }
}

void BFocusManager::layoutRoot() {
  auto r = root();
  if (r) {
    BPanel& panel = *r;
    panel._focusManager = this;
    if (panel.width < 0) {
      panel.actualWidth = _g.width - panel.margin.left - panel.margin.right;
      panel.x = 0;
    }
    else {
      panel.actualWidth = panel.width;
      panel.x = (_g.width + panel.width) / 2;
    }

    if (panel.height < 0) {
      panel.actualHeight = _g.height - panel.margin.top - panel.margin.bottom;
      panel.y = 0;
    }
    else {
      panel.actualHeight = panel.height;
      panel.y = (_g.height + panel.height) / 2;
    }
    touchTree(*r);
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
    view = _focused;
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
  if (root()) {
    broadcastCommandHelper(*root(), event);
  }
}


BTheme& BFocusManager::theme() {
  return *_theme;
}
  
BView* BFocusManager::focus(BView& view) {  
  if (_focused != &view)
  {
    auto old = _focused;  
    _focused = &view;

    BFocusInputEvent focusEvent;
    focusEvent.blurred = old;
    focusEvent.focused = _focused;

    if (old) {
      focusEvent.command = BCommandInputEvent::cmBlur;
      old->handleEvent(focusEvent);
    }

    focusEvent.command = BCommandInputEvent::cmFocus; 
    _focused->handleEvent(focusEvent);
    
    return old;
  }

  return _focused;
}

BView* BFocusManager::focusedControl() {
  return _focused;
}

BView* BFocusManager::focusNextHelper(BPanel& panel, int16_t tabIndex) {
  for(auto i = BPanel::Iterator(panel, tabIndex); i != panel.end(); i++) {    
    BPanel* ctlPanel = (*i).asPanel();
    if (ctlPanel && !ctlPanel->focusable) {
      return focusNextHelper(*ctlPanel, 0);
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
  for(auto i = BPanel::ReverseIterator(panel, tabIndex); i != panel.rend(); i++) {
    BPanel* ctlPanel = (*i).asPanel();
    if (ctlPanel && !ctlPanel->focusable) {            
      return focusPrevHelper(*ctlPanel, ctlPanel->_children.Length() - 1);
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

  _focused = nullptr;
  return _focused;
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

  _focused = nullptr;
  return _focused;
}

BView* BFocusManager::focusNext() {
  if (!_focused) {
    return focusFirst();
  }

  auto next = focusNextHelper(*_focused);
  if (next) {
    focus(*next);
    return next;
  }

  return focusFirst();
}

BView* BFocusManager::focusPrev() {
  if (!_focused) {
    return focusLast();
  }

  auto prev = focusPrevHelper(*_focused);
  if (prev) {
    focus(*prev);
    return prev;
  }
  return focusLast();
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
  if (root()) {
    findViewHelper(*root(), event.x, event.y, target);
  }
  if (target) Serial.println(target->tag);
  return target;
}

void BFocusManager::layoutPass(BPanel& panel) {
  if (panel.isDirtyLayout()) {        
    panel.layout();
    panel.clearDirtyLayout();
  }

  for(BView& v : panel) {
    BPanel* child = v.asPanel();
    if (child) {
      layoutPass(*child);
    }
  }
}

void BFocusManager::drawPass(BView& view, BGraphics& g) {
  if (view.isDirty()) {  
    view.draw(g);
    view.clearDirty();
  }

  BPanel* panel = view.asPanel();
  if (panel) {
    for(BView& v : *panel) {
      BGraphics gg(g);
      applyPadding(*panel, gg.x, gg.y, gg.width, gg.height);
      applyOffset(v, gg.x, gg.y, gg.width, gg.height);
      applyMargins(v, gg.x, gg.y, gg.width, gg.height);
      drawPass(v, gg);
    }
  }
}

void BFocusManager::loop() {
  BPanel* panel = root();
  if (panel) {
    if (_needsRootLayout) {
      _needsRootLayout = false;
      layoutRoot();
    }
    if (_needsLayout) {
      do {
        _needsLayout = false;
        layoutPass(*panel);
      } while (_needsLayout);
    }
    if (_isDirty) {
      BGraphics g(_g);
      applyOffset(*panel, g.x, g.y, g.width, g.height);
      applyMargins(*panel, g.x, g.y, g.width, g.height);
      drawPass(*panel, g);
      _isDirty = false;
    }
  }
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

void BFocusManager::push(BPanel& panel) {
  _stack.Add(&panel);
  layoutRoot();
}

void BFocusManager::pop() {
  _stack.Remove(_stack.Length() - 1);
  layoutRoot();
}
