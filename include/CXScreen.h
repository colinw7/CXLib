#ifndef CX_SCREEN_H
#define CX_SCREEN_H

#include <std_Xt.h>
#include <CImageLib.h>
#include <CXColor.h>

class CXColorMgr;
class CXWindow;

class CXScreen {
 public:
  CXScreen(int screen_num);
//CXScreen(const CXScreen &screen);

 ~CXScreen();

//CXScreen &operator=(const CXScreen &screen);

  Display  *getDisplay() const;
  int       getScreenNum() const;
  Screen   *getScreen() const;
  Visual   *getVisual() const;
  Colormap  getColormap() const;
  int       getX() const;
  int       getY() const;
  int       getWidth() const;
  int       getHeight() const;
  int       getDepth() const;
  bool      getGrayScale() const;
  bool      getHasColormap() const;

  int getNumColors() const { return num_colors_; }

  const CXColor &getBlackColor() const { return black_color_; }
  const CXColor &getWhiteColor() const { return white_color_; }

  Pixel getBlackPixel() const;
  Pixel getWhitePixel() const;

  void      setColormap(Colormap cmap);

  Window    getRoot() const;
  CXWindow *getCXRoot() const;

  void      addWindow(CXWindow *window);
  void      removeWindow(CXWindow *window);
  CXWindow *lookupWindow(Window xwin);

  Pixel     rgbToPixel(const CRGB &rgb);
  Pixel     rgbaToPixel(const CRGBA &rgba);
  Pixel     rgbToPixel(double r, double g, double b);
  Pixel     rgbaToPixel(double r, double g, double b, double a);
  Pixel     rgbaIToPixel(uint r, uint g, uint b, uint a);

  void      setGrayScale();

  CRGB      pixelToRGB(Pixel pixel);
  CRGBA     pixelToRGBA(Pixel pixel);

  Pixmap    createMask(const CImagePtr &image);

  void windowToImage(Drawable drawable, CImagePtr &image);

  void flushEvents() const;

  bool selectWMInput() const;

  bool getWindows(Window **windows, int *num_windows);

  const CXColor &getCXColor(const CRGB &rgb);
  const CXColor &getCXColor(const CRGBA &rgba);
  const CXColor &getCXColor(Pixel pixel);

  void refresh();

  bool   getPointerPosition(int *x, int *y);
  Window getPointerWindow();

  Window getCoordWindow(int x, int y);

 private:
  void init();
  void term();

  bool allocateOwnGrayColormap();
  void setColorsUsed();
  void freeAllocatedColors();

  void decodeVisualMask(uint full_mask, int *shift, uint *mask);

 private:
  typedef std::map<Window,CXWindow *> WindowMap;

  Display    *display_ { nullptr };
  int         screen_num_ { 0 };
  Screen     *screen_ { nullptr };
  Visual     *visual_ { nullptr };
  Colormap    cmap_;
  int         x_ { 0 }, y_ { 0 };
  int         width_ { 0 }, height_ { 0 };
  int         depth_ { 0 };
  bool        gray_scale_ { false };
  bool        has_colormap_ { false };
  int         num_colors_ { 0 };
  CXColor     black_color_, white_color_;

  std::vector<XColor> colors_;
  int                 num_used_colors_ { 0 };
  std::vector<bool>   color_used_;
  std::vector<bool>   color_allocated_;

  uint red_mask_ { 0 }, green_mask_ { 0 }, blue_mask_ { 0 }, alpha_mask_ { 0 };
  int  red_shift_ { 0 }, green_shift_ { 0 }, blue_shift_ { 0 }, alpha_shift_ { 0 };

  WindowMap   window_map_;

  CXColorMgr *color_mgr_ { nullptr };
};

#endif
