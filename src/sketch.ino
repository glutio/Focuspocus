/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_FT6206.h>
#include <Adafruit_SSD1306.h>
#include "Focuspocus.h"
#include "Bitmaps.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TS_CS 8

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_FT6206 ctp = Adafruit_FT6206();
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

BGraphics _gTft(tft);
BGraphics _gOled(display);
BTheme theme;
BBWTheme bwtheme;

namespace ButtonViewStatic {
  BButton button1;
  BButton button2;
  BView* rootContent[] { &button1, &button2 };
  BStackPanel root(rootContent);  
  
  BButton _button1;
  BButton _button2;
  BView* _rootContent[] { &_button1, &_button2 };
  BStackPanel _root(_rootContent);  

  void Initialize() {
    root.tag = "root";
    _root.tag = "_root";
    root.maxHeight = 200;
    root.maxWidth = 150;
    root.padding(1);
    root.orientation = BStackPanel::vertical;    
    //BView::showBoundingBox = true;
    button1.text = "button";
    button1.tag = "btn1";
    button1.background = 0xF0F0;
    button2.text = "button2";
    button2.tag = "btn2";    
    _root.padding(1);
    _root.orientation = BStackPanel::vertical;    
    _button1.text = "b1";
    _button2.text = "b2";
  }  
}

BView* stack[] = { &ButtonViewStatic::root };
BView* _stack[] = {&ButtonViewStatic::_root };

BFocusManager fm(_gOled, _stack, bwtheme);
BFocusManager fm2(_gTft, stack, theme);
BMouse mouse(fm2);
BKeyboard kbd(fm);
BKeyboard kbd2(fm2);

#define LEFT_PIN 7
#define CLICK_PIN 6
#define RIGHT_PIN 5

BDigitalPin btnLeft(LEFT_PIN, INPUT_PULLUP, 40);
BDigitalPin btnClick(CLICK_PIN, INPUT_PULLUP, 40);
BDigitalPin btnRight(RIGHT_PIN, INPUT_PULLUP, 40);
BDigitalPin encoderDT(2, INPUT, 0);
BDigitalPin encoderCLK(3, INPUT, 0);

void onChange(BDigitalPin* sender, bool state) {
  if (sender->pin() == LEFT_PIN) {
    //kbd.sendKey(BKeyboard::kbLeft, !state);
    if (!state) {
      fm.focusPrev();      
      fm2.focusPrev();
    }
  } else if (sender->pin() == RIGHT_PIN) {
    if (!state) {
      fm.focusNext();
      Serial.print("hi");
      Serial.print(fm.focusedView()->tag);
      fm2.focusNext();
    }
    //kbd.sendKey(BKeyboard::kbRight, !state);
  } else if (sender->pin() == CLICK_PIN) {
    kbd.sendKey(BKeyboard::kbEnter, !state);
    kbd2.sendKey(BKeyboard::kbEnter, !state);
  }
}

void clkChange(BDigitalPin* pin, bool state) {
  if (!state) {
  if (!encoderDT) {
    fm.focusNext();
    Serial.println("Rotated clockwise ⏩");
  }
  if (encoderDT) {
    fm.focusPrev();
    Serial.println("Rotated counterclockwise ⏪");
  }  

  }
}

void setup() {
  // pinMode(7, INPUT_PULLUP);
  // pinMode(6, INPUT_PULLUP);
  // pinMode(5, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
  ctp.begin(40);
  tft.begin();
  btnLeft.init();
  btnLeft.onChange += onChange;
  btnClick.init();
  btnClick.onChange += onChange;
  btnRight.init();
  btnRight.onChange += onChange;
  encoderCLK.init();
  encoderDT.init();
  encoderCLK.onChange += clkChange;
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.display();
  ButtonViewStatic::Initialize();
  fm.onAfterRender += [](BFocusManager*,bool) -> void { display.display(); };
}


void loop(void) {
  if (ctp.touched()) {
    TS_Point pt = ctp.getPoint();
    pt.x = tft.width() - pt.x;
    pt.y = tft.height() - pt.y;
    if (pt.z == 0) {
      mouse.update(false);
    }
    else {
      mouse.update(pt.x, pt.y, true);
    }
  }
  else {
    mouse.update(false);
  }  
 
  btnClick.update();
  btnLeft.update();
  btnRight.update();

  encoderDT.update();
  encoderCLK.update();

  fm.loop();
  fm2.loop();
  //display.display();
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  
  yield();
  tft.fillScreen(ILI9341_RED);
  yield();
  tft.fillScreen(ILI9341_GREEN);
  yield();
  tft.fillScreen(ILI9341_BLUE);
  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  return micros() - start;
}

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  tft.println("And hooptiously drangle me");
  tft.println("with crinkly bindlewurdles,");
  tft.println("Or I will rend thee");
  tft.println("in the gobberwarts");
  tft.println("with my blurglecruncheon,");
  tft.println("see if I don't!");
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  yield();
  
  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t     = micros() - start; // fillScreen doesn't count against timing

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);
  t    += micros() - start;

  yield();
  tft.fillScreen(ILI9341_BLACK);
  yield();

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, color);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, color);

  yield();
  return micros() - start;
}

unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = tft.width(), h = tft.height();

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(y=0; y<h; y+=5) tft.drawFastHLine(0, y, w, color1);
  for(x=0; x<w; x+=5) tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = tft.width()  / 2,
                cy = tft.height() / 2;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(tft.width(), tft.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    tft.drawRect(cx-i2, cy-i2, i, i, color);
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n = min(tft.width(), tft.height());
  for(i=n; i>0; i-=6) {
    i2    = i / 2;
    start = micros();
    tft.fillRect(cx-i2, cy-i2, i, i, color1);
    t    += micros() - start;
    // Outlines are not included in timing results
    tft.drawRect(cx-i2, cy-i2, i, i, color2);
    yield();
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                w = tft.width()  + radius,
                h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = tft.width()  / 2 - 1,
                      cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    tft.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      tft.color565(i, i, i));
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = tft.width()  / 2 - 1,
                   cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(0, i*10, i*10));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      tft.color565(i*10, i*10, 0));
    yield();
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  w     = min(tft.width(), tft.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    tft.drawRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = tft.width()  / 2 - 1,
                cy = tft.height() / 2 - 1;

  tft.fillScreen(ILI9341_BLACK);
  start = micros();
  for(i=min(tft.width(), tft.height()); i>20; i-=6) {
    i2 = i / 2;
    tft.fillRoundRect(cx-i2, cy-i2, i, i, i/8, tft.color565(0, i, 0));
    yield();
  }

  return micros() - start;
}