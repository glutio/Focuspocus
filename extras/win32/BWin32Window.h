#ifndef __BWIN32WINDOW_H__
#define __BWIN32WINDOW_H__

#include "BWin32Graphics.h"
#include "BFocusManager.h"
#include "BMouse.h"

// Presents a BWin32Graphics framebuffer in a window and translates window
// input into the same BMouse/BKeyboard calls a sketch makes from touch
// screens and buttons:
//   left mouse button  -> BMouse (touch)
//   Enter, arrow keys  -> BKeyboard
//   Tab / Shift+Tab    -> BFocusManager::focusNext() / focusPrev()
class BWin32Window {
protected:
  BWin32Graphics& _g;
  BFocusManager& _fm;
  BMouse& _mouse;
  BKeyboard& _keyboard;
  HWND _hwnd;
  int _zoom;
  bool _buttonDown;
  uint32_t _presented;

  static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
  BWin32Window(BWin32Graphics& g, BFocusManager& fm, BMouse& mouse, BKeyboard& keyboard,
               const wchar_t* title, int zoom = 2);
  ~BWin32Window();

  // Processes pending window messages; returns false once the window is closed.
  bool pump();
  // Repaints the window if the framebuffer changed since the last present.
  void present();
  HWND hwnd() { return _hwnd; }
};

#endif
