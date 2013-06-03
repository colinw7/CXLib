#include "CXLibI.h"

void
CXImage::
setPrototype()
{
  CImagePtr image(new CXImage());

  CImageMgrInst->setPrototype(image);
}

CXImage::
CXImage(CXScreen &screen) :
 screen_(screen)
{
  init();
}

CXImage::
CXImage() :
 screen_(*CXMachineInst->getCXScreen(0))
{
  init();
}

CXImage::
CXImage(CXScreen &screen, int width, int height) :
 CImage(width, height), screen_(screen)
{
  init();
}

CXImage::
CXImage(int width, int height) :
 CImage(width, height), screen_(*CXMachineInst->getCXScreen(0))
{
  init();
}

CXImage::
CXImage(CXScreen &screen, const CXImage &ximage) :
 CImage(ximage), screen_(screen)
{
  init();
}

CXImage::
CXImage(const CXImage &ximage) :
 CImage(ximage), screen_(*CXMachineInst->getCXScreen(0))
{
  init();
}

CXImage::
CXImage(CXScreen &screen, const CImage &image) :
 CImage(image), screen_(screen)
{
  init();
}

CXImage::
CXImage(const CImage &image) :
 CImage(image), screen_(*CXMachineInst->getCXScreen(0))
{
  init();
}

CXImage::
CXImage(CXScreen &screen, Drawable drawable, int x, int y, int width, int height) :
 CImage(width, height), screen_(screen)
{
  init();

  getXImage(drawable, x, y, width, height);
}

CXImage::
CXImage(Drawable drawable, int x, int y, int width, int height) :
 CImage(width, height), screen_(*CXMachineInst->getCXScreen(0))
{
  init();

  getXImage(drawable, x, y, width, height);
}

CXImage::
CXImage(CXScreen &screen, XImage *ximage) :
 CImage(ximage->width, ximage->height), screen_(screen)
{
  init();

  ximage_ = ximage;
}

CXImage::
CXImage(XImage *ximage) :
 CImage(ximage->width, ximage->height), screen_(*CXMachineInst->getCXScreen(0))
{
  init();

  ximage_ = ximage;

  createImageData();
}

CXImage::
~CXImage()
{
  reset();
}

CXImage &
CXImage::
operator=(const CXImage &image)
{
  CImage::operator=(image);

  reset();

  return *this;
}

CXImage &
CXImage::
operator=(const CImage &image)
{
  CImage::operator=(image);

  reset();

  return *this;
}

CImagePtr
CXImage::
dup() const
{
  CXImage *ximage = new CXImage(*this);

  return CImagePtr(ximage);
}

void
CXImage::
init()
{
  xdata_        = NULL;
  ximage_       = NULL;
  ximage_owner_ = false;
  pixmap_       = None;
  mask_         = None;
}

void
CXImage::
reset()
{
  delete [] xdata_;

  if (ximage_ != NULL) {
    if (ximage_owner_) {
      ximage_->data = NULL;

      XDestroyImage(ximage_);
    }
  }

  if (mask_ != None)
    XFreePixmap(screen_.getDisplay(), mask_);

  if (pixmap_ != None)
    XFreePixmap(screen_.getDisplay(), pixmap_);

  init();
}

void
CXImage::
getXImage(Drawable drawable, int x, int y, int width, int height)
{
  Display *display = screen_.getDisplay();

  ximage_ = XGetImage(display, drawable, x, y, width, height, AllPlanes, ZPixmap);

  ximage_owner_ = true;
}

void
CXImage::
createXImage()
{
  Display *display = screen_.getDisplay();
  Visual  *visual  = screen_.getVisual ();

  int format = ZPixmap;

  if (screen_.getDepth() == 1)
    format = XYBitmap;

  int xoffset = 0;

  int size = getDataSize();

  xdata_ = new uchar [size];

  int pad = BitmapPad(display);

  int bytes_per_line = getRowSize();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  int width  = std::max(1, x2 - x1 + 1);
  int height = std::max(1, y2 - y1 + 1);

  ximage_ = XCreateImage(display, visual, screen_.getDepth(), format, xoffset,
                         (char *) xdata_, width, height, pad, bytes_per_line);

  if (ximage_ == NULL)
    cerr << "Failed to create ximage" << endl;

  ximage_owner_ = true;

  //-----

  initXImage();
}

void
CXImage::
initXImage()
{
  if (ximage_ == NULL)
    return;

  CRGBA rgba;

  bool fast = (screen_.getDepth() == 32);

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  Pixel pixel;

  int width = getWidth();

  if (! hasColormap()) {
    int ind = y1*width;

    for (int y = y1; y <= y2; ++y) {
      int ind1 = ind + x1;

      for (int x = x1; x <= x2; ++x) {
        if (validPixel(x, y)) {
          if (fast)
            pixel = getData(ind1);
          else {
            getRGBAPixel(ind1, rgba);

            pixel = screen_.rgbaToPixel(rgba);
          }
        }
        else
          pixel = screen_.getBlackPixel();

        XPutPixel(ximage_, x - x1, y - y1, pixel);

        ++ind1;
      }

      ind += width;
    }
  }
  else {
    vector<Pixel> pixels;

    int num = getNumColors();

    CRGBA rgba;

    for (int i = 0; i < num; ++i) {
      getColorRGBA(i, rgba);

      pixels.push_back(CXColor(rgba).getPixel());
    }

    //------

    int ind = y1*width;

    for (int y = y1; y <= y2; ++y) {
      int ind1 = ind + x1;

      for (int x = x1; x <= x2; ++x) {
        if (validPixel(x, y))
          pixel = getColorIndexPixel(ind1);
        else
          pixel = 0;

        XPutPixel(ximage_, x - x1, y - y1, pixels[pixel]);

        ++ind1;
      }

      ind += width;
    }
  }
}

//----------------

int
CXImage::
getDepth() const
{
  return screen_.getDepth();
}

XImage *
CXImage::
getXImage() const
{
  if (ximage_ == NULL) {
    CXImage *th = const_cast<CXImage *>(this);

    th->createXImage();
  }

  return ximage_;
}

Pixmap
CXImage::
getXPixmap() const
{
  if (pixmap_ == None) {
    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    uint width  = x2 - x1 + 1;
    uint height = y2 - y1 + 1;

    //----

    CXImage *th = const_cast<CXImage *>(this);

    th->pixmap_ = CXMachineInst->createXPixmap(width, height);

    if (pixmap_ == None)
      return None;

    //----

    GC gc = CXMachineInst->createGC(pixmap_, 0, 0);

    if (ximage_ != NULL) {
      Pixel pixel;

      for (uint y = 0; y < height; ++y) {
        for (uint x = 0; x < width; ++x) {
          pixel = XGetPixel(ximage_, x, y);

          CXMachineInst->setForeground(gc, pixel);

          CXMachineInst->drawPoint(pixmap_, gc, x, y);
        }
      }
    }
    else {
      CXMachineInst->setForeground(gc, 0);

      for (uint y = 0; y < height; ++y) {
        for (uint x = 0; x < width; ++x) {
          CXMachineInst->drawPoint(pixmap_, gc, x, y);
        }
      }
    }

    CXMachineInst->freeGC(gc);
  }

  return pixmap_;
}

Pixmap
CXImage::
getXMask() const
{
  if (mask_ == None) {
    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    int width = getWidth();

    uint pwidth  = x2 - x1 + 1;
    uint pheight = y2 - y1 + 1;

    //----

    CXImage *th = const_cast<CXImage *>(this);

    th->mask_ = CXMachineInst->createXPixmap(pwidth, pheight, 1);

    if (mask_ == None)
      return None;

    //----

    GC gc = CXMachineInst->createGC(mask_, 0, 0);

    CXMachineInst->setForeground(gc, 0);

    CXMachineInst->fillRectangle(mask_, gc, 0, 0, pwidth, pheight);

    CXMachineInst->setForeground(gc, 1);

    double alpha;

    int ind = y1*width;

    for (int y = y1; y <= y2; ++y) {
      int ind1 = ind + x1;

      for (int x = x1; x <= x2; ++x) {
        if (validPixel(x, y)) {
          alpha = getAlpha(ind1);

          if (alpha >= 0.5)
            CXMachineInst->drawPoint(mask_, gc, x - x1, y - y1);
        }

        ++ind1;
      }

      ind += width;
    }

    CXMachineInst->freeGC(gc);
  }

  return mask_;
}

void
CXImage::
createImageData()
{
  CRGB *colors;
  int   num_colors;

  getImageColors(&colors, &num_colors);

  //------

  if (ximage_ == NULL)
    return;

  setDataSize(ximage_->width, ximage_->height);

  //------

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  //----

  if (num_colors > 0) {
    Pixel pixel;

    for (int i = 0; i < num_colors; ++i)
      addColor(colors[i].getRed(), colors[i].getGreen(), colors[i].getBlue());

    for (int iy = 0; iy < ximage_->height; ++iy) {
      for (int ix = 0; ix < ximage_->width; ++ix) {
        pixel = XGetPixel(ximage_, ix, iy);

        CImage::setColorIndexPixel(x1 + ix, y1 + iy, pixel);
      }
    }
  }
  else {
    Pixel pixel;

    for (int iy = 0; iy < ximage_->height; ++iy) {
      for (int ix = 0; ix < ximage_->width; ++ix) {
        pixel = XGetPixel(ximage_, ix, iy);

        pixel |= 0xFF000000;

        setData(x1 + ix, y1 + iy, pixel);
      }
    }
  }
}

void
CXImage::
getImageColors(CRGB **colors, int *num_colors)
{
  if (ximage_ == NULL) {
    *colors     = NULL;
    *num_colors = 0;

    return;
  }

  if (ximage_->depth > 8) {
    *colors     = NULL;
    *num_colors = 0;

    return;
  }

  if (ximage_->depth == 1) {
    *colors     = new CRGB [2];
    *num_colors = 2;

    (*colors)[0] = CRGB(0, 0, 0);
    (*colors)[1] = CRGB(1, 1, 1);

    return;
  }

  Display  *display = screen_.getDisplay();
  Visual   *visual  = screen_.getVisual();
  Colormap  cmap    = screen_.getColormap();

  int num_xcolors = visual->map_entries;

  vector<XColor> xcolors;

  xcolors.resize(num_xcolors);

  for (int i = 0; i < num_xcolors; ++i)
    xcolors[i].pixel = (uint) i;

  XQueryColors(display, cmap, &xcolors[0], num_xcolors);

  vector<int> used;

  used.resize(num_xcolors);

  int num_used = 0;

  for (int i = 0; i < num_xcolors; ++i)
    used[i] = 0;

  Pixel pixel;

  for (int y = 0; y < ximage_->height; ++y) {
    for (int x = 0; x < ximage_->width; ++x) {
      pixel = XGetPixel(ximage_, x, y);

      if ((int) pixel >= num_xcolors)
        continue;

      if (used[pixel] == 0)
        used[pixel] = ++num_used;

      XPutPixel(ximage_, x, y, used[pixel] - 1);
    }
  }

  if (num_used > 0) {
    *colors     = new CRGB [num_used];
    *num_colors = 0;

    for (int i = 0; i < num_used; ++i) {
      if (used[i] == 0)
        continue;

      int red   = xcolors[i].red   >> 8;
      int green = xcolors[i].green >> 8;
      int blue  = xcolors[i].blue  >> 8;

      (*colors)[used[i] - 1] = CRGB(red/255.0, green/255.0, blue/255.0);

      ++(*num_colors);
    }
  }
  else {
    *colors     = new CRGB [2];
    *num_colors = 2;

    (*colors)[0] = CRGB(0, 0, 0);
    (*colors)[1] = CRGB(1, 1, 1);
  }
}

bool
CXImage::
setPixel(int pos, const CXColor &color)
{
  if (ximage_ != NULL) {
    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    uint width = x2 - x1 + 1;

    XPutPixel(ximage_, pos % width, pos / width, color.getPixel());
  }

  return CImage::setRGBAPixel(pos, color.getRGBA());
}

bool
CXImage::
setPixel(int x, int y, const CXColor &color)
{
  if (ximage_ != NULL)
    XPutPixel(ximage_, x, y, color.getPixel());

  return CImage::setRGBAPixel(x, y, color.getRGBA());
}

bool
CXImage::
setColorIndexPixel(int pos, Pixel pixel)
{
  if (ximage_ != NULL) {
    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    uint width = x2 - x1 + 1;

    XPutPixel(ximage_, pos % width, pos / width, pixel);
  }

  return CImage::setColorIndexPixel(pos, pixel);
}

bool
CXImage::
setColorIndexPixel(int x, int y, Pixel pixel)
{
  if (ximage_ != NULL)
    XPutPixel(ximage_, x, y, pixel);

  return CImage::setColorIndexPixel(x, y, pixel);
}

void
CXImage::
setRGBAData(uint *data)
{
  CImage::setRGBAData(data);

  if (ximage_ != NULL) {
    delete ximage_;

    ximage_ = NULL;
  }
}

void
CXImage::
setRGBAData(const CRGBA &rgba)
{
  CImage::setRGBAData(rgba);
}

void
CXImage::
setRGBAData(const CRGBA &rgba, int left, int bottom, int right, int top)
{
  CImage::setRGBAData(rgba, left, bottom, right, top);

  if (ximage_ != NULL) {
    delete ximage_;

    ximage_ = NULL;
  }
}

bool
CXImage::
setRGBAPixel(int pos, const CRGBA &rgba)
{
  if (ximage_ != NULL) {
    Pixel pixel = screen_.rgbaToPixel(rgba);

    int x1, y1, x2, y2;

    getWindow(&x1, &y1, &x2, &y2);

    uint width = x2 - x1 + 1;

    XPutPixel(ximage_, pos % width, pos / width, pixel);
  }

  return CImage::setRGBAPixel(pos, rgba);
}

bool
CXImage::
setRGBAPixel(int x, int y, const CRGBA &rgba)
{
  if (ximage_ != NULL) {
    Pixel pixel = screen_.rgbaToPixel(rgba);

    XPutPixel(ximage_, x, y, pixel);
  }

  return CImage::setRGBAPixel(x, y, rgba);
}

int
CXImage::
getDataSize()
{
  int row_size = getRowSize();

  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  uint height = y2 - y1 + 1;

  return row_size*height;
}

int
CXImage::
getRowSize()
{
  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  uint width = x2 - x1 + 1;

  if      (screen_.getDepth() == 32)
    return 4*width;
  else if (screen_.getDepth() == 24)
    return 4*width;
  else if (screen_.getDepth() == 16)
    return 2*width;
  else if (screen_.getDepth() == 8)
    return width;
  else
    return ((width + 7)/8);
}

//----------------

void
CXImage::
draw(Display *display, Drawable drawable, GC gc, int x, int y)
{
  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  uint width  = x2 - x1 + 1;
  uint height = y2 - y1 + 1;

  draw(display, drawable, gc, 0, 0, x, y, width, height);
}

void
CXImage::
draw(CXScreen *cxscreen, Drawable drawable, GC gc, int x, int y)
{
  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  uint width  = x2 - x1 + 1;
  uint height = y2 - y1 + 1;

  draw(cxscreen->getDisplay(), drawable, gc, 0, 0, x, y, width, height);
}

void
CXImage::
draw(Display *, Drawable drawable, GC gc, int src_x, int src_y,
     int dst_x, int dst_y, int width, int height)
{
  int x1, y1, x2, y2;

  getWindow(&x1, &y1, &x2, &y2);

  uint iwidth  = x2 - x1 + 1;
  uint iheight = y2 - y1 + 1;

  if (src_x + width  > (int) iwidth)
    width = iwidth - src_x;

  if (src_y + height > (int) iheight)
    height = iheight - src_y;

  XImage *ximage = getXImage();

  if (ximage != NULL)
    CXMachineInst->putImage(drawable, gc, ximage, src_x, src_y, dst_x, dst_y, width, height);
}

void
CXImage::
dataChanged()
{
  reset();
}
