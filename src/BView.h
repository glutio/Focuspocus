#ifndef __BVIEW_H__
#define __BVIEW_H__

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <Buratino.h>
#include "BEvent.h"
#include "BNullable.h"


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

  bool contains(int16_t ptX, int16_t ptY) {
    return ptX >= x && ptX < x + width && ptY >= y && ptY < y + height;
  }

  BRect() : x(0), y(0), width(0), height(0) {}
  BRect(int16_t x, int16_t y, int16_t width, int16_t height) : x(x), y(y), width(width), height(height) {}
};

struct BPoint {
  int16_t x;
  int16_t y;
  BPoint() : x(0), y(0) {}
};

// Margins and padding are -128..127 pixels.
struct BMargin {
  int8_t top;
  int8_t left;
  int8_t bottom;
  int8_t right;
  
  BMargin() : top(0), left(0), bottom(0), right(0) {}

  void operator ()(int8_t val) {
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
  // One byte for all three flags.
  bool _isDirty : 1;
  bool _needsLayout : 1;
public:
  bool focusable : 1;

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
  // Animation hook, called about every 16 ms while the view's screen is
  // visible (not covered by another screen). Call dirty() to redraw.
  virtual void tick(unsigned long) {}

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
  bool _isDown : 1;
  bool _animate : 1;
public:
  BEVENT(Click, BButton, bool)

public:
  const char* text;

protected:
  virtual void drawContent(BGraphics& g);
  // Called on a tap released over the button, or Enter. Raises onClick.
  virtual void clicked();
  void handleMouse(BMouseInputEvent& event);
  void handleKeyboard(BKeyboardInputEvent& event);
  void handleFocus(BFocusInputEvent& event);

public:
  BButton();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
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
        Iterator(BPanel& panel, int index);
        bool operator!=(const Iterator& other) const;
        BView& operator*();
        Iterator& operator++();
    };

    class ReverseIterator {
    private:
        BPanel& panel;
        int index;
    public:
        ReverseIterator(BPanel& panel, int index);
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
    return view.width > 0 ? view.width : view.actualWidth;
  }
  int16_t viewHeight(BView& view) {
    return view.height > 0 ? view.height : view.actualHeight;
  }
  int16_t paddingWidth() {
    return padding.left + padding.right + border * 2;
  }
  int16_t paddingHeight() {
    return padding.top + padding.bottom + border * 2;
  }

protected:
  BFocusManager* _focusManager;
  Buratino::BList<BView*> _children;
  uint32_t _layoutHash;  // children's rectangles at the last layout

public:
  BMargin padding;
  bool border;
protected:
  uint16_t applyMinMax(uint16_t val, uint16_t minimum, uint16_t maximum);
  int16_t indexOf(BView& view);
  // Call at the end of layout(): redraws the panel only if a child moved or
  // resized, so a value changing in place redraws just that widget.
  void finishLayout();

public:
  // Fixed storage: the panel never allocates. nullptr slots are free;
  // add/insert fail once every slot is used.
  template<size_t N>
  BPanel(BView* (&children)[N]) 
    : _focusManager(nullptr), _children(children, N, N), _layoutHash(0), border(false) {
    focusable = false;
  }

  // Heap storage: the child list grows as needed.
  explicit BPanel(unsigned capacity = 0)
    : _focusManager(nullptr), _children(capacity), _layoutHash(0), border(false) {
    focusable = false;
  }

  // add() takes the first free slot, else appends (heap storage only).
  // Both return false when there is no room.
  bool add(BView& view);
  bool insert(uint16_t pos, BView& view);
  void remove(BView& view);

  virtual void draw(BGraphics& graphics);

  virtual void layout();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  
  // The area children are laid out in: inside the padding and border.
  BPoint clientOrigin() {
    BPoint pt;
    pt.x = padding.left + border;
    pt.y = padding.top + border;
    return pt;
  }
  int16_t clientWidth();
  int16_t clientHeight();

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
    : BPanel(children), spacing(0), orientation(B::horizontal), horizontalAlignment(B::left), verticalAlignment(B::top) {}

  explicit BStackPanel(unsigned capacity = 0)
    : BPanel(capacity), spacing(0), orientation(B::horizontal), horizontalAlignment(B::left), verticalAlignment(B::top) {}
  
  virtual void layout();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
};

#endif