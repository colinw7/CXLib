#include <CXPixmap.h>
#include <CXMachine.h>

CXPixmap::
CXPixmap(Window xwin, uint width, uint height) :
 xwin_(xwin), width_(width), height_(height)
{
  pixmap_ = CXMachineInst->createXPixmap(xwin_, width_, height_);

  setDrawable(pixmap_, width, height);
}

CXPixmap::
CXPixmap(uint width, uint height) :
 xwin_(CXMachineInst->getRoot()), width_(width), height_(height)
{
  pixmap_ = CXMachineInst->createXPixmap(xwin_, width_, height_);

  setDrawable(pixmap_, width, height);
}

CXPixmap::
~CXPixmap()
{
  CXMachineInst->freeXPixmap(pixmap_);
}

bool
CXPixmap::
resizePixmap(uint width, uint height)
{
  if (width_ == width && height_ == height)
    return false;

  // don't resize if smaller ?

  CXMachineInst->freeXPixmap(pixmap_);

  width_  = width;
  height_ = height;

  pixmap_ = CXMachineInst->createXPixmap(xwin_, width_, height_);

  return true;
}
