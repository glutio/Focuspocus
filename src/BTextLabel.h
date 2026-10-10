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
  // Changes the text at runtime and re-measures. The string is not copied:
  // keep it alive (a literal, a global or a static buffer).
  void setText(const char* value);
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  virtual void draw(BGraphics& g);
};

#endif