#ifndef __BPROGRESSBAR_H__
#define __BPROGRESSBAR_H__

#include "BView.h"

class BProgressBar: public BView, public BColorAware {
public:
  int16_t minimum;
  int16_t maximum;
  int16_t value;
  B::Orientation orientation;
protected:
  BRect clientRect();
  BRect progressRect();
  virtual void parentChanged(BPanel*);
  void onTimerTick(BFocusManager* fm, bool);
  void setValue(int16_t val);
public:
  BProgressBar();
  virtual void draw(BGraphics& g);
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);

};

#endif