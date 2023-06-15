#ifndef __BFOCUSMANAGER_H__
#define __BFOCUSMANAGER_H__

#include "BList.h"
#include "BGraphics.h"
#include "BMouse.h"

class BView;
class BPoint;
class BControl;
class BPanel;

using namespace Buratino;

struct BInputEvent {
  enum EventType {
    evNothing = 0x0000,
    evMouseDown = 0x0001,
    evMouseUp = 0x0002,
    evMouseMove = 0x0004,
    evMouseAuto = 0x0008,
    evMouseWheel = 0x0010,
    evMouse = 0x001f,

    evKeyDown = 0x0020,
    evKeyUp = 0x0040,
    evKeyboard = 0x0060,

    evCommand = 0x0100,
  };

  EventType type;
};

struct BMouseInputEvent: BInputEvent {
  uint16_t x;
  uint16_t y;
  bool buttonDown;
};

struct BKeyboardInputEvent: BInputEvent {
  uint16_t code;
};

struct BCommandInputEvent: BInputEvent {
  enum CommandType {
    cmFocus = 0x0001,
    cmBlur = 0x0002,
  };

  uint16_t command;

  BCommandInputEvent() { 
    type = evCommand;
  }
};

struct BFocusInputEvent: BCommandInputEvent {
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
  static bool findViewHelper(BView& view, int16_t x, int16_t y, BView*& target);
  static void mapScreenToViewHelper(BView& view, int16_t& x, int16_t& y);
  static void mapViewToScreenHelper(BView& view, int16_t& x, int16_t& y);
  static BControl* focusNextHelper(BPanel& panel, int16_t tabIndex);
  static BControl* focusNextHelper(BView& view);
  static BControl* focusPrevHelper(BPanel& panel, int16_t tabIndex);
  static BControl* focusPrevHelper(BControl& control);
  static BControl* focusFirstHelper(BView& view);
  static void focusLastHelper(BView& view, BControl*& control);
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

  static BControl* focus(BControl& view);
  static BControl* focusedControl();
  static BControl* focusFirst();
  static BControl* focusLast();
  static BControl* focusNext();
  static BControl* focusPrev();
  static void focusClick();

  static BView* findView(BMouseInputEvent& event);
  static BPoint mapScreenToView(BView& view, int16_t x, int16_t y);
  static BPoint mapViewToScreen(BView& view, int16_t x, int16_t y);
  static BGraphics getGraphics(BView& view);

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
  BMouse::initialize(-1, -1, false);
  BFocusManager::initialize(g, stack);
}

void focuspocus();

#endif