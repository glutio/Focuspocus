// Focuspocus demo: every widget on one screen, plus a popup.
// Hardware: Arduino Mega, Adafruit ILI9341 TFT with FT6206 capacitive touch,
// optional push buttons on pins 7 (previous), 6 (ok) and 5 (next).
// Simulate it on Wokwi: see wokwi-project.txt.
// The same screens run on Windows: extras/win32/demo.cpp.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Focuspocus.h>
#include "DemoScreens.h"

#define TFT_DC 9
#define TFT_CS 10

Adafruit_ILI9341 tft(TFT_CS, TFT_DC);
Adafruit_FT6206 touch;

BAdafruitGraphics graphics(tft);
BWinTheme theme;
BRootView* screens[] = { &MainScreen::screen, nullptr };  // room for the popup
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

  MainScreen::begin(tft.width(), tft.height(), theme);
  PopupScreen::begin();
}

void loop() {
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    mouse.move(tft.width() - p.x, tft.height() - p.y, true);  // FT6206 is mirrored
  } else {
    mouse.button(false);
  }
  prevButton.update();
  okButton.update();
  nextButton.update();
  fm.loop(millis());
}
