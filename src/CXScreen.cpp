#include <CXLibI.h>

CXScreen::
CXScreen(int screen_num) :
 screen_num_(screen_num), black_color_(*this), white_color_(*this)
{
  init();
}

CXScreen::
~CXScreen()
{
  term();
}

void
CXScreen::
init()
{
  gray_scale_ = false;

  display_ = CXMachineInst->getScreenDisplay(screen_num_);
  screen_  = ScreenOfDisplay(display_, screen_num_);
  visual_  = DefaultVisualOfScreen(screen_);
  cmap_    = DefaultColormapOfScreen(screen_);

  x_      = 0;
  y_      = 0;
  width_  = WidthOfScreen       (screen_);
  height_ = HeightOfScreen      (screen_);
  depth_  = DefaultDepthOfScreen(screen_);

  if (depth_ == 8 &&
      (IsVisualClass(visual_, PseudoColor) ||
       IsVisualClass(visual_, GrayScale  ))) {
    has_colormap_ = true;

    num_colors_ = (1 << depth_);

    colors_         .resize(num_colors_);
    color_used_     .resize(num_colors_);
    color_allocated_.resize(num_colors_);

    setColorsUsed();
  }
  else {
    has_colormap_ = false;

    num_colors_ = 0;

    num_used_colors_ = 0;

    CXUtil::decodeVisualMask(visual_->red_mask  , &red_shift_  , &red_mask_  );
    CXUtil::decodeVisualMask(visual_->green_mask, &green_shift_, &green_mask_);
    CXUtil::decodeVisualMask(visual_->blue_mask , &blue_shift_ , &blue_mask_ );

    alpha_shift_ = 24;
    alpha_mask_  = 0xFF;
  }

  black_color_.setPixel(XBlackPixel(display_, screen_num_));
  white_color_.setPixel(XWhitePixel(display_, screen_num_));

  color_mgr_ = new CXColorMgr(*this);
}

void
CXScreen::
term()
{
  delete color_mgr_;
}

Display *
CXScreen::
getDisplay() const
{
  return display_;
}

int
CXScreen::
getScreenNum() const
{
  return screen_num_;
}

Screen *
CXScreen::
getScreen() const
{
  return screen_;
}

Visual *
CXScreen::
getVisual() const
{
  return visual_;
}

Colormap
CXScreen::
getColormap() const
{
  return cmap_;
}

int
CXScreen::
getX() const
{
  return x_;
}

int
CXScreen::
getY() const
{
  return y_;
}

int
CXScreen::
getWidth() const
{
  return width_;
}

int
CXScreen::
getHeight() const
{
  return height_;
}

int
CXScreen::
getDepth() const
{
  return depth_;
}

bool
CXScreen::
getGrayScale() const
{
  return gray_scale_;
}

bool
CXScreen::
getHasColormap() const
{
  return has_colormap_;
}

Pixel
CXScreen::
getBlackPixel() const
{
  return black_color_.getPixel();
}

Pixel
CXScreen::
getWhitePixel() const
{
  return white_color_.getPixel();
}

void
CXScreen::
setColormap(Colormap cmap)
{
  if (cmap_ == cmap)
    return;

  if (has_colormap_)
    freeAllocatedColors();

  cmap_ = cmap;

  if (has_colormap_)
    setColorsUsed();

  CXMachineInst->installColormap(cmap_);
}

Window
CXScreen::
getRoot() const
{
  return RootWindowOfScreen(screen_);
}

CXWindow *
CXScreen::
getCXRoot() const
{
  return new CXWindow(getRoot());
}

void
CXScreen::
addWindow(CXWindow *window)
{
  window_map_[window->getXWindow()] = window;
}

void
CXScreen::
removeWindow(CXWindow *window)
{
  window_map_[window->getXWindow()] = NULL;
}

CXWindow *
CXScreen::
lookupWindow(Window xwin)
{
  WindowMap::iterator p = window_map_.find(xwin);

  if (p != window_map_.end())
    return (*p).second;

  return NULL;
}

Pixel
CXScreen::
rgbToPixel(const CRGB &rgb)
{
  double r, g, b;

  rgb.getRGB(&r, &g, &b);

  return rgbaToPixel(r, g, b, 1);
}

Pixel
CXScreen::
rgbaToPixel(const CRGBA &rgba)
{
  double r, g, b, a;

  rgba.getRGBA(&r, &g, &b, &a);

  return rgbaToPixel(r, g, b, a);
}

Pixel
CXScreen::
rgbToPixel(double r, double g, double b)
{
  return rgbaToPixel(r, g, b, 1);
}

Pixel
CXScreen::
rgbaToPixel(double r, double g, double b, double a)
{
  uint red   = uint(r*red_mask_  ) & red_mask_  ;
  uint green = uint(g*green_mask_) & green_mask_;
  uint blue  = uint(b*blue_mask_ ) & blue_mask_ ;
  uint alpha = uint(a*alpha_mask_) & alpha_mask_;

  return rgbaIToPixel(red, green, blue, alpha);
}

Pixel
CXScreen::
rgbaIToPixel(uint red, uint green, uint blue, uint alpha)
{
  if (! has_colormap_) {
    Pixel pixel = (red   << red_shift_  ) |
                  (green << green_shift_) |
                  (blue  << blue_shift_ ) |
                  (alpha << alpha_shift_);

    return pixel;
  }

  int i;

  if (! gray_scale_) {
    uint red1   = (red   << 8) | red;
    uint green1 = (green << 8) | green;
    uint blue1  = (blue  << 8) | blue;

    for (i = 0; i < num_colors_; ++i) {
      if (color_used_[i] &&
          colors_[i].red   == red1   &&
          colors_[i].green == green1 &&
          colors_[i].blue  == blue1)
        return colors_[i].pixel;
    }

    if (num_used_colors_ < num_colors_) {
      for (i = 0; i < num_colors_; ++i) {
        if (! color_used_[i])
          break;
      }

      color_used_[i] = true;

      colors_[i].pixel = i;
      colors_[i].red   = red1;
      colors_[i].green = green1;
      colors_[i].blue  = blue1;
      colors_[i].flags = DoRed | DoGreen | DoBlue;

      XStoreColor(display_, cmap_, &colors_[i]);

      CXMachineInst->flushEvents(true);

      num_used_colors_++;

      return colors_[i].pixel;
    }

    int nearest = 0;
    int diff    = 256*256*256;
    int diff1   = 0;

    for (i = 0; i < num_colors_; ++i) {
      uint red1   = colors_[i].red   & 0x00FF;
      uint green1 = colors_[i].green & 0x00FF;
      uint blue1  = colors_[i].blue  & 0x00FF;

      diff1 = abs(red1   - red  )*
              abs(green1 - green)*
              abs(blue1  - blue );

      if (diff1 < diff) {
        nearest = i;

        diff = diff1;
      }
    }

    return colors_[nearest].pixel;
  }
  else {
    uint gray = (red << 8) | red;

    for (i = 0; i < num_colors_; ++i) {
      if (color_used_[i] && colors_[i].red == gray)
        return colors_[i].pixel;
    }

    if (num_used_colors_ < num_colors_) {
      for (i = 0; i < num_colors_; ++i) {
        if (! color_used_[i])
          break;
      }

      color_used_[i] = true;

      colors_[i].pixel = i;
      colors_[i].red   = gray;
      colors_[i].green = gray;
      colors_[i].blue  = gray;
      colors_[i].flags = DoRed | DoGreen | DoBlue;

      XStoreColor(display_, cmap_, &colors_[i]);

      CXMachineInst->flushEvents(true);

      num_used_colors_++;

      return colors_[i].pixel;
    }

    int nearest = 0;
    int diff    = 256;
    int diff1   = 0;

    for (i = 0; i < num_colors_; ++i) {
      uint gray = colors_[i].red & 0x00FF;

      diff1 = abs(gray - red);

      if (diff1 < diff) {
        nearest = i;

        diff = diff1;
      }
    }

    return colors_[nearest].pixel;
  }
}

void
CXScreen::
setGrayScale()
{
  if (! has_colormap_)
    return;

  allocateOwnGrayColormap();

  gray_scale_ = true;
}

CRGB
CXScreen::
pixelToRGB(Pixel pixel)
{
  return pixelToRGBA(pixel).getRGB();
}

CRGBA
CXScreen::
pixelToRGBA(Pixel pixel)
{
  static double rgb_scale = 1.0/65535.0;

  double red   = 0;
  double green = 0;
  double blue  = 0;
  double alpha = 1;

  if (has_colormap_) {
    int i;

    for (i = 0; i < num_colors_; ++i) {
      if (! color_used_[i])
        continue;

      if (colors_[i].pixel == pixel)
        break;
    }

    if (i < num_colors_) {
      red   = colors_[i].red  *rgb_scale;
      green = colors_[i].green*rgb_scale;
      blue  = colors_[i].blue *rgb_scale;
    }
  }
  else {
    uint red1   = (pixel >> red_shift_  ) & red_mask_;
    uint green1 = (pixel >> green_shift_) & green_mask_;
    uint blue1  = (pixel >> blue_shift_ ) & blue_mask_;
    uint alpha1 = (pixel >> alpha_shift_) & alpha_mask_;

    red   = ((double) red1  )/red_mask_  ;
    green = ((double) green1)/green_mask_;
    blue  = ((double) blue1 )/blue_mask_ ;
    alpha = ((double) alpha1)/alpha_mask_;
  }

  return CRGBA(red, green, blue, alpha);
}

bool
CXScreen::
allocateOwnGrayColormap()
{
  Pixel  pixels     [256];
  XColor xcolors    [256];
  int    used_grays [256];
  bool   used_pixels[256];

  Window root = getRoot();

  Colormap new_cmap = XCreateColormap(display_, root, visual_, AllocNone);

  if (XAllocColorCells(display_, new_cmap, False, NULL, 0, pixels, 256) == 0) {
    fprintf(stderr, "Failed to allocated colormap colors_\n");
    return false;
  }

  /*---------*/

  Colormap default_cmap = DefaultColormap(display_, screen_num_);

  for (int i = 0; i < 256; ++i)
    xcolors[i].pixel = i;

  XQueryColors(display_, default_cmap, xcolors, 256);

  for (int i = 0; i < 256; ++i) {
    used_grays [i] = -1;
    used_pixels[i] = false;
  }

  for (int i = 255; i >= 0; i--) {
    int r = (xcolors[i].red   >> 8) & 0xFF;
    int g = (xcolors[i].green >> 8) & 0xFF;
    int b = (xcolors[i].blue  >> 8) & 0xFF;

    int gray = (r + g + b)/3;

    used_grays[gray] = i;
  }

  for (int i = 0; i < 256; ++i)
    if (used_grays[i] != -1)
      used_pixels[used_grays[i]] = true;

  /*---------*/

  for (int i = 0; i < 256; ++i) {
    XColor color;

    if (used_grays[i] == -1) {
      int j = 0;

      for ( ; j < 256; ++j)
        if (! used_pixels[j])
          break;

      color.pixel = j;

      used_pixels[j] = true;
    }
    else
      color.pixel = used_grays[i];

    int gray = (i << 8) | i;

    color.red   = gray;
    color.green = gray;
    color.blue  = gray;
    color.flags = DoRed | DoGreen | DoBlue;

    XStoreColor(display_, new_cmap, &color);
  }

  CXMachineInst->flushEvents(true);

  /*------*/

  setColormap(new_cmap);

  return true;
}

void
CXScreen::
setColorsUsed()
{
  for (int i = 0; i < num_colors_; ++i) {
    color_used_     [i] = true;
    color_allocated_[i] = false;

    colors_[i].pixel = i;
  }

  CXMachineInst->flushEvents(true);

  XQueryColors(display_, cmap_, &colors_[0], num_colors_);

  for (int i = 0; i < num_colors_; ++i) {
    Pixel pixel;

    if (! XAllocColorCells(display_, cmap_, False, NULL, 0, &pixel, 1) == 1)
      break;

    color_used_     [pixel] = false;
    color_allocated_[pixel] = true;

    colors_[pixel].pixel = pixel;
    colors_[pixel].red   = 0;
    colors_[pixel].green = 0;
    colors_[pixel].blue  = 0;
    colors_[pixel].flags = DoRed | DoGreen | DoBlue;

    XStoreColor(display_, cmap_, &colors_[pixel]);
  }

  CXMachineInst->flushEvents(true);

  num_used_colors_ = 0;

  for (int i = 0; i < num_colors_; ++i)
    if (color_used_[i])
      num_used_colors_++;
}

void
CXScreen::
freeAllocatedColors()
{
  for (int i = 0; i < num_colors_; ++i) {
    if (! color_allocated_[i])
      continue;

    colors_[i].red   = 0;
    colors_[i].green = 0;
    colors_[i].blue  = 0;

    XStoreColor(display_, cmap_, &colors_[i]);

    XFreeColors(display_, cmap_, &colors_[i].pixel, 1, 0);

    color_used_     [i] = false;
    color_allocated_[i] = false;
  }

  CXMachineInst->flushEvents(true);
}

Pixmap
CXScreen::
createMask(const CImagePtr &image)
{
  Window xwindow = getRoot();

  int width  = image->getWidth ();
  int height = image->getHeight();

  Pixmap mask = XCreatePixmap(display_, xwindow, width, height, 1);

  if (mask == None)
    return None;

  CImagePtr image_mask = image->createMask();

  GC gc = CXMachineInst->createGC(mask, 0, 0);

  if (image_mask.isValid()) {
    CXMachineInst->setForeground(gc, 0);

    CXMachineInst->fillRectangle(mask, gc, 0, 0, width, height);

    CXMachineInst->setForeground(gc, 1);

    int pos = 0;

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x, ++pos) {
        int pixel = image_mask->getColorIndexPixel(pos);

        if (pixel != 0)
          CXMachineInst->drawPoint(mask, gc, x, y);
      }
    }
  }
  else {
    CXMachineInst->setForeground(gc, 1);

    CXMachineInst->fillRectangle(mask, gc, 0, 0, width, height);
  }

  CXMachineInst->freeGC(gc);

  return mask;
}

void
CXScreen::
windowToImage(Drawable drawable, CImagePtr &image)
{
  CXGraphics *graphics = new CXGraphics(*this, drawable);

  int width, height;

  graphics->getSize(&width, &height);

  delete graphics;

  CXImage *ximage = new CXImage(*this, drawable, 0, 0, width, height);

  image = CImagePtr(ximage);
}

void
CXScreen::
flushEvents() const
{
  CXMachineInst->flushEvents();
}

bool
CXScreen::
selectWMInput() const
{
  CXMachineInst->trapStart();

  Window root = getRoot();

  bool rc = CXMachineInst->addInput(root, SubstructureRedirectMask);

  if (rc) {
    int event_mask = ButtonPressMask | ButtonReleaseMask  |
                     EnterWindowMask | LeaveWindowMask    |
                     KeyPressMask    | PropertyChangeMask;

    rc = CXMachineInst->addInput(root, event_mask);
  }

  if (! CXMachineInst->trapEnd())
    return false;

  return rc;
}

bool
CXScreen::
getWindows(Window **windows, int *num_windows)
{
  Window root = getRoot();

  return CXMachineInst->getWindowChildren(root, windows, num_windows);
}

const CXColor &
CXScreen::
getCXColor(const CRGB &rgb)
{
  return color_mgr_->getCXColor(rgb);
}

const CXColor &
CXScreen::
getCXColor(const CRGBA &rgba)
{
  return color_mgr_->getCXColor(rgba);
}

const CXColor &
CXScreen::
getCXColor(Pixel pixel)
{
  return color_mgr_->getCXColor(pixel);
}

void
CXScreen::
refresh()
{
  uint attr_mask = CWBackPixel | CWOverrideRedirect;

  XSetWindowAttributes attr;

  attr.background_pixel  = 0;
  attr.override_redirect = True;
  attr.backing_store     = NotUseful;

  Window window =
    CXMachineInst->createWindow(None, 0, 0, getWidth(), getHeight(), 0, attr_mask, &attr);

  CXMachineInst->mapWindow(window);

  CXMachineInst->destroyWindow(window);

  CXMachineInst->flushEvents();
}

bool
CXScreen::
getPointerPosition(int *x, int *y)
{
  return CXMachineInst->queryPointer(getRoot(), x, y);
}

Window
CXScreen::
getPointerWindow()
{
  int x, y;

  if (! getPointerPosition(&x, &y))
    return None;

  return getCoordWindow(x, y);
}

Window
CXScreen::
getCoordWindow(int x, int y)
{
  Window src_w = getRoot();

  int src_x = x;
  int src_y = y;

  Window dest_w = getRoot();

  Window child;
  int    dest_x, dest_y;

  if (! CXMachineInst->translateCoords(src_w, dest_w, src_x, src_y, &dest_x, &dest_y, &child))
    child = None;

  while (child != None) {
    src_w = dest_w;
    src_x = dest_x;
    src_y = dest_y;

    dest_w = child;

    if (! CXMachineInst->translateCoords(src_w, dest_w, src_x, src_y, &dest_x, &dest_y, &child))
      break;
  }

  return dest_w;
}
