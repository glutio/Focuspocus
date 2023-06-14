#ifndef __BVIEW_H__
#define __BVIEW_H__

#include "BList.h"
#include "Eventfun.h"
#include "BGraphics.h"

using namespace Buratino;

class BInputEvent;
class BMouseInputEvent;
class BFocusEventArgs;
class BGraphics;
class BPanel;

struct BRect {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;

  bool pointInRect(int16_t ptX, int16_t ptY) {
    return ptX >= x && ptX <= x + width && ptY >= y && ptY <= y + height;
  }
};

struct BPoint {
  int16_t x;
  int16_t y;
};

struct BMargin {
  int16_t top;
  int16_t left;
  int16_t bottom;
  int16_t right;
};

class BView {
private:
  int16_t _actualWidth;
  int16_t _actualHeight;
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
  const char* tag;

private:
  virtual BPanel* asPanel() {
    return nullptr;
  }

public:
  typedef EventDelegate<BView, BMouseInputEvent&> MouseEvent;
  EventSource<MouseEvent> onMouse;

public:
  BView();
  virtual void draw(BGraphics& g) = 0;
  void redraw();

  bool hitTest(uint16_t ptX, uint16_t ptY);

  void handleEvent(BInputEvent& event);
  int16_t actualWidth();
  int16_t actualHeight();

  void dirty();
  
  friend class BFocusManager;
  friend class BPanel;
  friend class BStackPanel;
};

class BControl: public BView {
public:
  uint16_t color;
  
public:
  typedef EventDelegate<BControl, BFocusEventArgs&> FocusEvent;

  EventSource<FocusEvent> onFocus;
public:
  void focus();
  bool isFocused();
};

class BFontAware {
public:
  uint8_t fontSize;
  uint16_t fontColor;
  BFontAware();
};

class BButton: public BControl, public BFontAware {
protected:
  bool _isDown;
  bool _capture;
public:
  typedef EventDelegate<BButton, bool> ClickEvent;
  EventSource<ClickEvent> onClick;
  const char* text;
protected:
  virtual void drawContent(BGraphics& g);

public:
  BButton();

  void handleFocus(BControl* sender, BFocusEventArgs& args);
  void handleMouse(BView* sender, BMouseInputEvent& event);

  virtual void draw(BGraphics& g);

  friend class BFocusManager;
};

class BPanel: public BView {
private:
  virtual BPanel* asPanel() {
    return this;
  }

protected:
  BList<BView*> _children;
  bool _needsLayout;
public:
  BMargin padding;

protected:
  int16_t applyMinMax(int16_t val, int16_t minimum, int16_t maximum);    
  void applyOffset(int16_t& x, int16_t& y);
  void applyOffset(int16_t& x, int16_t& y, int16_t& width, int16_t& height);

public:
  BPanel(BView* children[], unsigned count, unsigned capacity);

  template<size_t N>
  BPanel(BView* (&children)[N]) 
    : _children(children, N, N), _needsLayout(true) {
  }

  void add(BView* view);
  void remove(BView* view);

  virtual void draw(BGraphics& graphics);

  virtual void layout();
  int16_t clientWidth();
  int16_t clientHeight();

  friend class BFocusManager;
};

class BStackPanel: public BPanel {
private:
  virtual BPanel* asPanel() {
    return this;
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