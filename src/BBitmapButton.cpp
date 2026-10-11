#include "BGraphics.h"
#include "BFocusManager.h"
#include "BBitmapButton.h"

BBitmapButton::BBitmapButton() : bitmap(0), mask(0) {
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

    drawContent(g); // text in the focus color when focused, inherited from BButton
  } 
}