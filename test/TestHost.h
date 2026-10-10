#ifndef __TESTHOST_H__
#define __TESTHOST_H__

// Standard headers must precede Focuspocus headers, which define min/max macros.
#include <vector>
#include "doctest.h"
#include "BWin32Graphics.h"
#include "Focuspocus.h"

// Headless host: the same BWin32Graphics used by the demo window, driven by
// the same BMouse/BKeyboard abstractions a sketch uses, with a simulated clock.
// Screens are declared by each test like a sketch would (fixed child arrays,
// fixed screen stack); the host never allocates them.
struct TestHost {
  BWin32Graphics gfx;
  BTheme theme;
  BRootView* stack[4];
  BFocusManager fm;
  BMouse mouse;
  BKeyboard keyboard;
  unsigned long ms;

  explicit TestHost(BRootView& root, int16_t width = 240, int16_t height = 320)
    : gfx(width, height), stack{ &root, nullptr, nullptr, nullptr },
      fm(gfx, stack, theme), mouse(fm), keyboard(fm), ms(0) {
    run();
  }

  void run(int frames = 1) {
    for (int i = 0; i < frames; ++i) {
      ms += 20;
      fm.loop(ms);
    }
  }

  void advance(unsigned long elapsed) {
    ms += elapsed;
    fm.loop(ms);
  }

  BPoint screenPos(BView& view) {
    return fm.mapViewToScreen(view, 0, 0);
  }

  BPoint center(BView& view) {
    BPoint pt = screenPos(view);
    pt.x += view.actualWidth / 2;
    pt.y += view.actualHeight / 2;
    return pt;
  }

  void tap(int16_t x, int16_t y) {
    mouse.move(x, y, true);
    run();
    mouse.move(x, y, false);
    run();
  }

  void tap(BView& view) {
    BPoint pt = center(view);
    tap(pt.x, pt.y);
  }

  void press(uint16_t key) {
    keyboard.sendKey(key, true);
    run();
    keyboard.sendKey(key, false);
    run();
  }
};

// Minimal concrete view: fills its bounds and records mouse events it receives.
struct Box : public BView {
  uint16_t fill;
  std::vector<BInputEvent::EventType> mouseEvents;

  Box(int16_t w = 0, int16_t h = 0, uint16_t fill = 0x07E0) : fill(fill) {
    width = w;
    height = h;
    focusable = false;
  }

  virtual void handleEvent(BInputEvent& e) {
    if (e.type & BInputEvent::evMouse) mouseEvents.push_back(e.type);
    BView::handleEvent(e);
  }

  virtual void draw(BGraphics& g) {
    g.fillRect(0, 0, g.width, g.height, fill);
  }
};

#endif
