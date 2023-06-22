#ifndef __BBITMAPBUTTON_H__
#define __BBITMAPBUTTON_H__

#include "BView.h"

class BBitmapButton: public BButton {
public:
  uint16_t* bitmap;
  uint8_t* mask;
public:  
  BBitmapButton();
  virtual void draw(BGraphics& g);
};

#endif
