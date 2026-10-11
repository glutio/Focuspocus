#ifndef __BBUTTONPIN_H__
#define __BBUTTONPIN_H__

// A debounced push button on a digital pin (Arduino only).
// With INPUT_PULLUP (the default) wire the button between the pin and GND.
//
//   BButtonPin next(5);
//   void onNext(BButtonPin*, bool pressed) { if (pressed) fm.focusNext(); }
//   setup(): next.begin(); next.onChange += onNext;
//   loop():  next.update(millis());
#include <Arduino.h>
#include "BEvent.h"

class BButtonPin {
public:
  BEVENT(Change, BButtonPin, bool)

protected:
  uint8_t _pin;
  uint8_t _mode;
  bool _pressed;
  bool _pending;
  uint16_t _debounceMs;
  unsigned long _changedAt;

  bool readPressed() {
    bool high = digitalRead(_pin) == HIGH;
    return _mode == INPUT_PULLUP ? !high : high;
  }

public:
  BButtonPin(uint8_t pin, uint8_t mode = INPUT_PULLUP, uint16_t debounceMs = 20)
    : _pin(pin), _mode(mode), _pressed(false), _pending(false),
      _debounceMs(debounceMs), _changedAt(0) {}

  void begin() {
    pinMode(_pin, _mode);
    _pressed = readPressed();
  }

  // Call every loop() with millis(). Raises onChange(pressed) once the new state has been
  // stable for debounceMs.
  void update(unsigned long ms) {
    bool pressed = readPressed();
    if (pressed == _pressed) {
      _pending = false;
      return;
    }
    if (!_pending) {
      _pending = true;
      _changedAt = ms;
    }
    if (ms - _changedAt >= _debounceMs) {
      _pending = false;
      _pressed = pressed;
      raiseOnChange(pressed);
    }
  }

  bool pressed() { return _pressed; }
  uint8_t pin() { return _pin; }
};

#endif
