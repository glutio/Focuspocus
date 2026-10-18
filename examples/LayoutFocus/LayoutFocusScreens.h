// LayoutFocus's screens, shared by LayoutFocus.ino (Arduino) and
// extras/win32/layoutfocus.cpp (Windows), so both run exactly the same UI.
#ifndef __LAYOUTFOCUSSCREENS_H__
#define __LAYOUTFOCUSSCREENS_H__

#include <stdio.h>
#include "Focuspocus.h"

// A popup with two buttons. Closing it gives focus back to whatever had it.
namespace AskScreen {
  BTextLabel question;
  BButton yes;
  BButton no;
  BView* answersContent[] = { &yes, &no };
  BStackPanel answers(answersContent);
  BView* boxContent[] = { &question, &answers };
  BStackPanel box(boxContent);
  BView* popupContent[] = { &box };
  BPopup popup(popupContent);
  BScreen screen(popup);

  void close(BButton* sender, bool) {
    sender->focusManager().pop();
  }

  void begin() {
    question.text = "Focus goes back";
    yes.text = "Yes";
    no.text = "No";
    yes.onClick += close;
    no.onClick += close;
    // An auto-sized list: full-width buttons, as wide as the widest one.
    yes.width = -1;
    no.width = -1;
    answers.orientation = B::vertical;
    answers.spacing = 6;
    box.orientation = B::vertical;
    box.horizontalAlignment = B::center;
    box.spacing = 10;
    box.padding(8);
    box.border = true;
    popup.width = -1;
    popup.height = -1;
    screen.focused = &no;  // focused when the popup opens
  }
}

// Focus: Tab/Shift+Tab (or the prev/next pins) walk the focusable views in
// order, depth first through nested panels. Labels are not focusable.
namespace FocusScreen {
  BTextLabel status;
  BButton keys[9];
  BView* row1Content[] = { &keys[0], &keys[1], &keys[2] };
  BView* row2Content[] = { &keys[3], &keys[4], &keys[5] };
  BView* row3Content[] = { &keys[6], &keys[7], &keys[8] };
  BStackPanel row1(row1Content);
  BStackPanel row2(row2Content);
  BStackPanel row3(row3Content);
  BView* gridContent[] = { &row1, &row2, &row3 };
  BStackPanel grid(gridContent);

  BCheckBox check;
  BRadioButton red;
  BRadioButton blue;
  BView* choicesContent[] = { &check, &red, &blue };
  BStackPanel choices(choicesContent);

  BButton back;
  BButton ask;
  BView* barContent[] = { &back, &ask };
  BStackPanel bar(barContent);

  BView* pageContent[] = { &status, &grid, &choices, &bar };
  BStackPanel page(pageContent);
  BScreen screen(page);

  const char* names[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  void showFocus(const char* name) {
    static char line[20];
    snprintf(line, sizeof(line), "Focus: %s", name);
    status.setText(line);
  }

  void buttonFocus(BControl* sender, BFocusInputEvent& event) {
    if (event.command == BCommandInputEvent::cmFocus) {
      showFocus(((BButton*)sender)->text);
    }
  }

  void choiceFocus(BControl* sender, BFocusInputEvent& event) {
    if (event.command == BCommandInputEvent::cmFocus) {
      showFocus(((BCheckBoxBase*)sender)->text);
    }
  }

  void goBack(BButton* sender, bool) {
    sender->focusManager().pop();
  }

  void openAsk(BButton* sender, bool) {
    sender->focusManager().push(AskScreen::screen);
  }

  void begin() {
    status.text = "Focus: 5";

    for (int i = 0; i < 9; ++i) {
      keys[i].text = names[i];
      keys[i].width = -1;
      keys[i].height = -1;
      keys[i].onFocus += buttonFocus;
    }
    BStackPanel* rows[] = { &row1, &row2, &row3 };
    for (BStackPanel* row : rows) {
      row->width = -1;
      row->height = -1;
      row->spacing = 4;
    }
    grid.orientation = B::vertical;
    grid.width = -1;
    grid.height = -1;
    grid.spacing = 4;

    check.text = "Check";
    red.text = "Red";
    red.group = "color";
    blue.text = "Blue";
    blue.group = "color";
    check.onFocus += choiceFocus;
    red.onFocus += choiceFocus;
    blue.onFocus += choiceFocus;
    choices.spacing = 8;

    back.text = "< Back";
    back.width = -1;
    back.onFocus += buttonFocus;
    back.onClick += goBack;
    ask.text = "Popup";
    ask.width = -1;
    ask.onFocus += buttonFocus;
    ask.onClick += openAsk;
    bar.width = -1;
    bar.spacing = 4;

    page.orientation = B::vertical;
    page.width = -1;
    page.height = -1;
    page.padding(4);
    page.spacing = 6;
    page.horizontalAlignment = B::center;
    screen.focused = &keys[4];
  }
}

// Layout: stretch factors, min/max, margins, alignment and orientation.
namespace LayoutScreen {
  BTextLabel title;

  // Fixed 40px, then the rest split 1:2.
  BButton fixed;
  BButton one;
  BButton two;
  BView* stretchContent[] = { &fixed, &one, &two };
  BStackPanel stretch(stretchContent);

  // Three equal shares, but one is capped at 50 and one held at 110.
  BButton capped;
  BButton rest;
  BButton held;
  BView* limitsContent[] = { &capped, &rest, &held };
  BStackPanel limits(limitsContent);

  // A playground: the buttons below turn, align and space it.
  BButton a;
  BButton b;
  BButton c;
  BView* sampleContent[] = { &a, &b, &c };
  BStackPanel sample(sampleContent);

  BButton turn;
  BButton align;
  BButton space;
  BButton next;
  BView* controlsContent[] = { &turn, &align, &space, &next };
  BStackPanel controls(controlsContent);

  BView* pageContent[] = { &title, &stretch, &limits, &sample, &controls };
  BStackPanel page(pageContent);
  BScreen screen(page);

  void turnSample(BButton*, bool) {
    sample.orientation = sample.orientation == B::vertical ? B::horizontal : B::vertical;
    sample.dirtyLayout();
  }

  void alignSample(BButton*, bool) {
    if (sample.horizontalAlignment == B::left) {
      sample.horizontalAlignment = B::center;
      sample.verticalAlignment = B::center;
      align.text = "Mid";
    } else if (sample.horizontalAlignment == B::center) {
      sample.horizontalAlignment = B::right;
      sample.verticalAlignment = B::bottom;
      align.text = "End";
    } else {
      sample.horizontalAlignment = B::left;
      sample.verticalAlignment = B::top;
      align.text = "Start";
    }
    align.dirty();
    sample.dirtyLayout();
  }

  void spaceSample(BButton*, bool) {
    sample.spacing = sample.spacing ? 0 : 10;
    sample.dirtyLayout();
  }

  void showFocusScreen(BButton* sender, bool) {
    sender->focusManager().push(FocusScreen::screen);
  }

  void begin() {
    title.text = "Layout";

    fixed.text = "40";
    fixed.width = 40;
    one.text = "1x";
    one.width = -1;
    two.text = "2x";
    two.width = -2;
    stretch.width = -1;
    stretch.spacing = 2;
    stretch.padding(2);
    stretch.border = true;

    capped.text = "max50";
    capped.width = -1;
    capped.maxWidth = 50;
    rest.text = "1x";
    rest.width = -1;
    held.text = "min110";
    held.width = -1;
    held.minWidth = 110;
    limits.width = -1;
    limits.spacing = 2;
    limits.padding(2);
    limits.border = true;

    a.text = "A";
    b.text = "margin";
    b.margin(6);
    c.text = "tall";
    c.height = 50;
    sample.orientation = B::vertical;
    sample.width = -1;
    sample.height = -1;  // takes what the other rows leave
    sample.padding(4);
    sample.border = true;
    sample.background = 0xa69d;

    turn.text = "Turn";
    turn.onClick += turnSample;
    align.text = "Start";
    align.onClick += alignSample;
    space.text = "Space";
    space.onClick += spaceSample;
    next.text = "Focus";
    next.onClick += showFocusScreen;
    BButton* buttons[] = { &turn, &align, &space, &next };
    for (BButton* button : buttons) {
      button->width = -1;
    }
    controls.width = -1;
    controls.spacing = 3;

    page.orientation = B::vertical;
    page.width = -1;
    page.height = -1;
    page.padding(4);
    page.spacing = 6;
    page.horizontalAlignment = B::center;
    screen.focused = &turn;
  }
}

#endif
