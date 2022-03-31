#include <CXLib.h>
#include <CArgs.h>
#include <CImageLib.h>

extern void setRootImage(CImagePtr &image);

int
main(int argc, char **argv)
{
  CArgs args("-file:s (Image file");

  args.parse(&argc, argv);

  std::string filename = args.getStringArg("-file");

  if (! CFile::exists(filename) || ! CFile::isRegular(filename)) {
    args.usage("CXRootImage");
    exit(1);
  }

  CXMachineInst->openDisplay("");

  CImageFileSrc src(filename);

  CImagePtr image = CImageMgrInst->lookupImage(src);

  if (! image.isValid()) {
    args.usage("CXRootImage");
    exit(1);
  }

  setRootImage(image);

  return 0;
}

extern void
setRootImage(CImagePtr &image)
{
  Pixel bg = CXMachineInst->getBlackPixel();
  Pixel fg = CXMachineInst->getWhitePixel();

  GC gc = CXMachineInst->createGC(fg, bg);

  int w = CXMachineInst->getWidth ();
  int h = CXMachineInst->getHeight();

  Pixmap pixmap = CXMachineInst->createXPixmap(w, h);

  CXMachineInst->fillRectangle(pixmap, gc, 0, 0, w, h);

  CXImage *cximage = image.cast<CXImage>();

  if (cximage == NULL)
    exit(1);

  XImage *ximage = cximage->getXImage();

  int iw = image->getWidth ();
  int ih = image->getHeight();

  if (iw > w) w = iw;
  if (ih > h) h = ih;

  int dst_x = (w - iw)/2;
  int dst_y = (h - ih)/2;

  Window root = CXMachineInst->getRoot();

  CXMachineInst->putImage(pixmap, gc, ximage, 0, 0,
                          dst_x, dst_y, (uint) iw, (uint) ih);
  CXMachineInst->putImage(root  , gc, ximage, 0, 0,
                          dst_x, dst_y, (uint) iw, (uint) ih);

  CXMachineInst->setWindowBackgroundPixmap(root, pixmap);

  CXMachineInst->freeGC(gc);

  CXMachineInst->freeXPixmap(pixmap);

  CXMachineInst->clearWindow(root);

  CXMachineInst->flushEvents();

  const CXAtom &atom = CXMachineInst->getXSetRootIdAtom();

  Pixmap pixmap1;

  if (CXMachineInst->getPixmapProperty(root, atom, &pixmap1)) // delete ?
    CXMachineInst->killClient(pixmap1);

  CXMachineInst->deleteProperty(root, atom);

  CXMachineInst->flushEvents();
}
