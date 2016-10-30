#ifndef CX_PIXMAP_H
#define CX_PIXMAP_H

#include <CXDrawable.h>

class CXPixmap : public CXDrawable {
 public:
  CXPixmap(Window xwin, uint width, uint height);
  CXPixmap(uint width, uint height);

 ~CXPixmap();

  Window getWindow() const { return xwin_  ; }
  uint   getWidth () const { return width_ ; }
  uint   getHeight() const { return height_; }
  Pixmap getPixmap() const { return pixmap_; }

  bool resizePixmap(uint width, uint height);

 private:
  Window xwin_ { 0 };
  uint   width_ { 0 };
  uint   height_ { 0 };
  Pixmap pixmap_ { 0 };
};

#endif
