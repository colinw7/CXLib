#ifndef CXRT_FONT_H
#define CXRT_FONT_H

#include <std_Xt.h>
#include <string>

class CPixelRenderer;

class CXrtFont {
 public:
  CXrtFont(Display *display, XFontStruct *fs, double angle);
  CXrtFont(Display *display, const std::string &name, double angle);

  CXrtFont(Display *display, Window window, XFontStruct *fs, double angle);
  CXrtFont(Display *display, Window window, const std::string &name, double angle);

  CXrtFont(const CXrtFont &xrt_font);

 ~CXrtFont();

  void getExtents(int *width, int *ascent, int *descent);

  bool isProportional();

  void textExtents(const std::string &str, int *width, int *ascent, int *descent);

  void draw(Window window, GC gc, int x, int y, const std::string &str);

  void drawImage(Window window, GC gc, int x, int y, const std::string &str);

  void draw(CPixelRenderer *renderer, int x, int y, const std::string &str);

  void drawImage(CPixelRenderer *renderer, int x, int y, const std::string &str);

  XFontStruct *getFontStruct();

 private:
  void initFontStruct(const std::string &name);

  void init();

  void rotateChar(int c);

  static int XErrorHandler(Display *display, XErrorEvent *event);

 private:
  Display     *display_ { nullptr };
  Window       window_ { 0 };
  int          angle_ { 0 };
  std::string  name_;
  XFontStruct *fs_ { nullptr };
  Pixmap       pixmap1_ { 0 };
  Pixmap       pixmap2_ { 0 };
  XImage      *ximage_ { nullptr };
  GC           gc_ { 0 };
  int          width_ { 8 };
  int          ascent_ { 8 };
  int          descent_ { 2 };
  int          start_char_ { 0 };
  int          end_char_ { 0 };
  int          num_chars_ { 0 };
  bool        *rotated_ { nullptr };
};

#endif
