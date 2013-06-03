#ifndef CX_PIXMAP_H
#define CX_PIXMAP_H

#include <CXDrawable.h>

class CXPixmap : public CXDrawable {
 private:
  Window xwin_;
  uint   width_;
  uint   height_;
  Pixmap pixmap_;

 public:
  CXPixmap(Window xwin, uint width, uint height);
  CXPixmap(uint width, uint height);

 ~CXPixmap();

  Window getWindow() const { return xwin_  ; }
  uint   getWidth () const { return width_ ; }
  uint   getHeight() const { return height_; }
  Pixmap getPixmap() const { return pixmap_; }

  bool resizePixmap(uint width, uint height);
};

#endif
