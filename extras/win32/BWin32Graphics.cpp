#include "BWin32Graphics.h"
#include <math.h>
#include <string.h>

static int16_t isqrt(int32_t v) {
  return v > 0 ? (int16_t)sqrt((double)v) : 0;
}

uint32_t BWin32Graphics::toRGB(uint16_t c) {
  uint32_t r = (c >> 11) & 0x1f, g = (c >> 5) & 0x3f, b = c & 0x1f;
  r = (r << 3) | (r >> 2);
  g = (g << 2) | (g >> 4);
  b = (b << 3) | (b >> 2);
  return (r << 16) | (g << 8) | b;
}

uint16_t BWin32Graphics::to565(uint32_t rgb) {
  uint32_t r = (rgb >> 16) & 0xff, g = (rgb >> 8) & 0xff, b = rgb & 0xff;
  return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

BWin32Graphics::BWin32Graphics(int16_t width, int16_t height)
  : _width(width), _height(height), _dc(0), _bitmap(0), _oldBitmap(0), _bits(0), _version(0) {
  memset(_fonts, 0, sizeof(_fonts));
  _clip = { 0, 0, width, height };
  memset(&_info, 0, sizeof(_info));
  _info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  _info.bmiHeader.biWidth = width;
  _info.bmiHeader.biHeight = -height; // top-down
  _info.bmiHeader.biPlanes = 1;
  _info.bmiHeader.biBitCount = 32;
  _info.bmiHeader.biCompression = BI_RGB;
  _dc = CreateCompatibleDC(NULL);
  void* bits = 0;
  _bitmap = CreateDIBSection(_dc, &_info, DIB_RGB_COLORS, &bits, NULL, 0);
  _bits = (uint32_t*)bits;
  _oldBitmap = SelectObject(_dc, _bitmap);
  SetBkMode(_dc, TRANSPARENT);
  memset(_bits, 0, (size_t)width * height * 4);
}

BWin32Graphics::~BWin32Graphics() {
  SelectObject(_dc, _oldBitmap);
  for (auto f : _fonts) {
    if (f) DeleteObject(f);
  }
  DeleteObject(_bitmap);
  DeleteDC(_dc);
}

HFONT BWin32Graphics::font(uint8_t size) {
  if (size < 1) size = 1;
  if (size > 8) size = 8;
  HFONT& f = _fonts[size - 1];
  if (!f) {
    // Terminal is a raster font with a native 6x8 cell, matching the Adafruit
    // classic font; GDI scales raster fonts by pixel duplication.
    f = CreateFontA(8 * size, 6 * size, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, OEM_CHARSET,
                    OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
                    FIXED_PITCH | FF_MODERN, "Terminal");
  }
  return f;
}

void BWin32Graphics::setPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < _clip.left || y < _clip.top || x >= _clip.right || y >= _clip.bottom) return;
  _bits[y * _width + x] = toRGB(color);
}

uint16_t BWin32Graphics::pixel(int16_t x, int16_t y) {
  if (x < 0 || y < 0 || x >= _width || y >= _height) return 0;
  GdiFlush();
  return to565(_bits[y * _width + x]);
}

uint32_t BWin32Graphics::countPixels(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  uint32_t n = 0;
  for (int16_t j = y; j < y + h; ++j)
    for (int16_t i = x; i < x + w; ++i)
      if (i >= 0 && j >= 0 && i < _width && j < _height && pixel(i, j) == color) ++n;
  return n;
}

void BWin32Graphics::hline(int16_t x, int16_t y, int16_t w, uint16_t color) {
  for (int16_t i = 0; i < w; ++i) setPixel(x + i, y, color);
}

void BWin32Graphics::vline(int16_t x, int16_t y, int16_t h, uint16_t color) {
  for (int16_t i = 0; i < h; ++i) setPixel(x, y + i, color);
}

// Midpoint circle; quadrant bits: 1=top-left 2=top-right 4=bottom-right 8=bottom-left
void BWin32Graphics::circleQuadrants(int16_t x0, int16_t y0, int16_t r, uint8_t q, uint16_t c) {
  int16_t x = 0, y = r, d = 1 - r;
  while (x <= y) {
    if (q & 1) { setPixel(x0 - x, y0 - y, c); setPixel(x0 - y, y0 - x, c); }
    if (q & 2) { setPixel(x0 + x, y0 - y, c); setPixel(x0 + y, y0 - x, c); }
    if (q & 4) { setPixel(x0 + x, y0 + y, c); setPixel(x0 + y, y0 + x, c); }
    if (q & 8) { setPixel(x0 - x, y0 + y, c); setPixel(x0 - y, y0 + x, c); }
    ++x;
    if (d < 0) {
      d += 2 * x + 1;
    } else {
      --y;
      d += 2 * (x - y) + 1;
    }
  }
}

int16_t BWin32Graphics::displayWidth() {
  return _width;
}

int16_t BWin32Graphics::displayHeight() {
  return _height;
}

void BWin32Graphics::setClip(int16_t left, int16_t top, int16_t right, int16_t bottom) {
  _clip = { left < 0 ? 0 : left, top < 0 ? 0 : top, right > _width ? _width : right, bottom > _height ? _height : bottom };
  SelectClipRgn(_dc, NULL);
  IntersectClipRect(_dc, _clip.left, _clip.top, _clip.right, _clip.bottom);
}

// Sizes arrive as uint16_t but callers compute them as int16_t; like the
// Adafruit TFT drivers, treat them as signed and normalize negative extents.
static void normalize(int16_t& x, int16_t& y, int16_t& w, int16_t& h) {
  if (w < 0) { x += w + 1; w = -w; }
  if (h < 0) { y += h + 1; h = -h; }
}

void BWin32Graphics::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  ++_version;
  int16_t w = (int16_t)width, h = (int16_t)height;
  normalize(x, y, w, h);
  if (!w || !h) return;
  hline(x, y, w, color);
  hline(x, y + h - 1, w, color);
  vline(x, y, h, color);
  vline(x + w - 1, y, h, color);
}

void BWin32Graphics::fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color) {
  ++_version;
  int16_t w = (int16_t)width, h = (int16_t)height;
  normalize(x, y, w, h);
  for (int16_t j = 0; j < h; ++j) hline(x, y + j, w, color);
}

void BWin32Graphics::drawRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  ++_version;
  int16_t w = (int16_t)width, h = (int16_t)height;
  normalize(x, y, w, h);
  if (!w || !h) return;
  int16_t r = (int16_t)radius;
  int16_t maxR = (w < h ? w : h) / 2;
  if (r > maxR) r = maxR;
  if (r < 0) r = 0;
  hline(x + r, y, w - 2 * r, color);
  hline(x + r, y + h - 1, w - 2 * r, color);
  vline(x, y + r, h - 2 * r, color);
  vline(x + w - 1, y + r, h - 2 * r, color);
  circleQuadrants(x + r, y + r, r, 1, color);
  circleQuadrants(x + w - r - 1, y + r, r, 2, color);
  circleQuadrants(x + w - r - 1, y + h - r - 1, r, 4, color);
  circleQuadrants(x + r, y + h - r - 1, r, 8, color);
}

void BWin32Graphics::fillRoundRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t radius, uint16_t color) {
  ++_version;
  int16_t w = (int16_t)width, h = (int16_t)height;
  normalize(x, y, w, h);
  int16_t r = (int16_t)radius;
  int16_t maxR = (w < h ? w : h) / 2;
  if (r > maxR) r = maxR;
  if (r < 0) r = 0;
  for (int16_t i = 0; i < h; ++i) {
    int16_t d = 0;
    if (i < r) d = r - i;
    else if (i > h - 1 - r) d = i - (h - 1 - r);
    int16_t inset = r - isqrt((int32_t)r * r - (int32_t)d * d);
    hline(x + inset, y + i, w - 2 * inset, color);
  }
}

void BWin32Graphics::drawText(const char* str, int16_t x, int16_t y, uint8_t size, uint16_t color) {
  ++_version;
  if (!str) return;
  if (size < 1) size = 1;
  HGDIOBJ old = SelectObject(_dc, font(size));
  uint32_t rgb = toRGB(color);
  SetTextColor(_dc, RGB((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff));
  for (size_t i = 0; str[i]; ++i) {
    TextOutA(_dc, x + (int)(i * 6 * size), y, &str[i], 1);
  }
  SelectObject(_dc, old);
  GdiFlush();
}

BRect BWin32Graphics::getTextBounds(const char* str, uint8_t size) {
  BRect rt;
  if (!str || !*str) return rt;
  if (size < 1) size = 1;
  rt.width = (int16_t)(strlen(str) * 6 * size);
  rt.height = 8 * size;
  return rt;
}

void BWin32Graphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], uint16_t w, uint16_t h) {
  ++_version;
  for (int16_t j = 0; j < (int16_t)h; ++j)
    for (int16_t i = 0; i < (int16_t)w; ++i)
      setPixel(x + i, y + j, bitmap[j * w + i]);
}

void BWin32Graphics::drawBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], uint16_t w, uint16_t h) {
  ++_version;
  int16_t bw = (w + 7) / 8;
  for (int16_t j = 0; j < (int16_t)h; ++j)
    for (int16_t i = 0; i < (int16_t)w; ++i)
      if (mask[j * bw + i / 8] & (0x80 >> (i & 7)))
        setPixel(x + i, y + j, bitmap[j * w + i]);
}

void BWin32Graphics::drawCircle(int16_t cx, int16_t cy, uint16_t radius, uint16_t color) {
  ++_version;
  circleQuadrants(cx, cy, (int16_t)radius, 0x0f, color);
}

void BWin32Graphics::fillCircle(int16_t cx, int16_t cy, uint16_t radius, uint16_t color) {
  ++_version;
  int16_t r = (int16_t)radius;
  for (int16_t dy = -r; dy <= r; ++dy) {
    int16_t half = isqrt((int32_t)r * r - (int32_t)dy * dy);
    hline(cx - half, cy + dy, 2 * half + 1, color);
  }
}

void BWin32Graphics::drawShade(int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
  ++_version;
  for (int16_t j = y + (y & 1); j < y + height; j += 2)
    for (int16_t i = x; i < x + width; ++i)
      setPixel(i, j, (uint16_t)color);
}

void BWin32Graphics::fillScreen(uint16_t color) {
  ++_version;
  for (int16_t j = (int16_t)_clip.top; j < _clip.bottom; ++j) hline((int16_t)_clip.left, j, (int16_t)(_clip.right - _clip.left), color);
}

void BWin32Graphics::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  ++_version;
  int16_t dx = (int16_t)abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int16_t dy = (int16_t)-abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
  int16_t err = dx + dy;
  for (;;) {
    setPixel(x1, y1, color);
    if (x1 == x2 && y1 == y2) break;
    int16_t e2 = 2 * err;
    if (e2 >= dy) { err += dy; x1 += sx; }
    if (e2 <= dx) { err += dx; y1 += sy; }
  }
}
