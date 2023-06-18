#ifndef __BMOUSE_H__
#define __BMOUSE_H__

class BFocusManager;

class BMouse {
protected:
  int16_t _x;
  int16_t _y;
  bool _buttonDown;
  BFocusManager& _focusManager;
public:
  BMouse(BFocusManager& focusManager, int16_t x = -1, int16_t y = -1, bool buttonDown = false);
  void update(int16_t x, int16_t y, bool buttonDown);
  void update(bool buttonDown);
};

class BKeyboard {
protected:
  BFocusManager& _focusManager;
public:
  enum Keys {
    kbEnter = 0x0028,
    kbRight = 0x004f,
    kbLeft = 0x0050,
    kbDown = 0x0051,
    kbUp = 0x0052,
  };

public:
  BKeyboard(BFocusManager& focusManager);
  void sendKey(uint16_t code, bool isDown);
};


#endif