// HelloButton: a label and a button. Tap the button to count clicks.
// Hardware: Adafruit 2.8" TFT touch shield (ILI9341 + FT6206), any Arduino.
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Focuspocus.h>

Adafruit_ILI9341 tft(10, 9);  // CS, DC
Adafruit_FT6206 touch;

// The screen: a column holding a label and a button.
BTextLabel label;
BButton button;
BView* columnItems[] = { &label, &button };
BStackPanel column(columnItems);
BRootView mainScreen(column);

// The GUI: graphics + theme + screen stack -> focus manager -> input.
BAdafruitGraphics graphics(tft);
BTheme theme;
BRootView* screens[] = { &mainScreen };
BFocusManager fm(graphics, screens, theme);
BMouse mouse(fm);

int clicks = 0;
char text[24] = "Hello!";

void onClick(BButton*, bool) {
  ++clicks;
  strcpy(text, "Clicks: ");
  itoa(clicks, text + 8, 10);  // itoa is much smaller than snprintf
  label.setText(text);
}

void setup() {
  tft.begin();
  touch.begin(40);

  label.text = text;
  label.fontSize = 2;
  button.text = "Click me";
  button.onClick += onClick;

  column.width = -1;   // -1: fill the parent (here: the screen)
  column.height = -1;
  column.orientation = B::vertical;
  column.horizontalAlignment = B::center;
  column.verticalAlignment = B::center;
  column.spacing = 20;
}

void loop() {
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    mouse.move(tft.width() - p.x, tft.height() - p.y, true);  // FT6206 is mirrored
  } else {
    mouse.button(false);
  }
  fm.loop(millis());
}
