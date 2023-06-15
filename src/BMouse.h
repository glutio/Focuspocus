#ifndef __BMOUSE_H__
#define __BMOUSE_H__

class BMouse {
protected:
  static int16_t _x;
  static int16_t _y;
  static bool _buttonDown;
public:
  static void initialize(int16_t x, int16_t y, bool buttonDown);
  static void update(int16_t x, int16_t y, bool buttonDown);
  static void update(bool buttonDown);
};

class BKeyboard {
public:
  enum Keys {
    kbEnter = 0x0028,
    kbRight = 0x004f,
    kbLeft = 0x0050,
  };

public:
  static void sendKey(uint16_t code, bool isDown);
};


#endif