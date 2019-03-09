#include <CXColor.h>
#include <CXMachine.h>
#include <CXScreen.h>

CXColorMgr::
CXColorMgr() :
 screen_(*CXMachineInst->getCXScreen(0))
{
}

CXColorMgr::
CXColorMgr(CXScreen &screen) :
 screen_(screen)
{
}

CXColorMgr::
~CXColorMgr()
{
  CXColorMap::iterator pcolor1 = colormap_.begin();
  CXColorMap::iterator pcolor2 = colormap_.end();

  for ( ; pcolor1 != pcolor2; ++pcolor1)
    delete (*pcolor1).second;
}

const CXColor &
CXColorMgr::
getCXColor(const CRGB &rgb)
{
  CRGBA rgba(rgb);

  return getCXColor(rgba);
}

const CXColor &
CXColorMgr::
getCXColor(const CRGBA &rgba)
{
  Pixel pixel = screen_.rgbaToPixel(rgba);

  CXColorMap::iterator pcolor1 = colormap_.find(pixel);
  CXColorMap::iterator pcolor2 = colormap_.end();

  if (pcolor1 != pcolor2)
    return *(*pcolor1).second;

  CXColor *color = new CXColor(screen_, rgba, pixel);

  colormap_[pixel] = color;

  return *color;
}

const CXColor &
CXColorMgr::
getCXColor(Pixel pixel)
{
  CXColorMap::iterator pcolor1 = colormap_.find(pixel);
  CXColorMap::iterator pcolor2 = colormap_.end();

  if (pcolor1 != pcolor2)
    return *(*pcolor1).second;

  CXColor *color = new CXColor(screen_, pixel);

  colormap_[pixel] = color;

  return *color;
}

CXColor::
CXColor() :
 screen_(*CXMachineInst->getCXScreen(0))
{
  init();
}

CXColor::
CXColor(const CRGB &rgb) :
 screen_(*CXMachineInst->getCXScreen(0)), rgba_(rgb)
{
  init();
}

CXColor::
CXColor(const CRGBA &rgba) :
 screen_(*CXMachineInst->getCXScreen(0)), rgba_(rgba)
{
  init();
}

CXColor::
CXColor(CXScreen &screen) :
 screen_(screen)
{
  init();
}

CXColor::
CXColor(CXScreen &screen, const CRGB &rgb) :
 screen_(screen), rgba_(rgb)
{
  init();
}

CXColor::
CXColor(CXScreen &screen, const CRGBA &rgba) :
 screen_(screen), rgba_(rgba)
{
  init();
}

CXColor::
CXColor(Pixel pixel) :
 screen_(*CXMachineInst->getCXScreen(0))
{
  setPixel(pixel);
}

CXColor::
CXColor(CXScreen &screen, Pixel pixel) :
 screen_(screen)
{
  setPixel(pixel);
}

CXColor::
CXColor(CXScreen &screen, const CRGB &rgb, Pixel pixel) :
 screen_(screen), rgba_(rgb), pixel_(pixel)
{
  init();

  pixel_set_ = true;
}

CXColor::
CXColor(CXScreen &screen, const CRGBA &rgba, Pixel pixel) :
 screen_(screen), rgba_(rgba), pixel_(pixel)
{
  init();

  pixel_set_ = true;
}

CXColor::
CXColor(const CXColor &color) :
 screen_(color.screen_), rgba_(color.rgba_)
{
  init();

  pixel_     = color.pixel_;
  pixel_set_ = color.pixel_set_;
}

CXColor::
~CXColor()
{
}

void
CXColor::
init()
{
  pixel_set_ = false;

  dark_color_    = nullptr;
  light_color_   = nullptr;
  inverse_color_ = nullptr;
}

void
CXColor::
setRGB(double r, double g, double b)
{
  init();

  rgba_.setRGB(r, g, b);
}

void
CXColor::
setRGB(const CRGB &rgb)
{
  init();

  rgba_.setRGB(rgb);
}

void
CXColor::
setRGBA(double r, double g, double b, double a)
{
  init();

  rgba_.setRGBA(r, g, b, a);
}

void
CXColor::
setRGBA(const CRGBA &rgba)
{
  init();

  rgba_.setRGBA(rgba);
}

void
CXColor::
setPixel(Pixel pixel)
{
  init();

  pixel_     = pixel;
  pixel_set_ = true;

  rgba_.setRGBA(screen_.pixelToRGBA(pixel_));
}

Pixel
CXColor::
getPixel() const
{
  if (! pixel_set_) {
    CXColor *th = const_cast<CXColor *>(this);

    th->pixel_ = screen_.rgbaToPixel(rgba_);

    th->pixel_set_ = true;
  }

  return pixel_;
}

Pixel
CXColor::
getDarkPixel() const
{
  if (! dark_color_) {
    CXColor *th = const_cast<CXColor *>(this);

    th->dark_color_ = std::make_unique<CXColor>(screen_, rgba_.getDarkRGBA());
  }

  return dark_color_->getPixel();
}

Pixel
CXColor::
getLightPixel() const
{
  if (! light_color_) {
    CXColor *th = const_cast<CXColor *>(this);

    th->light_color_ = std::make_unique<CXColor>(screen_, rgba_.getLightRGBA());
  }

  return light_color_->getPixel();
}

Pixel
CXColor::
getInversePixel() const
{
  if (! inverse_color_) {
    CXColor *th = const_cast<CXColor *>(this);

    th->inverse_color_ = std::make_unique<CXColor>(screen_, rgba_.getInverseRGBA());
  }

  return inverse_color_->getPixel();
}
