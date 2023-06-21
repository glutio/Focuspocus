#include <Arduino.h>
#include "BGraphics.h"
#include "BFocusManager.h"
#include "BBitmapButton.h"

BBitmapButton::BBitmapButton() : bitmap(0) {
  _animate = false;
}

void BBitmapButton::draw(BGraphics& g) {
  BView::draw(g); // draw bounding rect for debug if turned on

  if (bitmap) {
    if (mask) { // draw with mask
      g.drawBitmap(0, 0, bitmap, mask, g.width, g.height);
    }
    else { // draw without mask
      g.drawBitmap(0, 0, bitmap, g.width, g.height);
    }
     
    if (isFocused()) {
      if (mask) {
        g.drawContour(0, 0, mask, g.width, g.height, focusManager().theme().focusBackground);
      }
      else {
        g.drawRect(0, 0, g.width, g.height, focusManager().theme().focusBackground);
      }
    }

    drawContent(g); // draw text, inherited from BButton
  } 
}