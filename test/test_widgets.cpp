#include "TestHost.h"
#include <string>

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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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

  // Shorter text over longer text: the old text right of the new one is cleared.
  BPoint p = host.screenPos(fixedSized);
  fixedSized.setText("WWWWWW");
  host.run();
  CHECK(host.gfx.countPixels(p.x + 20, p.y, 80, 20, ink) > 0);
  fixedSized.setText("I");
  host.run();
  CHECK(host.gfx.countPixels(p.x + 20, p.y, 80, 20, ink) == 0);
}

TEST_CASE("background: unset falls back to the nearest panel's, then the theme's") {
  BTextLabel inherited, own;
  inherited.text = own.text = "I";
  inherited.width = own.width = 100;
  inherited.height = own.height = 20;
  own.background = 0x001F;
  BView* innerItems[] = { &inherited, &own };
  BStackPanel inner(innerItems);  // no background: passes the outer one through
  column(inner);
  BView* items[] = { &inner };
  BStackPanel panel(items);
  column(panel);
  panel.background = 0xF800;
  BScreen root(panel);
  TestHost host(root);
  BPoint a = host.screenPos(inherited), b = host.screenPos(own);
  CHECK(host.gfx.countPixels(a.x + 20, a.y, 80, 20, 0xF800) == 80 * 20);
  CHECK(host.gfx.countPixels(b.x + 20, b.y, 80, 20, 0x001F) == 80 * 20);

  panel.background.clear();
  panel.dirty();
  inherited.dirty();
  host.run();
  CHECK(host.gfx.countPixels(a.x + 20, a.y, 80, 20, host.theme.background) == 80 * 20);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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
  BScreen root(panel);
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

TEST_CASE("hit test: overlapping views - the one drawn on top gets the tap") {
  Box below(50, 50, 0x001F), above(50, 50, 0xF800);
  BView* items[] = { &below, &above };
  BPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  BScreen root(panel);
  TestHost host(root);

  CHECK(host.gfx.pixel(10, 10) == 0xF800);
  host.tap(10, 10);
  CHECK_FALSE(above.mouseEvents.empty());
  CHECK(below.mouseEvents.empty());
}

TEST_CASE("draw: redrawing a view does not paint over the sibling on top") {
  Box below(50, 50, 0x001F), above(20, 20, 0xF800);
  above.x = 10;
  above.y = 10;
  BView* items[] = { &below, &above };
  BPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  BScreen root(panel);
  TestHost host(root);

  below.dirty();
  host.run();
  CHECK(host.gfx.pixel(15, 15) == 0xF800);
  CHECK(host.gfx.pixel(5, 5) == 0x001F);
}

TEST_CASE("draw: changing a child's margin clears where it was") {
  Box box(20, 20, 0xF800);
  BView* items[] = { &box };
  BPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  BScreen root(panel);
  TestHost host(root);
  CHECK(host.gfx.pixel(5, 5) == 0xF800);

  box.margin.left = 50;
  box.dirtyLayout();
  host.run();
  CHECK(host.gfx.pixel(55, 5) == 0xF800);
  CHECK(host.gfx.pixel(5, 5) == host.theme.background);
}

TEST_CASE("button: a press does not click after the screen changes") {
  BButton button;
  button.text = "OK";
  BView* items[] = { &button };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  Box box(240, 320);
  BView* popupItems[] = { &box };
  BPopup popup(popupItems);
  BScreen popupScreen(popup);
  TestHost host(root);
  clicks = 0;
  button.onClick += countClick;

  BPoint c = host.center(button);
  host.mouse.move(c.x, c.y, true);
  host.run();
  host.fm.push(popupScreen);
  host.run();
  host.fm.pop();
  host.run();
  host.mouse.move(c.x, c.y, false);
  host.run();
  CHECK(clicks == 0);
}

// A display whose size changes after the focus manager is built, like
// tft.setRotation() in setup() with a global BFocusManager.
struct RotatingGraphics : BWin32Graphics {
  RotatingGraphics() : BWin32Graphics(240, 320) {}
  void rotate() { int16_t w = _width; _width = _height; _height = w; }
};

TEST_CASE("layout: the root follows a display size change") {
  RotatingGraphics gfx;
  BTheme theme;
  Box box;
  BView* items[] = { &box };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  BScreen* stack[] = { &root };
  BFocusManager fm(gfx, stack, theme);

  gfx.rotate();
  fm.loop(20);
  CHECK(panel.actualWidth == 320);
  CHECK(panel.actualHeight == 240);

  gfx.rotate();
  fm.loop(40);
  CHECK(panel.actualWidth == 240);
  CHECK(panel.actualHeight == 320);
}

TEST_CASE("focus: removing the focused view then focusNext moves on") {
  BButton a, b;
  a.text = "A";
  b.text = "B";
  BView* items[] = { &a, &b };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
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
  BScreen root(panel);
  TestHost host(root);

  host.fm.focus(b);
  panel.remove(b);
  CHECK(host.fm.focusPrev() == &a);
}

static std::string focusLog;
static void logFocus(BControl* sender, BFocusInputEvent& event) {
  if (event.command == BCommandInputEvent::cmFocus) {
    focusLog += ((BButton*)sender)->text;
  }
}

TEST_CASE("focus: next and prev go into and out of nested panels, both ways") {
  // a [ b [ c d ] label e ] [ ] [ label ] checkbox f [ [ g ] ]
  BButton a, b, c, d, e, g;
  BCheckBox f;
  BTextLabel label1, label2;
  label1.text = label2.text = "-";
  BButton* buttons[] = { &a, &b, &c, &d, &e, &g };
  const char* names[] = { "a", "b", "c", "d", "e", "g" };
  for (int i = 0; i < 6; ++i) {
    buttons[i]->text = names[i];
  }
  f.text = "f";

  BView* innerItems[] = { &c, &d };
  BStackPanel inner(innerItems);
  BView* outerItems[] = { &b, &inner, &label1, &e };
  BStackPanel outer(outerItems);
  BView* emptyItems[] = { nullptr };
  BStackPanel empty(emptyItems);
  BView* labelsItems[] = { &label2 };
  BStackPanel labels(labelsItems);
  BView* deepItems[] = { &g };
  BStackPanel deep(deepItems);
  BView* deeperItems[] = { &deep };
  BStackPanel deeper(deeperItems);
  BView* items[] = { &a, &outer, &empty, &labels, &f, &deeper };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  TestHost host(root);
  host.fm.onFocus += logFocus;

  focusLog = "";
  host.fm.focus(a);
  for (int i = 0; i < 7; ++i) {
    host.fm.focusNext();
  }
  CHECK(focusLog == "abcdefga");  // wraps around at the end

  focusLog = "";
  for (int i = 0; i < 7; ++i) {
    host.fm.focusPrev();
  }
  CHECK(focusLog == "gfedcba");  // and at the start
}

TEST_CASE("remove: a removed view is not drawn by later setValue calls") {
  BProgressBar bar;
  bar.width = -1;
  bar.maximum = 100;
  bar.color = 0x001F;
  BView* items[] = { &bar };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
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
  BScreen root(panel);
  TestHost host(root);
  clicks = 0;
  a.onClick += countClick;

  host.fm.focus(a);
  panel.remove(a);
  host.press(BKeyboard::kbEnter);
  CHECK(clicks == 0);
}

TEST_CASE("progress bar: stays visible after setValue and a blinking neighbour") {
  BTextLabel reading;
  reading.text = "-";
  reading.fontSize = 4;
  BProgressBar bar;
  bar.maximum = 1023;
  bar.width = -1;
  bar.height = 20;
  BTextLabel alarm;
  alarm.text = "ALARM";
  alarm.fontSize = 3;
  BView* items[] = { &reading, &bar, &alarm };
  BStackPanel panel(items);
  column(panel);
  panel.horizontalAlignment = B::center;
  panel.spacing = 15;
  panel.padding(10);
  BScreen root(panel);
  TestHost host(root);
  host.run();
  bar.setValue(600);
  alarm.fontColor = 0;
  alarm.dirty();
  host.run(3);
  BPoint p = host.screenPos(bar);
  CHECK(host.gfx.pixel(p.x + 5, p.y + 10) == (uint16_t)host.theme.color);
  CHECK(host.gfx.pixel(p.x + bar.actualWidth - 1, p.y + 10) == (uint16_t)host.theme.color);
}

// --- fixes from the code review ---------------------------------------------

TEST_CASE("hit test: neighbouring buttons do not share their edge pixel") {
  BButton a, b;
  a.text = "A";
  b.text = "B";
  static int aClicks, bClicks;
  aClicks = bClicks = 0;
  a.onClick += [](BButton*, bool) { ++aClicks; };
  b.onClick += [](BButton*, bool) { ++bClicks; };
  BView* items[] = { &a, &b };
  BStackPanel panel(items);  // horizontal, no spacing: b starts right after a
  BScreen root(panel);
  TestHost host(root);

  BPoint pa = host.screenPos(a);
  host.tap(pa.x + a.actualWidth - 1, pa.y + 5);  // a's last column
  host.tap(pa.x + a.actualWidth, pa.y + 5);      // b's first column
  CHECK(aClicks == 1);
  CHECK(bClicks == 1);
}

TEST_CASE("layout: an empty auto-sized stack panel is just its padding") {
  BStackPanel empty;
  empty.width = 0;
  empty.height = 0;
  empty.spacing = 4;
  empty.padding(3);
  BView* items[] = { &empty };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  TestHost host(root);

  CHECK(empty.actualWidth == 6);
  CHECK(empty.actualHeight == 6);
}

TEST_CASE("focus: a view that is not focusable cannot take the focus") {
  BButton button;
  button.text = "OK";
  BTextLabel label;
  label.text = "text";
  BView* items[] = { &button, &label };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  TestHost host(root);

  CHECK(host.fm.focus(button));
  CHECK_FALSE(host.fm.focus(label));
  CHECK(host.fm.focusedView() == &button);
}

TEST_CASE("checkbox: press, drag off and release does not toggle") {
  BCheckBox cb;
  cb.text = "Check me";
  BView* items[] = { &cb };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  TestHost host(root);

  BPoint p = host.screenPos(cb);
  host.mouse.move(p.x + cb.actualWidth - 3, p.y + 2, true);  // on the text
  host.run();
  host.mouse.move(p.x + cb.actualWidth + 20, p.y + 2, true);
  host.run();
  host.mouse.move(p.x + cb.actualWidth + 20, p.y + 2, false);
  host.run();
  CHECK_FALSE(cb.state);
}

// --- BGauge -------------------------------------------------------------------


TEST_CASE("gauge: integer sine is close to the real one") {
  CHECK(BGauge::sin1024(0) == 0);
  CHECK(BGauge::sin1024(90) == 1024);
  CHECK(BGauge::sin1024(-90) == -1024);
  CHECK(abs(BGauge::sin1024(30) - 512) <= 2);
}

TEST_CASE("gauge: setValue clamps and moves the needle") {
  BGauge gauge;
  gauge.needleColor = 0x001F;
  BView* items[] = { &gauge };
  BStackPanel panel(items);
  column(panel);
  BScreen root(panel);
  TestHost host(root);
  BPoint pos = host.screenPos(gauge);
  int16_t half = gauge.actualWidth / 2 - 5;  // left/right of the hub
  int16_t top = gauge.actualHeight - 10;
  auto left = [&] { return host.gfx.countPixels(pos.x, pos.y, half, top, 0x001F); };
  auto right = [&] { return host.gfx.countPixels(pos.x + gauge.actualWidth - half, pos.y, half, top, 0x001F); };

  CHECK(gauge.actualWidth == 120);
  CHECK(left() == 0);  // at minimum the needle lies flat, below `top`
  gauge.setValue(25);
  CHECK(left() > 10);
  CHECK(right() == 0);
  gauge.setValue(500);
  CHECK(gauge.value == 100);
  gauge.setValue(75);
  CHECK(left() == 0);  // old needle erased
  CHECK(right() > 10);
}
