#ifndef __BPOPUP_H__
#define __BPOPUP_H__

#include "BView.h"

class BPopup: public BStackPanel {
public:
  template<size_t N>
  BPopup(BView* (&children)[N]) : BStackPanel(children) {
    verticalAlignment = B::center;
    horizontalAlignment = B::center;
  }

  virtual void draw(BGraphics& g) {
    auto c = focusManager().theme().popupScreenColor;
    g.drawShade(0, 0, g.width, g.height, c);
  }

};

#endif