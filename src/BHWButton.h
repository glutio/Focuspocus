#ifndef __BHWBUTTON_H__
#define __BHWBUTTON_H__

class BDigitalPin {
protected:
  int _pin; // pin number
  bool _value; // last value read from the pin
  uint8_t _mode;
  unsigned long _msDebounce;
  unsigned long _ms;
  bool _debouncing;
public:
  // declare event
  BEVENT(Change, BDigitalPin, bool)

public:
  // initialize DigitalPin object
  BDigitalPin(int pin, int pin_mode, int msDebounce = 0) 
    : _pin(pin), _mode(pin_mode), _msDebounce(msDebounce), _debouncing(false) {
  }

  // read the pin value and only trigger the event if there is a change
  void update() {    
    auto value = digitalRead(_pin);    
    if (value != _value) {
      if (_debouncing && millis() - _ms > _msDebounce)
      {
        _debouncing = false;
        _value = value; // update to the new value
        onChange(this, _value); // trigger event
      }
      else if (!_debouncing) {
        _ms = millis();
        _debouncing = true;
      }
    }      
  }

  // initialize
  void init() {
    pinMode(_pin, _mode);
    if (_mode != OUTPUT) {
      _value = digitalRead(_pin);
    }
  }

  // accessor for _pin
  int pin() {
    return _pin;
  }
  
  // for convenience overload () operator to set value
  operator()(bool value) {
    if (_mode == OUTPUT) {
      _value = value;
      digitalWrite(_pin, value);
    }
  }

  // for convenience overload bool operator to return value
  operator bool() {
    return _value;
  }
};

#endif