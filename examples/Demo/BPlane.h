// A custom view: a rectangle tilted in 3D. Shows how to write your own view
// (layout() computes, draw() draws, drawing outside draw() via getGraphics).
#ifndef __BPLANE_H__
#define __BPLANE_H__

#include <math.h>
#include "Focuspocus.h"

class BPlane: public BView, public BColorAware {
protected:
  int16_t rotated[4][2];

  void drawPlane(BGraphics& g, uint16_t color) {
    for (int i = 0; i < 4; ++i) {
      g.drawLine(rotated[i][0], rotated[i][1], rotated[(i + 1) % 4][0], rotated[(i + 1) % 4][1], color);
    }
  }

public:
  int16_t coronalAngle;
  int16_t transverseAngle;

  BPlane() : rotated(), coronalAngle(0), transverseAngle(0) {
    focusable = false;
  }

  // Erase the current outline before changing the angles.
  void hidePlane() {
    BGraphics g = focusManager().getGraphics(*this);
    drawPlane(g, viewBackground(*this));
  }

  virtual void draw(BGraphics& g) {
    BView::draw(g);
    drawPlane(g, viewColor(*this));
  }

  virtual void layout() {
    const double pi = 3.14159265358979323846;
    double rad1 = coronalAngle * pi / 180.0;
    double rad2 = transverseAngle * pi / 180.0;
    double sin1 = sin(rad1), cos1 = cos(rad1);
    double sin2 = sin(rad2), cos2 = cos(rad2);
    int16_t halfWidth = actualWidth / 2;
    int16_t halfHeight = actualHeight / 2;
    int16_t corners[4][2] = {
      { (int16_t)-halfWidth, (int16_t)-halfHeight },
      { halfWidth, (int16_t)-halfHeight },
      { halfWidth, halfHeight },
      { (int16_t)-halfWidth, halfHeight },
    };
    for (int i = 0; i < 4; ++i) {
      double x = corners[i][0] * cos2;              // skew for the x/y tilt
      double y = corners[i][1] - corners[i][0] * sin2;
      rotated[i][0] = halfWidth + (x * cos1 - y * sin1);   // rotate around z
      rotated[i][1] = halfHeight + (x * sin1 + y * cos1);
    }
    clearDirtyLayout();
    dirty();
  }
};

#endif
