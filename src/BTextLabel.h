#ifndef __BTEXTLABEL_H__
#define __BTEXTLABEL_H__

#include "BView.h"

class BTextLabel : public BView
{
public:
  B::Alignment horizontalAlignment;
  B::Alignment verticalAlignment;
  const char* text;
  BNullable<uint8_t> fontSize;   // unset: the theme's
  BNullable<uint16_t> fontColor; // unset: the theme's
  uint8_t actualFontSize() { return themed(fontSize, &BTheme::fontSize); }
  uint16_t actualFontColor() { return themed(fontColor, &BTheme::fontColor); }
public:
  BTextLabel();
  // Changes the text at runtime and re-measures. The string is not copied:
  // keep it alive (a literal, a global or a static buffer).
  void setText(const char* value);
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  virtual void draw(BGraphics& g);
};

#endif