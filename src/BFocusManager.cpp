#include <Arduino.h>
#include "BFocusManager.h"
#include "BView.h"

BGraphics* BFocusManager::_g = nullptr;
BList<BView*> BFocusManager::_stack;
BControl* BFocusManager::_focused = nullptr;
BView* BFocusManager::_capture = nullptr;
bool BFocusManager::_isDirty = true;
bool BFocusManager::_needsLayout = true;

bool BFocusManager::findViewHelper(BView& view, int16_t x, int16_t y, BView*& target) {  
  if (view.hitTest(x, y)) {
    target = &view;
    BPanel* panel = view.asPanel();
    if (panel) {
      panel->applyOffset(x, y, -1);
      for(unsigned i = 0; i < panel->_children.Length(); ++i) {
        if (panel->_children[i]) {
          if (findViewHelper(*panel->_children[i], x, y, target)) {
            return true;
          }
        }
      }
    }
    return true;
  }
  return false;
}

void BFocusManager::mapScreenToViewHelper(BView& view, int16_t& x, int16_t& y) {
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->applyOffset(x, y, -1);
  } else {
    x -= view.x;
    y -= view.y;
  }
  if (view.parent()) {
    mapScreenToViewHelper(*view.parent(), x, y);
  }
}

void BFocusManager::mapViewToScreenHelper(BView& view, int16_t& x, int16_t& y) {
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->applyOffset(x, y);
  } else {
    x += view.x;
    y += view.y;
  }

  if (view.parent()) {
    mapViewToScreenHelper(*view.parent(), x, y);
  }
}

void BFocusManager::draw() {
  auto r = root();
  if (root()) {
    r->draw(*_g);
  }
}

void BFocusManager::layoutRoot() {
  if (root()) {
    BView& view = *root();
    if (view.width < 0) {
      view._actualWidth = _g->width;
      view.x = 0;
    }
    else {
      view._actualWidth = view.width;
      view.x = (_g->width + view.width) / 2;
    }

    if (view.height < 0) {
      view._actualHeight = _g->height;
      view.y = 0;
    }
    else {
      view._actualHeight = view.height;
      view.y = (_g->height + view.height) / 2;
    }

    // BPanel* panel = view.asPanel();
    // if (panel) {
    //   panel->layout();
    // }
  }
}

void BFocusManager::handleEvent(BInputEvent& event) {  
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

BControl* BFocusManager::focus(BControl& view) {  
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

BControl* BFocusManager::focusedControl() {
  return _focused;
}

BControl* BFocusManager::focusNextHelper(BPanel& panel, int16_t tabIndex) {
  for(unsigned i = tabIndex; i < panel._children.Length(); ++i) {
    if (panel._children[i]) {
      BControl* ctl = panel._children[i]->asControl();      
      if (ctl) {
        return ctl;
      }

      BPanel* ctlPanel = panel._children[i]->asPanel();
      if (ctlPanel) {            
        return focusNextHelper(*ctlPanel, 0);
      }
    }
  }

  return nullptr;
}

BControl* BFocusManager::focusNextHelper(BView& view) {
  int16_t tabIndex;
  auto parent = view.parent();
  BView* pview = &view;
  if (parent) {
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
  }

  return nullptr;
}




BControl* BFocusManager::focusPrevHelper(BPanel& panel, int16_t tabIndex) {
  for(unsigned i = tabIndex; i < panel._children.Length(); ++i) {
    if (panel._children[i]) {
      BControl* ctl = panel._children[i]->asControl();      
      if (ctl) {
        return ctl;
      }

      BPanel* ctlPanel = panel._children[i]->asPanel();
      if (ctlPanel) {            
        return focusNextHelper(*ctlPanel, 0);
      }
    }
  }

  return nullptr;
}

BControl* BFocusManager::focusPrevHelper(BControl& control) {
  int16_t tabIndex;
  auto parent = control.parent();
  if (parent) {
    while (parent) {      
      BPanel* panel = parent->asPanel();
      if (panel) {
        tabIndex = panel->indexOf(control);
        auto ctl = focusNextHelper(*panel, tabIndex + 1);
        if (ctl) {
          return ctl;
        }        
      }
      parent = parent->parent();
    }
  }

  return nullptr;
}



BControl* BFocusManager::focusFirstHelper(BView& view) {
  BControl* ctl = view.asControl();
  if (ctl) {
    return ctl;
  }
  
  BPanel* panel = view.asPanel();
  if (panel) {
    for(unsigned i = 0; i < panel->_children.Length(); ++i) {
      if (panel->_children[i]) {
        ctl = focusFirstHelper(*panel->_children[i]);
        if (ctl) {
          return ctl;
        }
      }
    }
  }

  return nullptr;
}

void BFocusManager::focusLastHelper(BView& view, BControl*& control) {
  auto ctl = view.asControl();
  if (ctl) {
    control = ctl;
  }

  BPanel* panel = view.asPanel();
  if (panel) {
    for(unsigned i = 0; i < panel->_children.Length(); ++i) {
      if (panel->_children[i]) {
        focusLastHelper(*panel->_children[i], control);
      }
    }
  }
}

BControl* BFocusManager::focusFirst() {
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

BControl* BFocusManager::focusLast() {
  auto r = root();  
  if (r) {
    BControl* last = nullptr;
    focusLastHelper(*r, last);
    if (last) {
      focus(*last);
      return last;
    }
  }

  _focused = nullptr;
  return _focused;
}

BControl* BFocusManager::focusNext() {
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

BControl* BFocusManager::focusPrev() {
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

// void BFocusManager::focusClick() {
//   if (_focused) {
//     BKeyboardInputEvent event;
//     event.type = InputEvent::evKeyDown;
//     event.type = BCommandInputEvent::cmClick;
//     _focused->handleEvent(event);
//   }
// }

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

BGraphics BFocusManager::getGraphics(BView& view) {
  BGraphics g(*_g);
  g.width = view.actualWidth();
  g.height = view.actualHeight();
  mapViewToScreenHelper(view, g.x, g.y);
  return g;
}

BView* BFocusManager::findView(BMouseInputEvent& event) {
  BView* target = nullptr;
  if (root()) {
    findViewHelper(*root(), event.x, event.y, target);
  }
  return target;
}

void BFocusManager::loop() {
  BView* r = root();
  if (r) {
    if (_needsLayout) {
      _needsLayout = false;
      BPanel* panel = r->asPanel();
      if (panel) {        
        panel->layout();
      }
    }
    if (_isDirty) {
      _isDirty = false;
      draw();
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

void focuspocus() {
  BFocusManager::loop();
}
