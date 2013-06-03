#ifndef CX_IMAGE_H
#define CX_IMAGE_H

class CXScreen;

#include <std_Xt.h>
#include <CImage.h>
#include <CXColor.h>

class CXImage : public CImage {
 private:
  CXScreen &screen_;
  uchar    *xdata_;
  XImage   *ximage_;
  bool      ximage_owner_;
  Pixmap    pixmap_;
  Pixmap    mask_;

  // Create

 protected:
  friend class CXGraphics;
  friend class CXImageFile;
  friend class CXImageSizedFile;
  friend class CXScreen;
  friend class CXWindow;

  explicit CXImage(CXScreen &screen);
  CXImage();

  CXImage(CXScreen &screen, int width, int height);
  CXImage(int width, int height);

  CXImage(CXScreen &screen, const CXImage &ximage);

  CXImage(CXScreen &screen, const CImage &image);
  CXImage(const CImage &image);

  CXImage(CXScreen &screen, Drawable drawable, int x, int y,
          int width, int height);
  CXImage(Drawable drawable, int x, int y, int width, int height);

  CXImage(CXScreen &screen, XImage *image);
  explicit CXImage(XImage *image);

 public:
  static void setPrototype();

  CXImage(const CXImage &ximage);

  virtual ~CXImage();

 protected:
  CXImage &operator=(const CImage &image);
  CXImage &operator=(const CXImage &image);

 public:
  virtual CImagePtr dup() const;

  //------

  void getXImage(Drawable drawable, int x, int y, int width, int height);

  //------

  CXScreen &getCXScreen() const;
  XImage   *getXImage  () const;
  int       getDepth   () const;
  Pixmap    getXPixmap () const;
  Pixmap    getXMask   () const;

  void draw(Display *display, Drawable drawable, GC gc, int x, int y);
  void draw(CXScreen *cxscreen, Drawable drawable, GC gc, int x, int y);
  void draw(Display *display, Drawable drawable, GC gc, int src_x, int src_y,
            int dst_x, int dst_y, int width, int height);

  void createImageData();
  void getImageColors(CRGB **colors, int *num_colors);

  bool setPixel(int pos, const CXColor &color);
  bool setPixel(int x, int y, const CXColor &color);

  virtual bool setColorIndexPixel(int pos, Pixel pixel);
  virtual bool setColorIndexPixel(int x, int y, Pixel pixel);

  virtual void setRGBAData(uint *data);

  virtual void setRGBAData(const CRGBA &rgba);
  virtual void setRGBAData(const CRGBA &rgba, int left, int bottom,
                           int right, int top);

  virtual bool setRGBAPixel(int pos, const CRGBA &rgba);
  virtual bool setRGBAPixel(int x, int y, const CRGBA &rgba);

//uint rgbaToPixel(const CRGBA &rgba) const;
//void pixelToRGBA(uint pixel, CRGBA &rgba) const;

  void dataChanged();

  //------

 private:
  void init();
  void reset();
  void createXImage();
  void initXImage();

  int getDataSize();
  int getRowSize();
};

#endif
