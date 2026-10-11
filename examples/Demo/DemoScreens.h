// The demo's screens, shared by Demo.ino (Arduino) and extras/win32/demo.cpp
// (Windows), so both run exactly the same UI.
#ifndef __DEMOSCREENS_H__
#define __DEMOSCREENS_H__

#include "Focuspocus.h"
#include "BPlane.h"
#ifndef PROGMEM
#define PROGMEM  // Windows build
#endif
#include "Bitmaps.h"

// A progress bar that animates itself using the framework's tick().
class PulsingBar: public BProgressBar {
  BInterval _step;
public:
  PulsingBar() : _step(50) {}
  virtual void tick(unsigned long ms) {
    if (_step.elapsed(ms)) {
      setValue(value >= maximum ? minimum : value + 1);
    }
  }
};

namespace PopupScreen {
  BTextLabel message;
  BButton ok;
  BView* boxContent[] = { &message, &ok };
  BStackPanel box(boxContent);
  BView* popupContent[] = { &box };
  BPopup popup(popupContent);
  BRootView screen(popup);

  void handleOk(BButton* sender, bool) {
    sender->focusManager().pop();
  }

  void begin() {
    message.text = "Hello from a popup!";
    ok.text = "Ok";
    ok.onClick += handleOk;
    box.orientation = B::vertical;
    box.horizontalAlignment = B::center;
    box.spacing = 10;
    box.padding(8);
    box.border = true;
    popup.width = -1;
    popup.height = -1;
    screen.focused = &ok;
  }
}

namespace MainScreen {
  BBitmapButton bitmapButton;
  BButton taller;
  BButton margin;
  BView* buttonsContent[] = { &bitmapButton, &taller, &margin };
  BStackPanel buttons(buttonsContent);

  BCheckBox checkbox1;
  BCheckBox checkbox2;
  BRadioButton radio1;
  BRadioButton radio2;
  BView* leftContent[] = { &checkbox1, &radio1 };
  BStackPanel left(leftContent);
  BView* rightContent[] = { &checkbox2, &radio2 };
  BStackPanel right(rightContent);
  BView* checkboxesContent[] = { &left, &right };
  BStackPanel checkboxes(checkboxesContent);

  BTextLabel label1;
  BTextLabel label2;
  BView* labelsContent[] = { &label1, &label2 };
  BStackPanel labels(labelsContent);

  PulsingBar progress;
  BPlane plane;

  BView* mainContent[] = { &buttons, &checkboxes, &labels, &progress, &plane };
  BStackPanel main(mainContent);
  BScrollbar hscrollbar;
  BScrollbar vscrollbar;

  BView* rootContent[] = { &main, &hscrollbar, &vscrollbar };
  BPanel root(rootContent);
  BRootView screen(root);

  void showPopup(BButton* sender, bool) {
    sender->focusManager().push(PopupScreen::screen);
  }

  void growTaller(BButton* sender, bool) {
    sender->height *= 2;
    sender->dirtyLayout();
  }

  void tilt(BScrollbar*, int16_t) {
    plane.hidePlane();
    plane.transverseAngle = hscrollbar.value;
    plane.coronalAngle = vscrollbar.value;
    plane.dirtyLayout();
  }

  void begin(int16_t screenWidth, int16_t screenHeight, BTheme& theme) {
    bitmapButton.bitmap = buttonPixMap;
    bitmapButton.mask = buttonAlphaMask;
    bitmapButton.width = BUTTON_W;
    bitmapButton.height = BUTTON_H;
    bitmapButton.text = "popup";
    bitmapButton.onClick += showPopup;

    taller.height = -1;
    taller.text = "taller";
    taller.onClick += growTaller;
    margin.height = -1;
    margin.text = "margin";
    margin.margin(10);

    buttons.spacing = 4;
    buttons.width = -1;
    buttons.horizontalAlignment = B::center;
    buttons.border = true;
    buttons.padding(3);

    checkbox1.text = "Left aligned";
    checkbox2.text = "Right aligned";
    checkbox2.alignment = B::right;
    radio1.text = "Option 1";
    radio1.group = "options";
    radio2.text = "Option 2";
    radio2.group = "options";
    radio2.alignment = B::right;

    left.orientation = B::vertical;
    left.width = -1;
    left.spacing = 5;
    right.orientation = B::vertical;
    right.width = -1;
    right.spacing = 5;
    right.horizontalAlignment = B::right;
    checkboxes.width = -1;
    checkboxes.padding(3);
    checkboxes.border = true;

    label1.text = "use color";
    label1.fontColor = 0xa0b6;
    label1.fontSize = 2;
    label1.background = 0xa69d;
    label2.text = "and font";
    label2.fontColor = 0xe361;
    label2.fontSize = 3;
    label2.background = 0xa69d;
    labels.orientation = B::vertical;
    labels.spacing = 5;
    labels.background = 0xa69d;
    labels.border = true;
    labels.width = -1;

    progress.minimum = 0;
    progress.maximum = 100;
    progress.width = -1;
    progress.height = 20;
    progress.color = 0x0999;

    plane.margin(20);
    plane.width = 50;
    plane.height = 50;

    main.spacing = 10;
    main.orientation = B::vertical;
    main.x = 2;  // 2px gap on every side, scrollbars included
    main.y = 2;
    main.width = screenWidth - theme.scrollbarSize - 5;
    main.height = screenHeight - theme.scrollbarSize - 5;
    main.horizontalAlignment = B::center;

    hscrollbar.x = main.x;
    hscrollbar.width = main.width;
    hscrollbar.y = screenHeight - theme.scrollbarSize - 1;
    hscrollbar.minimum = -90;
    hscrollbar.maximum = 90;
    hscrollbar.step = 20;
    hscrollbar.onChange += tilt;
    vscrollbar.height = screenHeight;
    vscrollbar.x = screenWidth - theme.scrollbarSize - 1;
    vscrollbar.orientation = B::vertical;
    vscrollbar.minimum = -90;
    vscrollbar.maximum = 90;
    vscrollbar.step = 20;
    vscrollbar.onChange += tilt;

    root.width = -1;
    root.height = -1;
  }
}

#endif
