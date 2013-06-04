#include <CXrtFont.h>

#include <std_Xt.h>

#define XRT_CHAR_BORDER_RIGHT 3

using std::string;

CXrtFont::
CXrtFont(Display *display, const string &name, double angle) :
 display_(display), window_(None), angle_((int) angle), name_(name), fs_(NULL)
{
  window_ = DefaultRootWindow(display_);

  initFontStruct(name);
}

CXrtFont::
CXrtFont(Display *display, XFontStruct *fs, double angle) :
 display_(display), window_(None), angle_((int) angle), name_(""), fs_(fs)
{
  window_ = DefaultRootWindow(display_);

  init();
}

CXrtFont::
CXrtFont(Display *display, Window window, const string &name, double angle) :
 display_(display), window_(window), angle_((int) angle), name_(name), fs_(NULL)
{
  initFontStruct(name);
}

CXrtFont::
CXrtFont(Display *display, Window window, XFontStruct *fs, double angle) :
 display_(display), window_(window), angle_((int) angle), name_(""), fs_(fs)
{
  init();
}

CXrtFont::
CXrtFont(const CXrtFont &xrt_font) :
 display_(xrt_font.display_), window_(xrt_font.window_),
 angle_(xrt_font.angle_), name_(xrt_font.name_), fs_(xrt_font.fs_)
{
  init();
}

CXrtFont::
~CXrtFont()
{
  if (fs_ != NULL)
    XFreeFont(display_, fs_);

  if (pixmap1_ != None)
    XFreePixmap(display_, pixmap1_);

  if (pixmap2_ != pixmap1_)
    XFreePixmap(display_, pixmap2_);

  if (ximage_ != NULL)
    XDestroyImage(ximage_);

  if (gc_ != 0)
    XFreeGC(display_, gc_);

  delete [] rotated_;
}

void
CXrtFont::
initFontStruct(const string &name)
{
  fs_ = XLoadQueryFont(display_, (char *) name.c_str());

  if (fs_ == NULL)
    fs_ = XLoadQueryFont(display_, "fixed");

  if (fs_ == NULL)
    throw "Failed to load font";

  init();
}

void
CXrtFont::
init()
{
  while (angle_ < 0)
    angle_ += 360;

  while (angle_ >= 360)
    angle_ -= 360;

  pixmap1_ = None;
  pixmap2_ = None;

  ximage_ = NULL;

  width_   = 0;
  ascent_  = 0;
  descent_ = 0;

  start_char_ = fs_->min_char_or_byte2;
  end_char_   = fs_->max_char_or_byte2;
  num_chars_  = end_char_ - start_char_ + 1;

  for (int i = 0; i < num_chars_; i++) {
    width_   = std::max(width_  , (int) fs_->per_char[i].width  );
    ascent_  = std::max(ascent_ , (int) fs_->per_char[i].ascent );
    descent_ = std::max(descent_, (int) fs_->per_char[i].descent);
  }

  width_ += 4;

  int (*error_handler)(Display *, XErrorEvent *) =
    XSetErrorHandler(CXrtFont::XErrorHandler);

  pixmap1_ = XCreatePixmap(display_, window_, num_chars_*width_, ascent_ + descent_, 1);

  if      (angle_ == 90 || angle_ == 270)
    pixmap2_ = XCreatePixmap(display_, window_, ascent_ + descent_, num_chars_*width_, 1);
  else if (angle_ == 180)
    pixmap2_ = XCreatePixmap(display_, window_, num_chars_*width_, ascent_ + descent_, 1);
  else
    pixmap2_ = pixmap1_;

  XSetErrorHandler(error_handler);

  XGCValues gc_values;

  gc_values.function   = GXcopy;
  gc_values.background = 0;
  gc_values.foreground = 1;
  gc_values.plane_mask = AllPlanes;

  gc_ = XCreateGC(display_, pixmap1_,
                  GCForeground | GCFunction | GCBackground | GCPlaneMask, &gc_values);

  XSetForeground(display_, gc_, 0);

  if      (angle_ == 90 || angle_ == 270)
    XFillRectangle(display_, pixmap2_, gc_, 0, 0, ascent_ + descent_, num_chars_*width_);
  else if (angle_ == 180)
    XFillRectangle(display_, pixmap2_, gc_, 0, 0, num_chars_*width_, ascent_ + descent_);

  XFillRectangle(display_, pixmap1_, gc_, 0, 0, num_chars_*width_, ascent_ + descent_);

  XSetForeground(display_, gc_, 1);

  XSetFont(display_, gc_, fs_->fid);

  rotated_ = new bool [num_chars_];

  for (int i = 0; i < num_chars_; i++)
    rotated_[i] = false;
}

void
CXrtFont::
getExtents(int *width, int *ascent, int *descent)
{
  if (width != NULL)
    *width = width_ - 4;

  if (ascent != NULL)
    *ascent = ascent_;

  if (descent != NULL)
    *descent = descent_;
}

bool
CXrtFont::
isProportional()
{
  return (fs_->max_bounds.width != fs_->min_bounds.width);
}

void
CXrtFont::
textExtents(const string &str, int *width, int *ascent, int *descent)
{
  int         ascent1;
  XCharStruct overall;
  int         descent1;
  int         direction;

  XTextExtents(fs_, (char *) str.c_str(), str.size(), &direction, &ascent1, &descent1, &overall);

  if (width != NULL)
    *width = overall.width;

  if (ascent != NULL)
    *ascent = ascent_;

  if (descent != NULL)
    *descent = descent_;
}

void
CXrtFont::
draw(Window window, GC gc, int x, int y, const string &str)
{
  bool changed = false;

  int len = str.size();

  for (int i = 0; i < len; i++) {
    int c = str[i];

    if (c < 0 || c >= num_chars_)
      continue;

    if (! rotated_[c - start_char_]) {
      rotateChar(c);

      changed = true;
    }
  }

  if (changed || ximage_ == NULL) {
    if (ximage_ != NULL)
      XDestroyImage(ximage_);

    if (angle_ == 0 || angle_ == 180)
      ximage_ = XGetImage(display_, pixmap2_, 0, 0, num_chars_*width_,
                          ascent_ + descent_, AllPlanes, XYPixmap);
    else
      ximage_ = XGetImage(display_, pixmap2_, 0, 0, ascent_ + descent_,
                          num_chars_*width_, AllPlanes, XYPixmap);
  }

  if (ximage_ == NULL)
    return;

  len = str.size();

  for (int i = 0; i < len; i++) {
    int c = str[i] - start_char_;

    if (c < 0 || c >= num_chars_)
      continue;

    int wc = fs_->per_char[c].width;

    if      (angle_ == 90) {
      int ys = (num_chars_ - 1 - c)*width_;
      int yd = y - wc - XRT_CHAR_BORDER_RIGHT;

      for (int yy = 0; yy < wc + XRT_CHAR_BORDER_RIGHT; yy++, ys++, yd++) {
        int xs = 0;
        int xd = x;

        for (int xx = 0; xx < ascent_ + descent_; xx++, xs++, xd++) {
          Pixel pixel = XGetPixel(ximage_, xs, ys);

          if (pixel != 0)
            XDrawPoint(display_, window, gc, xd, yd);
        }
      }

      y -= wc;
    }
    else if (angle_ == 270) {
      int ys = c*width_;
      int yd = y;

      for (int yy = 0; yy < wc + XRT_CHAR_BORDER_RIGHT; yy++, ys++, yd++) {
        int xs = 0;
        int xd = x - ascent_ - descent_;

        for (int xx = 0; xx < ascent_ + descent_; xx++, xs++, xd++) {
          Pixel pixel = XGetPixel(ximage_, xs, ys);

          if (pixel != 0)
            XDrawPoint(display_, window, gc, xd, yd);
        }
      }

      y += wc;
    }
    else if (angle_ == 180) {
      int ys = 0;
      int yd = y - ascent_ - descent_;

      for (int yy = 0; yy < ascent_ + descent_; yy++, ys++, yd++) {
        int xs = (num_chars_ - 1 - c)*width_;
        int xd = x - wc - XRT_CHAR_BORDER_RIGHT;

        for (int xx = 0; xx < wc + XRT_CHAR_BORDER_RIGHT; xx++, xs++, xd++) {
          Pixel pixel = XGetPixel(ximage_, xs, ys);

          if (pixel != 0)
            XDrawPoint(display_, window, gc, xd, yd);
        }
      }

      x -= wc;
    }
    else {
      int ys = 0;
      int yd = y;

      for (int yy = 0; yy < ascent_ + descent_; yy++, ys++, yd++) {
        int xs = c*width_;
        int xd = x;

        for (int xx = 0; xx < wc + XRT_CHAR_BORDER_RIGHT; xx++, xs++, xd++) {
          Pixel pixel = XGetPixel(ximage_, xs, ys);

          if (pixel != 0)
            XDrawPoint(display_, window, gc, xd, yd);
        }
      }

      x += wc;
    }
  }
}

void
CXrtFont::
drawImage(Window window, GC gc, int x, int y, const string &str)
{
  XGCValues gc_values;

  XGetGCValues(display_, gc, GCForeground | GCBackground, &gc_values);

  XSetForeground(display_, gc, gc_values.background);

  int x1 = x;
  int y1 = y;

  int len = str.size();

  for (int i = 0; i < len; i++) {
    int c = str[i] - start_char_;

    if (c < 0 || c >= num_chars_)
      continue;

    int wc = fs_->per_char[c].width;

    if      (angle_ == 90) {
      XFillRectangle(display_, window, gc, x1, y1 - wc, ascent_ + descent_, wc);

      y1 -= wc;
    }
    else if (angle_ == 270) {
      XFillRectangle(display_, window, gc, x1 - ascent_ - descent_, y1,
                     ascent_ + descent_, wc);

      y1 += wc;
    }
    else if (angle_ == 180) {
      XFillRectangle(display_, window, gc, x1 - wc, y1 - ascent_ - descent_,
                     wc, ascent_ + descent_);

      x1 -= wc;
    }
    else {
      XFillRectangle(display_, window, gc, x1, y1, wc, ascent_ + descent_);

      x1 += wc;
    }
  }

  XSetForeground(display_, gc, gc_values.foreground);

  draw(window, gc, x, y, str);
}

void
CXrtFont::
rotateChar(int c)
{
  int c1 = c - start_char_;

  int x1 = c1*width_;

  int x2 = 0;

  if      (angle_ == 90)
    x2 = (num_chars_ - c1)*width_ - 1;
  else if (angle_ == 270)
    x2 = c1*width_;
  else if (angle_ == 180)
    x2 = (num_chars_ - c1)*width_ - 1;

  int xo = width_ - fs_->per_char[c1].width;

  char c2 = c;

  XDrawString(display_, pixmap1_, gc_, x1, ascent_, &c2, 1);

  int char_width  = fs_->per_char[c1].width;
  int char_height = ascent_ + descent_;

  if      (angle_ == 90) {
    for (int y = 0; y < char_height; y++)
      for (int x = 0; x < char_width + XRT_CHAR_BORDER_RIGHT; x++)
        XCopyPlane(display_, pixmap1_, pixmap2_, gc_, x1 + x, y, 1, 1,
                   y, x2 - x - xo + XRT_CHAR_BORDER_RIGHT, 1);
  }
  else if (angle_ == 270) {
    for (int y = 0; y < char_height; y++)
      for (int x = 0; x < char_width + XRT_CHAR_BORDER_RIGHT; x++)
        XCopyPlane(display_, pixmap1_, pixmap2_, gc_, x1 + x, y, 1, 1,
                   char_height - y - 1, x2 + x, 1);
  }
  else if (angle_ == 180) {
    for (int y = 0; y < char_height; y++)
      for (int x = 0; x < char_width + XRT_CHAR_BORDER_RIGHT; x++)
        XCopyPlane(display_, pixmap1_, pixmap2_, gc_, x1 + x, y, 1, 1,
                   x2 - x - xo + XRT_CHAR_BORDER_RIGHT, char_height - y - 1, 1);
  }

  rotated_[c1] = true;
}

XFontStruct *
CXrtFont::
getFontStruct()
{
  return fs_;
}

int
CXrtFont::
XErrorHandler(Display *, XErrorEvent *)
{
  return False;
}
