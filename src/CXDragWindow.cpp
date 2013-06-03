#include "CXLibI.h"

CXDragWindow::
CXDragWindow(CXScreen &screen, Window window, int width, int height)
{
  cxwindow_ = new CXWindow(screen, window, 0, 0, width, height);

  cxwindow_->unmap();
}

CXDragWindow::
~CXDragWindow()
{
  delete cxwindow_;
}

void
CXDragWindow::
dragStart(int x, int y)
{
  cxwindow_->setPosition(x, y);

  cxwindow_->map();

  cxwindow_->getCXScreen().flushEvents();
}

void
CXDragWindow::
dragMotion(int x, int y)
{
  cxwindow_->setPosition(x, y);

  cxwindow_->getCXScreen().flushEvents();
}

void
CXDragWindow::
dragEnd(int, int)
{
  cxwindow_->unmap();

  cxwindow_->getCXScreen().flushEvents();
}

void
CXDragWindow::
setSize(int width, int height)
{
  cxwindow_->setSize(width, height);
}
