// OledMenu: a settings menu on a 128x64 monochrome OLED, driven by three push
// buttons (no touch). Previous/Next move the focus, OK toggles.
// The LED setting blinks the built-in LED at the selected speed, using the
// GUI's timer (onTimerTick) so it keeps running whatever screen is shown.
// Hardware: SSD1306 128x64 I2C OLED; buttons from pins 4, 5, 6 to GND.
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Focuspocus.h>

Adafruit_SSD1306 oled(128, 64, &Wire);

BCheckBox ledOn;
BRadioButton slow;
BRadioButton fast;
BView* columnItems[] = { &ledOn, &slow, &fast };
BStackPanel column(columnItems);
BScreen mainScreen(column);

BAdafruitGraphics graphics(oled);
BBWTheme theme;  // black and white, small sizes for 128x64
BScreen* screens[] = { &mainScreen };
BFocusManager fm(graphics, screens, theme);
BKeyboard keyboard(fm);

BButtonPin prevButton(4);
BButtonPin okButton(5);
BButtonPin nextButton(6);

void onPrev(BButtonPin*, bool pressed) { if (pressed) fm.focusPrev(); }
void onNext(BButtonPin*, bool pressed) { if (pressed) fm.focusNext(); }
void onOk(BButtonPin*, bool pressed) { keyboard.sendKey(BKeyboard::kbEnter, pressed); }

// The OLED draws from a buffer: send it after the GUI has drawn.
void showFrame(BFocusManager*, bool) { oled.display(); }

BInterval blink(500);
void blinkLed(BFocusManager*, unsigned long ms) {
  blink.interval = fast.state ? 100 : 500;
  if (blink.elapsed(ms)) {
    digitalWrite(LED_BUILTIN, ledOn.state && !digitalRead(LED_BUILTIN));
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  prevButton.begin();
  okButton.begin();
  nextButton.begin();
  prevButton.onChange += onPrev;
  okButton.onChange += onOk;
  nextButton.onChange += onNext;
  fm.onAfterRender += showFrame;
  fm.onTimerTick += blinkLed;

  ledOn.text = "Blink LED";
  slow.text = "Slow";
  slow.group = "speed";
  slow.state = true;
  fast.text = "Fast";
  fast.group = "speed";

  column.width = -1;
  column.height = -1;
  column.orientation = B::vertical;
  column.spacing = 4;
  column.padding(4);
  fm.focusFirst();
}

void loop() {
  unsigned long ms = millis();
  prevButton.update(ms);
  okButton.update(ms);
  nextButton.update(ms);
  fm.loop(ms);
}
