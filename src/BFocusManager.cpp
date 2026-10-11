#include "BFocusManager.h"
#include "BView.h"

bool BFocusManager::findViewHelper(BView& view, int16_t x, int16_t y, BView*& target) {
  x -= view.x + view.margin.left;
  y -= view.y + view.margin.top;
  if (!view.hitTest(x, y)) {
    return false;
  }
  target = &view;
  BPanel* panel = view.asPanel();
  if (panel) {
    BPoint client = panel->clientOrigin();
    // Later children are drawn on top, so they are hit first.
    for (auto i = panel->rbegin(); i != panel->rend(); ++i) {
      if (findViewHelper(*i, x - client.x, y - client.y, target)) {
        return true;
      }
    }
  }
  return true;
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
  BPoint origin = mapViewToScreen(view, 0, 0);
  BPoint pt;
  pt.x = x - origin.x;
  pt.y = y - origin.y;
  return pt;
}

// A view's x/y plus its top/left margin is where it sits in its parent's
// client area, which starts at the parent's padding and border.
BPoint BFocusManager::mapViewToScreen(BView& view, int16_t x, int16_t y) {
  BPoint pt;
  pt.x = x;
  pt.y = y;
  for (BView* v = &view; v; v = v->parent()) {
    pt.x += v->x + v->margin.left;
    pt.y += v->y + v->margin.top;
    if (v->parent()) {
      BPoint client = v->parent()->clientOrigin();
      pt.x += client.x;
      pt.y += client.y;
    }
  }
  return pt;
}
BGraphics BFocusManager::getGraphics() {
  return _g;
}

BRect BFocusManager::getTextBounds(const char* text, uint8_t size) {
  return _g.getTextBounds(text, size);
}

// For drawing outside the draw pass. Views that are not on the top screen
// (covered or removed) get a graphics that ignores drawing; they are redrawn
// when shown again.
BGraphics BFocusManager::getGraphics(BView& view) {
  BGraphics g(_g);
  BPoint pos = mapViewToScreen(view, 0, 0);
  g.x += pos.x;
  g.y += pos.y;
  g.width = view.actualWidth;
  g.height = view.actualHeight;
  // clip to every ancestor's client area, walking up from the view
  for (BView* v = &view; v->parent(); v = v->parent()) {
    BPanel& p = *v->parent();
    pos.x -= v->x + v->margin.left;  // now p's client origin on screen
    pos.y -= v->y + v->margin.top;
    g.clip(pos.x - g.x, pos.y - g.y, p.clientWidth(), p.clientHeight());
    BPoint client = p.clientOrigin();
    pos.x -= client.x;  // now p's own position
    pos.y -= client.y;
  }
  g.visible = g.visible && isOnScreen(view) && g.inside();
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
  gv.x += view.x + view.margin.left;
  gv.y += view.y + view.margin.top;
  gv.width = view.actualWidth;
  gv.height = view.actualHeight;
  if (view.isDirty() && gv.inside()) {  // partly outside its panel (e.g. scrolled): not drawn
    view.draw(gv);
  }

  BPanel* panel = view.asPanel();
  if (panel) {
    BPoint client = panel->clientOrigin();
    gv.x += client.x;
    gv.y += client.y;
    gv.width = panel->clientWidth();
    gv.height = panel->clientHeight();
    gv.clip(0, 0, gv.width, gv.height);  // children draw inside the client area only
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

void BFocusManager::loop(unsigned long ms) {
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

    if (ms - _msTimer >= 16) {
      _msTimer = ms;
      onTimerTick(this, ms);
      if (root() == &panel) {  // a handler may have pushed/popped a screen
        tickPass(panel, ms);
      }
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
  }
}

void BFocusManager::beginDraw() {
  raiseOnBeforeRender(true);
}
void BFocusManager::endDraw() {
  raiseOnAfterRender(true);
}


static bool isWithin(BView* view, BView& ancestor) {
  for (; view; view = view->parent()) {
    if (view == &ancestor) {
      return true;
    }
  }
  return false;
}

// A removed view (or anything inside it) must not keep focus or mouse capture,
// on any screen.
void BFocusManager::viewRemoved(BView& view) {
  for (unsigned i = 0; i < _stack.Length(); ++i) {
    if (isWithin(_stack[i]->focused, view)) {
      _stack[i]->focused = nullptr;
    }
  }
  if (isWithin(_capture, view)) {
    _capture = nullptr;
  }
}

bool BFocusManager::isOnScreen(BView& view) {
  BView* v = &view;
  while (BPanel* p = v->parent()) {
    if (p->indexOf(*v) < 0) {
      return false;  // removed
    }
    v = p;
  }
  return v == root();
}

void BFocusManager::tickPass(BView& view, unsigned long ms) {
  view.tick(ms);
  BPanel* panel = view.asPanel();
  if (panel) {
    for (BView& child : *panel) {
      tickPass(child, ms);
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

bool BFocusManager::push(BRootView& root) {
  if (!_stack.Add(&root)) {
    return false;
  }
  _needsRootLayout = true;
  return true;
}

void BFocusManager::pop() {
  if (_stack.Length()) {
    _stack.Remove(_stack.Length() - 1);    
  }
  _needsRootLayout = true;
}
