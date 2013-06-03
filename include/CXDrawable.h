#ifndef CX_DRAWABLE_H
#define CX_DRAWABLE_H

#include <CXGraphics.h>

class CXDrawable {
 protected:
  Drawable    drawable_;
  uint        width_;
  uint        height_;
  CXGraphics *graphics_;

 public:
  CXDrawable();
  CXDrawable(Drawable drawable, uint width, uint height);

  virtual ~CXDrawable();

  Drawable getDrawable() const { return drawable_; }
  uint     getWidth   () const { return width_   ; }
  uint     getHeight  () const { return height_  ; }

  void setDrawable(Drawable drawable, uint width, uint height);

  virtual void updateSize(uint width, uint height) {
    width_  = width;
    height_ = height;
  }

  void getFont(CFontPtr &font) const;

  void setFont(CFontPtr font);

  void startDoubleBuffer(bool clear=true);
  void endDoubleBuffer();
  void copyDoubleBuffer();

  void clear();
  void fill();

  void setForeground(const CRGB &rgb);
  void setForeground(const CRGBA &rgba);
  void setForeground(const CXColor &color);

  void setBackground(const CRGB &rgb);
  void setBackground(const CRGBA &rgba);
  void setBackground(const CXColor &color);

  void getForeground(CRGB &rgb);
  void getBackground(CRGB &rgb);

  void drawLine(int x1, int y1, int x2, int y2);

  void drawRectangle(int x, int y, int width, int height);
  void fillRectangle(int x, int y, int width, int height);

  void drawPolygon(int *x, int *y, uint num_xy);
  void fillPolygon(int *x, int *y, uint num_xy);

  void drawCircle(int x, int y, int r);
  void fillCircle(int x, int y, int r);

  void drawEllipse(int x, int y, int xr, int yr);
  void fillEllipse(int x, int y, int xr, int yr);

  void drawArc(int x, int y, int xr, int yr, double angle1, double angle2);
  void fillArc(int x, int y, int xr, int yr, double angle1, double angle2);

  void drawPoint(int x, int y);

  void drawImage(const CImagePtr &image, int x, int y);

  void drawSubImage(const CImagePtr &image, int src_x, int src_y,
                    int dst_x, int dst_y, int width, int height);

  void drawAlphaImage(const CImagePtr &image, int x, int y);

  void drawText(int x, int y, const std::string &str);

  void setLineType(CXLineType line_type);

  void setLineWidth(int line_width);

  void setLineDash(int offset=0, char *dashes=NULL, uint num_dashes=0);
  void setLineDash(int offset, int *dashes, uint num_dashes);
  void setLineDash(const CILineDash &line_dash);

  void setFillComplex(bool comp);

  int getCharWidth();
  int getCharHeight();

  uint getStringWidth(const std::string &str);

  CImagePtr getImage();

  CImagePtr getImage(int x, int y, uint width, uint height);
};

#endif
