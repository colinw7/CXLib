#include <CXLibI.h>

CXDrawable::
CXDrawable()
{
}

CXDrawable::
CXDrawable(Drawable drawable, uint width, uint height)
{
  setDrawable(drawable, width, height);
}

CXDrawable::
~CXDrawable()
{
  delete graphics_;
}

void
CXDrawable::
setDrawable(Drawable drawable, uint width, uint height)
{
  width_    = width;
  height_   = height;
  drawable_ = drawable;

  graphics_ = new CXGraphics(drawable_);
}

void
CXDrawable::
getFont(CFontPtr &font) const
{
  graphics_->getFont(font);
}

void
CXDrawable::
setFont(CFontPtr font)
{
  graphics_->setFont(font);
}

void
CXDrawable::
startDoubleBuffer(bool clear)
{
  graphics_->startDoubleBuffer(clear);
}

void
CXDrawable::
endDoubleBuffer()
{
  graphics_->endDoubleBuffer();
}

void
CXDrawable::
copyDoubleBuffer()
{
  graphics_->copyDoubleBuffer();
}

void
CXDrawable::
clear()
{
  graphics_->clear(false);
}

void
CXDrawable::
fill()
{
  graphics_->fill();
}

void
CXDrawable::
setForeground(const CRGB &rgb)
{
  graphics_->setForeground(rgb);
}

void
CXDrawable::
setForeground(const CRGBA &rgba)
{
  graphics_->setForeground(rgba);
}

void
CXDrawable::
setForeground(const CXColor &color)
{
  graphics_->setForeground(color);
}

void
CXDrawable::
setBackground(const CRGB &rgb)
{
  graphics_->setBackground(rgb);
}

void
CXDrawable::
setBackground(const CRGBA &rgba)
{
  graphics_->setBackground(rgba);
}

void
CXDrawable::
setBackground(const CXColor &color)
{
  graphics_->setBackground(color);
}

void
CXDrawable::
getForeground(CRGB &rgb)
{
  graphics_->getForeground(rgb);
}

void
CXDrawable::
getBackground(CRGB &rgb)
{
  graphics_->getBackground(rgb);
}

void
CXDrawable::
drawLine(int x1, int y1, int x2, int y2)
{
  graphics_->drawLine(x1, y1, x2, y2);
}

void
CXDrawable::
drawRectangle(int x, int y, int width, int height)
{
  graphics_->drawRectangle(x, y, width, height);
}

void
CXDrawable::
fillRectangle(int x, int y, int width, int height)
{
  graphics_->fillRectangle(x, y, width, height);
}

void
CXDrawable::
drawPolygon(int *x, int *y, uint num_xy)
{
  graphics_->drawPolygon(x, y, num_xy);
}

void
CXDrawable::
fillPolygon(int *x, int *y, uint num_xy)
{
  graphics_->fillPolygon(x, y, num_xy);
}

void
CXDrawable::
drawCircle(int x, int y, int r)
{
  graphics_->drawCircle(x, y, r);
}

void
CXDrawable::
fillCircle(int x, int y, int r)
{
  graphics_->fillCircle(x, y, r);
}

void
CXDrawable::
drawEllipse(int x, int y, int xr, int yr)
{
  graphics_->drawEllipse(x, y, xr, yr);
}

void
CXDrawable::
fillEllipse(int x, int y, int xr, int yr)
{
  graphics_->fillEllipse(x, y, xr, yr);
}

void
CXDrawable::
drawArc(int x, int y, int xr, int yr, double a1, double a2)
{
  graphics_->drawArc(x, y, xr, yr, a1, a2);
}

void
CXDrawable::
fillArc(int x, int y, int xr, int yr, double a1, double a2)
{
  graphics_->fillArc(x, y, xr, yr, a1, a2);
}

void
CXDrawable::
drawPoint(int x, int y)
{
  graphics_->drawPoint(x, y);
}

void
CXDrawable::
drawImage(const CImagePtr &image, int x, int y)
{
  graphics_->drawImage(image, x, y);
}

void
CXDrawable::
drawSubImage(const CImagePtr &image, int src_x, int src_y,
             int dst_x, int dst_y, int width1, int height1)
{
  CXImage *ximage = image.cast<CXImage>();

  XImage *ximg = ximage->getXImage();

  int width2  = image->getWidth();
  int height2 = image->getHeight();

  if (src_x + width1 > width2)
    width1 = width2 - src_x;

  if (src_y + height1 > height2)
    height1 = height2 - src_y;

  graphics_->drawSubImage(ximg,
                          src_x, src_y,
                          dst_x, dst_y,
                          width1, height1);
}

void
CXDrawable::
drawAlphaImage(const CImagePtr &image, int x, int y)
{
  graphics_->drawAlphaImage(image, x, y);
}

void
CXDrawable::
drawText(int x, int y, const std::string &str)
{
  graphics_->drawText(x, y, str);
}

void
CXDrawable::
setLineType(CXLineType line_type)
{
  graphics_->setLineType(line_type);
}

void
CXDrawable::
setLineWidth(int line_width)
{
  graphics_->setLineWidth(line_width);
}

void
CXDrawable::
setLineDash(int offset, char *dashes, uint num_dashes)
{
  graphics_->setLineDash(offset, dashes, num_dashes);
}

void
CXDrawable::
setLineDash(int offset, int *dashes, uint num_dashes)
{
  graphics_->setLineDash(offset, dashes, num_dashes);
}

void
CXDrawable::
setLineDash(const CILineDash &line_dash)
{
  graphics_->setLineDash(line_dash);
}

void
CXDrawable::
setFillComplex(bool comp)
{
  graphics_->setFillComplex(comp);
}

int
CXDrawable::
getCharWidth()
{
  return graphics_->getCharWidth();
}

int
CXDrawable::
getCharHeight()
{
  return graphics_->getCharHeight();
}

uint
CXDrawable::
getStringWidth(const std::string &str)
{
  return graphics_->getStringWidth(str);
}

CImagePtr
CXDrawable::
getImage()
{
  return getImage(0, 0, getWidth(), getHeight());
}

CImagePtr
CXDrawable::
getImage(int x, int y, uint width, uint height)
{
  if (! graphics_)
    return CImagePtr();

  CImagePtr image;

  graphics_->getImage(x, y, width, height, image);

  return image;
}
