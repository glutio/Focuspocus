#ifndef __BVIEW_H__
#define __BVIEW_H__

#include "BList.h"
#include "BEvent.h"
#include "BNullable.h"

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

  void operator ()(int16_t val) {
    top = val;
    left = val;
    bottom = val;
    right = val;
  }
};

struct B {
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
};

class BView {
private:
  virtual void setParent(BPanel& parent);

private:
  BPanel* _parent;

protected:
  bool _isDirty;
  bool _needsLayout;

public:
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;
  uint16_t minWidth;
  uint16_t minHeight;
  uint16_t maxWidth;
  uint16_t maxHeight;
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
  BEVENT(Mouse, BView, BMouseInputEvent&)
  
public:
  BView();
  
  virtual void parentChanged(BPanel* oldParent);
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  virtual void layout();
  virtual void draw(BGraphics& g) = 0;
  virtual bool hitTest(int16_t ptX, int16_t ptY);
  virtual void handleEvent(BInputEvent& event);

  BPanel* parent();
  BFocusManager& focusManager();

  void dirty();
  bool isDirty();
  void clearDirty();

  void dirtyLayout();
  bool isDirtyLayout();
  void clearDirtyLayout();

  void focus();
  bool isFocused();

  friend class BPanel;
  friend class BFocusManager;
};

class BFontAware {
public:
  BNullable<uint8_t> fontSize;
  BNullable<uint16_t> fontColor;

  template<typename T>
  uint8_t viewFontSize(T& view) {
    if (view.fontSize.hasValue())
      return view.fontSize;
    return view.focusManager().theme().fontSize;
  }
  template<typename T>
  uint16_t viewFontColor(T& view) {
    if (view.fontColor.hasValue())
      return view.fontColor;
    return view.focusManager().theme().fontColor;
  }
};

class BColorAware {
public:
  BNullable<uint16_t> color;
  BNullable<uint16_t> background;

  template<typename T>
  uint16_t viewColor(T& view) {
    if (view.color.hasValue())
      return view.color;
    return view.focusManager().theme().color;
  }

  template<typename T>
  uint16_t viewBackground(T& view) {
    if (view.background.hasValue())
      return view.background;
    return view.focusManager().theme().background;
  }
};

class BControl: public BView {
public:
  BEVENT(Focus, BControl, BFocusInputEvent&)
public:
  virtual void handleEvent(BInputEvent& event);
};

class BButton: public BControl, public BFontAware, public BColorAware {
protected:
  bool _isDown;
  bool _animate;
public:
  BEVENT(Click, BButton, bool)

public:
  const char* text;

protected:
  virtual void drawContent(BGraphics& g);
  void handleMouse(BMouseInputEvent& event);
  void handleKeyboard(BKeyboardInputEvent& event);
  void handleFocus(BFocusInputEvent& event);

public:
  BButton();
  virtual void measure(uint16_t availableHeight, uint16_t availableWidth);
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
  int16_t paddingWidth() {
    return padding.left + padding.right + border * 2;
  }
  int16_t paddingHeight() {
    return padding.top + padding.bottom + border * 2;
  }

protected:
  BFocusManager* _focusManager;
  BList<BView*> _children;

public:
  BMargin padding;
  bool border;
protected:
  uint16_t applyMinMax(uint16_t val, uint16_t minimum, uint16_t maximum);
  int16_t indexOf(BView& view);

public:
  template<size_t N>
  BPanel(BView* (&children)[N]) 
    : _children(children, N, N), border(false) {
    focusable = false;
  }

  void add(BView& view);
  void insert(uint16_t pos, BView& view);
  void remove(BView& view);

  virtual void draw(BGraphics& graphics);

  virtual void layout();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  
  virtual int16_t clientWidth();
  virtual int16_t clientHeight();

  BFocusManager& focusManager();

  friend class BFocusManager;
};

class BStackPanel: public BPanel {
public:
  int16_t spacing;
  B::Orientation orientation;
  B::Alignment horizontalAlignment;
  B::Alignment verticalAlignment;

public:
  template<size_t N>
  BStackPanel(BView* (&children)[N]) 
    : BPanel(children), orientation(B::horizontal), horizontalAlignment(B::left), verticalAlignment(B::top) {}
  
  virtual void layout();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  void layoutHorizontal();
  void layoutVertical();
};

#endif