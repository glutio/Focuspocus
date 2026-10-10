#include "TestHost.h"

static BProgressBar* animated;
static void animate(BFocusManager*, unsigned long) {
  animated->setValue(animated->value >= animated->maximum ? 0 : animated->value + 1);
}

// A sketch animates a progress bar from onTimerTick; a popup covers it.
TEST_CASE("covered screen does not draw over the popup; it is repainted on pop") {
  BProgressBar bar;
  bar.width = -1;
  bar.height = 20;
  bar.minimum = 0;
  bar.maximum = 100;
  bar.value = 50;
  bar.color = 0x001F;
  BView* mainItems[] = { &bar };
  BStackPanel main(mainItems);
  main.width = -1;
  main.height = -1;
  BRootView mainScreen(main);

  Box box(100, 50, 0xF800);
  BView* popupItems[] = { &box };
  BPopup popup(popupItems);
  popup.width = -1;
  popup.height = -1;
  BRootView popupScreen(popup);

  TestHost host(mainScreen);
  CHECK(host.gfx.pixel(5, 10) == 0x001F);

  animated = &bar;
  host.fm.onTimerTick += animate;
  CHECK(host.fm.push(popupScreen));
  host.run();
  uint32_t drawn = host.gfx.version();
  int16_t before = bar.value;
  for (int i = 0; i < 6; ++i) {
    host.advance(600);
  }
  CHECK(bar.value != before);           // still animating...
  CHECK(host.gfx.version() == drawn);   // ...but not drawing

  host.fm.onTimerTick -= animate;
  host.fm.pop();
  host.run();
  // Progress fill matches the current value; the rest of the bar is background.
  int16_t fillEnd = 2 + (bar.value - bar.minimum) * (240 - 4) / 100;
  CHECK(host.gfx.pixel(3, 10) == (bar.value > 0 ? 0x001F : host.theme.background));
  CHECK(host.gfx.pixel(fillEnd + 1, 10) == host.theme.background);
}

// Blinks between two colors; counts ticks.
struct Blinker : public Box {
  BInterval blink;
  int ticks;
  Blinker() : Box(20, 20, 0x001F), blink(500), ticks(0) {}
  virtual void tick(unsigned long ms) {
    ++ticks;
    if (blink.elapsed(ms)) {
      fill = (fill == 0x001F) ? 0xF800 : 0x001F;
      dirty();
    }
  }
};

TEST_CASE("tick animates views on the visible screen only") {
  Blinker blinker;
  BView* mainItems[] = { &blinker };
  BStackPanel main(mainItems);
  main.width = -1;
  main.height = -1;
  BRootView mainScreen(main);
  BView* popupItems[] = { nullptr };
  BPopup popup(popupItems);
  popup.width = -1;
  popup.height = -1;
  BRootView popupScreen(popup);

  TestHost host(mainScreen);
  CHECK(blinker.ticks > 0);
  uint16_t first = host.gfx.pixel(5, 5);
  host.advance(500);
  CHECK(host.gfx.pixel(5, 5) != first);   // blinked and redrawn in the same loop
  host.advance(100);
  CHECK(host.gfx.pixel(5, 5) != first);   // not yet time to blink back

  host.fm.push(popupScreen);
  host.run();
  int ticks = blinker.ticks;
  host.advance(2000);
  CHECK(blinker.ticks == ticks);          // covered: no ticks

  host.fm.pop();
  host.run();
  CHECK(blinker.ticks > ticks);
}
