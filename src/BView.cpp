#include "BView.h"
#include "BFocusManager.h"
#include "BMinMax.h"


void BView::handleEvent(BInputEvent& event) {
  if (event.type & BInputEvent::evMouse) {
    onMouse(this, (BMouseInputEvent&)event);
  }
}

BView::BView() 
: _parent(nullptr), _isDirty(true), _needsLayout(false), focusable(true),
  x(0), y(0), width(0), height(0), 
  minWidth(0), minHeight(0), maxWidth(-1), maxHeight(-1), 
  actualWidth(0), actualHeight(0), tag(nullptr) {}

bool BView::hitTest(int16_t ptX, int16_t ptY) {
  return BRect(0, 0, actualWidth, actualHeight).contains(ptX, ptY);
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
  dirty();  // own content changed; parents redraw only if a child moves
  focusManager().dirtyLayout();

  // The parent re-places its children; panels sized by their content pass it on.
  for (BPanel* p = parent(); p; p = p->parent()) {
    p->_needsLayout = true;
    if (p->width && p->height) {
      break;
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

  return _parent->focusManager();
}

void BView::parentChanged(BPanel*) {
}

void BView::measure(uint16_t, uint16_t) {
}

void BView::layout() {
  clearDirtyLayout();
}

void BView::draw(BGraphics& g) {
#ifdef FOCUSPOCUS_SHOW_BOUNDS  // debug: outline each view and its margin in red
  g.drawRect(-margin.left, -margin.top, g.width + margin.left + margin.right, g.height + margin.top + margin.bottom, 0xF800);
#else
  (void)g;
#endif
}

/* 
  BControl
 */
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

/*
 BButton
*/
BButton::BButton() : _isDown(false), _animate(true), text(nullptr) {
}

void BButton::clicked() {
  onClick(this, true);
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
          clicked();
        }
      }
      break;
    }
    default:
      break;
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
        clicked();
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
    default:
      break;
  }
}

void BButton::measure(uint16_t, uint16_t) {
  if (!width || !height) {
    int16_t themeWidth = focusManager().theme().buttonWidth;
    auto themeHeight = focusManager().theme().buttonHeight;
    if (text) {
      BRect rt = focusManager().getTextBounds(text, viewFontSize(*this));
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
      actualWidth = max(minWidth, min(maxWidth, (uint16_t)actualWidth));
    }
    if (!height) {
      actualHeight = max(minHeight, min(maxHeight, (uint16_t)actualHeight)); 
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
  g.drawText(text, (g.width - rt.width)/2, (g.height - rt.height)/2, viewFontSize(*this), c);
}

/*
  BPanel
*/
BPanel::Iterator::Iterator(BPanel& panel, int index) : panel(panel), index(index < 0 ? 0 : index) {
    while (this->index < (int)panel._children.Length() && !panel._children[this->index]) {
        ++this->index;
    }
}

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

BPanel::ReverseIterator::ReverseIterator(BPanel& panel, int index) : panel(panel) {
    int last = (int)panel._children.Length() - 1;
    this->index = index > last ? last : (index < -1 ? -1 : index);
    while (this->index >= 0 && !panel._children[this->index]) {
        --this->index;
    }
}

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
    return Iterator(*this, 0);
}

BPanel::Iterator BPanel::end() {
    return Iterator(*this, _children.Length());
}

BPanel::ReverseIterator BPanel::rbegin() {
    return ReverseIterator(*this, (int)_children.Length() - 1);
}

BPanel::ReverseIterator BPanel::rend() {
    return ReverseIterator(*this, -1);
}

bool BPanel::add(BView& view) {
  bool added = false;
  for (unsigned i = 0; i < _children.Length() && !added; ++i) {
    if (!_children[i]) {
      _children[i] = &view;
      added = true;
    }
  }
  if (!added) {
    added = _children.Add(&view);
  }
  if (added && _focusManager) {
    dirtyLayout();
  }
  return added;
}

bool BPanel::insert(uint16_t pos, BView& view) {
  bool added;
  if (pos < _children.Length() && !_children[pos]) {
    _children[pos] = &view;
    added = true;
  } else {
    added = _children.Insert(pos, &view);
  }
  if (added && _focusManager) {
    dirtyLayout();
  }
  return added;
}

void BPanel::remove(BView& view) {
  int16_t i = indexOf(view);
  if (i < 0) {
    return;
  }
  _children[i] = nullptr;
  if (_focusManager) {
    _focusManager->viewRemoved(view);
    dirtyLayout();
  }
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

BFocusManager& BPanel::focusManager() {
  return *_focusManager;
}

void BPanel::layout() {
  for(BView& view: *this) {
    if (view.width > 0) {
      view.actualWidth = view.width;
    }
    if (view.height > 0) {
      view.actualHeight = view.height;
    }
    view.layout();
  }
  finishLayout();
}

// Repaint the panel only when a child moved or resized, so the area a child
// left is cleared. Children's old rectangles are already overwritten by
// measure() and layout(), so a 4-byte fingerprint of them is kept instead of
// 8 bytes per child. add/insert/remove always repaint (see dirtyLayout()), and
// a fingerprint collision (about 1 in 4 billion) only skips that repaint.
void BPanel::finishLayout() {
  uint32_t hash = 2166136261UL;  // FNV-1a style, one 16-bit value per step
  for (BView& v : *this) {
    const int16_t r[4] = { v.x, v.y, v.actualWidth, v.actualHeight };
    for (uint8_t k = 0; k < 4; ++k) {
      hash = (hash ^ (uint16_t)r[k]) * 16777619UL;
    }
  }
  if (hash != _layoutHash) {
    _layoutHash = hash;
    dirty();
  }
  clearDirtyLayout();
}

void BPanel::measure(uint16_t availableWidth, uint16_t availableHeight) {
  // Sized by content: the children's bounding box (empty panel: just padding).
  int16_t minX = 0, minY = 0, maxX = 0, maxY = 0;
  bool first = true;
  for(BView& view : *this) {
    view.measure(availableWidth - paddingWidth() - marginWidth(view), availableHeight - paddingHeight() - marginHeight(view));
    int16_t right = view.x + view.actualWidth + marginWidth(view);
    int16_t bottom = view.y + view.actualHeight + marginHeight(view);
    minX = first ? view.x : min(view.x, minX);
    minY = first ? view.y : min(view.y, minY);
    maxX = first ? right : max(right, maxX);
    maxY = first ? bottom : max(bottom, maxY);
    first = false;
  }
  if (width == 0) {
    actualWidth = max(minWidth, min(maxWidth, (uint16_t)(maxX - minX + paddingWidth())));
  }
  if (height == 0) {
    actualHeight = max(minHeight, min(maxHeight, (uint16_t)(maxY - minY + paddingHeight())));
  }
}
int16_t BPanel::clientWidth() {
  return actualWidth - paddingWidth();
};

int16_t BPanel::clientHeight() {
  return actualHeight - paddingHeight();
}

void BStackPanel::measure(uint16_t availableWidth, uint16_t availableHeight) {
  int16_t len = 0;
  int16_t w = 0, h = 0;
  // uint16_t hmax = 0, wmax = 0;
  for(BView& view : *this) {
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
      actualWidth = w + spacing * (len ? len - 1 : 0) + paddingWidth();
    } else {
      actualWidth = w + paddingWidth();
    }
    actualWidth = max(minWidth, min(maxWidth, (uint16_t)actualWidth));
  }
  if (!height) {
    if (orientation == B::horizontal) {
      actualHeight = h + paddingHeight();
    } else {
      actualHeight = h + spacing * (len ? len - 1 : 0) + paddingHeight();
    }
    actualHeight = max(minHeight, min(maxHeight, (uint16_t)actualHeight));
  }
}

// Stretched children share what fixed children leave; with no room left they get 0.
static int16_t unitShare(int16_t space, int16_t totalFactor) {
  return (totalFactor && space > 0) ? space / totalFactor : 0;
}

static int16_t share(int16_t factor, int16_t unitSize, int16_t margin) {
  int16_t size = abs(factor) * unitSize - margin;
  return size > 0 ? size : 0;
}

// One layout for both orientations: "main" is the stacking axis, "cross" the other.
void BStackPanel::layout() {
  const bool h = orientation == B::horizontal;
  int16_t BView::* const size = h ? &BView::width : &BView::height;
  int16_t BView::* const crossSize = h ? &BView::height : &BView::width;
  int16_t BView::* const actual = h ? &BView::actualWidth : &BView::actualHeight;
  int16_t BView::* const crossActual = h ? &BView::actualHeight : &BView::actualWidth;
  uint16_t BView::* const minSize = h ? &BView::minWidth : &BView::minHeight;
  uint16_t BView::* const maxSize = h ? &BView::maxWidth : &BView::maxHeight;
  uint16_t BView::* const crossMin = h ? &BView::minHeight : &BView::minWidth;
  uint16_t BView::* const crossMax = h ? &BView::maxHeight : &BView::maxWidth;
  int16_t BView::* const pos = h ? &BView::x : &BView::y;
  int16_t BView::* const crossPos = h ? &BView::y : &BView::x;
  const int16_t client = h ? clientWidth() : clientHeight();
  const int16_t crossClient = h ? clientHeight() : clientWidth();
  const B::Alignment align = h ? horizontalAlignment : verticalAlignment;
  const B::Alignment crossAlign = h ? verticalAlignment : horizontalAlignment;
  const B::Alignment end = h ? B::right : B::bottom;
  const B::Alignment crossEnd = h ? B::bottom : B::right;

  int16_t len = 0;
  int16_t totalFactor = 0;  // sum of stretch factors
  int16_t fixedSize = 0;    // sum of fixed sizes, margins included

  for(BView& view : *this) {
    int16_t margin = h ? marginWidth(view) : marginHeight(view);
    if (view.*size < 0) {
      totalFactor += abs(view.*size);
      view.*actual = -1;  // stretched, sized below
    } else {
      view.*actual = view.*size ? view.*size : view.*actual;
      fixedSize += view.*actual + margin;
    }

    if (view.*crossSize < 0) {
      int16_t crossMargin = h ? marginHeight(view) : marginWidth(view);
      view.*crossActual = applyMinMax(crossClient - crossMargin, view.*crossMin, view.*crossMax);
    }
    else if (view.*crossSize > 0) {
      view.*crossActual = view.*crossSize;
    }

    if (view.actualHeight && view.actualWidth) {
      ++len;
    }
  }

  if (!len) {
    finishLayout();
    return;
  }

  int16_t totalSize = client - (spacing * (len - 1));
  int16_t unitSize = unitShare(totalSize - fixedSize, totalFactor);
  // a child hitting its min/max becomes fixed; repeat until every share fits
  for(unsigned i = 0; i < 10 * _children.Length(); ++i) {
    bool converged = true;
    for(BView& view : *this) {
      if (view.*size < 0 && view.*actual == -1) {
        int16_t margin = h ? marginWidth(view) : marginHeight(view);
        uint16_t proposedSize = share(view.*size, unitSize, margin);
        uint16_t minmaxSize = applyMinMax(proposedSize, view.*minSize, view.*maxSize);
        if (proposedSize != minmaxSize) {
          view.*actual = minmaxSize;
          fixedSize += minmaxSize + margin;
          totalFactor -= abs(view.*size);
          unitSize = unitShare(totalSize - fixedSize, totalFactor);
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
    int16_t margin = h ? marginWidth(view) : marginHeight(view);
    if (view.*size < 0 && view.*actual == -1) {
      view.*actual = share(view.*size, unitSize, margin);
    }
    totalSize += view.*actual + margin;
  }

  int16_t startPos = 0;
  if (align == end) {
    startPos = client - totalSize - ((len - 1) * spacing);
  } else if (align == B::center) {
    startPos = (client - totalSize - ((len - 1) * spacing)) / 2;
  }

  for(BView& view : *this) {
    int16_t margin = h ? marginWidth(view) : marginHeight(view);
    int16_t crossMargin = h ? marginHeight(view) : marginWidth(view);
    view.*pos = startPos;
    if (view.actualWidth && view.actualHeight) {  // 0x0 views take no spacing
      startPos += view.*actual + margin + spacing;
    }

    if (crossAlign == crossEnd) {
      view.*crossPos = crossClient - view.*crossActual - crossMargin;
    } else if (crossAlign == B::center) {
      view.*crossPos = (crossClient - view.*crossActual - crossMargin) / 2;
    } else {
      view.*crossPos = 0;
    }

    view.layout();
  }
  finishLayout();
}