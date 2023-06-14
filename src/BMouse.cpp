#include <Arduino.h>
#include "BMouse.h"
#include "BFocusManager.h"

int16_t BMouse::_x = 0;
int16_t BMouse::_y = 0;
bool BMouse::_buttonDown;

void BMouse::initialize(int16_t x, int16_t y, bool buttonDown) {
  _x = x; _y = y; _buttonDown = buttonDown;
}

void BMouse::update(int16_t x, int16_t y, bool buttonDown) {
  BMouseInputEvent event;
  event.x = x; 
  event.y = y;
  event.buttonDown = buttonDown;

  if (_buttonDown != buttonDown) {
    _buttonDown = buttonDown;
    event.type = (buttonDown) ? BInputEvent::evMouseDown : BInputEvent::evMouseUp;    
    BFocusManager::handleEvent(event);
  }

  if (_x != x || _y != y) {
    _x = x; _y = y;
    event.type = BInputEvent::evMouseMove;
    BFocusManager::handleEvent(event);
  }
}

void BMouse::update(bool buttonDown) {
  update(_x, _y, buttonDown);
}
