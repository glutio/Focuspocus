// Dialog: a second screen shown as a popup over the first one.
// Tap "Switch" to ask for confirmation; Yes/No closes the popup.
// Hardware: Adafruit 2.8" TFT touch shield (ILI9341 + FT6206), any Arduino.
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Focuspocus.h>

Adafruit_ILI9341 tft(10, 9);  // CS, DC
Adafruit_FT6206 touch;
bool lightOn = false;

// Each screen lives in its own namespace.
namespace ConfirmScreen {
  BTextLabel question;
  BButton yes;
  BButton no;
  BView* buttonItems[] = { &yes, &no };
  BStackPanel buttons(buttonItems);
  BView* boxItems[] = { &question, &buttons };
  BStackPanel box(boxItems);
  BView* popupItems[] = { &box };
  BPopup popup(popupItems);       // shades the screen below, centers its content
  BRootView screen(popup);
}

namespace MainScreen {
  BTextLabel status;
  BButton toggle;
  BView* columnItems[] = { &status, &toggle };
  BStackPanel column(columnItems);
  BRootView screen(column);

  void showStatus() {
    status.setText(lightOn ? "Light is ON" : "Light is OFF");
  }
}

BAdafruitGraphics graphics(tft);
BTheme theme;
BRootView* screens[] = { &MainScreen::screen, nullptr };  // room for the popup
BFocusManager fm(graphics, screens, theme);
BMouse mouse(fm);

void onToggle(BButton*, bool) {
  fm.push(ConfirmScreen::screen);
}

void onYes(BButton*, bool) {
  lightOn = !lightOn;
  MainScreen::showStatus();
  fm.pop();
}

void onNo(BButton*, bool) {
  fm.pop();
}

void setup() {
  tft.begin();
  touch.begin(40);

  using namespace MainScreen;
  status.fontSize = 2;
  showStatus();
  toggle.text = "Switch";
  toggle.onClick += onToggle;
  column.width = -1;
  column.height = -1;
  column.orientation = B::vertical;
  column.horizontalAlignment = B::center;
  column.verticalAlignment = B::center;
  column.spacing = 20;

  ConfirmScreen::question.text = "Are you sure?";
  ConfirmScreen::yes.text = "Yes";
  ConfirmScreen::yes.onClick += onYes;
  ConfirmScreen::no.text = "No";
  ConfirmScreen::no.onClick += onNo;
  ConfirmScreen::buttons.spacing = 10;
  ConfirmScreen::box.orientation = B::vertical;
  ConfirmScreen::box.horizontalAlignment = B::center;
  ConfirmScreen::box.spacing = 10;
  ConfirmScreen::box.padding(10);
  ConfirmScreen::box.border = true;
  ConfirmScreen::popup.width = -1;
  ConfirmScreen::popup.height = -1;
  ConfirmScreen::screen.focused = &ConfirmScreen::no;  // the safe choice
}

void loop() {
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    mouse.move(tft.width() - p.x, tft.height() - p.y, true);  // FT6206 is mirrored
  } else {
    mouse.button(false);
  }
  fm.loop(millis());
}
