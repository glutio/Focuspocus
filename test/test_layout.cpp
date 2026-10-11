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
  BRootView root(row);
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
  BRootView root(column);
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
  BRootView root(row);
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
  BRootView root(row);
  TestHost host(root);

  // 240 - 30 = 210 left, split 1:2
  at(host, fixed, 0, 0, 30, 10);
  at(host, one, 30, 0, 70, 10);
  at(host, two, 100, 0, 140, 10);
}

TEST_CASE("layout: maxWidth caps a stretched child; the rest goes to the others") {
  Box capped(0, 10), rest(0, 10);
  capped.width = -1;
  capped.maxWidth = 40;
  rest.width = -1;
  BView* items[] = { &capped, &rest };
  BStackPanel row(items);
  fill(row, B::horizontal);
  BRootView root(row);
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
  BRootView root(column);
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
  BRootView root(column);
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
  BRootView root(column);
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
  BRootView root(column);
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
  BRootView root(panel);
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
  BRootView root(row);
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
  BRootView root(panel);
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
  BRootView root(panel);
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
  BRootView root(panel);
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
