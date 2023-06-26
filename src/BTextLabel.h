#ifndef __BTEXTLABEL_H__
#define __BTEXTLABEL_H__

#include "BView.h"

class BTextLabel : public BView, public BColorAware, public BFontAware
{
public:
  B::Alignment horizontalAlignment;
  B::Alignment verticalAlignment;
  const char* text;
public:
  BTextLabel();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  virtual void draw(BGraphics& g);
};

#endif