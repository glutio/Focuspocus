#include <Arduino.h>
#include "BMouse.h"
#include "BFocusManager.h"

BMouse::BMouse(BFocusManager& focusManager, int16_t x, int16_t y, bool buttonDown) :
 _x(x), _y(y), _buttonDown(buttonDown), _focusManager(focusManager) {
}

void BMouse::update(int16_t x, int16_t y, bool buttonDown) {
  BMouseInputEvent event;
  event.x = x; 
  event.y = y;
  event.buttonDown = buttonDown;

  if (_buttonDown != buttonDown) {
    _buttonDown = buttonDown;
    event.type = (buttonDown) ? BInputEvent::evMouseDown : BInputEvent::evMouseUp;    
    _focusManager.handleEvent(event);
  }

  if (_x != x || _y != y) {
    _x = x; _y = y;
    event.type = BInputEvent::evMouseMove;
    _focusManager.handleEvent(event);
  }
}

void BMouse::update(bool buttonDown) {
  update(_x, _y, buttonDown);
}

BKeyboard::BKeyboard(BFocusManager& focusManager) : _focusManager(focusManager) {
}

void BKeyboard::sendKey(uint16_t code, bool isDown) {
  BKeyboardInputEvent event;
  event.type = (isDown) ? BInputEvent::evKeyDown : BInputEvent::evKeyUp;
  event.code = code;
  _focusManager.handleEvent(event);
}