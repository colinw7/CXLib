#ifndef CX_COLOR_H
#define CX_COLOR_H

#include <CRGBA.h>
#include <std_Xt.h>
#include <map>
#include <memory>

class CXScreen;
class CXColor;

class CXColorMgr {
 public:
  CXColorMgr();
  CXColorMgr(CXScreen &screen);
 ~CXColorMgr();

  const CXColor &getCXColor(const CRGB &rgb);
  const CXColor &getCXColor(const CRGBA &rgba);
  const CXColor &getCXColor(Pixel pixel);

 private:
  typedef std::map<Pixel, CXColor *> CXColorMap;

  CXScreen&  screen_;
  CXColorMap colormap_;
};

//------

class CXColor {
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

 private:
  using ColorP = std::unique_ptr<CXColor>;

  CXScreen& screen_;
  CRGBA     rgba_;
  Pixel     pixel_     { 0 };
  bool      pixel_set_ { false };
  ColorP    dark_color_;
  ColorP    light_color_;
  ColorP    inverse_color_;
};

#endif
