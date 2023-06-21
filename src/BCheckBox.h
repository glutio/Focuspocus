#ifndef __BCHECKBOX_H__
#define __BCHECKBOX_H__

#include "BView.h"
#include "BTextLabel.h"
#include "BFocusManager.h"


class BCheckBoxBase: public BStackPanel, BFontAware {
public:
  typedef EventDelegate<BCheckBoxBase, bool> ChangedEvent;
  EventSource<ChangedEvent> onChange;

protected:
  class BBox: public BButton {
  protected:
    BCheckBoxBase& _checkbox;
  public:
    BBox(BCheckBoxBase& checkbox) : _checkbox(checkbox) {
    }
    virtual void draw(BGraphics& g) {
      _checkbox.handleDraw(g);
    }

    virtual void measure(int16_t availableWidth, int16_t availableHeight) {
      width = height = availableHeight;
    }
    bool isDown() {
      return _isDown;
    }

    void isDown(bool isDown) {
      if (_isDown != isDown) {;
        _isDown = isDown;
        dirty();
      }
    }
  };

protected:
  BBox _box;
  BTextLabel _label;
  BView* _content[2];
public:
  enum Alignment {
    left,
    right
  };

  Alignment alignment;
  bool state;
  const char* text;
public:
  BCheckBoxBase() : _content{0}, BStackPanel(_content), alignment(left), state(false), text(0), _box(*this)
  {
    width = 0;
    height = 0;
    spacing = 4;
    _box.onClick += BButton::ClickEvent(this, &BCheckBoxBase::handleClick);
    _label.onMouse += BView::MouseEvent(this, &BCheckBoxBase::handleMouse);
  };

  ~BCheckBoxBase() {
    _box.onClick -= BButton::ClickEvent(this, &BCheckBoxBase::handleClick);
    _label.onMouse -= BView::MouseEvent(this, &BCheckBoxBase::handleMouse);
  };

  virtual void handleDraw(BGraphics& g) {
      int16_t c = color;
      int16_t b = color;
      if (_box.isFocused()) {
        c = focusManager().theme().focusColor;
        b = focusManager().theme().focusBackground;
      }
      
      g.drawRect(0, 0, g.width, g.height, c);
      g.drawRect(1, 1, g.width - 2, g.height - 2, background);
      if (_box.isDown()) {
        g.drawRect(2, 2, g.width - 4, g.height - 4, background);
      }
      else {
        if (!state) {
          b = background;
        }
      }
      int pad = (_box.isDown()) ? 3 : 2;
      g.fillRect(pad, pad, g.width - pad * 2, g.height - pad * 2, b);
  }

  void applyStyle() {
    _label.text = text;
    _label.fontSize = fontSize;
    _label.fontColor = fontColor;
    _label.color = color;
    _label.background = background;
  }

  virtual void layout() {
    applyStyle();
    
    _label.verticalAlignment = BTextLabel::center;
    if (alignment == left) {
      horizontalAlignment = BStackPanel::left;
      _content[0] = &_box;
      _content[1] = &_label;
      _label.horizontalAlignment = BTextLabel::left;
    } else {
      _content[0] = &_label;
      _content[1] = &_box;
      _label.horizontalAlignment = BTextLabel::right;
      horizontalAlignment = BStackPanel::right;
    }

    if (!width && _box.actualHeight != clientHeight()) {            
      actualWidth += clientHeight() + padding.left + padding.right;
      parent()->dirtyLayout();
    }

    BStackPanel::layout();
  }

  virtual void measure(int16_t availableWidth, int16_t availableHeight) {
    if (!width || !height) {
      BRect rt = focusManager().getGraphics().getTextBounds(text, fontSize);
      if (!height) {
        actualHeight = rt.height + padding.top + padding.bottom;
      }
      if (!width && _box.actualHeight != clientHeight()) {
        actualWidth = rt.width + spacing;
      }      
    }
  }

  raiseOnChange(bool state) {
    onChange(this, state);
  }

  virtual void handleClick(BButton* sender, bool isClick) {
    if (isClick) {
      state = !state;
      raiseOnChange(state);
    }
  }

  void handleMouse(BView* sender, BMouseInputEvent& event) {
    switch(event.type) {
      case BInputEvent::evMouseDown: {
        _box.focus();
        focusManager().captureMouse(*sender);
        _box.isDown(true);
        break;
      }
      case BInputEvent::evMouseUp: {
        focusManager().releaseMouse(*sender);
        BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
        bool hit = hitTest(pt.x, pt.y);
        handleClick(&_box, hit);
        _box.isDown(false);
        break;
      }
      case BInputEvent::evMouseMove: {
        BPoint pt = focusManager().mapScreenToView(*this, event.x, event.y);
        bool hit = hitTest(pt.x, pt.y);
        _box.isDown(hit);
        break;
      }
    }
  }
};

class BCheckBox: public BCheckBoxBase {
};

class BRadioGroupInputEvent: public BCommandInputEvent {
public:
  const char* group;
  BRadioGroupInputEvent() {
    command = BCommandInputEvent::cmRadioGroup;
  }
};

class BRadioButton: public BCheckBoxBase {
public:
  const char* group;

protected:
  virtual void handleDraw(BGraphics& g) {
    int16_t c = color;
    int16_t b = color;
    if (_box.isFocused()) {
      c = focusManager().theme().focusColor;
      b = focusManager().theme().focusBackground;
    }
    auto org = g.height / 2;
    g.drawCircle(org, org, g.height / 2, c);
    g.drawCircle(org, org, (g.height - 2) / 2, background);
    if (_box.isDown()) {
      g.drawCircle(org, org, (g.height - 4) / 2, background);
    }
    else {
      if (!state) {
        b = background;
      }
    }
    int pad = (_box.isDown()) ? 3 : 2;
    g.fillCircle(org, org, (g.height - pad * 2) / 2, b);
  }

  virtual void handleClick(BButton* sender, bool isClick) {
    if (isClick) {
      if (!state) {
        BRadioGroupInputEvent event;
        event.group = group;
        focusManager().broadcastCommand(event);
        state = !state;
        raiseOnChange(state);
      }
    }
  }

  virtual void handleEvent(BInputEvent& event) {    
    BStackPanel::handleEvent(event);
    if (event.isCommand() && event.asCommand().command == BCommandInputEvent::cmRadioGroup) {
      if (state) {        
        BRadioGroupInputEvent& cmd = (BRadioGroupInputEvent&)event;
        if (!strcmp(group, cmd.group)) {
          state = !state;
          dirty();
          raiseOnChange(state);
        };
      }
    }
  }
};

#endif