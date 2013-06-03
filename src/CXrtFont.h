#ifndef CXRT_FONT_H
#define CXRT_FONT_H

#include <std_Xt.h>

class CXrtFont {
 private:
  Display     *display_;
  Window       window_;
  int          angle_;
  string       name_;
  XFontStruct *fs_;
  Pixmap       pixmap1_;
  Pixmap       pixmap2_;
  XImage      *ximage_;
  GC           gc_;
  int          width_;
  int          ascent_;
  int          descent_;
  int          start_char_;
  int          end_char_;
  int          num_chars_;
  bool        *rotated_;

 public:
  CXrtFont(Display *display, XFontStruct *fs, double angle);
  CXrtFont(Display *display, const string &name, double angle);

  CXrtFont(Display *display, Window window, XFontStruct *fs, double angle);
  CXrtFont(Display *display, Window window, const string &name, double angle);

  CXrtFont(const CXrtFont &xrt_font);

 ~CXrtFont();

  void getExtents(int *width, int *ascent, int *descent);

  bool isProportional();

  void textExtents(const string &str, int *width, int *ascent, int *descent);

  void draw(Window window, GC gc, int x, int y, const string &str);

  void drawImage(Window window, GC gc, int x, int y, const string &str);

  XFontStruct *getFontStruct();

 private:
  void initFontStruct(const string &name);

  void init();

  void rotateChar(int c);

  static int XErrorHandler(Display *display, XErrorEvent *event);
};

#endif
