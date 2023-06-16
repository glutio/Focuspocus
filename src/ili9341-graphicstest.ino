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
#include "Focuspocus.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TS_CS 8

Adafruit_FT6206 ctp = Adafruit_FT6206();
// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

BGraphics _g(tft);

namespace ButtonViewStatic {
  // BButton button3;
  // BButton buttonOk;
  // BButton buttonCancel;
  // BView* confirmContent[] = { &buttonOk, &buttonCancel }; 
  // BStackPanel confirm(confirmContent);

  BButton button1;
   BButton button2;
  BView* rootContent[] = { &button1, &button2, /*&confirm, &button3*/ };
  BStackPanel root(rootContent);


  void onClick(BButton* sender, bool clicked) {
  //  Serial.println(sender->tag);
  }

  void Initialize() {
    root.tag = "Panel";
    root.padding.top = 10;
    root.padding.left = 10;
    root.padding.bottom = 10;
    root.padding.right = 10;
    //root.spacing = 10;
    // root.margin.top = 10;
    // root.margin.bottom = 10;
    // root.margin.left = 10;
    // root.margin.right = 10;
    root.orientation = BStackPanel::vertical;  
    root.horizontalAlignment = BStackPanel::right;  
    root.verticalAlignment = BStackPanel::center;  
  //   button1.x = 0;
  //   button1.y = 0;
  //  button1.width = 50;
  //   button1.maxWidth = 105;
    button1.height = -1;
    button1.color = 0xFFFF;
    button1.tag = "Button1";
    button1.fontColor = 0x8020;
    button1.text = "OK";
    button1.fontSize = 3;
    button1.margin.left = 10;    
    button2.minHeight = 200;
    button2.margin.top=50;
    //button1.margin.right = 10;
    //button1.margin.top = 10;
    //button1.margin.bottom = 10;
    
    // button2.x = 80;
    // button2.y = 0;
    // button2.width = 50;
    // button2.height = 25;
    button2.color = 0xFFFF;
    button2.tag = "Button2";
    button2.text = "Cancel";
    // button2.margin.right = 10;

    // button3.tag = "button3";
    // confirm.spacing = 10;
    // confirm.padding.left = 10;
    // confirm.padding.right = 10;
    // confirm.padding.top = 10;
    // confirm.padding.bottom = 10;
    // confirm.tag = "Confirm";
    // buttonOk.text = "Yes";
    // buttonOk.color = 0xFFFF;
    // buttonOk.tag = "Yes";
    // buttonCancel.text = "No";
    // buttonCancel.tag="No";
    // buttonCancel.color = 0xAA00;
    // //buttonCancel.maxWidth = 20;
    // buttonCancel.width=-2;

    button1.onClick += onClick;
   // button2.onClick += onClick;
  }  

}

BView* _stack[] = { &ButtonViewStatic::root };
// struct ButtonViewDynamic : BPanel {
//   BButton button1;
//   BButton button2;
//   BView* rootContent[2];
//   ButtonViewDynamic() : BPanel(rootContent) 
//   {
//     rootContent[0] = &button1;
//     rootContent[1] = &button2;
//   }
// };

#define LEFT_PIN 7
#define CLICK_PIN 6
#define RIGHT_PIN 5

BDigitalPin btnLeft(LEFT_PIN, INPUT_PULLUP, 40);
BDigitalPin btnClick(CLICK_PIN, INPUT_PULLUP, 40);
BDigitalPin btnRight(RIGHT_PIN, INPUT_PULLUP, 40);

void onChange(BDigitalPin* sender, bool state) {
  if (sender->pin() == LEFT_PIN) {
    if (!state) BFocusManager::focusPrev();      
  } else if (sender->pin() == RIGHT_PIN) {
    if (!state) BFocusManager::focusNext();
  } else if (sender->pin() == CLICK_PIN) {
    BKeyboard::sendKey(BKeyboard::kbEnter, !state);
  }
}

void setup() {
  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
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

  ButtonViewStatic::Initialize();
  setupFocuspocus(_g, _stack);

}


void loop(void) {
  if (ctp.touched()) {
    TS_Point pt = ctp.getPoint();
    pt.x = tft.width() - pt.x;
    pt.y = tft.height() - pt.y;
    if (pt.z == 0) {
      BMouse::update(false);
    }
    else {
      BMouse::update(pt.x, pt.y, true);
    }
  }
  else {
    BMouse::update(false);
  }  
 
  btnClick.update();
  btnLeft.update();
  btnRight.update();

  focuspocus();
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