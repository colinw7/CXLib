#ifndef CX_DRAG_WINDOW_H
#define CX_DRAG_WINDOW_H

#include <CXWindow.h>

class CXDragWindow {
 private:
  CXWindow *cxwindow_;

 public:
  CXDragWindow(CXScreen &screen, Window window, int width, int height);
 ~CXDragWindow();

  CXWindow *getCXWindow() const { return cxwindow_; }

  void dragStart(int x, int y);
  void dragMotion(int x, int y);
  void dragEnd(int x, int y);

  void setSize(int width, int height);
};

#endif
