// Layout and focus, shown on two screens and a popup.
// Layout screen: fixed and stretched sizes, min/max, margins, and a sample
// panel the buttons below turn (orientation), align and space.
// Focus screen: walk the focus with the prev/next pins (Tab/Shift+Tab on
// Windows) through a nested grid, checkbox, radios and buttons; the label
// shows which view has it. Each screen remembers its focus: going back or
// closing the popup returns it to where it was.
// Hardware: Arduino Mega, Adafruit ILI9341 TFT with FT6206 capacitive touch,
// optional push buttons on pins 7 (previous), 6 (ok) and 5 (next).
// The same screens run on Windows: extras/win32/layoutfocus.cpp.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Focuspocus.h>
#include "LayoutFocusScreens.h"

#define TFT_DC 9
#define TFT_CS 10

Adafruit_ILI9341 tft(TFT_CS, TFT_DC);
Adafruit_FT6206 touch;

BAdafruitGraphics graphics(tft);
BWinTheme theme;
BRootView* screens[] = { &LayoutScreen::screen, nullptr, nullptr };  // + focus screen + popup
BFocusManager fm(graphics, screens, theme);
BMouse mouse(fm);
BKeyboard keyboard(fm);

BButtonPin prevButton(7);
BButtonPin okButton(6);
BButtonPin nextButton(5);

void onPrev(BButtonPin*, bool pressed) { if (pressed) fm.focusPrev(); }
void onNext(BButtonPin*, bool pressed) { if (pressed) fm.focusNext(); }
void onOk(BButtonPin*, bool pressed) { keyboard.sendKey(BKeyboard::kbEnter, pressed); }

void setup() {
  tft.begin();
  touch.begin(40);
  prevButton.begin();
  okButton.begin();
  nextButton.begin();
  prevButton.onChange += onPrev;
  okButton.onChange += onOk;
  nextButton.onChange += onNext;

  LayoutScreen::begin();
  FocusScreen::begin();
  AskScreen::begin();
}

void loop() {
  // One read: touched() then getPoint() can see a finger lift in between and return 0,0.
  TS_Point p = touch.getPoint();
  if (p.z) {
    mouse.move(tft.width() - p.x, tft.height() - p.y, true);  // FT6206 is mirrored
  } else {
    mouse.button(false);
  }
  prevButton.update();
  okButton.update();
  nextButton.update();
  fm.loop(millis());
}
