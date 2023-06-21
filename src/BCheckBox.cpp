#include <Arduino.h>
#include "BCheckBox.h"

// void BCheckBoxBase::BBox::draw(BGraphics& g) {
//   auto c = color;
//   auto b = background;
//   if (isFocused) {
//     c = focusManager().theme().focusColor;
//     b = focusManager().theme().focusBackground;
//   }

//   g.drawRect(0, 0, g.width, g.height, c)
//   g.drawRect(1, 1, g.width - 2, g.height - 2, b);
//   if (!checkbox.state) c = b;
//   g.fillRect(2, 2, g.width - 4, g.height - 4, c);
// }

// void BCheckBoxBase::BBox::measure(int16_t availableWidth, int16_t availableHeight) {
//   actualWidth = actualHeight = availableHeight;
// }

// BCheckBoxBase::BCheckBoxBase() : _box(*this), alignment(left), state(false) {
//   _box.onClick += BButton::ClickEvent(this, &BCheckBoxBase<T>::handleClick);
// };

// BCheckBoxBase::~BCheckBoxBase() {
//   _box.onClick -= BButton::ClickEvent(this, &BCheckBoxBase<T>::handleClick);
// };

// void BCheckBoxBase::layout() {
//   _label.verticalAlignment = BTextLabel::center;
//   if (alignment == left) {
//     horizontalAlignment = BPanel::left;
//     _content[0] = _box;
//     _content[1] = _label;
//     _label.horizontalAlignment = BTextLabel::left;
//   } else {
//     _content[0] = _label;
//     _content[1] = _box;
//     _label.horizontalAlignment = BTextLabel::right;
//     horizontalAlignment = BPanel::right;
//   }
// }

// void BCheckBoxBase::handleClick(BButton* sender, bool isClick) {
//   if (isClick) {
//     state != state;
//     dirty();
//     onChange((T)this, state);
//   }
// }