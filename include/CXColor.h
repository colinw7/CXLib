#ifndef CX_COLOR_H
#define CX_COLOR_H

#include <CRGBA.h>
#include <CRGB.h>
#include <CAutoPtr.h>

#include <map>

class CXScreen;
class CXColor;

class CXColorMgr {
  typedef std::map<Pixel, CXColor *> CXColorMap;

 private:
  CXScreen   &screen_;
  CXColorMap  colormap_;

 public:
  CXColorMgr();
  CXColorMgr(CXScreen &screen);
 ~CXColorMgr();

  const CXColor &getCXColor(const CRGB &rgb);
  const CXColor &getCXColor(const CRGBA &rgba);
  const CXColor &getCXColor(Pixel pixel);
};

class CXColor {
 private:
  CXScreen &screen_;
  CRGBA     rgba_;
  Pixel     pixel_;
  bool      pixel_set_;

  CAutoPtr<CXColor> dark_color_;
  CAutoPtr<CXColor> light_color_;
  CAutoPtr<CXColor> inverse_color_;

 public:
  CXColor();

  explicit CXColor(const CRGB &rgb);
  explicit CXColor(const CRGBA &rgba);

  const CRGBA &getRGBA() const { return rgba_; }

  CXColor(CXScreen &screen);

  CXColor(CXScreen &screen, const CRGB &rgb);
  CXColor(CXScreen &screen, const CRGBA &rgba);

  explicit CXColor(Pixel pixel);

  CXColor(CXScreen &screen, Pixel pixel);
  CXColor(CXScreen &screen, const CRGB &rgb, Pixel pixel);
  CXColor(CXScreen &screen, const CRGBA &rgba, Pixel pixel);

  CXColor(const CXColor &color);

 ~CXColor();

  CXColor &operator=(const CXColor &color) {
    if (&color == this)
      return *this;

    rgba_ = color.rgba_;

    pixel_     = color.pixel_;
    pixel_set_ = color.pixel_set_;

    return *this;
  }

  void setRGB(double r, double g, double b);
  void setRGB(const CRGB &rgb);

  void setRGBA(double r, double g, double b, double a);
  void setRGBA(const CRGBA &rgba);

  void unsetPixel() { pixel_set_ = false; }

  void setPixel(Pixel pixel);

  Pixel getPixel() const;
  Pixel getDarkPixel() const;
  Pixel getLightPixel() const;
  Pixel getInversePixel() const;

  friend bool operator==(const CXColor &lhs, const CXColor &rhs) {
    return lhs.getPixel() == rhs.getPixel();
  }

  friend bool operator!=(const CXColor &lhs, const CXColor &rhs) {
    return lhs.getPixel() != rhs.getPixel();
  }

  friend bool operator<(const CXColor &lhs, const CXColor &rhs) {
    return lhs.getPixel() < rhs.getPixel();
  }

 private:
  void init();
};

#endif