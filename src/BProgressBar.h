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
public:
  BProgressBar();
  // Clamps to [minimum, maximum] and redraws only the part that changed.
  void setValue(int16_t val);
  virtual void draw(BGraphics& g);
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
};

#endif
