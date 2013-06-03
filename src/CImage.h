#ifndef CIMAGE_H
#define CIMAGE_H

#include <cstddef>
#include <CRGBA.h>
#include <CAlignType.h>
#include <CISize2D.h>
#include <CIBBox2D.h>
#include <CFileType.h>
#include <COptVal.h>
#include <CImagePtr.h>
#include <CColorComponent.h>
#include <vector>

//--------------------

typedef unsigned char uchar;

typedef uint   CIMAGE_INT32;
typedef ushort CIMAGE_INT16;
typedef uchar  CIMAGE_INT8;

enum CImageResizeType {
  CIMAGE_RESIZE_NEAREST,
  CIMAGE_RESIZE_AVERAGE,
  CIMAGE_RESIZE_BILINEAR
};

enum CImageCopyType {
  CIMAGE_COPY_ALL              = 0,
  CIMAGE_COPY_SKIP_TRANSPARENT = (1<<0)
};

class CFile;
class CLinearGradient;
class CRadialGradient;
class CImageAnim;
class CImage;
class CImagePixelIterator;

//--------------------

struct CImageTile {
  CHAlignType halign;
  CVAlignType valign;

  CImageTile(CHAlignType halign1=CHALIGN_TYPE_CENTER, CVAlignType valign1=CVALIGN_TYPE_CENTER) :
   halign(halign1), valign(valign1) {
  }
};

//--------------------

class CImage {
 private:
  class Border {
   private:
    int left_;
    int bottom_;
    int right_;
    int top_;

   public:
    Border(int left=0, int bottom=0, int right=0, int top=0) :
     left_(left), bottom_(bottom), right_(right), top_(top) {
    }

    void set(int left, int bottom, int right, int top) {
      left_ = left; bottom_ = bottom; right_ = right; top_ = top;
    }

    void get(int *left, int *bottom, int *right, int *top) {
      *left = left_; *bottom = bottom_; *right = right_; *top = top_;
    }

    int getLeft  () const { return left_  ; }
    int getBottom() const { return bottom_; }
    int getRight () const { return right_ ; }
    int getTop   () const { return top_   ; }
  };

  typedef std::vector<CRGBA>     ColorList;
  typedef std::vector<CImagePtr> ImagePtrList;

  static bool            combine_enabled_;
  static CRGBACombineDef combine_def_;

  CFileType  type_;
  CISize2D   size_;
  Border     border_;
  uint      *data_;
  ColorList  colors_;
  CIBBox2D   window_;
  CRGBA      bg_;

  friend class CImageMgr;

 public:
  class PixelIterator {
   public:
    typedef std::forward_iterator_tag iterator_category;
    typedef uint                      value_type;
    typedef ptrdiff_t                 difference_type;
    typedef uint *                    pointer;
    typedef uint &                    reference;

   private:
    struct current {
      const CImage *image;
      uint          pos1, pos2;
      uint          width;

      current() :
       image(NULL), pos1(0), pos2(0), width(0) {
      }

      void inc() {
        ++pos1;
      }
    };

    current cur_;

   public:
    PixelIterator() :
     cur_() {
    }

    PixelIterator(const CImage *image, bool is_end) :
     cur_() {
      cur_.image = image;

      cur_.pos2  = cur_.image->size_.area();
      cur_.width = cur_.image->size_.width;

      if (! is_end)
        cur_.pos1 = 0;
      else
        cur_.pos1 = cur_.pos2;
    }

    uint       *operator->()       { return &cur_.image->data_[cur_.pos1]; }
    const uint *operator->() const { return &cur_.image->data_[cur_.pos1]; }
    uint       *operator* ()       { return &cur_.image->data_[cur_.pos1]; }
    const uint *operator* () const { return &cur_.image->data_[cur_.pos1]; }

    PixelIterator &operator++() {
      cur_.inc();

      return *this;
    }

    PixelIterator operator++(int) {
      PixelIterator t = *this;

      cur_.inc();

      return t;
    }

    void getRGBPixel(CRGBA &rgb) const {
      return cur_.image->getRGBPixel(cur_.pos1, rgb);
    }

    bool operator==(const PixelIterator &i) const {
      return (cur_.pos1 == i.cur_.pos2);
    }

    bool operator!=(const PixelIterator &i) const {
      return (cur_.pos1 != i.cur_.pos2);
    }
  };

  // Combine Mode
 public:
  static void setCombineEnabled(bool enabled) {
    combine_enabled_ = enabled;
  }

  static void setCombineSrcMode(CRGBACombineMode mode) {
    combine_def_.src_mode = mode;
  }

  static CRGBACombineMode getCombineSrcMode() {
    return combine_def_.src_mode;
  }

  static void setCombineDstMode(CRGBACombineMode mode) {
    combine_def_.dst_mode = mode;
  }

  static CRGBACombineMode getCombineDstMode() {
    return combine_def_.dst_mode;
  }

  static void setCombineFunc(CRGBACombineFunc combine_func) {
    combine_def_.func = combine_func;
  }

  static CRGBACombineFunc getCombineFunc() {
    return combine_def_.func;
  }

  static void setCombineFactor(const CRGBA &rgba) {
    combine_def_.factor = rgba;
  }

  static CRGBA &getCombineFactor() {
    return combine_def_.factor;
  }

  // Create
 protected:
  CImage();
  CImage(const CISize2DT<int> &size);
  CImage(int width, int height);
  CImage(const CImage &image, int x, int y, int width, int height);

 public:
  CImage(const CImage &image);

  virtual ~CImage();

 protected:
  CImage &operator=(const CImage &image);

 public:
  bool isValid() const {
    return (size_.area() > 0);
  }

  virtual CImagePtr dup() const;

  void replace(CImagePtr image);

  //----

  // Get/Set Data

 public:
  CFileType getType() const { return type_; }
  void      setType(CFileType type) { type_ = type; }

  std::string getTypeName() const;

  void setSize(const CISize2D &size);
  void setSize(int width, int height);

  void setDataSize(const CISize2D &size);
  void setDataSize(int width, int height);

  virtual void setDataSizeV(int width, int height);

  uint getWidth () const { return size_.width ; }
  uint getHeight() const { return size_.height; }

  void getSize(CISize2D &size) const {
    size = size_;
  }

  void getSize(int *width, int *height) const {
    *width  = size_.width;
    *height = size_.height;
  }

  CISize2D getSize() const {
    return size_;
  }

  void setBorder(int  left, int  bottom, int  right, int  top);
  void getBorder(int *left, int *bottom, int *right, int *top);

  int getLeft  () const { return border_.getLeft  (); }
  int getBottom() const { return border_.getBottom(); }
  int getRight () const { return border_.getRight (); }
  int getTop   () const { return border_.getTop   (); }

  void setWindow(const CIBBox2D &bbox);
  void setWindow(int  left, int  bottom, int  right, int  top);

  void getWindow(int *left, int *bottom, int *right, int *top) const;

  void resetWindow();

  void setBackground(const CRGBA &bg) { bg_ = bg; }

  const CRGBA &getBackground() const { return bg_; }

  //----

  // Read/Write

 public:
  bool read(const std::string &filename);

  bool read(const uchar *data, size_t len);

  bool read(CFile *file);
  bool read(CFile *file, CFileType type);

  bool readHeader(const std::string &filename);
  bool readHeader(const uchar *data, size_t len);

  bool readHeader(CFile *file);
  bool readHeader(CFile *file, CFileType type);

  static CImagePtr create(const std::string &filename);
  static CImagePtr create(const std::string &filename, CFileType type);

  static CImagePtr create(const char *filename);
  static CImagePtr create(const char *filename, CFileType type);

  static CImagePtr create(const uchar *data, size_t len);

  static CImagePtr create(CFile *file);
  static CImagePtr create(CFile *file, CFileType type);

  static CImagePtr create(const char **strings, uint num_strings, CFileType type);

  static CImagePtr createHeader(const std::string &filename);
  static CImagePtr createHeader(const std::string &filename, CFileType type);

  static CImagePtr createHeader(CFile *file);
  static CImagePtr createHeader(CFile *file, CFileType type);

 public:
  bool write(const std::string &filename);
  bool write(const std::string &filename, CFileType type);

  bool write(const char *filename);
  bool write(const char *filename, CFileType type);

  bool write(CFile *file);
  bool write(CFile *file, CFileType type);

  //----

  // Functions

 public:
  void clamp(int *x, int *y) {
    *x = std::min(std::max(0, *x), size_.width  - 1);
    *y = std::min(std::max(0, *y), size_.height - 1);
  }

  bool validPixel(const CIPoint2D &point) const;
  bool validPixel(int x, int y) const;
  bool validPixel(int pos) const;

  void setColorIndexData(uchar *data);
  void setColorIndexData(uint *data);
  void setColorIndexData(uint pixel);
  void setColorIndexData(uint pixel, int left, int bottom, int right, int top);

  int getColorIndexPixel(int x, int y) const;
  int getColorIndexPixel(int pos) const;

  virtual bool setColorIndexPixel(int x, int y, uint pixel);
  virtual bool setColorIndexPixel(int pos, uint pixel);

  virtual void setRGBAData(uint *data);

  virtual void setRGBAData(const CRGBA &rgba);
  virtual void setRGBAData(const CRGBA &rgba, int left, int bottom, int right, int top);

  bool setRGBAPixel(int x, int y, double r, double g, double b, double a=1.0);
  bool setRGBAPixel(int pos, double r, double g, double b, double a=1.0);

  bool setRGBAPixel(const CIPoint2D &point, const CRGBA &rgba);

  virtual bool setRGBAPixel(int x, int y, const CRGBA &rgba);
  virtual bool setRGBAPixel(int pos, const CRGBA &rgba);

  bool setGrayPixel(int x, int y, double gray);
  bool setGrayPixel(int pos, double gray);

  void getRGBAPixel(int x, int y, double *r, double *g, double *b, double *a) const;
  void getRGBAPixel(int ind, double *r, double *g, double *b, double *a) const;
  void getRGBAPixel(const CIPoint2D &point, CRGBA &rgba) const;
  void getRGBAPixel(int x, int y, CRGBA &rgba) const;
  void getRGBAPixel(int ind, CRGBA &rgba) const;

  void getRGBAPixelI(int ind, uint *r, uint *g, uint *b, uint *a) const;
  void getRGBAPixelI(int x, int y, uint *r, uint *g, uint *b, uint *a) const;

  void getRGBPixel(int ind, CRGBA &rgb) const;
  void getRGBPixel(int x, int y, CRGBA &rgb) const;

  void getGrayPixel(int ind, double *gray) const;
  void getGrayPixel(int x, int y, double *gray) const;
  void getGrayPixelI(int ind, uint *gray) const;
  void getGrayPixelI(int x, int y, uint *gray) const;

  bool hasColormap() const;

  int addColor(double r, double g, double b, double a=1.0);
  int addColorI(uint r, uint g, uint b, uint a=255);
  int addColor(const CRGBA &rgba);

  CRGBA getColor(uint pixel) const;
  void  getColor(uint pixel, CRGBA &rgba) const;
  void  setColor(uint pixel, const CRGBA &rgba);

  void getColorRGBA(uint pixel, double *r, double *g, double *b, double *a) const;
  void getColorRGBA(uint pixel, CRGBA &rgba) const;

  void getColorRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a) const;

  void getColorRGB(uint pixel, CRGBA &rgb) const;

  void getColorGray(uint pixel, double *gray) const;

  int getNumColors() const;

  void setTransparentColor(const CRGBA &rgba);
  void setTransparentColor(uint pixel);

  int  getTransparentColor() const;

  bool isTransparentColor(uint pixel) const;

  bool isTransparent(COptReal tol=COptReal(0.1)) const;
  bool isTransparent(int x, int y, COptReal tol=COptReal(0.1)) const;
  bool isTransparent(int pos, COptReal tol=COptReal(0.1)) const;

  bool isTransparentI(COptInt tol=COptInt(5)) const;
  bool isTransparentI(int x, int y, COptInt tol=COptInt(5)) const;
  bool isTransparentI(int pos, COptInt tol=COptInt(5)) const;

  double getAlpha(int x, int y) const;
  double getAlpha(int pos) const;

  uint getAlphaI(int x, int y) const;
  uint getAlphaI(int pos) const;

  void deleteColors();

  void setAlphaByGray(bool positive=true);
  void setGrayByAlpha(bool positive=true);

  void setAlphaByColor(const CRGBA &rgb, double a=1.0);

  void setAlpha(double a);
  void setAlphaByImage(CImagePtr image);

  void scaleAlpha(double a);

  //----

  static uint rgbaToPixel(const CRGBA &rgba);

  static uint rgbaToPixel(double r, double g, double b, double a=1.0);

  static uint grayToPixel(double gray);
  static uint grayToPixelI(uint gray);

  static uint rgbaToPixelI(uint r, uint g, uint b, uint a=255);

  //----

  static void pixelToRGBA(uint pixel, CRGBA &rgba);

  static void pixelToRGBA(uint pixel, double *r, double *g, double *b, double *a);

  static void pixelToGray(uint pixel, double *gray);
  static void pixelToGrayI(uint pixel, uint *gray);

  static void pixelToRGBAI(uint pixel, uint *r, uint *g, uint *b, uint *a);
  static void pixelToRGBI(uint pixel, uint *r, uint *g, uint *b);

  static void pixelToAlpha(uint pixel, double *a);
  static void pixelToAlphaI(uint pixel, uint *a);

  //----

  int findColor(const CRGBA &rgba);

  //----

  uint memUsage() const;

  //----

 public:
  uint *getData() const {
    return data_;
  }

  uint getData(int ind) const {
    return data_[ind];
  }

  uint getData(int x, int y) const {
    return data_[y*size_.width + x];
  }

  void setData(int ind, uint data) const {
    data_[ind] = data;
  }

  void setData(int x, int y, uint data) const {
    data_[y*size_.width + x] = data;
  }

  uint getDataSize() const {
    return size_.width*size_.height;
  }

  //------

  virtual void dataChanged() { }

  //------

 public:
  typedef PixelIterator pixel_iterator;

  pixel_iterator       pixel_begin();
  const pixel_iterator pixel_begin() const;
  pixel_iterator       pixel_end  ();
  const pixel_iterator pixel_end  () const;

  //------

 public:
  double boxScale(int w, int h) const;

  //------

  // Resize (New Image), Reshape (Existing Image)

 private:
  static CImageResizeType resize_type;

 public:
  static CImageResizeType setResizeType(CImageResizeType type);

  CImagePtr scale(double s) const;
  CImagePtr scale(double xs, double ys) const;

  CImagePtr scaleKeepAspect(double s) const;
  CImagePtr scaleKeepAspect(double xs, double ys) const;

  CImagePtr resize(const CISize2D &size) const;
  CImagePtr resize(int width, int height) const;

  CImagePtr resizeWidth(int width) const;
  CImagePtr resizeHeight(int height) const;

  CImagePtr resizeMax(int size) const;
  CImagePtr resizeMin(int size) const;

  CImagePtr resizeKeepAspect(const CISize2D &size) const;
  CImagePtr resizeKeepAspect(int width, int height) const;

  bool reshape(int width, int height);

  bool reshapeWidth(int width);
  bool reshapeHeight(int height);

  bool reshapeMax(int size);
  bool reshapeMin(int size);

  bool reshapeKeepAspect(const CISize2D &size);
  bool reshapeKeepAspect(int width, int height);

  void sampleNearest(double x, double y, CRGBA &rgb) const;
  void sampleBilinear(double x, double y, CRGBA &rgb) const;

 private:
  static void reshapeNearest(CImagePtr old_image, CImagePtr &new_image);
  void reshapeNearest(CImagePtr &new_image) const;

  static void reshapeAverage(CImagePtr old_image, CImagePtr &new_image);
  void reshapeAverage(CImagePtr &new_image) const;

  static void reshapeBilinear(CImagePtr old_image, CImagePtr &new_image);
  void reshapeBilinear(CImagePtr &new_image) const;

  //------

  // Rotate

 public:
  CImagePtr rotate(double angle);

  //------

  // Scroll

 public:
  void scroll(int dx, int dy);

  void scrollX(int offset);
  void scrollY(int offset);

  //------

  // Flip

 public:
  CImagePtr flippedH() const;
  CImagePtr flippedV() const;
  CImagePtr flippedHV() const;

  void flipH();
  void flipV();
  void flipHV();

  //------

  // Tile

 public:
  CImagePtr tile(int width, int height, const CImageTile &tile);

  //------

  // Convert

 private:
  static CRGBA  convertBg_;
  static double convertAlphaTol_;

 public:
  enum ConvertMethod {
    CONVERT_NEAREST_LOGICAL,  // replace with nearest RGB of new colors
    CONVERT_NEAREST_PHYSICAL  // replace with nearest pixel of new color
  };

  static void setConvertBg(const CRGBA &bg) { convertBg_ = bg; }

  static void setConvertAlphaTol(double tol) {
    assert(tol >= 0.0 && tol <= 1.0);

    convertAlphaTol_ = tol;
  }

  void convertToNColors(uint ncolors=256, ConvertMethod method=CONVERT_NEAREST_LOGICAL);

  void convertToColorIndex();

  void convertToRGB();

  //------

  // Mask

 public:
  CImagePtr createMask() const;

  void alphaMask(CImagePtr mask, int xo = 0, int yo = 0);
  void alphaMaskRGBA(CImagePtr mask, const CRGBA &rgba, int xo = 0, int yo = 0);

  CImagePtr createRGBAMask(const CRGBA &rgba = CRGBA(0.2125, 0.7154, 0.0721));

  //------

  // Gray Scale

 public:
  CImagePtr grayScaled() const;

  void grayScale();

  //------

  // Draw

 public:
  void fillRGBARectangle(int x1, int y1, int x2, int y2, CRGBA &rgba);
  void fillColorIndexRectangle(int x1, int y1, int x2, int y2, int ind);

  void drawColorIndexPoint(int x, int y, int color_ind);
  void drawColorIndexPoint(int i, int color_ind);

  void drawRGBAPoint(int x, int y, const CRGBA &rgba);
  void drawRGBAPoint(int i, const CRGBA &rgba);

  //------

  // Line Art

 public:
  void lineArt(double tolerance);

  //------

  // Combine

 public:
  static void combine(CImagePtr image1, CImagePtr image2, const CRGBACombineDef &def);

  void combine(CImagePtr image, const CRGBACombineDef &def);

  static void combineDef(CImagePtr image1, CImagePtr image2);

  void combineDef(CImagePtr image);

  static void combine(CImagePtr image1, CImagePtr image2);

  void combine(CImagePtr image);

  //------

  // Merge

 public:
  static CImagePtr merge(CImagePtr image1, CImagePtr image2);

  //------

  // Invert

 public:
  CImagePtr inverted() const;

  void invert();

  //------

  // Copy

 private:
  static CImageCopyType copy_type;

 public:
  static CImageCopyType setCopyType(CImageCopyType type);

  CImagePtr subImage(int x=0, int y=0, int width=-1, int height=-1) const;

  void copyFrom(CImagePtr src);
  void copyFrom(CImagePtr src, int dest_x, int dest_y);

  void copyTo(CImagePtr &dst, int dest_x=0, int dest_y=0) const;

  void subCopyFrom(CImagePtr  src, int src_x=0, int src_y=0, int width=-1, int height=-1,
                   int dest_x=0, int dest_y=0);
  void subCopyTo  (CImagePtr &dst, int src_x=0, int src_y=0, int width=-1, int height=-1,
                   int dest_x=0, int dest_y=0) const;

  void combine(CImagePtr image, int x, int y);

  static CImagePtr combine(CImagePtr image1, CImagePtr image2, int x, int y);

  void combineAlpha(CImagePtr image, int x, int y);

  //------

  // Process

 public:
  void removeSinglePixels();

  ImagePtrList colorSplit();
  CImagePtr    colorSplit(int i);

  void setNumColors(int num_colors);

  void sepia();
  void monochrome();
  void twoColor(const CRGBA &bg, const CRGBA &fg);

  void applyColorMatrix(double *m);

  void rotateHue(double dh);

  void saturate(double ds);

  void luminanceToAlpha();

  void linearFunc(CColorComponent component, double scale, double offset);

  void gammaFunc(CColorComponent component, double amplitude, double exponent, double offset);

  void tableFunc(CColorComponent component, const std::vector<double> &values);

  void discreteFunc(CColorComponent component, const std::vector<double> &values);

 private:
  CImagePtr colorSplitByData(uint data);

  void getClosestColors(int &i1, int &i2);

  void replaceColor(int i1, int i2);

  //------

  // Filter

 public:
  static void unsharpMask(CImagePtr src, CImagePtr &dst, double strength = 2.0);

  CImagePtr unsharpMask(double strength = 2.0);

  void unsharpMask(CImagePtr &dst, double strength = 2.0);

  static void convolve(CImagePtr src, CImagePtr &dst, const char *kernel, int size, int divisor);

  CImagePtr convolve(const char *kernel, int size, int divisor);

  void convolve(CImagePtr &dst, const char *kernel, int size, int divisor);

  static bool gaussianBlur(CImagePtr src, CImagePtr &dst, double bx, double by, int nx=0, int ny=0);

  bool gaussianBlur(double bx, double by, int nx=0, int ny=0);

  bool gaussianBlur(CImagePtr &dst, double bx, double by, int nx=0, int ny=0);

  void turbulence(bool fractal, double baseFreq, int numOctaves, int seed);

  //------

  // Flood Fill

 public:
  void floodFill(int x, int y, const CRGBA &rgba);
  void floodFill(int x, int y, int pixel);

  void fillLargestRect(int x, int y, const CRGBA &rgba);
  void fillLargestRect(int x, int y, int pixel);

  //------

  // Gradient

 public:
  void linearGradient(const CLinearGradient &gradient);
  void radialGradient(const CRadialGradient &gradient);

  //------

  // Util

 public:
  static CIMAGE_INT32 swapBytes32(CIMAGE_INT32 i);
  static CIMAGE_INT16 swapBytes16(CIMAGE_INT16 i);

  //--------------------------

  // BMP

 protected:
  static CImagePtr createBMP(CFile *file);
  static CImagePtr createBMPHeader(CFile *file);

 public:
  bool readBMP(CFile *file);
  bool readBMPHeader(CFile *file);

  bool writeBMP(CFile *file);

  //------

  // EPS

 protected:
  static CImagePtr createEPS(CFile *file);
  static CImagePtr createEPSHeader(CFile *file);

 public:
  bool readEPS(CFile *file);
  bool readEPSHeader(CFile *file);

  bool writeEPS(CFile *file);

  //------

  // GIF

 protected:
  static CImagePtr createGIF(CFile *file);
  static CImagePtr createGIFHeader(CFile *file);

 public:
  bool readGIF(CFile *file);
  bool readGIFHeader(CFile *file);

  bool writeGIF(CFile *file);

  static CImageAnim *createGIFAnim(CFile *file);

  //------

  // ICO

 protected:
  static CImagePtr createICO(CFile *file);
  static CImagePtr createICOHeader(CFile *file);

 public:
  bool readICO(CFile *file);
  bool readICOHeader(CFile *file);

  bool writeICO(CFile *file);

  //------

  // IFF

 protected:
  static CImagePtr createIFF(CFile *file);
  static CImagePtr createIFFHeader(CFile *file);

 public:
  bool readIFF(CFile *file);
  bool readIFFHeader(CFile *file);

  bool writeIFF(CFile *file);

  //------

  // JPG

 protected:
  static CImagePtr createJPG(CFile *file);
  static CImagePtr createJPGHeader(CFile *file);

 public:
  bool readJPG(CFile *file);
  bool readJPGHeader(CFile *file);

  bool writeJPG(CFile *file);

  //------

  // PCX

 protected:
  static CImagePtr createPCX(CFile *file);
  static CImagePtr createPCXHeader(CFile *file);

 public:
  bool readPCX(CFile *file);
  bool readPCXHeader(CFile *file);

  bool writePCX(CFile *file);

  //------

  // PNG

 protected:
  static CImagePtr createPNG(CFile *file);
  static CImagePtr createPNGHeader(CFile *file);

 public:
  bool readPNG(CFile *file);
  bool readPNGHeader(CFile *file);

  bool writePNG(const std::string &filename);
  bool writePNG(CFile *file);

  //------

  // PPM

 protected:
  static CImagePtr createPPM(CFile *file);
  static CImagePtr createPPMHeader(CFile *file);

 public:
  bool readPPM(CFile *file);
  bool readPPMHeader(CFile *file);

  bool writePPM(CFile *file);

  //------

  // PS

 protected:
  static CImagePtr createPS(CFile *file);
  static CImagePtr createPSHeader(CFile *file);

 public:
  bool readPS(CFile *file);
  bool readPSHeader(CFile *file);

  bool writePS(CFile *file);

  //------

  // PSP

 protected:
  static CImagePtr createPSP(CFile *file);
  static CImagePtr createPSPHeader(CFile *file);

 public:
  bool readPSP(CFile *file);
  bool readPSPHeader(CFile *file);

  bool writePSP(CFile *file);

  //------

  // SGI

 protected:
  static CImagePtr createSGI(CFile *file);
  static CImagePtr createSGIHeader(CFile *file);

 public:
  bool readSGI(CFile *file);
  bool readSGIHeader(CFile *file);

  bool writeSGI(CFile *file);

  //------

  // SVG

 protected:
  static CImagePtr createSVG(CFile *file);
  static CImagePtr createSVGHeader(CFile *file);

 public:
  bool readSVG(CFile *file);
  bool readSVGHeader(CFile *file);

  bool writeSVG(CFile *file);

  //------

  // TGA

 protected:
  static CImagePtr createTGA(CFile *file);
  static CImagePtr createTGAHeader(CFile *file);

 public:
  bool readTGA(CFile *file);
  bool readTGAHeader(CFile *file);

  bool writeTGA(CFile *file);

  //------

  // TIF

 protected:
  static CImagePtr createTIF(CFile *file);
  static CImagePtr createTIFHeader(CFile *file);

 public:
  bool readTIF(CFile *file);
  bool readTIFHeader(CFile *file);

  bool writeTIF(CFile *file);

  //------

  // XBM

 protected:
  static CImagePtr createXBM(CFile *file);
  static CImagePtr createXBM(uchar *data, int width, int height);
  static CImagePtr createXBMHeader(CFile *file);

 public:
  bool readXBM(CFile *file);
  bool readXBMHeader(CFile *file);

  bool writeXBM(CFile *file);

  bool readXBM(const uchar *data, int w, int h);

  //------

  // XPM

 public:
  static void setXPMHotSpot(int x, int y);

 protected:
  static CImagePtr createXPM(CFile *file);
  static CImagePtr createXPM(const char **strings, uint num_strings);

  static CImagePtr createXPMHeader(CFile *file);

 public:
  bool readXPM(CFile *file);
  bool readXPMHeader(CFile *file);

  bool writeXPM(CFile *file);

  bool readXPM(const char **strs, uint num_strs);

  //------

  // XWD

 protected:
  static CImagePtr createXWD(CFile *file);
  static CImagePtr createXWDHeader(CFile *file);

 public:
  bool readXWD(CFile *file);
  bool readXWDHeader(CFile *file);

  bool writeXWD(CFile *file);
};

#endif
