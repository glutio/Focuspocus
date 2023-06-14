#ifndef __BFOCUSMANAGER_H__
#define __BFOCUSMANAGER_H__

#include "BList.h"
#include "BGraphics.h"
#include "BMouse.h"

class BView;
class BPoint;
class BControl;

using namespace Buratino;

class BInputEvent {
public:
  enum EventType {
    evNothing = 0x0000,
    evMouseDown = 0x0001,
    evMouseUp = 0x0002,
    evMouseMove = 0x0004,
    evMouseAuto = 0x0008,
    evMouseWheel = 0x0020,
    evMouse = 0x002f,
    evCommand = 0x0100,
  };

  EventType type;
};

class BMouseInputEvent: public BInputEvent {
public:
  uint16_t x;
  uint16_t y;
  bool buttonDown;
};

struct BFocusEventArgs {
  BControl* blurred;
  BControl* focused;
};

class BFocusManager {
protected:
  static BList<BView*> _stack;
  static BGraphics* _g;
  static BControl* _focused;
  static BView* _capture;
  static bool _isDirty;
  static bool _needsLayout;

protected:
  static bool findView(BView& view, int16_t offsetX, int16_t offsetY, BPoint& pt, BView*& target);
  static bool mapScreenToView(BView& view, int16_t offsetX, int16_t offsetY, BView& target, BPoint& pt);
  static bool getGraphics(BView& view, int16_t offsetX, int16_t offsetY, int16_t width, int16_t height, BView& target, BGraphics& g);
  static bool getParent(BView& view, BView& target, BView*& parent);
public:
  template<size_t N>
  static void BFocusManager::initialize(BGraphics& g, BView* (&stack)[N]) {
    _g = &g;
    _stack = BList<BView*>(stack, N, N);
    layoutRoot();
  }

  static BView* root() {
    for(unsigned i = _stack.Length() - 1; i >= 0; --i) {
      if (_stack[i]) {
        return _stack[i];
      } 
    }
    return nullptr;
  }

  static void draw();
  static void layoutRoot();
  static void handleEvent(BInputEvent& event);

  static BView* focus(BView& view);
  static BView* focusedElement();
  static BView* focusNext();

  static BPoint mapScreenToView(uint16_t x, uint16_t y, BView& view);
  static BView* findView(BMouseInputEvent& event);
  static BGraphics getGraphics(BView& view);
  static BView* getParent(BView& view);

  static void captureMouse(BView& view);
  static void releaseMouse(BView& view);
  static BView* capturingView();

  static void dirty();
  static void dirtyLayout();

  static void loop();

  template<size_t N>
  friend void setupFocuspocus(BGraphics&, BView* (&)[N]);
  friend void focuspocus();
};

template<size_t N>
void setupFocuspocus(BGraphics& g, BView* (&stack)[N]) {
  BFocusManager::initialize(g, stack);
}

void focuspocus();

#endif