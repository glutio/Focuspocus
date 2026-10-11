// Windows host for the demo in examples/Demo: the same screens
// (DemoScreens.h), drawn by BWin32Graphics instead of an Adafruit display.
//
// Controls: click = touch, Tab/Shift+Tab = focus next/prev,
// Enter = press focused control, arrows = scrollbars, Esc = quit.
#include "BWin32Window.h"
#include "DemoScreens.h"

BWin32Graphics gfx(240, 320); // ILI9341 portrait
BWinTheme theme;
BRootView* screens[] = { &MainScreen::screen, nullptr };
BFocusManager fm(gfx, screens, theme);
BMouse mouse(fm);
BKeyboard keyboard(fm);

int main() {
  MainScreen::begin(gfx.displayWidth(), gfx.displayHeight(), theme);

  BWin32Window window(gfx, fm, mouse, keyboard, L"Focuspocus", 2);
  while (window.pump()) {
    fm.loop(GetTickCount());
    window.present();
    Sleep(5);
  }
  return 0;
}
