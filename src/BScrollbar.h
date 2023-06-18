#ifndef __BSCROLLBAR_H__
#define __BSCROLLBAR_H__

#include "BView.h"

class BScrollbar: public BControl, public BColorAware {
public:
  enum Orientation {
    vertical,
    horizontal
  };
public:
  typedef EventDelegate<BScrollbar, int16_t> ChangeEvent;
  EventSource<ChangeEvent> onChange;

protected:
  int16_t _oldX;
  int16_t _oldY;
  int16_t _thumbX;
  int16_t _thumbY;
  int16_t _thumbSize;

public:
  int16_t minimum;
  int16_t maximum;
  int16_t value;
  Orientation orientation;

protected:
  void redrawThumb(int16_t x, int16_t y);
  void moveThumb(BMouseInputEvent& event);
  bool thumbHitTest(BMouseInputEvent& event);
  int16_t pad();
  
  void handleMouse(BMouseInputEvent& event);
  void handleKeyboard(BKeyboardInputEvent& event);
public:
  BScrollbar();  
  virtual void draw(BGraphics& g);
  virtual void handleEvent(BInputEvent& event);
  virtual void layout();

};

#endif