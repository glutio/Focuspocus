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
#include "BAdafruitGraphics.h"
#include "Focuspocus.h"
#include "Bitmaps.h"
#include "BPlane.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TS_CS 8

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_FT6206 ctp = Adafruit_FT6206();
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

BAdafruitGraphics _gTft(tft);
BAdafruitGraphics _gOled(display);
BWinTheme theme;
BBWTheme bwtheme;

namespace ButtonViewStatic {
  BBitmapButton bitmapButton;
  BButton buttonYes;
  BButton buttonNo;
  
  BView* buttonsContent[] = { &bitmapButton, &buttonYes, &buttonNo };
  BStackPanel buttons(buttonsContent);

  BCheckBox checkbox1;
  BCheckBox checkbox2;
  BRadioButton radio1;
  BRadioButton radio2;

  BView* checkboxLeftContent[] = { &checkbox1, &radio1 };
  BStackPanel checkboxLeft(checkboxLeftContent);
  BView* checkboxRightContent[] = { &checkbox2, &radio2 };
  BStackPanel checkboxRight(checkboxRightContent);
  BView* checkboxesContent[] { &checkboxLeft, &checkboxRight };
  BStackPanel checkboxes(checkboxesContent);

  BTextLabel label1;
  BTextLabel label2;
  BView* labelsContent[] { &label1, &label2 };
  BStackPanel labels(labelsContent);
  
  BProgressBar hbar;

  BPlane plane;

  BView* mainContent[] = { &buttons, &checkboxes, &labels, &hbar, &plane };
  BStackPanel main(mainContent);
  BScrollbar hscrollbar;
  BScrollbar vscrollbar;

  BView* rootContent[] = {&main, &hscrollbar, &vscrollbar};
  BPanel root(rootContent);

  BRootView rv(root);
  void Initialize() {
    //BView::showBoundingBox = true;

    bitmapButton.bitmap = buttonPixMap;
    bitmapButton.mask = buttonAlphaMask;
    bitmapButton.width = 89;
    bitmapButton.height = 40;
    bitmapButton.text = "fixed size";
    bitmapButton.tag = "bmp";

    buttonYes.height = -1;
    buttonYes.text = "auto";
    buttonYes.tag = "yes";
    buttonYes.onClick += [](BButton* b, bool click)->void { if (click) { b->height*=2; b->dirtyLayout(); }};
    buttonNo.height = -1;
    buttonNo.text = "height";
    buttonNo.tag = "no";
    buttonNo.margin(10);

    buttons.spacing = 5;
    buttons.width = -1;
    buttons.horizontalAlignment = B::center;
    buttons.border = true;
    buttons.padding(3);
    buttons.tag="buttons";

    checkbox1.text = "Left aligned";
    checkbox1.tag="box1";
    checkbox2.text = "Right aligned";
    checkbox2.alignment = B::right;
    checkbox2.tag="box2";

    radio1.text = "Option 1";
    radio1.tag="radio1";
    radio2.text = "Option 2";
    radio2.alignment = B::right;
    radio2.tag="radio1";

    checkboxLeft.orientation = B::vertical;
    checkboxLeft.width = -1;
    checkboxLeft.spacing = 5;
    checkboxRight.orientation = B::vertical;
    checkboxRight.width = -1;
    checkboxRight.spacing = 5;
    checkboxRight.horizontalAlignment = B::right;

    checkboxes.width = -1;
    checkboxes.padding(3);
    checkboxes.border = true;
    checkboxes.tag="checkboxes";

    label1.text = "use color";
    label1.fontColor = 0xa0b6;
    label1.fontSize = 2;
    label1.background = 0xa69d;
    label1.tag = "label1";
    label2.text = "and font";
    label2.fontColor = 0xe361;
    label2.fontSize = 3;
    label2.background = 0xa69d;
    label2.tag = "label2";
    labels.orientation = B::vertical;
    labels.spacing = 5;
    labels.background = 0xa69d;
    labels.border = true;
    labels.width = -1;
    labels.tag="labels";
    
    hbar.value = -20;
    hbar.minimum = -100;
    hbar.maximum = 0;
    hbar.width = -1;
    hbar.color = 0x0999;
    hbar.height = 30;

    plane.margin(20);
    plane.width = 50;
    plane.height = 50;
    plane.tag = "plane";


    main.spacing = 10;
    main.orientation = B::vertical;
    main.width = _gTft.displayWidth() - theme.scrollbarSize - 5;
    main.height = _gTft.displayHeight() - theme.scrollbarSize - 5;
    main.horizontalAlignment = B::center;
    main.tag= "main";
    hscrollbar.width = main.width;
    hscrollbar.orientation = B::horizontal;
    hscrollbar.y = _gTft.displayHeight() - theme.scrollbarSize - 1;
    hscrollbar.minimum = -90; 
    hscrollbar.maximum = 90;
    hscrollbar.step = 20;
    hscrollbar.onChange += [](BScrollbar*, int16_t value) -> void { plane.hidePlane(); plane.transverseAngle = value; plane.dirtyLayout(); };
    hscrollbar.tag = "hscroll";
    vscrollbar.height = _gTft.displayHeight();
    vscrollbar.x = _gTft.displayWidth() - theme.scrollbarSize - 1;
    vscrollbar.orientation = B::vertical;
    vscrollbar.minimum = -90; 
    vscrollbar.maximum = 90;
    vscrollbar.step = 20;
    vscrollbar.onChange += [](BScrollbar*, int16_t value) -> void { plane.hidePlane(); plane.coronalAngle = value; plane.dirtyLayout(); };
    vscrollbar.tag = "vscroll";
    root.width = -1;
    root.height = -1;
    root.tag = "root";
  }  
}

BRootView* stack[] = { &ButtonViewStatic::rv, 0 };
//BView* _stack[] = {&ButtonViewStatic::_root };

//BFocusManager fm(_gOled, _stack, bwtheme);
BFocusManager fm2(_gTft, stack, theme);
BMouse mouse(fm2);
//BKeyboard kbd(fm);
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
     // fm.focusPrev();      
      fm2.focusPrev();
    }
  } else if (sender->pin() == RIGHT_PIN) {
    if (!state) {
     // fm.focusNext();
      //Serial.print("hi");
      //Serial.print(fm.focusedView()->tag);
      fm2.focusNext();
    }
    //kbd.sendKey(BKeyboard::kbRight, !state);
  } else if (sender->pin() == CLICK_PIN) {
    //kbd.sendKey(BKeyboard::kbEnter, !state);
    kbd2.sendKey(BKeyboard::kbEnter, !state);
  }
}

void clkChange(BDigitalPin* pin, bool state) {
  if (!state) {
  if (!encoderDT) {
  //  fm.focusNext();
    Serial.println("Rotated clockwise ⏩");
  }
  if (encoderDT) {
  //  fm.focusPrev();
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
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
//  display.display();
  ButtonViewStatic::Initialize();
  //fm.onAfterRender += [](BFocusManager*,bool) -> void { display.display(); };
}


void loop(void) {
  if (ctp.touched()) {
    TS_Point pt = ctp.getPoint();
    pt.x = tft.width() - pt.x;
    pt.y = tft.height() - pt.y;
    if (pt.z == 0) {
      mouse.button(false);
    }
    else {
      mouse.move(pt.x, pt.y, true);
    }
  }
  else {
    mouse.button(false);
  }  
 
  btnClick.update();
  btnLeft.update();
  btnRight.update();

  encoderDT.update();
  encoderCLK.update();

  //fm.loop();
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