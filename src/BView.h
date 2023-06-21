#ifndef __BVIEW_H__
#define __BVIEW_H__

#include "BList.h"
#include "Eventfun.h"
#include "BColor.h"

using namespace Buratino;

class BInputEvent;
class BMouseInputEvent;
class BFocusInputEvent;
class BKeyboardInputEvent;
class BFocusManager;

class BGraphics;
class BPanel;
class BControl;

struct BRect {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;

  bool pointInRect(int16_t ptX, int16_t ptY) {
    return ptX >= x && ptX <= x + width && ptY >= y && ptY <= y + height;
  }

  BRect() : x(0), y(0), width(0), height(0) {}
};

struct BPoint {
  int16_t x;
  int16_t y;
  BPoint() : x(0), y(0) {}
};

struct BMargin {
  int16_t top;
  int16_t left;
  int16_t bottom;
  int16_t right;
  
  BMargin() : top(0), left(0), bottom(0), right(0) {}

  operator ()(int16_t val) {
    top = val;
    left = val;
    bottom = val;
    right = val;
  }
};

class BView {
private:
  virtual void setParent(BPanel& parent);

private:
  BPanel* _parent;

protected:
  bool _isDirty;

public:
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  int16_t minWidth;
  int16_t minHeight;
  int16_t maxWidth;
  int16_t maxHeight;
  BMargin margin;
  int16_t actualWidth;
  int16_t actualHeight;
  const char* tag;
  bool focusable;
  static bool showBoundingBox;

protected:
  virtual BPanel* asPanel() {
    return nullptr;
  }

public:
  typedef EventDelegate<BView, BMouseInputEvent&> MouseEvent;
  EventSource<MouseEvent> onMouse;
  
public:
  BView();
  
  virtual void parentChanged(BPanel* oldParent);
  virtual void measure(int16_t availableWidth, int16_t availableHeight);
  virtual void layout();
  virtual void draw(BGraphics& g) = 0;
  virtual bool hitTest(int16_t ptX, int16_t ptY);
  virtual void handleEvent(BInputEvent& event);

  BPanel* parent();
  BFocusManager& focusManager();

  void dirty();
  bool isDirty();
  void clearDirty();

  void focus();
  bool isFocused();

  friend class BPanel;
  friend class BFocusManager;
};

class BFontAware {
public:
  uint8_t fontSize;
  BColor fontColor;
  BFontAware();
};

class BColorAware {
public:
  BColor color;
  BColor background;
  BColorAware();
};

class BControl: public BView {
public:
  typedef EventDelegate<BControl, BFocusInputEvent&> FocusEvent;
  EventSource<FocusEvent> onFocus;
public:
  virtual void handleEvent(BInputEvent& event);
};

class BButton: public BControl, public BFontAware, public BColorAware {
protected:
  bool _isDown;
  bool _animate;
public:
  typedef EventDelegate<BButton, bool> ClickEvent;
  EventSource<ClickEvent> onClick;
  const char* text;

protected:
  virtual void drawContent(BGraphics& g);
  void handleMouse(BMouseInputEvent& event);
  void handleKeyboard(BKeyboardInputEvent& event);
  void handleFocus(BFocusInputEvent& event);

public:
  BButton();

  virtual void handleEvent(BInputEvent& event);
  virtual void draw(BGraphics& g);
};

class BPanel: public BControl, public BColorAware {
public:
    class Iterator {
    private:
        BPanel& panel;
        int index;
    public:
        Iterator(BPanel& panel, unsigned index);
        bool operator!=(const Iterator& other) const;
        BView& operator*();
        Iterator& operator++();
    };

    class ReverseIterator {
    private:
        BPanel& panel;
        int index;
    public:
        ReverseIterator(BPanel& panel, unsigned index);
        bool operator!=(const ReverseIterator& other) const;
        BView& operator*();
        ReverseIterator& operator++();
    };

    Iterator begin();
    Iterator end();
    ReverseIterator rbegin();
    ReverseIterator rend();
private: 
  virtual void setParent(BPanel& parent);

protected:
  virtual BPanel* asPanel() {
    return this;
  }

protected:
  BFocusManager* _focusManager;
  BList<BView*> _children;
  bool _needsLayout;

public:
  BMargin padding;

protected:
  int16_t applyMinMax(int16_t val, int16_t minimum, int16_t maximum);    
  int16_t indexOf(BView& view);
  void touchView(BView& view);
  void setViewParent(BView& view);
public:
  BPanel(BView* children[], unsigned count, unsigned capacity);

  template<size_t N>
  BPanel(BView* (&children)[N]) 
    : _children(children, N, N), _needsLayout(true) {
    focusable = false;
  }

  void add(BView* view);
  void remove(BView* view);

  virtual void draw(BGraphics& graphics);

  virtual void layout();
  void dirtyLayout();
  bool isDirtyLayout();
  void clearDirtyLayout();
  
  int16_t clientWidth();
  int16_t clientHeight();

  BFocusManager& focusManager();

  friend class BFocusManager;
};

class BStackPanel: public BPanel {
private:
  int16_t marginWidth(BView& view) {
    return view.margin.left + view.margin.right;
  }
  int16_t marginHeight(BView& view) {
    return view.margin.top + view.margin.bottom;
  }
  int16_t viewWidth(BView& view) {
    return (view.width) ? view.width : view.actualWidth;
  }
  int16_t viewHeight(BView& view) {
    return (view.height) ? view.height : view.actualHeight;
  }
public:
  enum Orientation {
    vertical,
    horizontal
  };

  enum Alignment {
    top,
    bottom,
    center,
    left,
    right
  };

public:
  int16_t spacing;
  Orientation orientation;
  Alignment horizontalAlignment;
  Alignment verticalAlignment;

public:
  template<size_t N>
  BStackPanel::BStackPanel(BView* (&children)[N]) 
    : BPanel(children), orientation(horizontal), horizontalAlignment(left), verticalAlignment(top) {}
  
  virtual void layout();

  void layoutHorizontal();
  void layoutVertical();
};

#endif