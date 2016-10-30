#include <CXLibI.h>
#include <CXrtFont.h>

bool   CXGraphics::error_trapped = false;
XPoint CXGraphics::poly_point[];

CXGraphics::
CXGraphics(Window window) :
 screen_(*CXMachineInst->getCXScreen(0)), window_(window)
{
  init();
}

CXGraphics::
CXGraphics(CXScreen &screen, Window window) :
 screen_(screen), window_(window)
{
  init();
}

CXGraphics::
~CXGraphics()
{
  CXMachineInst->freeGC(gc_);
}

void
CXGraphics::
init()
{
  display_ = screen_.getDisplay();

  is_pixmap_ = isPixmapWindow();

  if (is_pixmap_)
    screen_.flushEvents();

  bg_ = screen_.getWhiteColor();
  fg_ = screen_.getBlackColor();

  gc_ = CXMachineInst->createGC(screen_.getRoot(), fg_, bg_);

  if (! is_pixmap_)
    XSetWindowBackground(display_, window_, bg_.getPixel());

  font_ = CFontMgrInst->lookupFont("courier", CFONT_STYLE_NORMAL, 13);

  pixmap_ = NULL;

  in_double_buffer_ = false;
  fill_complex_     = false;
}

void
CXGraphics::
startDoubleBuffer(bool clear)
{
  if (in_double_buffer_)
    return;

  int width, height;

  getSize(&width, &height);

  bool update = false;

  if (! pixmap_) {
    pixmap_ = CXMachineInst->createPixmap(width, height);

    update = true;
  }
  else
    update = pixmap_->resizePixmap(width, height);

  if (update) {
    CXMachineInst->flushEvents(true);

    clear = true;
  }

  if (clear)
    CXMachineInst->fillRectangle(pixmap_->getPixmap(), gc_,
                                 0, 0, width, height);

  in_double_buffer_ = true;
}

void
CXGraphics::
endDoubleBuffer()
{
  if (! in_double_buffer_)
    return;

  copyDoubleBuffer();

  in_double_buffer_ = false;
}

void
CXGraphics::
copyDoubleBuffer()
{
  if (! pixmap_)
    return;

  CXMachineInst->copyArea(pixmap_->getPixmap(), window_, gc_, 0, 0,
                          pixmap_->getWidth(), pixmap_->getHeight(), 0, 0);

  CXMachineInst->flushEvents(true);
}

void
CXGraphics::
setXor()
{
  CXMachineInst->freeGC(gc_);

  gc_ = CXMachineInst->createXorGC(screen_.getRoot(), fg_, bg_);
}

void
CXGraphics::
clear(bool redraw)
{
  CXMachineInst->setForeground(gc_, bg_);

  if (! is_pixmap_) {
    if (pixmap_ != NULL) {
      CXMachineInst->fillRectangle(pixmap_->getPixmap(), gc_, 0, 0,
                                   pixmap_->getWidth(), pixmap_->getHeight());

      if (redraw)
        XClearArea(display_, window_, 0, 0, 1, 1, True);
    }
    else {
      XSetWindowBackground(display_, window_, bg_.getPixel());

      if (redraw) {
        int width, height;

        getSize(&width, &height);

        XClearArea(display_, window_, 0, 0, width, height, True);
      }
      else
        CXMachineInst->clearWindow(window_);
    }
  }
  else {
    int width, height;

    getSize(&width, &height);

    CXMachineInst->fillRectangle(window_, gc_, 0, 0, width, height);
  }

  CXMachineInst->setForeground(gc_, fg_);
}

void
CXGraphics::
fill()
{
  int width, height;

  getSize(&width, &height);

  if (pixmap_ != NULL)
    CXMachineInst->fillRectangle(pixmap_->getPixmap(), gc_,
                                 0, 0, width, height);
  else
    CXMachineInst->fillRectangle(window_, gc_, 0, 0, width, height);
}

void
CXGraphics::
setForeground(const CRGB &rgb)
{
  setForeground(CXColor(screen_, rgb));
}

void
CXGraphics::
setForeground(const CRGBA &rgba)
{
  // TODO: use id && pixel
  if (fg_.getRGBA() == rgba)
    return;

  setForeground(CXColor(screen_, rgba));
}

void
CXGraphics::
setForeground(Pixel pixel)
{
  setForeground(CXColor(screen_, pixel));
}

void
CXGraphics::
setForeground(const CXColor &color)
{
  fg_ = color;

  CXMachineInst->setForeground(gc_, fg_.getPixel());
}

void
CXGraphics::
setBackground(const CRGB &rgb)
{
  setBackground(CXColor(screen_, rgb));
}

void
CXGraphics::
setBackground(const CRGBA &rgba)
{
  setBackground(CXColor(screen_, rgba));
}

void
CXGraphics::
setBackground(Pixel pixel)
{
  setBackground(CXColor(screen_, pixel));
}

void
CXGraphics::
setBackground(const CXColor &color)
{
  bg_ = color;

  CXMachineInst->setBackground(gc_, bg_.getPixel());

  if (! is_pixmap_)
    XSetWindowBackground(display_, window_, bg_.getPixel());
}

void
CXGraphics::
getForeground(CRGB &rgb)
{
  rgb = fg_.getRGBA().getRGB();
}

void
CXGraphics::
getForeground(CRGBA &rgb)
{
  rgb = fg_.getRGBA();
}

void
CXGraphics::
getForegroundPixel(Pixel &pixel)
{
  pixel = fg_.getPixel();
}

void
CXGraphics::
getBackground(CRGB &rgb)
{
  rgb = bg_.getRGBA().getRGB();
}

void
CXGraphics::
getBackground(CRGBA &rgba)
{
  rgba = bg_.getRGBA();
}

void
CXGraphics::
getBackgroundPixel(Pixel &pixel)
{
  pixel = bg_.getPixel();
}

void
CXGraphics::
setFont(CFontPtr font)
{
  font_ = font;
}

void
CXGraphics::
drawLine(int x1, int y1, int x2, int y2)
{
  if (pixmap_ != NULL)
    CXMachineInst->drawLine(pixmap_->getPixmap(), gc_, x1, y1, x2, y2);
  else
    CXMachineInst->drawLine(window_, gc_, x1, y1, x2, y2);
}

void
CXGraphics::
drawRectangle(int x, int y, int width, int height)
{
  if (pixmap_ != NULL)
    CXMachineInst->drawRectangle(pixmap_->getPixmap(), gc_,
                                 x, y, (uint) width, (uint) height);
  else
    CXMachineInst->drawRectangle(window_, gc_,
                                 x, y, (uint) width, (uint) height);
}

void
CXGraphics::
fillRectangle(int x, int y, int width, int height)
{
  if (pixmap_ != NULL)
    CXMachineInst->fillRectangle(pixmap_->getPixmap(), gc_,
                                 x, y, width, height);
  else
    CXMachineInst->fillRectangle(window_, gc_, x, y, width, height);
}

void
CXGraphics::
drawPolygon(int *x, int *y, int num_xy)
{
  if (num_xy < 3)
    return;

  if (num_xy > MAX_POLY_POINTS) {
    CTHROW("Too many points in Polygon");
    return;
  }

  for (int i = 0; i < num_xy; ++i) {
    poly_point[i].x = x[i];
    poly_point[i].y = y[i];
  }

  if (pixmap_ != NULL)
    XDrawLines(display_, pixmap_->getPixmap(), gc_,
               poly_point, num_xy, CoordModeOrigin);
  else
    XDrawLines(display_, window_, gc_,
               poly_point, num_xy, CoordModeOrigin);
}

void
CXGraphics::
fillPolygon(int *x, int *y, int num_xy)
{
  if (num_xy < 3)
    return;

  if (num_xy > MAX_POLY_POINTS) {
    CTHROW("Too many points in Polygon");
    return;
  }

  for (int i = 0; i < num_xy; ++i) {
    poly_point[i].x = x[i];
    poly_point[i].y = y[i];
  }

  if (pixmap_ != NULL)
    XFillPolygon(display_, pixmap_->getPixmap(), gc_,
                 poly_point, num_xy,
                 fill_complex_ ? Complex : Convex,
                 CoordModeOrigin);
  else
    XFillPolygon(display_, window_, gc_,
                 poly_point, num_xy,
                 fill_complex_ ? Complex : Convex,
                 CoordModeOrigin);
}

void
CXGraphics::
drawCircle(int x, int y, int r)
{
  if (pixmap_ != NULL)
    XDrawArc(display_, pixmap_->getPixmap(), gc_,
             x - r, y - r, 2*r, 2*r, 0, 360*64);
  else
    XDrawArc(display_, window_, gc_, x - r, y - r, 2*r, 2*r, 0, 360*64);
}

void
CXGraphics::
fillCircle(int x, int y, int r)
{
  if (pixmap_ != NULL)
    XFillArc(display_, pixmap_->getPixmap(), gc_,
             x - r, y - r, 2*r, 2*r, 0, 360*64);
  else
    XFillArc(display_, window_, gc_, x - r, y - r, 2*r, 2*r, 0, 360*64);
}

void
CXGraphics::
drawEllipse(int x, int y, int xr, int yr)
{
  if (pixmap_ != NULL)
    XDrawArc(display_, pixmap_->getPixmap(), gc_,
             x - xr, y - yr, 2*xr, 2*yr, 0, 360*64);
  else
    XDrawArc(display_, window_, gc_, x - xr, y - yr, 2*xr, 2*yr, 0, 360*64);
}

void
CXGraphics::
fillEllipse(int x, int y, int xr, int yr)
{
  if (pixmap_ != NULL)
    XFillArc(display_, pixmap_->getPixmap(), gc_,
             x - xr, y - yr, 2*xr, 2*yr, 0, 360*64);
  else
    XFillArc(display_, window_, gc_, x - xr, y - yr, 2*xr, 2*yr, 0, 360*64);
}

void
CXGraphics::
drawArc(int x, int y, int xr, int yr, double angle1, double angle2)
{
  if (pixmap_ != NULL)
    XDrawArc(display_, pixmap_->getPixmap(), gc_,
             x - xr, y - yr, 2*xr, 2*yr,
             (int) (angle1*64), (int) (-angle2*64));
  else
    XDrawArc(display_, window_, gc_, x - xr, y - yr, 2*xr, 2*yr,
             (int) (angle1*64), (int) (-angle2*64));
}

void
CXGraphics::
fillArc(int x, int y, int xr, int yr, double angle1, double angle2)
{
  if (pixmap_ != NULL)
    XFillArc(display_, pixmap_->getPixmap(), gc_,
             x - xr, y - yr, 2*xr, 2*yr,
             (int) (angle1*64), (int) (-angle2*64));
  else
    XFillArc(display_, window_, gc_, x - xr, y - yr, 2*xr, 2*yr,
             (int) (angle1*64), (int) (-angle2*64));
}

void
CXGraphics::
drawPoint(int x, int y)
{
  if (pixmap_ != NULL)
    CXMachineInst->drawPoint(pixmap_->getPixmap(), gc_, x, y);
  else
    CXMachineInst->drawPoint(window_, gc_, x, y);
}

void
CXGraphics::
drawImage(const CImagePtr &image, int x, int y)
{
  if (pixmap_ != NULL)
    CXMachineInst->drawImage(pixmap_->getPixmap(), gc_, image, x, y);
  else
    CXMachineInst->drawImage(window_, gc_, image, x, y);
}

void
CXGraphics::
drawSubImage(const CImagePtr &image, int src_x, int src_y,
             int dst_x, int dst_y, int width, int height)
{
  if (pixmap_ != NULL)
    CXMachineInst->drawImage(pixmap_->getPixmap(), gc_, image,
                             src_x, src_y, dst_x, dst_y, width, height);
  else
    CXMachineInst->drawImage(window_, gc_, image,
                             src_x, src_y, dst_x, dst_y, width, height);
}

void
CXGraphics::
drawSubImage(XImage *ximage, int src_x, int src_y, int dst_x, int dst_y,
             int width, int height)
{
  if (pixmap_ != NULL)
    CXMachineInst->putImage(pixmap_->getPixmap(), gc_, ximage, src_x, src_y,
                            dst_x, dst_y, width, height);
  else
    CXMachineInst->putImage(window_, gc_, ximage, src_x, src_y,
                            dst_x, dst_y, width, height);
}

void
CXGraphics::
drawAlphaImage(const CImagePtr &image, int x, int y)
{
  if (pixmap_ != NULL) {
    //CXMachineInst->drawImage(pixmap_->getPixmap(), gc_, image, x, y);

    uint width  = image->getWidth ();
    uint height = image->getHeight();

    for (uint iy = 0; iy < height; ++iy) {
      for (uint ix = 0; ix < width; ++ix) {
        if (! image->isTransparent(ix, iy)) {
          CRGBA rgba;

          image->getRGBAPixel(ix, iy, rgba);

          setForeground(rgba);

          drawPoint(x + ix, y + iy);
        }
      }
    }
  }
  else {
    uint width  = image->getWidth ();
    uint height = image->getHeight();

    for (uint iy = 0; iy < height; ++iy) {
      for (uint ix = 0; ix < width; ++ix) {
        if (! image->isTransparent(ix, iy)) {
          CRGBA rgba;

          image->getRGBAPixel(ix, iy, rgba);

          setForeground(rgba);

          drawPoint(x + ix, y + iy);
        }
      }
    }
  }
}

void
CXGraphics::
drawSubAlphaImage(const CImagePtr &image, int src_x, int src_y,
                  int dst_x, int dst_y, int width, int height)
{
  if (! image->isTransparent(COptReal(0.5)))
    return drawSubImage(image, src_x, src_y, dst_x, dst_y, width, height);

  CRGBA rgba;

  src_x  = std::max(src_x , 0);
  src_y  = std::max(src_y , 0);
  width  = std::min(width , (int) image->getWidth ());
  height = std::min(height, (int) image->getHeight());

  int xx, yy;

  yy = dst_y;

  for (int y1 = src_y; y1 < height; ++y1, ++yy) {
    xx = dst_x;

    for (int x1 = src_x; x1 < width; ++x1, ++xx) {
      image->getRGBAPixel(x1, y1, rgba);

      if (rgba.getAlpha() >= 0.5) {
        setForeground(rgba);

        drawPoint(xx, yy);
      }
    }
  }
}

void
CXGraphics::
drawSubAlphaImage(XImage *image, int src_x, int src_y, int dst_x, int dst_y,
                  int width, int height)
{
  CImagePtr pimage = CImagePtr(new CXImage(image));

  drawSubAlphaImage(pimage, src_x, src_y, dst_x, dst_y, width, height);
}

bool
CXGraphics::
getImage(int x, int y, int width, int height, CImagePtr &image)
{
  XImage *ximage;

  if (! getImage(x, y, width, height, &ximage))
    return false;

  if (ximage == NULL)
    return false;

  CXImage *cimage = new CXImage(ximage);

  image = CImagePtr(cimage);

  return true;
}

bool
CXGraphics::
getImage(int x, int y, int width, int height, XImage **ximage)
{
  *ximage = NULL;

  if (pixmap_ != NULL)
    *ximage = XGetImage(display_, pixmap_->getPixmap(), x, y, width, height,
                        AllPlanes, ZPixmap);
  else
    *ximage = XGetImage(display_, window_, x, y, width, height,
                        AllPlanes, ZPixmap);

  return true;
}

void
CXGraphics::
drawText(int x, int y, const string &str)
{
  if (! font_.isValid()) {
    cerr << "Bad Font" << endl;
    return;
  }

  CXFont *xfont = font_.cast<CXFont>();

  if (xfont == NULL)
    return;

  CXrtFont *xrt_font = xfont->getXrtFont();

  if (pixmap_ != NULL)
    xrt_font->draw(pixmap_->getPixmap(), gc_, x, y, str);
  else
    xrt_font->draw(window_, gc_, x, y, str);
}

#if 0
void
CXGraphics::
drawTextImage(int x, int y, const string &str)
{
  if (! font_.isValid()) {
    cerr << "Bad Font" << endl;
    return;
  }

  CXFont *xfont = font_.cast<CXFont>();

  if (xfont == NULL)
    return;

  CXrtFont *xrt_font = xfont->getXrtFont();

  if (pixmap_ != NULL)
    xrt_font->drawImage(pixmap_->getPixmap(), gc_, x, y, str);
  else
    xrt_font->drawImage(window_, gc_, x, y, str);
}
#endif

void
CXGraphics::
startClip(int x, int y, int width, int height)
{
  XPoint points[4];

  points[0].x = x;
  points[0].y = y;
  points[2].x = x + width  - 1;
  points[2].y = y + height - 1;

  points[1].x = points[2].x;
  points[1].y = points[0].y;
  points[3].x = points[0].x;
  points[3].y = points[2].y;

  Region region = XPolygonRegion(points, 4, EvenOddRule);

  XSetRegion(display_, gc_, region);

  XDestroyRegion(region);
}

void
CXGraphics::
startClip(Pixmap pixmap, int dx, int dy)
{
  XSetClipMask  (display_, gc_, pixmap);
  XSetClipOrigin(display_, gc_, dx, dy);
}

void
CXGraphics::
endClip()
{
  XSetClipMask  (display_, gc_, None);
  XSetClipOrigin(display_, gc_, 0, 0);
}

void
CXGraphics::
copyArea(const CXGraphics &src, int src_x, int src_y,
         int dst_x, int dst_y, int width, int height)
{
  if (pixmap_ != NULL)
    CXMachineInst->copyArea(src.getXWindow(), pixmap_->getPixmap(), gc_,
                            src_x, src_y, width, height, dst_x, dst_y);
  else
    CXMachineInst->copyArea(src.getXWindow(), window_, gc_,
                            src_x, src_y, width, height, dst_x, dst_y);
}

void
CXGraphics::
setLineType(CXLineType line_type)
{
  XGCValues gc_values;

  if (line_type == CX_LINE_TYPE_SOLID)
    gc_values.line_style = LineSolid;
  else
    gc_values.line_style = LineOnOffDash;

  XChangeGC(display_, gc_, GCLineStyle, &gc_values);
}

void
CXGraphics::
setLineWidth(int line_width)
{
  XGCValues gc_values;

  gc_values.line_width = line_width;

  XChangeGC(display_, gc_, GCLineWidth, &gc_values);
}

void
CXGraphics::
setLineDash(int offset, char *dashes, int num_dashes)
{
  if (num_dashes > 0) {
    setLineType(CX_LINE_TYPE_DASHED);

    XSetDashes(display_, gc_, offset, dashes, num_dashes);
  }
  else
    setLineType(CX_LINE_TYPE_SOLID);
}

void
CXGraphics::
setLineDash(int offset, int *dashes, int num_dashes)
{
  if (num_dashes > 0) {
    static char cdashes[256];

    for (int i = 0; i< num_dashes; ++i)
      cdashes[i] = dashes[i];

    setLineType(CX_LINE_TYPE_DASHED);

    XSetDashes(display_, gc_, offset, cdashes, num_dashes);
  }
  else
    setLineType(CX_LINE_TYPE_SOLID);
}

void
CXGraphics::
setLineDash(const CILineDash &line_dash)
{
  setLineDash(line_dash.getOffset(), line_dash.getLengths(),
              line_dash.getNumLengths());
}

void
CXGraphics::
setFillComplex(bool comp)
{
  fill_complex_ = comp;
}

bool
CXGraphics::
isPixmapWindow() const
{
  error_trapped = false;

  XErrorHandler oldErrorHandler = XSetErrorHandler(newErrorHandler);

  XWindowAttributes xwinattr;

  XGetWindowAttributes(display_, window_, &xwinattr);

  CXMachineInst->flushEvents(false);

  XSetErrorHandler(oldErrorHandler);

  return error_trapped;
}

void
CXGraphics::
getSize(int *width, int *height) const
{
  XErrorHandler oldErrorHandler = XSetErrorHandler(newErrorHandler);

  *width  = 1;
  *height = 1;

  if (! is_pixmap_) {
    XWindowAttributes xwinattr;

    XGetWindowAttributes(display_, window_, &xwinattr);

    *width  = xwinattr.width;
    *height = xwinattr.height;
  }
  else {
    int    x;
    int    y;
    Window root;
    uint   depth;
    uint   width1;
    uint   height1;
    uint   border_width;

    XGetGeometry(display_, window_, &root, &x, &y, &width1, &height1,
                 &border_width, &depth);

    *width  = width1;
    *height = height1;
  }

  CXMachineInst->flushEvents(false);

  XSetErrorHandler(oldErrorHandler);
}

int
CXGraphics::
getCharWidth()
{
  if (font_.isValid())
    return font_->getICharWidth();
  else
    return 8;
}

int
CXGraphics::
getCharHeight()
{
  if (font_.isValid())
    return font_->getICharHeight();
  else
    return 10;
}

int
CXGraphics::
getStringWidth(const string &str)
{
  if (font_.isValid())
    return font_->getIStringWidth(str);
  else
    return getCharWidth()*str.size();
}

void
CXGraphics::
flushEvents()
{
  screen_.flushEvents();
}

int
CXGraphics::
newErrorHandler(Display *, XErrorEvent *)
{
  error_trapped = true;

  return 0;
}
