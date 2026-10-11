// The demo's screens, shared by Demo.ino (Arduino) and extras/win32/demo.cpp
// (Windows), so both run exactly the same UI.
#ifndef __DEMOSCREENS_H__
#define __DEMOSCREENS_H__

#include "Focuspocus.h"
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
  BBitmapButton helloButton;
  BButton colorButton;
  BButton margin;
  BView* buttonsContent[] = { &helloButton, &colorButton, &margin };
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
  BGauge gauge;
  BTextLabel more;
  BButton toTop;

  BView* mainContent[] = { &buttons, &checkboxes, &labels, &progress, &gauge, &toTop, &more };
  BStackPanel main(mainContent);
  BView* viewportContent[] = { &main };
  BPanel viewport(viewportContent);  // clips main; main.y scrolls it
  BScrollbar hscrollbar;
  BScrollbar vscrollbar;

  BView* rootContent[] = { &viewport, &hscrollbar, &vscrollbar };
  BPanel root(rootContent);
  BRootView screen(root);
}

// A popup used as a list box: picking a color updates the main screen.
namespace ListScreen {
  struct Choice { const char* name; uint16_t color; };
  const Choice choices[] = { { "Orange", 0xe361 }, { "Green", 0x0560 }, { "Blue", 0x001F } };
  BButton items[3];
  BView* listContent[] = { &items[0], &items[1], &items[2] };
  BStackPanel list(listContent);
  BView* popupContent[] = { &list };
  BPopup popup(popupContent);
  BRootView screen(popup);

  void pick(BButton* sender, bool) {
    const Choice& c = choices[sender - items];
    MainScreen::colorButton.text = c.name;
    MainScreen::colorButton.dirtyLayout();
    MainScreen::label2.fontColor = c.color;
    sender->focusManager().pop();
  }

  void begin() {
    for (int i = 0; i < 3; ++i) {
      items[i].text = choices[i].name;
      items[i].width = -1;
      items[i].onClick += pick;
    }
    list.orientation = B::vertical;
    list.spacing = 4;
    list.padding(6);
    list.border = true;
    popup.width = -1;
    popup.height = -1;
    screen.focused = &items[0];
  }
}

namespace MainScreen {
  void showPopup(BButton* sender, bool) {
    sender->focusManager().push(PopupScreen::screen);
  }

  void showList(BButton* sender, bool) {
    sender->focusManager().push(ListScreen::screen);
  }

  void turnGauge(BScrollbar*, int16_t value) {
    gauge.setValue(value);
  }

  void scroll(BScrollbar*, int16_t value) {
    main.y = -value;
    viewport.dirtyLayout();
  }

  void scrollToTop(BButton*, bool) {
    vscrollbar.setValue(0);
    scroll(&vscrollbar, 0);
  }

  void begin(int16_t screenWidth, int16_t screenHeight, BTheme& theme) {
    PopupScreen::begin();
    ListScreen::begin();

    helloButton.bitmap = buttonPixMap;
    helloButton.mask = buttonAlphaMask;
    helloButton.width = BUTTON_W;
    helloButton.height = BUTTON_H;
    helloButton.text = "hello";
    helloButton.onClick += showPopup;

    colorButton.height = -1;
    colorButton.text = ListScreen::choices[0].name;
    colorButton.onClick += showList;
    margin.height = -1;
    margin.text = "margin";
    margin.margin(10);

    buttons.spacing = 4;
    buttons.width = -1;
    buttons.horizontalAlignment = B::center;
    buttons.verticalAlignment = B::center;  // hello (fixed 40px) in the 50px row
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
    label2.fontColor = ListScreen::choices[0].color;
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

    gauge.width = 160;
    gauge.needleColor = 0x0999;
    more.text = "You scrolled to the end!";
    toTop.text = "back to top";
    toTop.onClick += scrollToTop;

    main.spacing = 10;
    main.orientation = B::vertical;
    main.height = 380;  // taller than the viewport: scroll to see the rest
    main.horizontalAlignment = B::center;
    viewport.x = 2;  // 2px gap on every side, scrollbars included
    viewport.y = 2;
    viewport.width = screenWidth - theme.scrollbarSize - 5;
    viewport.height = screenHeight - theme.scrollbarSize - 5;
    main.width = viewport.width;

    hscrollbar.x = viewport.x;
    hscrollbar.width = viewport.width;
    hscrollbar.y = screenHeight - theme.scrollbarSize - 1;
    hscrollbar.minimum = 0;
    hscrollbar.maximum = 100;
    hscrollbar.step = 10;
    hscrollbar.onChange += turnGauge;
    vscrollbar.height = screenHeight;
    vscrollbar.x = screenWidth - theme.scrollbarSize - 1;
    vscrollbar.orientation = B::vertical;
    vscrollbar.minimum = 0;
    vscrollbar.maximum = main.height - viewport.height;
    vscrollbar.step = 20;
    vscrollbar.onChange += scroll;

    root.width = -1;
    root.height = -1;
  }
}

#endif