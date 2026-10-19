// Windows host for examples/LayoutFocus: the same screens
// (LayoutFocusScreens.h), drawn by BWin32Graphics instead of an Adafruit display.
//
// Controls: click = touch, Tab/Shift+Tab = focus next/prev,
// Enter = press focused control, Esc = quit.
#include "BWin32Window.h"
#include "LayoutFocusScreens.h"

BWin32Graphics gfx(240, 320); // ILI9341 portrait
BWinTheme theme;
BScreen* screens[] = { &LayoutScreen::screen, nullptr, nullptr };
BFocusManager fm(gfx, screens, theme);
BMouse mouse(fm);
BKeyboard keyboard(fm);

int main() {
  LayoutScreen::begin();
  FocusScreen::begin(fm);
  AskScreen::begin();

  BWin32Window window(gfx, fm, mouse, keyboard, L"Focuspocus - layout and focus", 2);
  while (window.pump()) {
    fm.loop(GetTickCount());
    window.present();
    Sleep(5);
  }
  return 0;
}
