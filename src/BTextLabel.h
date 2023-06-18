#ifndef __BTEXTLABEL_H__
#define __BTEXTLABEL_H__

#include "BView.h"

class BTextLabel : public BView, public BColorAware, public BFontAware 
{
public:
  enum Alignment {
    top,
    bottom,
    center,
    left,
    right
  };
  Alignment horizontalAlignment;
  Alignment verticalAlignment;
  const char* text;
public:
  BTextLabel();
  virtual void measure(int16_t availableWidth, int16_t availableHeight);
  virtual void draw(BGraphics& g);
};

#endif