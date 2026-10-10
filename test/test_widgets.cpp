#include "TestHost.h"

static int clicks;
static int changes;
static int lastValue;

static void countClick(BButton*, bool) { ++clicks; }
static void countChange(BCheckBoxBase*, bool) { ++changes; }
static void scrollChange(BScrollbar*, int16_t v) { ++changes; lastValue = v; }

static void column(BStackPanel& p) {
  p.width = -1;
  p.height = -1;
  p.orientation = B::vertical;
}

// --- BButton ---------------------------------------------------------------

TEST_CASE("button: tap clicks once; Enter clicks once") {
  BButton button;
  button.text = "OK";
  BView* items[] = { &button };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  clicks = 0;
  button.onClick += countClick;

  host.tap(button);
  CHECK(clicks == 1);
  CHECK(button.isFocused());

  host.press(BKeyboard::kbEnter);
  CHECK(clicks == 2);
}

TEST_CASE("button: press, drag off and release does not click") {
  BButton button;
  button.text = "OK";
  BView* items[] = { &button };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  clicks = 0;
  button.onClick += countClick;

  BPoint c = host.center(button);
  host.mouse.move(c.x, c.y, true);
  host.run();
  host.mouse.move(200, 300, true);
  host.run();
  host.mouse.move(200, 300, false);
  host.run();
  CHECK(clicks == 0);
}

TEST_CASE("button: text uses the theme font size when fontSize is not set") {
  BButton themed, explicitSize;
  themed.text = "MM";
  explicitSize.text = "MM";
  explicitSize.fontSize = 2;
  BView* items[] = { &themed, &explicitSize };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  host.theme.fontSize = 2;
  panel.dirtyLayout();
  host.run();

  BPoint a = host.screenPos(themed);
  BPoint b = host.screenPos(explicitSize);
  uint16_t ink = host.theme.fontColor;
  CHECK(themed.actualWidth == explicitSize.actualWidth);
  CHECK(host.gfx.countPixels(a.x, a.y, themed.actualWidth, themed.actualHeight, ink) ==
        host.gfx.countPixels(b.x, b.y, explicitSize.actualWidth, explicitSize.actualHeight, ink));
}

// --- BCheckBox / BRadioButton ------------------------------------------------

TEST_CASE("checkbox: tap on box or label toggles; Enter toggles") {
  BCheckBox cb;
  cb.text = "Check me";
  BView* items[] = { &cb };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  changes = 0;
  cb.onChange += countChange;

  BPoint p = host.screenPos(cb);
  host.tap(p.x + 3, p.y + cb.actualHeight / 2);   // the box
  CHECK(cb.state);
  host.tap(p.x + cb.actualWidth - 3, p.y + cb.actualHeight / 2);  // the label
  CHECK_FALSE(cb.state);
  host.press(BKeyboard::kbEnter);
  CHECK(cb.state);
  CHECK(changes == 3);
}

TEST_CASE("checkbox: label keeps its font color after losing focus") {
  BCheckBox cb;
  cb.text = "Check me";
  cb.fontColor = 0xF800;
  BButton other;
  other.text = "X";
  BView* items[] = { &cb, &other };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  BPoint p = host.screenPos(cb);
  uint32_t before = host.gfx.countPixels(p.x, p.y, cb.actualWidth, cb.actualHeight, 0xF800);
  CHECK(before > 0);
  host.fm.focusFirst();
  host.run();
  host.fm.focusNext();
  host.run();
  CHECK(host.gfx.countPixels(p.x, p.y, cb.actualWidth, cb.actualHeight, 0xF800) == before);
}

TEST_CASE("radio: selecting one clears the others in the same group only") {
  BRadioButton a, b, c;
  a.text = "A"; a.group = "g1";
  b.text = "B"; b.group = "g1";
  c.text = "C"; c.group = "g2";
  BView* items[] = { &a, &b, &c };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  host.tap(c);
  host.tap(a);
  CHECK(a.state);
  CHECK(c.state);
  host.tap(b);
  CHECK_FALSE(a.state);
  CHECK(b.state);
  CHECK(c.state);
  host.tap(b);            // tapping the selected radio keeps it selected
  CHECK(b.state);
}

TEST_CASE("radio: buttons without a group form one group") {
  BRadioButton a, b;
  a.text = "A";
  b.text = "B";
  BView* items[] = { &a, &b };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  host.tap(a);
  host.tap(b);
  CHECK_FALSE(a.state);
  CHECK(b.state);
}

// --- BTextLabel --------------------------------------------------------------

TEST_CASE("label: setText redraws and leaves no old text behind") {
  BTextLabel autoSized, fixedSized;
  autoSized.text = "WWWWWW";
  fixedSized.text = "WWWWWW";
  fixedSized.width = 100;
  fixedSized.height = 20;
  BView* items[] = { &autoSized, &fixedSized };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  uint16_t ink = host.theme.fontColor;
  CHECK(host.gfx.countPixels(0, 0, 240, 320, ink) > 0);

  autoSized.setText("");
  fixedSized.setText("");
  host.run();
  CHECK(host.gfx.countPixels(0, 0, 240, 320, ink) == 0);
  fixedSized.setText("I");
  host.run();
  CHECK(host.gfx.countPixels(0, 0, 240, 320, ink) > 0);
}

// --- BScrollbar --------------------------------------------------------------

TEST_CASE("scrollbar: arrow keys step and clamp; onChange reports the value") {
  BScrollbar sb;
  sb.width = -1;
  sb.minimum = 0;
  sb.maximum = 10;
  sb.step = 4;
  BView* items[] = { &sb };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  changes = 0;
  sb.onChange += scrollChange;

  host.fm.focusFirst();
  host.press(BKeyboard::kbLeft);
  CHECK(sb.value == 0);
  CHECK(changes == 0);
  host.press(BKeyboard::kbRight);
  host.press(BKeyboard::kbRight);
  host.press(BKeyboard::kbRight);
  CHECK(sb.value == 10);
  CHECK(lastValue == 10);
  CHECK(changes == 3);
  host.press(BKeyboard::kbUp);     // wrong axis for a horizontal bar
  CHECK(sb.value == 10);
}

TEST_CASE("scrollbar: dragging the thumb past either end clamps to minimum/maximum") {
  BScrollbar sb;
  sb.width = -1;
  sb.minimum = 0;
  sb.maximum = 1000;
  BView* items[] = { &sb };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  BPoint p = host.screenPos(sb);
  int16_t y = p.y + sb.actualHeight / 2;
  host.mouse.move(p.x + 5, y, true);    // thumb starts at the left end
  host.run();
  host.mouse.move(p.x + 500, y, true);
  host.run();
  CHECK(sb.value == 1000);
  host.mouse.move(p.x - 500, y, true);
  host.run();
  CHECK(sb.value == 0);
  host.mouse.move(p.x + sb.actualWidth / 2, y, true);
  host.run();
  host.mouse.move(p.x + sb.actualWidth / 2, y, false);
  host.run();
  CHECK(sb.value > 400);
  CHECK(sb.value < 600);
}

// --- BProgressBar ------------------------------------------------------------

TEST_CASE("progress bar: value only changes when the sketch sets it") {
  BProgressBar bar;
  bar.width = -1;
  bar.maximum = 100;
  bar.value = 50;
  BView* items[] = { &bar };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  for (int i = 0; i < 10; ++i) {
    host.advance(600);
  }
  CHECK(bar.value == 50);
}

TEST_CASE("progress bar: setValue clamps and redraws only the change") {
  BProgressBar bar;
  bar.width = -1;
  bar.maximum = 100;
  bar.color = 0x001F;
  BView* items[] = { &bar };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  int16_t y = host.center(bar).y;

  bar.setValue(150);
  CHECK(bar.value == 100);
  CHECK(host.gfx.pixel(237, y) == 0x001F);
  bar.setValue(50);
  CHECK(host.gfx.pixel(100, y) == 0x001F);
  CHECK(host.gfx.pixel(200, y) == host.theme.background);
  bar.setValue(-5);
  CHECK(bar.value == 0);
  CHECK(host.gfx.pixel(3, y) == host.theme.background);
}

TEST_CASE("margins: progress bar and scrollbar draw inside their own bounds") {
  BProgressBar bar;
  bar.width = 100;
  bar.maximum = 10;
  bar.value = 10;
  bar.color = 0x001F;
  bar.margin(10);
  BScrollbar sb;
  sb.width = 100;
  sb.maximum = 10;
  sb.margin(10);
  BView* items[] = { &bar, &sb };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  BPoint p = host.screenPos(bar);    // top-left inside the margin
  CHECK(p.x == 10);
  CHECK(host.gfx.countPixels(0, 0, 240, 320, 0x001F) ==
        host.gfx.countPixels(p.x, p.y, bar.actualWidth, bar.actualHeight, 0x001F));
  // A tap at the far right of the track steps the scrollbar up.
  BPoint s = host.screenPos(sb);
  host.tap(s.x + sb.actualWidth - 3, s.y + sb.actualHeight / 2);
  CHECK(sb.value == 1);
}

// --- Layout / hit testing / focus -------------------------------------------

TEST_CASE("layout: left/right padding does not shrink child height") {
  Box child(0, 0);
  child.width = -1;
  child.height = -1;
  BView* items[] = { &child };
  BStackPanel panel(items);
  column(panel);
  panel.padding.left = 30;
  panel.padding.right = 30;
  BRootView root(panel);
  TestHost host(root);

  CHECK(child.actualWidth == 240 - 60);
  CHECK(child.actualHeight == 320);
}

TEST_CASE("hit test: overlapping views - the one drawn on top gets the tap") {
  Box below(50, 50, 0x001F), above(50, 50, 0xF800);
  BView* items[] = { &below, &above };
  BPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  BRootView root(panel);
  TestHost host(root);

  CHECK(host.gfx.pixel(10, 10) == 0xF800);
  host.tap(10, 10);
  CHECK_FALSE(above.mouseEvents.empty());
  CHECK(below.mouseEvents.empty());
}

TEST_CASE("focus: removing the focused view then focusNext moves on") {
  BButton a, b;
  a.text = "A";
  b.text = "B";
  BView* items[] = { &a, &b };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  host.fm.focus(a);
  panel.remove(a);
  CHECK(host.fm.focusNext() == &b);
}

TEST_CASE("focus: removing the focused view then focusPrev moves on") {
  BButton a, b;
  a.text = "A";
  b.text = "B";
  BView* items[] = { &a, &b };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  host.fm.focus(b);
  panel.remove(b);
  CHECK(host.fm.focusPrev() == &a);
}

TEST_CASE("remove: a removed view is not drawn by later setValue calls") {
  BProgressBar bar;
  bar.width = -1;
  bar.maximum = 100;
  bar.color = 0x001F;
  BView* items[] = { &bar };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);

  panel.remove(bar);
  host.run();
  bar.setValue(100);
  CHECK(host.gfx.countPixels(0, 0, 240, 320, 0x001F) == 0);
}

TEST_CASE("focus: a removed button no longer receives Enter") {
  BButton a, b;
  a.text = "A";
  b.text = "B";
  BView* items[] = { &a, &b };
  BStackPanel panel(items);
  column(panel);
  BRootView root(panel);
  TestHost host(root);
  clicks = 0;
  a.onClick += countClick;

  host.fm.focus(a);
  panel.remove(a);
  host.press(BKeyboard::kbEnter);
  CHECK(clicks == 0);
}
