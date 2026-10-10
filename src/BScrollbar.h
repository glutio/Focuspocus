#ifndef __BSCROLLBAR_H__
#define __BSCROLLBAR_H__

#include "BView.h"

class BScrollbar: public BControl, public BColorAware {
public:
  BEVENT(Change, BScrollbar, int16_t)

protected:
  int16_t _grabOffset;
  int16_t _thumbPos;
  int16_t _thumbSize;

public:
  int16_t minimum;
  int16_t maximum;
  int16_t value;
  int16_t step;
  B::Orientation orientation;

protected:
  void updateThumb();
  void stepBy(int16_t delta);
  void moveThumb(BMouseInputEvent& pos);
  bool thumbHitTest(BMouseInputEvent& event);
  BRect clientRect();
  void hideThumb(BGraphics& g);
  void showThumb(BGraphics& g);
  void handleMouse(BMouseInputEvent& event);
  void handleKeyboard(BKeyboardInputEvent& event);
public:
  BScrollbar();
  // Clamps to [minimum, maximum] and redraws. Does not raise onChange
  // (onChange reports user input only). After changing minimum, maximum or
  // step at runtime, call dirtyLayout().
  void setValue(int16_t val);
  virtual void draw(BGraphics& g);
  virtual void handleEvent(BInputEvent& event);
  virtual void layout();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
};

#endif