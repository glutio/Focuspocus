#include "TestHost.h"

// Layout tests on a 240x320 screen. Expected numbers are worked out by hand;
// positions are checked on screen, so margins and padding are included.

static void fill(BStackPanel& p, B::Orientation o) {
  p.width = -1;
  p.height = -1;
  p.orientation = o;
}

static bool at(TestHost& host, BView& v, int16_t x, int16_t y, int16_t w, int16_t h) {
  BPoint p = host.screenPos(v);
  INFO("at ", p.x, ",", p.y, " size ", v.actualWidth, "x", v.actualHeight);
  CHECK(p.x == x);
  CHECK(p.y == y);
  CHECK(v.actualWidth == w);
  CHECK(v.actualHeight == h);
  return p.x == x && p.y == y && v.actualWidth == w && v.actualHeight == h;
}

TEST_CASE("layout: horizontal stack places children left to right with spacing") {
  Box a(20, 10), b(30, 10), c(20, 10);
  BView* items[] = { &a, &b, &c };
  BStackPanel row(items);
  fill(row, B::horizontal);
  row.spacing = 5;
  BScreen root(row);
  TestHost host(root);

  at(host, a, 0, 0, 20, 10);
  at(host, b, 25, 0, 30, 10);
  at(host, c, 60, 0, 20, 10);
}

TEST_CASE("layout: vertical stack aligns right/bottom and center") {
  Box a(20, 10), b(40, 10);
  BView* items[] = { &a, &b };
  BStackPanel column(items);
  fill(column, B::vertical);
  column.spacing = 4;
  column.horizontalAlignment = B::right;
  column.verticalAlignment = B::bottom;
  BScreen root(column);
  TestHost host(root);

  at(host, a, 220, 296, 20, 10);
  at(host, b, 200, 310, 40, 10);

  column.horizontalAlignment = B::center;
  column.verticalAlignment = B::center;
  column.dirtyLayout();
  host.run();
  at(host, a, 110, 148, 20, 10);
  at(host, b, 100, 162, 40, 10);

  column.horizontalAlignment = B::left;
  column.verticalAlignment = B::top;
  column.dirtyLayout();
  host.run();
  at(host, a, 0, 0, 20, 10);
  at(host, b, 0, 14, 40, 10);
}

TEST_CASE("layout: horizontal stack aligns bottom, then back to top") {
  Box a(20, 10), b(20, 30);
  BView* items[] = { &a, &b };
  BStackPanel row(items);
  fill(row, B::horizontal);
  row.verticalAlignment = B::bottom;
  BScreen root(row);
  TestHost host(root);

  at(host, a, 0, 310, 20, 10);
  at(host, b, 20, 290, 20, 30);

  row.verticalAlignment = B::top;
  row.dirtyLayout();
  host.run();
  at(host, a, 0, 0, 20, 10);
  at(host, b, 20, 0, 20, 30);
}

TEST_CASE("layout: negative sizes share the space left by fixed children") {
  Box fixed(30, 10), one(0, 10), two(0, 10);
  one.width = -1;
  two.width = -2;
  BView* items[] = { &fixed, &one, &two };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BScreen root(row);
  TestHost host(root);

  // 240 - 30 = 210 left, split 1:2
  at(host, fixed, 0, 0, 30, 10);
  at(host, one, 30, 0, 70, 10);
  at(host, two, 100, 0, 140, 10);
}

// The same stack in main/cross terms, so horizontal and vertical can be compared.
struct AxisScene {
  Box a, b, c, d;
  BView* items[4];
  BStackPanel stack;
  BScreen root;
  TestHost host;

  static void size(BView& v, B::Orientation o, int16_t main, int16_t cross) {
    (o == B::horizontal ? v.width : v.height) = main;
    (o == B::horizontal ? v.height : v.width) = cross;
  }

  AxisScene(B::Orientation o, B::Alignment mainAlign)
    : items{ &a, &b, &c, &d }, stack(items), root(stack), host(root, 200, 200) {
    bool h = o == B::horizontal;
    fill(stack, o);
    stack.spacing = 4;
    (h ? stack.horizontalAlignment : stack.verticalAlignment) = mainAlign;
    (h ? stack.verticalAlignment : stack.horizontalAlignment) = h ? B::bottom : B::right;

    size(a, o, 30, 12);  // fixed, with margins on all sides
    (h ? a.margin.left : a.margin.top) = 3;
    (h ? a.margin.right : a.margin.bottom) = 1;
    (h ? a.margin.top : a.margin.left) = 2;
    size(b, o, -1, 15);  // stretched, capped
    (h ? b.maxWidth : b.maxHeight) = 40;
    size(c, o, -2, -1);  // stretched twice as much, fills across
    (h ? c.maxHeight : c.maxWidth) = 50;
    size(d, o, 8, 6);    // fixed
    stack.dirtyLayout();
    host.run();
  }

  // main position, cross position, main size, cross size
  void check(BView& v, int16_t mp, int16_t cp, int16_t ms, int16_t cs) {
    bool h = stack.orientation == B::horizontal;
    at(host, v, h ? mp : cp, h ? cp : mp, h ? ms : cs, h ? cs : ms);
  }
};

TEST_CASE("layout: horizontal and vertical stacks lay out the same along their axis") {
  for (B::Orientation o : { B::horizontal, B::vertical }) {
    CAPTURE(o);
    {
      // main: 200 - 3 spacings - 34 (a) - 8 (d) = 146; b capped at 40, c gets 106
      AxisScene s(o, B::left);
      s.check(s.a, 3, 188, 30, 12);
      s.check(s.b, 38, 185, 40, 15);
      s.check(s.c, 82, 150, 106, 50);
      s.check(s.d, 192, 194, 8, 6);
    }
    {
      // b and c both capped leave room, so the group centers
      AxisScene s(o, B::center);
      (o == B::horizontal ? s.c.maxWidth : s.c.maxHeight) = 60;
      s.stack.dirtyLayout();
      s.host.run();
      // 34 + 40 + 60 + 8 + 12 spacing = 154; (200 - 154) / 2 = 23
      s.check(s.a, 26, 188, 30, 12);
      s.check(s.b, 61, 185, 40, 15);
      s.check(s.c, 105, 150, 60, 50);
      s.check(s.d, 169, 194, 8, 6);
    }
  }
}

TEST_CASE("layout: maxWidth caps a stretched child; the rest goes to the others") {
  Box capped(0, 10), rest(0, 10);
  capped.width = -1;
  capped.maxWidth = 40;
  rest.width = -1;
  BView* items[] = { &capped, &rest };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BScreen root(row);
  TestHost host(root);

  at(host, capped, 0, 0, 40, 10);
  at(host, rest, 40, 0, 200, 10);
}

TEST_CASE("layout: minHeight holds a stretched child; the rest goes to the others") {
  Box held(10, 0), rest(10, 0);
  held.height = -1;
  held.minHeight = 200;
  rest.height = -3;  // would get 3/4 of 320 without the minimum
  BView* items[] = { &held, &rest };
  BStackPanel column(items);
  fill(column, B::vertical);
  BScreen root(column);
  TestHost host(root);

  at(host, held, 0, 0, 10, 200);
  at(host, rest, 0, 200, 10, 120);
}

TEST_CASE("layout: padding and margins offset children; width -1 fills between them") {
  Box a(20, 10), b(0, 10);
  a.margin.left = 5;
  a.margin.top = 3;
  a.margin.bottom = 2;
  b.width = -1;
  b.margin.left = 4;
  b.margin.right = 6;
  BView* items[] = { &a, &b };
  BStackPanel column(items);
  fill(column, B::vertical);
  column.padding(10);
  BScreen root(column);
  TestHost host(root);

  at(host, a, 15, 13, 20, 10);
  at(host, b, 14, 25, 240 - 20 - 10, 10);  // 10 + 3 + 10 + 2 = 25
}

TEST_CASE("layout: a stack with size 0 fits its children") {
  Box a(20, 10), b(30, 16);
  BView* rowItems[] = { &a, &b };
  BStackPanel row(rowItems);  // width/height 0: size to content
  row.spacing = 5;
  row.padding(2);
  BView* items[] = { &row };
  BStackPanel column(items);
  fill(column, B::vertical);
  column.horizontalAlignment = B::center;
  column.verticalAlignment = B::center;
  BScreen root(column);
  TestHost host(root);

  CHECK(row.actualWidth == 2 + 20 + 5 + 30 + 2);
  CHECK(row.actualHeight == 2 + 16 + 2);
  at(host, row, (240 - 59) / 2, (320 - 20) / 2, 59, 20);
  at(host, a, (240 - 59) / 2 + 2, (320 - 20) / 2 + 2, 20, 10);
}

TEST_CASE("layout: a child growing re-sizes and re-centers its auto-sized parent") {
  Box a(20, 10), b(20, 10);
  BView* rowItems[] = { &a, &b };
  BStackPanel row(rowItems);
  BView* items[] = { &row };
  BStackPanel column(items);
  fill(column, B::vertical);
  column.horizontalAlignment = B::center;
  BScreen root(column);
  TestHost host(root);
  at(host, b, 120, 0, 20, 10);

  a.width = 60;
  a.height = 30;
  a.dirtyLayout();
  host.run();
  CHECK(row.actualWidth == 80);
  CHECK(row.actualHeight == 30);
  at(host, a, 80, 0, 60, 30);
  at(host, b, 140, 0, 20, 10);
}

TEST_CASE("layout: BPanel keeps children at their own x/y") {
  Box a(20, 10), b(30, 30);
  a.x = 50;
  a.y = 60;
  b.x = 100;
  b.y = 5;
  BView* items[] = { &a, &b };
  BPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  panel.padding(4);
  BScreen root(panel);
  TestHost host(root);

  at(host, a, 54, 64, 20, 10);
  at(host, b, 104, 9, 30, 30);
}

TEST_CASE("layout: a hidden-size child (0x0) takes no spacing") {
  Box a(20, 10), gone(0, 0), c(20, 10);
  BView* items[] = { &a, &gone, &c };
  BStackPanel row(items);
  fill(row, B::horizontal);
  row.spacing = 10;
  BScreen root(row);
  TestHost host(root);

  at(host, a, 0, 0, 20, 10);
  at(host, c, 30, 0, 20, 10);
}

TEST_CASE("layout: width -1 fills a centered column") {
  BProgressBar bar;
  bar.width = -1;
  bar.height = 20;
  Box small(40, 10);
  BView* items[] = { &small, &bar };
  BStackPanel panel(items);
  fill(panel, B::vertical);
  panel.horizontalAlignment = B::center;
  panel.padding(10);
  BScreen root(panel);
  TestHost host(root);
  host.run();
  CHECK(bar.actualWidth == panel.actualWidth - 20);
}

TEST_CASE("layout: left/right padding does not shrink child height") {
  Box child(0, 0);
  child.width = -1;
  child.height = -1;
  BView* items[] = { &child };
  BStackPanel panel(items);
  fill(panel, B::vertical);
  panel.padding.left = 30;
  panel.padding.right = 30;
  BScreen root(panel);
  TestHost host(root);

  CHECK(child.actualWidth == 240 - 60);
  CHECK(child.actualHeight == 320);
}

TEST_CASE("layout: a label changing text in place does not redraw its neighbours") {
  struct CountingBox : Box {
    int draws = 0;
    CountingBox() : Box(50, 20) {}
    void draw(BGraphics& g) override { ++draws; Box::draw(g); }
  };
  BTextLabel label;
  label.text = "600";
  CountingBox box;
  BView* items[] = { &label, &box };
  BStackPanel panel(items);
  fill(panel, B::vertical);
  panel.horizontalAlignment = B::center;
  BScreen root(panel);
  TestHost host(root);
  host.run();
  int before = box.draws;

  label.setText("601");  // same size: only the label redraws
  host.run(2);
  CHECK(box.draws == before);

  label.setText("60000");  // wider: the label moves, the panel redraws
  host.run(2);
  CHECK(box.draws == before + 1);
}

// --- too little room: fixed sizes and minimums win, stretched children get 0 ---

TEST_CASE("layout: no room left - a stretched child gets 0, fixed children keep their size") {
  Box a(200, 10), b(100, 10), stretched(0, 10);
  stretched.width = -1;
  BView* items[] = { &a, &stretched, &b };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BScreen root(row);
  TestHost host(root);

  CHECK(stretched.actualWidth == 0);
  at(host, b, 200, 0, 100, 10);  // overflows the 240 px screen
}

TEST_CASE("layout: a share smaller than the child's margins gives 0, not a huge size") {
  Box fixed(220, 10), one(0, 10), two(0, 10);
  one.width = -1;
  one.margin.left = 15;
  two.width = -1;
  BView* items[] = { &fixed, &one, &two };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BScreen root(row);
  TestHost host(root);

  CHECK(one.actualWidth == 0);  // share 10, margin 15
  CHECK(two.actualWidth == 10);
}

TEST_CASE("layout: minimums that do not fit are kept and overflow") {
  Box a(0, 10), b(0, 10);
  a.width = -1;
  a.minWidth = 150;
  b.width = -1;
  b.minWidth = 150;
  BView* items[] = { &a, &b };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BScreen root(row);
  TestHost host(root);

  at(host, a, 0, 0, 150, 10);
  at(host, b, 150, 0, 150, 10);
}

TEST_CASE("layout: too tall for the screen - a vertical stack overflows at the bottom") {
  Box a(10, 200), stretched(10, 0), b(10, 200);
  stretched.height = -1;
  BView* items[] = { &a, &stretched, &b };
  BStackPanel column(items);
  fill(column, B::vertical);
  BScreen root(column);
  TestHost host(root);

  CHECK(stretched.actualHeight == 0);
  at(host, b, 0, 200, 10, 200);
}

// --- scrolling: move a tall stack inside a fixed-size BPanel ---

TEST_CASE("scroll: views are clipped to the BPanel; taps stay outside") {
  const uint16_t RED = 0xF800, GREEN = 0x07E0, BLUE = 0x001F;
  Box header(240, 50, RED), footer(240, 50, BLUE);
  Box i1(100, 40, GREEN), i2(100, 40, GREEN), i3(100, 40, GREEN), i4(100, 40, GREEN);
  BView* listItems[] = { &i1, &i2, &i3, &i4 };
  BStackPanel list(listItems);  // 160 px tall, sized by its content
  list.orientation = B::vertical;
  BView* viewportItems[] = { &list };
  BPanel viewport(viewportItems);
  viewport.width = -1;
  viewport.height = 100;
  BView* items[] = { &header, &viewport, &footer };
  BStackPanel column(items);
  fill(column, B::vertical);
  BScreen root(column);
  TestHost host(root);

  list.y = -50;  // scroll down by 50 px
  viewport.dirty();
  host.run();

  CHECK(host.gfx.countPixels(0, 0, 240, 50, RED) == 240 * 50);
  CHECK(host.gfx.countPixels(0, 150, 240, 50, BLUE) == 240 * 50);
  // i2 and i4 are partly outside the viewport: only their inside part is drawn.
  CHECK(host.gfx.countPixels(0, 80, 100, 40, GREEN) == 100 * 40);
  CHECK(host.gfx.countPixels(0, 50, 100, 30, GREEN) == 100 * 30);
  CHECK(host.gfx.countPixels(0, 120, 100, 30, GREEN) == 100 * 30);
  CHECK(host.gfx.countPixels(0, 0, 240, 320, GREEN) == 100 * 100);  // nothing leaks out
  at(host, i2, 0, 40 - 50 + 50, 100, 40);  // list starts 50 px above the viewport

  host.tap(50, 160);  // i4 is scrolled under the footer here
  CHECK(i4.mouseEvents.empty());
  CHECK_FALSE(footer.mouseEvents.empty());
}

TEST_CASE("scroll: text cut by the BPanel edge draws its visible part") {
  const uint16_t RED = 0xF800, CYAN = 0x07FF;
  Box header(240, 50, RED);
  BTextLabel label;
  label.text = "HHHH";
  label.fontSize = 2;  // 16 px tall
  label.fontColor = CYAN;
  label.y = -8;        // top half above the viewport
  BView* viewportItems[] = { &label };
  BPanel viewport(viewportItems);
  viewport.width = -1;
  viewport.height = 100;
  BView* items[] = { &header, &viewport };
  BStackPanel column(items);
  fill(column, B::vertical);
  BScreen root(column);
  TestHost host(root);

  CHECK(host.gfx.countPixels(0, 50, 240, 8, CYAN) > 0);
  CHECK(host.gfx.countPixels(0, 0, 240, 50, CYAN) == 0);
}

TEST_CASE("layout: margins are 8-bit but their sums are not") {
  Box a(50, 10);
  a.margin.left = 100;
  a.margin.right = 100;  // 200 in total, past int8_t
  a.margin.top = 120;
  BView* innerItems[] = { &a };
  BStackPanel inner(innerItems);  // sized by its content
  inner.padding(127);
  BView* items[] = { &inner };
  BStackPanel panel(items);
  panel.width = -1;
  panel.height = -1;
  panel.orientation = B::vertical;
  BScreen root(panel);
  TestHost host(root);

  CHECK(inner.actualWidth == 50 + 200 + 254);
  CHECK(inner.actualHeight == 10 + 120 + 254);
  CHECK(host.screenPos(a).x == host.screenPos(inner).x + 127 + 100);
}

TEST_CASE("layout: auto-sized vertical list of stretched buttons fits them all") {
  BButton a, b, c;
  a.text = "A";
  b.text = "A longer item";
  c.text = "C";
  BView* items[] = { &a, &b, &c };
  BStackPanel list(items);
  for (BView* v : items) v->width = -1;
  list.orientation = B::vertical;
  list.spacing = 4;
  list.padding(6);
  BView* outer[] = { &list };
  BStackPanel page(outer);
  fill(page, B::vertical);
  BScreen root(page);
  TestHost host(root);

  int16_t h = a.actualHeight;
  CHECK(list.actualHeight == 3 * h + 2 * 4 + 12);
  BRect text = host.fm.getTextBounds(b.text, 1);
  CHECK(list.actualWidth >= text.width + 12);
  for (BView* v : items) CHECK(v->actualWidth == list.actualWidth - 12);
  BPoint pl = host.screenPos(list), pc = host.screenPos(c);
  CHECK(pc.y + c.actualHeight == pl.y + list.actualHeight - 6);
}
