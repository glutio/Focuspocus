#include <string.h>
#include "BCheckBox.h"
#include "BMinMax.h"

BCheckBoxBase::BCheckBoxBase() : alignment(B::left), state(false) {
}

int16_t BCheckBoxBase::boxPixels() {
  if (!boxSize.hasValue()) {
    return focusManager().theme().checkboxSize;
  }
  return boxSize ? (int16_t)boxSize : textBounds().height;
}

BRect BCheckBoxBase::textBounds() {
  return text ? focusManager().getTextBounds(text, actualFontSize()) : BRect();
}

void BCheckBoxBase::clicked() {
  state = !state;
  dirty();
  raiseOnChange(state);
}

void BCheckBoxBase::measure(uint16_t, uint16_t) {
  BRect rt = textBounds();
  int16_t box = boxPixels();
  if (width <= 0) {
    actualWidth = box + spacing + rt.width;
  }
  if (height <= 0) {
    actualHeight = max(box, rt.height);
  }
}

void BCheckBoxBase::draw(BGraphics& g) {
  BView::draw(g);
  BRect rt = textBounds();
  int16_t box = boxPixels();
  int16_t x = alignment == B::left ? 0 : g.width - (box + spacing + rt.width);
  int16_t textX = alignment == B::left ? x + box + spacing : x;
  int16_t boxX = alignment == B::left ? x : x + rt.width + spacing;

  g.fillRect(0, 0, g.width, g.height, actualBackground());

  BGraphics gb(g);
  gb.x += boxX;
  gb.y += (g.height - box) / 2;
  gb.width = box;
  gb.height = box;
  drawBox(gb);

  // Focus highlights the text like a focused button.
  bool f = isFocused();
  int16_t textY = (g.height - rt.height) / 2;
  g.fillRect(textX, textY, rt.width, rt.height, f ? focusManager().theme().focusBackground : actualBackground());
  if (text) {
    g.drawText(text, textX, textY, actualFontSize(), f ? focusManager().theme().focusColor : actualFontColor());
  }
}

void BCheckBoxBase::drawBox(BGraphics& g) {
  uint16_t c = actualColor();
  uint16_t b = actualBackground();
  g.drawRect(0, 0, g.width, g.height, c);
  g.drawRect(1, 1, g.width - 2, g.height - 2, b);
  if (_isDown) {
    g.drawRect(2, 2, g.width - 4, g.height - 4, b);
  }
  int16_t pad = _isDown ? 3 : 2;
  g.fillRect(pad, pad, g.width - pad * 2, g.height - pad * 2, (state || _isDown) ? c : b);
}

void BRadioButton::drawBox(BGraphics& g) {
  uint16_t c = actualColor();
  uint16_t b = actualBackground();
  int16_t org = g.height / 2;
  g.drawCircle(org, org, g.height / 2, c);
  g.drawCircle(org, org, (g.height - 2) / 2, b);
  if (_isDown) {
    g.fillCircle(org, org, (g.height - 4) / 2, b);
  }
  int16_t pad = _isDown ? 3 : 2;
  g.fillCircle(org, org, (g.height - pad * 2) / 2, (state || _isDown) ? c : b);
}

void BRadioButton::clicked() {
  if (!state) {
    BRadioGroupInputEvent event;
    event.group = group;
    focusManager().broadcastCommand(event);
    BCheckBoxBase::clicked();
  }
}

void BRadioButton::handleEvent(BInputEvent& event) {
  BCheckBoxBase::handleEvent(event);
  if (state && event.isCommand() && event.asCommand().command == BCommandInputEvent::cmRadioGroup) {
    const char* other = ((BRadioGroupInputEvent&)event).group;
    if (group == other || (group && other && !strcmp(group, other))) {
      state = false;
      dirty();
      raiseOnChange(state);
    }
  }
}