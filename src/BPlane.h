#ifndef __BPLANE_H__
#define __BPLANE_H__

#include "BView.h"
class BPlane: public BView, public BColorAware {
protected:
  int rotated[4][2];

public:
  int16_t coronalAngle;
  int16_t transverseAngle;

  void hidePlane() {
    auto g = focusManager().getGraphics(*this);
    g.drawRect(0,0,g.width, g.height, 0x0FF0);
    drawPlane(g, viewBackground(*this));
  }

  void drawPlane(BGraphics& g, uint16_t color) {
    // Drawing the rectangle
    // Draw the rectangle
    for (int i = 0; i < 4; ++i) {
        g.drawLine(rotated[i][0], rotated[i][1], rotated[(i+1)%4][0], rotated[(i+1)%4][1], color);
    }   
  }

  virtual void draw(BGraphics& g) { 
    BView::draw(g);
    drawPlane(g, 0xFFFF);
  }

  virtual void layout() {
    double rad1 = coronalAngle * PI / 180.0;
    double rad2 = transverseAngle * PI / 180.0;

    double sin1 = sin(rad1);
    double cos1 = cos(rad1);

    double sin2 = sin(rad2);
    double cos2 = cos(rad2);

    // Offset by half the width and height
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    int corners[4][2] = {
        {-halfWidth, -halfHeight},
        { halfWidth, -halfHeight},
        { halfWidth,  halfHeight},
        {-halfWidth,  halfHeight},
    };

    rotated[4][2];

    for (int i = 0; i < 4; ++i) {
        // Skew and scale for x/y plane "rotation"
        double x = corners[i][0] * cos2;
        double y = corners[i][1] - corners[i][0] * sin2;

        // Rotate around the z-axis
        rotated[i][0] = halfWidth + (x * cos1 - y * sin1);
        rotated[i][1] = halfHeight + (x * sin1 + y * cos1);
    }

    clearDirtyLayout();
    dirty();
  }
};
#endif