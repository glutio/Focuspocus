#ifndef __BCHECKBOX_H__
#define __BCHECKBOX_H__

#include "BView.h"
#include "BTextLabel.h"
#include "BFocusManager.h"

class BCheckBoxBase: public BStackPanel, public BFontAware {
public:
  BEVENT(Change, BCheckBoxBase, bool)

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
  B::Alignment alignment;
  bool state;
  BNullable<int16_t> boxSize;
  const char* text;
public:
  BCheckBoxBase() : BStackPanel(_content), alignment(B::left), state(false), text(0), _box(*this)
  {
    width = 0;
    height = 0;
    spacing = 4;
    border = false;  
    _label.width = 0;
    _label.height = 0;
    _box.width = 0;
    _box.height = 0;
    verticalAlignment = B::center;  
    _box.onClick += BButton::ClickEvent(this, &BCheckBoxBase::handleClick);
    _label.onMouse += BView::MouseEvent(this, &BCheckBoxBase::handleMouse);
    _box.onFocus += BControl::FocusEvent(this, &BCheckBoxBase::handleFocus);
    _content[0] = &_box;
    _content[1] = &_label;
  };

  ~BCheckBoxBase() {
    _box.onClick -= BButton::ClickEvent(this, &BCheckBoxBase::handleClick);
    _label.onMouse -= BView::MouseEvent(this, &BCheckBoxBase::handleMouse);
  };

  void handleFocus(BControl*, BFocusInputEvent& event) {
    _label.fontColor = !_box.isFocused() ? viewColor(*this) : focusManager().theme().focusColor;
    _label.dirty();
  }

  virtual void handleDraw(BGraphics& g) {
      int16_t c = (!_box.isFocused()) ? viewColor(*this) : focusManager().theme().focusColor;
      int16_t b = c;
      g.drawRect(0, 0, g.width, g.height, c);
      g.drawRect(1, 1, g.width - 2, g.height - 2, viewBackground(*this));
      if (_box.isDown()) {
        g.drawRect(2, 2, g.width - 4, g.height - 4, viewBackground(*this));
      }
      else {
        if (!state) {
          b = viewBackground(*this);
        }
      }
      int pad = (_box.isDown()) ? 3 : 2;
      g.fillRect(pad, pad, g.width - pad * 2, g.height - pad * 2, b);
  }

  void applyStyle() {
    _label.text = text;
    _label.fontSize = viewFontSize(*this);
    _label.fontColor = !_box.isFocused() ? viewFontColor(*this) : focusManager().theme().focusColor;
    _label.color = viewColor(*this);
    _label.background = viewBackground(*this);
  }

  void applyAlignment() {
    if (alignment == B::left) {
      horizontalAlignment = B::left;
      _content[0] = &_box;
      _content[1] = &_label;
    } else {
      _content[0] = &_label;
      _content[1] = &_box;
      horizontalAlignment = B::right;
    }
  }

  virtual void measure(uint16_t availableWidth, uint16_t availableHeight) {
    applyStyle();
    applyAlignment();
    _label.measure(availableWidth, availableHeight);    
    if (boxSize.hasValue()) {
      _box.actualHeight = _box.actualWidth = (!boxSize) ? _label.actualHeight : (int16_t)boxSize;
    } else {
      _box.actualHeight = _box.actualWidth = focusManager().theme().checkboxSize;
    }
    if (!height) {
      actualHeight = max(_box.actualHeight, _label.actualHeight) + paddingHeight();
    }
    if (!width) {
      actualWidth = _label.actualWidth + _box.actualWidth + spacing + paddingWidth();
    }
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
        focusManager().captureMouse(*sender);
        _box.focus();        
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
        if (focusManager().capturingView() == sender) {
          BPoint pt = focusManager().mapScreenToView(_label, event.x, event.y);
          bool hit = _label.hitTest(pt.x, pt.y);
          _box.isDown(hit);
        }
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
    int16_t c = (!_box.isFocused()) ? viewColor(*this) : focusManager().theme().focusColor;
    int16_t b = c;

    auto org = g.height / 2;
    g.drawCircle(org, org, g.height / 2, c);
    g.drawCircle(org, org, (g.height - 2) / 2, viewBackground(*this));
    if (_box.isDown()) {
      g.fillCircle(org, org, (g.height - 4) / 2, viewBackground(*this));
    }
    else {
      if (!state) {
        b = viewBackground(*this);
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