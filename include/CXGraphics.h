#ifndef CX_GRAPHICS_H
#define CX_GRAPHICS_H

#include <CRGBA.h>
#include <CILineDash.h>
#include <CFont.h>
#include <CXColor.h>
#include <CImage.h>
#include <CFontStyle.h>
#include <CAutoPtr.h>

enum CXLineType {
  CX_LINE_TYPE_SOLID,
  CX_LINE_TYPE_DASHED,
};

class CXScreen;
class CXPixmap;

class CXGraphics {
 public:
  CXGraphics(Window window);
  CXGraphics(CXScreen &screen, Window window);
 ~CXGraphics();

  CXScreen  &getCXScreen() const { return screen_; }
  Display   *getXDisplay() const { return display_; }
  Window     getXWindow () const { return window_; }
  bool       isPixmap   () const { return is_pixmap_; }
  GC         getXGC     () const { return gc_; }

  void getFont(CFontPtr &font) const { font = font_; }

  void startDoubleBuffer(bool clear=true);
  void endDoubleBuffer();
  void copyDoubleBuffer();

  void setXor();

  void clear(bool redraw);
  void fill();

  void setForeground(const CRGB &rgb);
  void setForeground(const CRGBA &rgba);
  void setForeground(Pixel pixel);
  void setForeground(const CXColor &color);

  void setBackground(const CRGB &rgb);
  void setBackground(const CRGBA &rgba);
  void setBackground(Pixel pixel);
  void setBackground(const CXColor &color);

  void getForeground(CRGB &rgb);
  void getForeground(CRGBA &rgb);

  void getForegroundPixel(Pixel &pixel);

  void getBackground(CRGB &rgb);
  void getBackground(CRGBA &rgb);

  void getBackgroundPixel(Pixel &pixel);

  void setFont(CFontPtr font);

  void drawLine(int x1, int y1, int x2, int y2);

  void drawRectangle(int x, int y, int width, int height);
  void fillRectangle(int x, int y, int width, int height);

  void drawPolygon(int *x, int *y, int num_xy);
  void fillPolygon(int *x, int *y, int num_xy);

  void drawCircle(int x, int y, int r);
  void fillCircle(int x, int y, int r);

  void drawEllipse(int x, int y, int xr, int yr);
  void fillEllipse(int x, int y, int xr, int yr);

  void drawArc(int x, int y, int xr, int yr, double angle1, double angle2);
  void fillArc(int x, int y, int xr, int yr, double angle1, double angle2);

  void drawPoint(int x, int y);

  void drawImage(const CImagePtr &image, int x, int y);
  void drawImage(XImage *ximage, int x, int y);

  void drawSubImage(const CImagePtr &image, int src_x, int src_y,
                    int dst_x, int dst_y, int width, int height);
  void drawSubImage(XImage *ximage, int src_x, int src_y,
                    int dst_x, int dst_y, int width, int height);

  void drawAlphaImage(const CImagePtr &image, int x, int y);
  void drawAlphaImage(XImage *ximage, int x, int y);

  void drawSubAlphaImage(const CImagePtr &image, int src_x, int src_y,
                         int dst_x, int dst_y, int width, int height);
  void drawSubAlphaImage(XImage *image, int src_x, int src_y,
                         int dst_x, int dst_y, int width, int height);

  bool getImage(int x, int y, int width, int height, CImagePtr &image);
  bool getImage(int x, int y, int width, int height, XImage **ximage);

  void drawText(int x, int y, const std::string &str);
  void drawTextImage(int x, int y, const std::string &str);

  void startClip(int x, int y, int width, int height);
  void startClip(Pixmap pixmap, int dx, int dy);
  void endClip();

  void copyArea(const CXGraphics &src, int src_x, int src_y,
                int dst_x, int dst_y, int width, int height);

  void setLineType(CXLineType line_type);

  void setLineWidth(int line_width);

  void setLineDash(int offset=0, char *dashes=nullptr, int num_dashes=0);
  void setLineDash(int offset, int *dashes, int num_dashes);
  void setLineDash(const CILineDash &line_dash);

  void setFillComplex(bool comp);

  void getSize(int *width, int *height) const;

  int getCharWidth();
  int getCharHeight();

  int getStringWidth(const std::string &str);

  void flushEvents();

 private:
  void init();

  bool isPixmapWindow() const;

  static int newErrorHandler(Display *display, XErrorEvent *event);

 private:
  enum { MAX_POLY_POINTS=100 };

  CXScreen&           screen_;
  Window              window_           { 0 };
  Display*            display_          { nullptr };
  bool                is_pixmap_        { false };
  GC                  gc_;
  CXColor             bg_;
  CXColor             fg_;
  CFontPtr            font_;
  CAutoPtr<CXPixmap>  pixmap_;
  bool                in_double_buffer_ { false };
  bool                fill_complex_     { false };

  static bool   error_trapped;
  static XPoint poly_point[MAX_POLY_POINTS];
};

#endif
