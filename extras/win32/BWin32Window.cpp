#include "BWin32Window.h"
#include <windowsx.h>

static const wchar_t* kClassName = L"FocuspocusWin32Window";

BWin32Window::BWin32Window(BWin32Graphics& g, BFocusManager& fm, BMouse& mouse, BKeyboard& keyboard,
                           const wchar_t* title, int zoom)
  : _g(g), _fm(fm), _mouse(mouse), _keyboard(keyboard), _hwnd(0),
    _zoom(zoom < 1 ? 1 : zoom), _buttonDown(false), _presented(0) {
  HINSTANCE instance = GetModuleHandleW(NULL);
  WNDCLASSW wc = {};
  wc.lpfnWndProc = wndProc;
  wc.hInstance = instance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = kClassName;
  RegisterClassW(&wc); // fails harmlessly if already registered

  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  RECT rc = { 0, 0, g.displayWidth() * _zoom, g.displayHeight() * _zoom };
  AdjustWindowRect(&rc, style, FALSE);
  _hwnd = CreateWindowW(kClassName, title, style, CW_USEDEFAULT, CW_USEDEFAULT,
                        rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, instance, this);
  ShowWindow(_hwnd, SW_SHOW);
}

BWin32Window::~BWin32Window() {
  if (_hwnd) {
    SetWindowLongPtrW(_hwnd, GWLP_USERDATA, 0);
    DestroyWindow(_hwnd);
  }
}

bool BWin32Window::pump() {
  MSG msg;
  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      return false;
    }
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return _hwnd != 0;
}

void BWin32Window::present() {
  if (_hwnd && _g.version() != _presented) {
    _presented = _g.version();
    InvalidateRect(_hwnd, NULL, FALSE);
  }
}

LRESULT CALLBACK BWin32Window::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_NCCREATE) {
    auto cs = (CREATESTRUCTW*)lParam;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    ((BWin32Window*)cs->lpCreateParams)->_hwnd = hwnd;
  }
  auto self = (BWin32Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  if (self) {
    return self->handleMessage(hwnd, msg, wParam, lParam);
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT BWin32Window::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(_hwnd, &ps);
      int w = _g.displayWidth(), h = _g.displayHeight();
      StretchDIBits(dc, 0, 0, w * _zoom, h * _zoom, 0, 0, w, h,
                    _g.bits(), &_g.bitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
      EndPaint(_hwnd, &ps);
      return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE: {
      int16_t x = (int16_t)(GET_X_LPARAM(lParam) / _zoom);
      int16_t y = (int16_t)(GET_Y_LPARAM(lParam) / _zoom);
      if (msg == WM_LBUTTONDOWN) {
        SetCapture(_hwnd);
        _buttonDown = true;
      } else if (msg == WM_LBUTTONUP) {
        ReleaseCapture();
        _buttonDown = false;
      } else if (!_buttonDown) {
        return 0; // a touch screen only reports position while touched
      }
      _mouse.move(x, y, _buttonDown);
      return 0;
    }
    case WM_KEYDOWN:
    case WM_KEYUP: {
      bool down = msg == WM_KEYDOWN;
      switch (wParam) {
        case VK_TAB:
          if (down) {
            if (GetKeyState(VK_SHIFT) & 0x8000) _fm.focusPrev();
            else _fm.focusNext();
          }
          break;
        case VK_RETURN: _keyboard.sendKey(BKeyboard::kbEnter, down); break;
        case VK_LEFT:   _keyboard.sendKey(BKeyboard::kbLeft, down); break;
        case VK_RIGHT:  _keyboard.sendKey(BKeyboard::kbRight, down); break;
        case VK_UP:     _keyboard.sendKey(BKeyboard::kbUp, down); break;
        case VK_DOWN:   _keyboard.sendKey(BKeyboard::kbDown, down); break;
        case VK_ESCAPE: if (down) DestroyWindow(_hwnd); break;
      }
      return 0;
    }
    case WM_DESTROY:
      _hwnd = 0;
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}
