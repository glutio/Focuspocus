#ifndef __BCHECKBOX_H__
#define __BCHECKBOX_H__

#include "BView.h"
#include "BFocusManager.h"

// A box (or circle) followed by text, drawn as one control. Taps anywhere on
// it, or Enter while focused, toggle `state` and raise onChange.
class BCheckBoxBase: public BButton {
public:
  BEVENT(Change, BCheckBoxBase, bool)

public:
  B::Alignment alignment;     // B::left: box then text; B::right: text then box
  bool state;
  BNullable<int16_t> boxSize; // unset: theme checkboxSize; 0: text height

protected:
  static const int16_t spacing = 4;
  int16_t boxPixels();
  BRect textBounds();
  virtual void clicked();
  virtual void drawBox(BGraphics& g);

public:
  BCheckBoxBase();
  virtual void measure(uint16_t availableWidth, uint16_t availableHeight);
  virtual void draw(BGraphics& g);
};

class BCheckBox: public BCheckBoxBase {
};

class BRadioGroupInputEvent: public BCommandInputEvent {
public:
  const char* group;
  BRadioGroupInputEvent() {
    command = BCommandInputEvent::cmRadioGroup;
  }
};

// Radios with the same group (compared as strings) exclude each other;
// radios without a group form one group.
class BRadioButton: public BCheckBoxBase {
public:
  const char* group;
  BRadioButton() : group(nullptr) {}

protected:
  virtual void clicked();
  virtual void drawBox(BGraphics& g);

public:
  virtual void handleEvent(BInputEvent& event);
};

#endif