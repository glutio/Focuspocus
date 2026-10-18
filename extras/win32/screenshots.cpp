// Renders the README screenshots (docs/images/*.png): each widget on its own,
// then the Demo and LayoutFocus screens. Run from the repository root:
//   build\Debug\focuspocus_screenshots.exe
// With "gif" it records a Demo session as frames (build\gif\*.png) instead;
// extras\win32\demogif.py turns them into examples\Demo\demo.gif.
#undef WIN32_LEAN_AND_MEAN  // GDI+ needs the full Windows headers
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>
using std::min;
using std::max;
#include <gdiplus.h>
#include "BWin32Graphics.h"
#include "Focuspocus.h"
#include "DemoScreens.h"
#include "LayoutFocusScreens.h"

static const int ZOOM = 2;

// Saves the framebuffer, zoomed, as a PNG.
static void savePng(BWin32Graphics& gfx, const std::wstring& path) {
  int w = gfx.displayWidth(), h = gfx.displayHeight();
  std::vector<uint32_t> big(w * ZOOM * h * ZOOM);
  for (int y = 0; y < h * ZOOM; ++y) {
    for (int x = 0; x < w * ZOOM; ++x) {
      big[y * w * ZOOM + x] = gfx.bits()[(y / ZOOM) * w + x / ZOOM];
    }
  }
  Gdiplus::Bitmap bmp(w * ZOOM, h * ZOOM, w * ZOOM * 4, PixelFormat32bppRGB, (BYTE*)big.data());
  CLSID png = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };
  bmp.Save(path.c_str(), &png, nullptr);
}

static void save(BWin32Graphics& gfx, const wchar_t* name) {
  savePng(gfx, std::wstring(L"docs\\images\\") + name + L".png");
}

// A scripted Demo session, tapping and dragging like a finger on the touch
// screen. One frame per 60 ms.
static void recordGif() {
  CreateDirectoryW(L"build\\gif", nullptr);
  BWin32Graphics gfx(240, 320);
  BWinTheme theme;
  BScreen* screens[] = { &MainScreen::screen, nullptr };
  BFocusManager fm(gfx, screens, theme);
  BMouse mouse(fm);
  MainScreen::begin(240, 320, theme);
  unsigned long ms = 0;
  int frame = 0;
  // Where the finger is on each frame, for demogif.py's touch indicator.
  FILE* touches = _wfopen(L"build\\gif\\touches.txt", L"w");
  int16_t touchX = 120, touchY = 200;
  bool touching = false;
  auto wait = [&](int msecs) {
    for (int t = 0; t < msecs; t += 20) {
      fm.loop(ms += 20);
      if (t % 60 == 0) {
        wchar_t path[64];
        swprintf(path, 64, L"build\\gif\\%04d.png", frame++);
        savePng(gfx, path);
        fprintf(touches, "%d %d %d\n", touchX, touchY, touching ? 1 : 0);
      }
    }
  };
  auto move = [&](int16_t x, int16_t y, bool down) {
    mouse.move(x, y, down);
    touchX = x;
    touchY = y;
    touching = down;
  };
  // The finger glides to the next spot (no touch) before pressing there.
  auto glide = [&](int16_t x, int16_t y) {
    int16_t fromX = touchX, fromY = touchY;
    for (int i = 1; i <= 6; ++i) {
      touchX = fromX + (x - fromX) * i / 6;
      touchY = fromY + (y - fromY) * i / 6;
      wait(60);
    }
  };
  auto center = [&](BView& v, int16_t& x, int16_t& y) {
    BGraphics g = fm.getGraphics(v);
    x = g.x + g.width / 2;
    y = g.y + g.height / 2;
  };
  auto tap = [&](BView& v, int hold) {
    int16_t x, y;
    center(v, x, y);
    glide(x, y);
    move(x, y, true);
    wait(200);
    move(x, y, false);
    wait(hold);
  };
  wait(1200);
  tap(MainScreen::helloButton, 1500);
  tap(PopupScreen::ok, 600);
  tap(MainScreen::colorButton, 1200);
  tap(ListScreen::items[1], 1000);
  tap(MainScreen::checkbox1, 400);
  tap(MainScreen::checkbox2, 400);
  tap(MainScreen::radio1, 400);
  tap(MainScreen::radio2, 600);
  int16_t x, y;
  center(MainScreen::hscrollbar, x, y);
  BGraphics h = fm.getGraphics(MainScreen::hscrollbar);
  glide(h.x + 8, y);
  move(h.x + 8, y, true);  // the thumb, at the left end
  wait(100);
  for (int i = 1; i <= 25; ++i) {
    move(h.x + 8 + (h.width - 16) * 8 / 10 * i / 25, y, true);
    wait(60);
  }
  move(h.x + 8 + (h.width - 16) * 8 / 10, y, false);
  wait(800);
  BGraphics v = fm.getGraphics(MainScreen::vscrollbar);
  glide(v.x + v.width / 2, v.y + 8);
  move(v.x + v.width / 2, v.y + 8, true);  // the thumb, at the top
  wait(100);
  for (int i = 1; i <= 30; ++i) {
    move(v.x + v.width / 2, v.y + 8 + v.height * i / 30, true);
    wait(60);
  }
  move(v.x + v.width / 2, v.y + v.height, false);
  wait(1500);
  tap(MainScreen::toTop, 1500);
  fclose(touches);
}

// Lays out one widget, centered on a small screen, and saves it.
static void shot(const wchar_t* name, BView& view, int16_t w, int16_t h, void (*act)(BFocusManager&) = nullptr) {
  BView* items[] = { &view };
  BStackPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  panel.horizontalAlignment = B::center;
  panel.verticalAlignment = B::center;
  BScreen root(panel);
  BWin32Graphics gfx(w, h);
  BWinTheme theme;
  BScreen* screens[] = { &root };
  BFocusManager fm(gfx, screens, theme);
  fm.loop(20);
  if (act) {
    act(fm);
    fm.loop(40);
  }
  save(gfx, name);
}

static void focus(BFocusManager& fm) { fm.focusNext(); }

int main(int argc, char** argv) {
  Gdiplus::GdiplusStartupInput input;
  ULONG_PTR token;
  Gdiplus::GdiplusStartup(&token, &input, nullptr);
  if (argc > 1 && strcmp(argv[1], "gif") == 0) {
    recordGif();
    Gdiplus::GdiplusShutdown(token);
    return 0;
  }
  CreateDirectoryW(L"docs", nullptr);
  CreateDirectoryW(L"docs\\images", nullptr);

  BButton button;
  button.text = "Button";
  shot(L"button", button, 120, 50);
  BButton focused;
  focused.text = "Focused";
  shot(L"button-focused", focused, 120, 50, focus);

  BCheckBox check;
  check.text = "Check box";
  check.state = true;
  shot(L"checkbox", check, 140, 40);
  BRadioButton radio;
  radio.text = "Radio button";
  radio.state = true;
  shot(L"radio", radio, 140, 40);

  BTextLabel label;
  label.text = "Label";
  label.fontSize = 3;
  label.fontColor = 0xe361;
  shot(L"label", label, 120, 50);

  BProgressBar bar;
  bar.maximum = 100;
  bar.value = 60;
  bar.width = 140;
  bar.height = 16;
  shot(L"progressbar", bar, 160, 40);

  BScrollbar scroll;
  scroll.maximum = 100;
  scroll.value = 30;
  scroll.width = 140;
  shot(L"scrollbar", scroll, 160, 40);

  BGauge gauge;
  gauge.maximum = 100;
  gauge.value = 70;
  gauge.needleColor = 0x0999;
  shot(L"gauge", gauge, 140, 80);

  // The Demo, as on the device.
  BWin32Graphics gfx(240, 320);
  BWinTheme theme;
  BScreen* screens[] = { &MainScreen::screen, nullptr };
  BFocusManager fm(gfx, screens, theme);
  MainScreen::begin(240, 320, theme);
  unsigned long ms = 0;
  auto run = [&](int frames) { for (int i = 0; i < frames; ++i) fm.loop(ms += 20); };
  run(30);
  save(gfx, L"demo-main");
  MainScreen::showList(&MainScreen::colorButton, true);
  run(30);
  save(gfx, L"demo-list");
  ListScreen::pick(&ListScreen::items[2], true);
  MainScreen::hscrollbar.setValue(80);
  MainScreen::turnGauge(&MainScreen::hscrollbar, 80);
  MainScreen::vscrollbar.setValue(60);  // halfway: the top rows are cut
  MainScreen::scroll(&MainScreen::vscrollbar, 60);
  run(30);
  save(gfx, L"demo-scrolled");
  MainScreen::showPopup(&MainScreen::helloButton, true);
  run(30);
  save(gfx, L"demo-popup");

  // LayoutFocus: layout, focus and an auto-sized popup.
  BWin32Graphics lgfx(240, 320);
  BScreen* lscreens[] = { &LayoutScreen::screen, nullptr, nullptr };
  BFocusManager lfm(lgfx, lscreens, theme);
  LayoutScreen::begin();
  FocusScreen::begin();
  AskScreen::begin();
  auto lrun = [&](int frames) { for (int i = 0; i < frames; ++i) lfm.loop(ms += 20); };
  lrun(30);
  save(lgfx, L"layout");
  lfm.push(FocusScreen::screen);
  lrun(30);
  save(lgfx, L"layout-focus");
  lfm.push(AskScreen::screen);
  lrun(30);
  save(lgfx, L"layout-ask");

  Gdiplus::GdiplusShutdown(token);
  return 0;
}
