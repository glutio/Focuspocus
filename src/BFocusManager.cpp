#include <Arduino.h>
#include "BFocusManager.h"
#include "BView.h"

BGraphics* BFocusManager::_g = nullptr;
BList<BView*> BFocusManager::_stack;
BControl* BFocusManager::_focused = nullptr;
BView* BFocusManager::_capture = nullptr;
bool BFocusManager::_isDirty = true;
bool BFocusManager::_needsLayout = true;

bool BFocusManager::findView(BView& view, int16_t offsetX, int16_t offsetY, BPoint& pt, BView*& target) {
  auto x = pt.x - offsetX;
  auto y = pt.y - offsetY;
  if (view.hitTest(x, y)) {
    target = &view;
    BPanel* panel = view.asPanel();
    if (panel) {
      panel->applyOffset(offsetX, offsetY);
      for(unsigned i = 0; i < panel->_children.Length(); ++i) {
        if (findView(*panel->_children[i], offsetX, offsetY, pt, target)) {
          return true;
        }
      }
    }
    return true;
  }
  return false;
}

bool BFocusManager::mapScreenToView(BView& view, int16_t offsetX, int16_t offsetY, BView& target, BPoint& pt) {
  if (&view == &target) {
    pt.x -= offsetX;
    pt.y -= offsetY;
    return true;
  }
  
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->applyOffset(offsetX, offsetY);
    for(unsigned i = 0; i < panel->_children.Length(); ++i) {
      if (mapScreenToView(*panel->_children[i], offsetX, offsetY, target, pt)) {
        return true;
      }
    }
  }

  return false;
}

bool BFocusManager::getGraphics(BView& view, int16_t offsetX, int16_t offsetY, int16_t width, int16_t height, BView& target, BGraphics& g) {
  if (&view == &target) {
    g.x = offsetX;
    g.y = offsetY;
    g.width = width;
    g.height = height;
    return true;
  }
  
  BPanel* panel = view.asPanel();
  if (panel) {
    panel->applyOffset(offsetX, offsetY, width, height);
    for(unsigned i = 0; i < panel->_children.Length(); ++i) {
      if (getGraphics(*panel->_children[i], offsetX, offsetY, width, height, target, g)) {
        return true;
      }
    }
  }

  return false;
}

bool BFocusManager::getParent(BView& view, BView& target, BView*& parent) {
  if (&view == &target) {
    return true;
  }

  BPanel* panel = view.asPanel();
  if (panel) {
    parent = &view;
    for(unsigned i = 0; i < panel->_children.Length(); ++i) {
      if (getParent(*panel->_children[i], target, parent)) {
        return true;
      }
    }
  }
  return false;
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
  if (view) {
    view->handleEvent(event);
  }
}

BView* BFocusManager::focus(BView& view) {
  auto old = _focused;
  _focused = &view;
  return old;
}

BView* BFocusManager::focusedElement() {
  return _focused;
}

BGraphics BFocusManager::getGraphics(BView& view) {
  BGraphics g(*_g);
  if (root()) {
    getGraphics(*root(), 0, 0, g.width, g.height, view, g);
  }
  return g;
}

BView* BFocusManager::findView(BMouseInputEvent& event) {
  BView* target = nullptr;
  if (root()) {
    BPoint pt;
    pt.x = event.x;
    pt.y = event.y;
    findView(*root(), 0, 0, pt, target);
  }
  return target;
}

BPoint BFocusManager::mapScreenToView(uint16_t x, uint16_t y, BView& view) {
  BPoint pt;
  if (root()) {
    pt.x = x;
    pt.y = y;
    mapScreenToView(*root(), 0, 0, view, pt);
  }
  return pt;
}

BView* BFocusManager::getParent(BView& view) {
  if (root()) {
    BView* parent = root();
    getParent(*root(), view, parent);
    return parent;
  }
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
