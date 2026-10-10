#include "TestHost.h"

// Horizontal stack filling the screen; children are laid out left to right
// with no spacing, so each child's x position reveals its order.
static void row(BStackPanel& p) {
  p.width = -1;
  p.height = -1;
  p.orientation = B::horizontal;
}

TEST_CASE("fixed panel: add fills free (nullptr) slots and fails when full") {
  Box a(10, 10), b(20, 10), c(30, 10);
  BView* slots[2] = { nullptr, nullptr };
  BStackPanel panel(slots);
  row(panel);
  BRootView root(panel);
  TestHost host(root);

  CHECK(panel.add(a));
  CHECK(panel.add(b));
  CHECK_FALSE(panel.add(c));
  panel.dirtyLayout();
  host.run();

  CHECK(host.screenPos(a).x == 0);
  CHECK(host.screenPos(b).x == 10);
}

TEST_CASE("fixed panel: add reuses the slot freed by remove") {
  Box a(10, 10), b(20, 10), c(30, 10);
  BView* slots[] = { &a, &b };
  BStackPanel panel(slots);
  row(panel);
  BRootView root(panel);
  TestHost host(root);

  CHECK_FALSE(panel.add(c));
  panel.remove(a);
  CHECK(panel.add(c));
  panel.dirtyLayout();
  host.run();

  CHECK(host.screenPos(c).x == 0);
  CHECK(host.screenPos(b).x == 30);
}

TEST_CASE("heap panel: grows and holds heap-allocated views") {
  BStackPanel panel;
  row(panel);
  Box* boxes[10];
  for (int i = 0; i < 10; ++i) {
    boxes[i] = new Box(10, 10);
    CHECK(panel.add(*boxes[i]));
  }
  BRootView root(panel);
  TestHost host(root);

  CHECK(host.screenPos(*boxes[9]).x == 90);
  host.tap(*boxes[9]);
  CHECK(boxes[9]->mouseEvents.size() > 0);
  CHECK(boxes[8]->mouseEvents.empty());

  for (int i = 0; i < 10; ++i) {
    delete boxes[i];
  }
}

TEST_CASE("heap panel: insert places a view before existing ones") {
  Box a(10, 10), b(20, 10), c(30, 10);
  BStackPanel panel(1);
  row(panel);
  CHECK(panel.add(a));
  CHECK(panel.add(b));
  CHECK(panel.insert(0, c));
  CHECK_FALSE(panel.insert(9, c));
  BRootView root(panel);
  TestHost host(root);

  CHECK(host.screenPos(c).x == 0);
  CHECK(host.screenPos(a).x == 30);
  CHECK(host.screenPos(b).x == 40);
}

TEST_CASE("push fails when the screen stack is full and works again after pop") {
  Box a(10, 10);
  BView* items[] = { &a };
  BStackPanel panel(items);
  BRootView root(panel), extra(panel);
  TestHost host(root);  // stack of 4, one used

  CHECK(host.fm.push(extra));
  CHECK(host.fm.push(extra));
  CHECK(host.fm.push(extra));
  CHECK_FALSE(host.fm.push(extra));
  host.fm.pop();
  CHECK(host.fm.push(extra));
}

namespace {
struct Counter {
  int calls = 0;
  void handle(BView*, BMouseInputEvent&) { ++calls; }
};
}

TEST_CASE("events: one handler per widget event, -= frees the slot, duplicates are ignored") {
  Box a(50, 50);
  BView* items[] = { &a };
  BStackPanel panel(items);
  BRootView root(panel);
  TestHost host(root);
  Counter first, second;

  CHECK((a.onMouse += BView::MouseEvent(&first, &Counter::handle)));
  CHECK((a.onMouse += BView::MouseEvent(&first, &Counter::handle)));
  CHECK_FALSE((a.onMouse += BView::MouseEvent(&second, &Counter::handle)));
  host.tap(a);
  CHECK(first.calls > 0);
  CHECK(second.calls == 0);

  a.onMouse -= BView::MouseEvent(&first, &Counter::handle);
  CHECK((a.onMouse += BView::MouseEvent(&second, &Counter::handle)));
  int before = first.calls;
  host.tap(a);
  CHECK(first.calls == before);
  CHECK(second.calls > 0);
}
