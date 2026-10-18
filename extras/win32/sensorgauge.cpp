// Windows host for examples/SensorGauge, with four gauges to show sizing:
// one stretched to the column width (width = -1), and a row of three with
// fixed widths. Height is left at 0, so each gauge picks it from its width.
// The "sensor" sweeps 0..1023..0 every 8 seconds. Esc = quit.
#include <stdlib.h>
#include "BWin32Window.h"
#include "Focuspocus.h"

BTextLabel title;
BTextLabel reading;
BGauge big;
BGauge medium;
BGauge small;
BGauge tiny;
BProgressBar bar;
BView* rowItems[] = { &medium, &small, &tiny };
BStackPanel row(rowItems);
BView* columnItems[] = { &title, &reading, &big, &row, &bar };
BStackPanel column(columnItems);
BScreen mainScreen(column);

BWin32Graphics gfx(240, 320); // ILI9341 portrait
BTheme theme;
BScreen* screens[] = { &mainScreen };
BFocusManager fm(gfx, screens, theme);
BMouse mouse(fm);
BKeyboard keyboard(fm);

char readingText[8] = "-";

int main() {
  title.text = "Sensor A0";
  reading.text = readingText;
  reading.fontSize = 4;
  big.width = -1;  // full width
  medium.width = 100;
  small.width = 70;
  tiny.width = 40;
  medium.needleColor = 0xF800;  // red
  small.needleColor = 0x07E0;   // green
  tiny.needleColor = 0xFFE0;    // yellow
  BGauge* gauges[] = { &big, &medium, &small, &tiny };
  for (BGauge* g : gauges) {
    g->maximum = 1023;
  }
  bar.maximum = 1023;
  bar.width = -1;
  bar.height = 20;
  row.orientation = B::horizontal;
  row.spacing = 5;
  column.width = -1;
  column.height = -1;
  column.orientation = B::vertical;
  column.horizontalAlignment = B::center;
  column.spacing = 10;
  column.padding(10);

  BWin32Window window(gfx, fm, mouse, keyboard, L"Focuspocus - SensorGauge", 2);
  BInterval readEvery(200);
  while (window.pump()) {
    unsigned long now = GetTickCount();
    if (readEvery.elapsed(now)) {
      long t = now % 8000;
      int value = (int)((t < 4000 ? t : 8000 - t) * 1023 / 4000);
      _itoa_s(value, readingText, 10);
      reading.setText(readingText);
      for (BGauge* g : gauges) {
        g->setValue(g == &big || g == &small ? value : 1023 - value);
      }
      bar.setValue(value);
    }
    fm.loop(now);
    window.present();
    Sleep(5);
  }
  return 0;
}
