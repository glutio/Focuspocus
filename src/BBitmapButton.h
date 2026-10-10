#ifndef __BBITMAPBUTTON_H__
#define __BBITMAPBUTTON_H__

#include "BView.h"

class BBitmapButton: public BButton {
public:
  const uint16_t* bitmap;  // RGB565, width x height (PROGMEM on AVR)
  const uint8_t* mask;     // optional 1-bit transparency mask
public:  
  BBitmapButton();
  virtual void draw(BGraphics& g);
};

#endif
