#ifndef __BFOCUSMANAGER_H__
#define __BFOCUSMANAGER_H__

#include "BList.h"
#include "BGraphics.h"
#include "BMouse.h"
#include "BTheme.h"

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

class BView;
class BPoint;
class BView;
class BPanel;

using namespace Buratino;

class BMouseInputEvent;
class BKeyboardInputEvent;
class BCommandInputEvent;

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

  bool isMouse() {
    return type & evMouse;
  }
  bool isKeyboard() {
    return type & evKeyboard;
  }
  bool isCommand() {
    return type == evCommand;
  }

  BMouseInputEvent& asMouse() {
    return (BMouseInputEvent&)*this;
  }

  BKeyboardInputEvent& asKeyboard() {
    return (BKeyboardInputEvent&)*this;
  }

  BCommandInputEvent& asCommand() {
    return (BCommandInputEvent&)*this;
  }

};

struct BMouseInputEvent: BInputEvent {
  int16_t x;
  int16_t y;
  bool buttonDown;
};

struct BKeyboardInputEvent: BInputEvent {
  uint16_t code;
};

struct BCommandInputEvent: BInputEvent {
  enum CommandType {
    cmFocus,
    cmBlur,
    cmRadioGroup
  };

  uint16_t command;

  BCommandInputEvent() { 
    type = evCommand;
  }
};

struct BFocusInputEvent: BCommandInputEvent {
  BView* blurred;
  BView* focused;
};

class BFocusManager {
protected:
  BList<BPanel*> _stack;
  BGraphics& _g;
  BView* _focused;
  BView* _capture;
  bool _isDirty;
  bool _needsLayout;
  bool _needsRootLayout;
  BTheme* _theme;

protected:
  bool findViewHelper(BView& view, int16_t x, int16_t y, BView*& target);
  
  void mapScreenToViewHelper(BView& view, int16_t& x, int16_t& y);
  void mapViewToScreenHelper(BView& view, int16_t& x, int16_t& y);

  BView* focusNextHelper(BPanel& panel, int16_t tabIndex);
  BView* focusNextHelper(BView& view);
  BView* focusPrevHelper(BPanel& panel, int16_t tabIndex);
  BView* focusPrevHelper(BView& view);
  BView* focusFirstHelper(BView& view);
  void focusLastHelper(BView& view, BView*& control);

  void applyOffset(BView& view, int16_t& x, int16_t& y, int8_t sign = 1);
  void applyOffset(BView& view, int16_t& x, int16_t& y, uint16_t& width, uint16_t& height);
  void applyMargins(BView& view, int16_t& x, int16_t& y, int8_t sign = 1);
  void applyMargins(BView& view, int16_t& x, int16_t& y, uint16_t& width, uint16_t& height);
  void applyPadding(BPanel& panel, int16_t& x, int16_t& y, int8_t sign = 1);
  void applyPadding(BPanel& panel, int16_t& x, int16_t& y, uint16_t& width, uint16_t& height);

  void drawPass(BView& view, BGraphics& g);
  void layoutPass(BPanel& panel);

  void touchTree(BView& view);

  void broadcastCommandHelper(BView& view, BCommandInputEvent& event);
public:
  template<size_t N>
  BFocusManager(BGraphics& g, BView* (&stack)[N], BTheme& theme) 
    : _g(g), _stack(stack, N, N), _theme(&theme) {
    for(int i = _stack.Length() - 1; i >= 0; --i) {
      if (!_stack[i]) {
        _stack.Remove(i);
      }
    }
    _needsRootLayout = true;
  }
  
  BPanel* root();
  BTheme& theme();
  
  void handleEvent(BInputEvent& event);
  void broadcastCommand(BCommandInputEvent& event);
  
  BView* focus(BView& view);
  BView* focusedControl();
  BView* focusFirst();
  BView* focusLast();
  BView* focusNext();
  BView* focusPrev();

  BView* findView(BMouseInputEvent& event);
  BPoint mapScreenToView(BView& view, int16_t x, int16_t y);
  BPoint mapViewToScreen(BView& view, int16_t x, int16_t y);
  BGraphics getGraphics(BView& view);
  BGraphics getGraphics();

  void captureMouse(BView& view);
  void releaseMouse(BView& view);
  BView* capturingView();

  void dirty();
  void dirtyLayout();

  void layoutRoot();

  void push(BPanel& panel);
  void pop();
  void popToTop(BPanel& panel);

  void loop();

  friend void focuspocus();
};

void focuspocus();

#endif