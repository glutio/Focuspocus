// SensorGauge: shows an analog reading (e.g. a potentiometer on A0) as a
// number, a bar and a dial, and blinks an alarm above a threshold.
// Shows: updating widgets from the sketch, and animating a custom view with
// tick(), which the GUI calls about every 16 ms while the view is on screen.
// Gauge.h is a complete custom control (dial + needle) written outside the
// library; copy it as a template for your own widgets.
// Hardware: Adafruit ILI9341 TFT (no touch needed), any Arduino.
#include <Adafruit_ILI9341.h>
#include <Focuspocus.h>
#include "Gauge.h"  // a custom control, built in this sketch folder

const int SENSOR_PIN = A0;
const int ALARM_LEVEL = 800;

Adafruit_ILI9341 tft(10, 9);  // CS, DC

// A label that blinks while `alarm` is true.
class AlarmLabel: public BTextLabel {
  BInterval _blink;
  bool _shown;
public:
  bool alarm;
  AlarmLabel() : _blink(400), _shown(true), alarm(false) {}

  virtual void tick(unsigned long ms) {
    if (!_blink.elapsed(ms)) {
      return;
    }
    bool show = alarm && !_shown;
    if (show != _shown) {
      _shown = show;
      fontColor = show ? ILI9341_RED : ILI9341_BLACK;  // black = invisible
      dirty();  // redraw on this loop
    }
  }
};

BTextLabel title;
BTextLabel reading;
BProgressBar bar;
Gauge gauge;
AlarmLabel alarmLabel;
BView* columnItems[] = { &title, &reading, &gauge, &bar, &alarmLabel };
BStackPanel column(columnItems);
BRootView mainScreen(column);

BAdafruitGraphics graphics(tft);
BTheme theme;
BRootView* screens[] = { &mainScreen };
BFocusManager fm(graphics, screens, theme);

BInterval readEvery(200);
char readingText[8] = "-";

void setup() {
  tft.begin();

  title.text = "Sensor A0";
  reading.text = readingText;
  reading.fontSize = 4;
  bar.minimum = 0;
  bar.maximum = 1023;
  bar.width = -1;    // full width
  bar.height = 20;
  gauge.maximum = 1023;
  gauge.width = 160;
  alarmLabel.text = "ALARM";
  alarmLabel.fontSize = 3;

  column.width = -1;
  column.height = -1;
  column.orientation = B::vertical;
  column.horizontalAlignment = B::center;
  column.spacing = 15;
  column.padding(10);
}

void loop() {
  unsigned long now = millis();
  if (readEvery.elapsed(now)) {
    int value = analogRead(SENSOR_PIN);
    itoa(value, readingText, 10);
    reading.setText(readingText);
    bar.setValue(value);
    gauge.setValue(value);
    alarmLabel.alarm = value > ALARM_LEVEL;
  }
  fm.loop(now);
}
