#include "SPI.h"
#include "Adafruit_GFX.h"
#include <Adafruit_SSD1306.h>

#include "BAdafruitGraphics.h" // Adafruit_GFX-based graphics implementation
#include "Focuspocus.h" // main header file

// For the Adafruit shield, these are the default.
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

namespace HelloWorld {
  BTextLabel label; // declare a label
  BButton button; // declare a button
  BView* mainContent[] { &label, &button }; // create content array
  BStackPanel main(mainContent); // create panel with content
  BRootView root(main);
  // button click handler
  void handleButtonClick(BButton* sender, bool isClicked) {
    if (isClicked) { // if click not cancelled
      // pop the top screen from the stack
      sender->focusManager().pop();
    }
  }

  // initialize screen content
  void init() {
    label.text = "Hello world!";

    button.text = "Ok";
    button.onClick += handleButtonClick;

    main.spacing = 10;
    main.orientation = B::vertical;
    main.horizontalAlignment = B::center;
    main.border = true;
    main.padding(5);

    root.focused = &button;
  }
}

// create a namespace for screen content
namespace MainScreen {
  BButton button; // declare a button
  BView* mainContent[] = { &button }; // create the content array
  BStackPanel main(mainContent); // create a panel with content
  BRootView root(main);
  // declare an event handler
  void handleButtonClick(BButton* sender, bool isClicked) {
    if (isClicked) { // if the click was not cancelled
      // push a screen onto the stack
      sender->focusManager().push(HelloWorld::root);
    }
  }

  // init screen content
  void init() {
    button.text = "Click me";
    button.fontSize = 2;
    button.onClick += handleButtonClick;

    main.verticalAlignment = B::center;
    main.horizontalAlignment = B::center;
    main.width = -1; // full screen
    main.height = -1; // fullscreen

    root.focused = &button;
  }  
}

// Adafruit-based graphics
BAdafruitGraphics graphics(display);
// stack of screens
BRootView* stack[] = { &MainScreen::root, 0 };
// color scheme
BBWTheme theme;

// instantiate focus manager
BFocusManager focusManager(graphics, stack, theme);
// virtual mouse
BKeyboard kbd(focusManager);

#define LEFT_PIN 7
#define CLICK_PIN 6
#define RIGHT_PIN 5

BDigitalPin btnLeft(LEFT_PIN, INPUT_PULLUP, 40);
BDigitalPin btnClick(CLICK_PIN, INPUT_PULLUP, 40);
BDigitalPin btnRight(RIGHT_PIN, INPUT_PULLUP, 40);

void onChange(BDigitalPin* sender, bool state) {
  if (sender->pin() == LEFT_PIN) {
    if (!state) {
      focusManager.focusPrev();
    }
  } else if (sender->pin() == RIGHT_PIN) {
    if (!state) {
      focusManager.focusNext();
    }
  } else if (sender->pin() == CLICK_PIN) {
    kbd.sendKey(BKeyboard::kbEnter, !state);
  }
}

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);

  btnLeft.init();
  btnLeft.onChange += onChange;
  btnClick.init();
  btnClick.onChange += onChange;
  btnRight.init();
  btnRight.onChange += onChange;

  // init screens
  HelloWorld::init();
  MainScreen::init();

  focusManager.onAfterRender += [] (BFocusManager*, bool)->void{ display.display(); };
}

void loop(void) {
  btnClick.update();
  btnLeft.update();
  btnRight.update();

  // measure/layout/render
  focusManager.loop();
}