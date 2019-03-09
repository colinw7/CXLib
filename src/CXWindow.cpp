#include <CXWindow.h>
#include <CXLibPixelRenderer.h>
#include <COSUser.h>

CWindow *
CXWindowFactory::
createWindow(int x, int y, uint width, uint height)
{
  CXWindow *window = new CXWindow(x, y, width, height);

  return window;
}

CWindow *
CXWindowFactory::
createWindow(CWindow *parent, int x, int y, uint width, uint height)
{
  CXWindow *xparent = dynamic_cast<CXWindow *>(parent);

  assert(xparent);

  CXWindow *window = new CXWindow(xparent, x, y, width, height);

  return window;
}

//-----------

CXWindow::
CXWindow(CWindowType type) :
 screen_(*CXMachineInst->getCXScreen(0)), parent_xwindow_(screen_.getRoot())
{
  init(type);
}

CXWindow::
CXWindow(CXScreen &screen, Window parent_window, int x, int y, uint width, uint height) :
 screen_(screen), parent_xwindow_(parent_window), x_(x), y_(y), width_(width), height_(height)
{
  init(CWINDOW_TYPE_NORMAL);
}

CXWindow::
CXWindow(CXWindow *parent_window, int x, int y, uint width, uint height) :
 screen_(parent_window ? parent_window->getCXScreen() : *CXMachineInst->getCXScreen(0)),
 parent_window_ (parent_window), parent_xwindow_(parent_window ? parent_window->getXWindow() : 0),
 x_(x), y_(y), width_(width), height_(height)
{
  init(CWINDOW_TYPE_NORMAL);
}

CXWindow::
CXWindow(CXScreen &screen, int x, int y, uint width, uint height, CWindowType type) :
 screen_(screen), parent_xwindow_(screen_.getRoot()), x_(x), y_(y), width_(width), height_(height)
{
  init(type);
}

CXWindow::
CXWindow(int x, int y, uint width, uint height, CWindowType type) :
 screen_(*CXMachineInst->getCXScreen(0)), parent_xwindow_(screen_.getRoot()),
 x_(x), y_(y), width_(width), height_(height)
{
  init(type);
}

CXWindow::
CXWindow(uint width, uint height, CWindowType type) :
 screen_(*CXMachineInst->getCXScreen(0)), parent_xwindow_(screen_.getRoot()),
 width_(width), height_(height)
{
  init(type);
}

CXWindow::
CXWindow(CXScreen &screen, Window window) :
 screen_(screen), parent_xwindow_(screen_.getRoot()), window_(window)
{
  init(window_);
}

CXWindow::
CXWindow(CXScreen &screen, Window window, uint width, uint height) :
 screen_(screen), parent_xwindow_(screen_.getRoot()), window_(window)
{
  init(window_, width, height);
}

CXWindow::
CXWindow(Window window) :
 screen_(*CXMachineInst->getCXScreen(0)), parent_xwindow_(screen_.getRoot()), window_(window)
{
  init(window_);
}

CXWindow::
CXWindow(Window window, uint width, uint height) :
 screen_(*CXMachineInst->getCXScreen(0)), parent_xwindow_(screen_.getRoot()), window_(window)
{
  init(window_, width, height);
}

CXWindow::
~CXWindow()
{
  term();
}

void
CXWindow::
init(CWindowType type)
{
  init1(type);

  create();
}

void
CXWindow::
init(Window window)
{
  init1(CWINDOW_TYPE_NORMAL);

  window_ = window;

  screen_.addWindow(this);

  graphics_ = std::make_unique<CXGraphics>(screen_, window_);

  CXMachineInst->getWindowGeometry(window_, &x_, &y_, &width_, &height_, nullptr);
}

void
CXWindow::
init(Window window, uint width, uint height)
{
  init1(CWINDOW_TYPE_NORMAL);

  window_ = window;

  screen_.addWindow(this);

  graphics_ = std::make_unique<CXGraphics>(screen_, window_);

  x_      = 0;
  y_      = 0;
  width_  = width;
  height_ = height;

  last_width_  = 0;
  last_height_ = 0;
}

void
CXWindow::
init1(CWindowType type)
{
  type_ = type;

  mapped_ = false;

  display_ = screen_.getDisplay();

  window_ = None;

  graphics_     = nullptr;
  xor_graphics_ = nullptr;

  cursor_ = nullptr;

  renderer_ = nullptr;

  renderer_alloc_ = false;

  event_adapter_ = nullptr;

  setEventAdapter(new CXWindowEventAdapter(this));

  last_width_  = 0;
  last_height_ = 0;
}

void
CXWindow::
term()
{
  destroy();
}

void
CXWindow::
create()
{
  if (window_ == None) {
    XSetWindowAttributes attrs;

    uint attr_mask = 0;

    attr_mask         |= CWBitGravity;
    attrs.bit_gravity  = ForgetGravity;

    if (type_ == CWINDOW_TYPE_OVERRIDE) {
      attr_mask               |= CWOverrideRedirect;
      attrs.override_redirect  = True;
    }

    window_ =
      CXMachineInst->createWindow(parent_xwindow_,
                                  x_, y_, width_, height_, 0,
                                  attr_mask, &attrs);

    if (type_ == CWINDOW_TYPE_OVERRIDE)
      CXMachineInst->setWMTransientFor(window_, parent_xwindow_);

    screen_.addWindow(this);

    graphics_ = std::make_unique<CXGraphics>(screen_, window_);

    cursor_ = std::make_unique<CXCursor>(CURSOR_TYPE_ARROW);

    setWindowTitle("Window");
  }
  else {
    move(x_, y_);

    resize(width_, height_);
  }
}

void
CXWindow::
destroy()
{
  screen_.removeWindow(this);

  CXMachineInst->destroyWindow(window_);

  if (renderer_alloc_)
    delete renderer_;

  renderer_ = nullptr;

  init1(type_);
}

CXLibPixelRenderer *
CXWindow::
getPixelRenderer() const
{
  if (! renderer_) {
    CXWindow *th = const_cast<CXWindow *>(this);

    th->renderer_ = new CXLibPixelRenderer(th);
  }

  return renderer_;
}

CXScreen &
CXWindow::
getCXScreen() const
{
  return screen_;
}

CXWindow *
CXWindow::
getTopWindow()
{
  if (parent_window_)
    return parent_window_->getTopWindow();

  return this;
}

CXGraphics *
CXWindow::
getCXGraphics() const
{
  return graphics_.get();
}

CXGraphics *
CXWindow::
getXorCXGraphics() const
{
  return xor_graphics_.get();
}

void
CXWindow::
getFont(CFontPtr &font) const
{
  graphics_->getFont(font);
}

void
CXWindow::
setFont(const std::string &name, CFontStyle style, int size)
{
  CFontPtr font = CFontMgrInst->lookupFont(name, style, size);

  setFont(font);
}

void
CXWindow::
setFont(const std::string &full_name)
{
  CFontPtr font = CFontMgrInst->lookupFont(full_name);

  setFont(font);
}

void
CXWindow::
setFont(CFontPtr font)
{
  if (graphics_)
    graphics_->setFont(font);

  if (xor_graphics_)
    xor_graphics_->setFont(font);
}

CXCursor &
CXWindow::
getCXCursor() const
{
  return *cursor_;
}

void
CXWindow::
setCursor(CCursorType type)
{
  cursor_ = std::make_unique<CXCursor>(screen_, type);

  CXMachineInst->setCursor(window_, cursor_->getXCursor());
}

void
CXWindow::
setCursor(CXCursor &cursor)
{
  cursor_ = std::make_unique<CXCursor>(cursor);

  CXMachineInst->setCursor(window_, cursor_->getXCursor());
}

void
CXWindow::
unsetCursor()
{
  cursor_ = nullptr;

  CXMachineInst->unsetCursor(window_);
}

uint
CXWindow::
getWidth() const
{
  getSize(nullptr, nullptr);

  return width_;
}

uint
CXWindow::
getHeight() const
{
  getSize(nullptr, nullptr);

  return height_;
}

void
CXWindow::
setEventAdapter(CEventAdapter *adapter)
{
  event_adapter_ = EventAdapterP(adapter);
}

CXEventAdapter *
CXWindow::
getXEventAdapter() const
{
  return dynamic_cast<CXEventAdapter *>(event_adapter_.get());
}

void
CXWindow::
setPosition(int x, int y)
{
  move(x, y);
}

void
CXWindow::
getPosition(int *x, int *y) const
{
  *x = getX();
  *y = getY();
}

void
CXWindow::
getRootPosition(int *x, int *y) const
{
  CXMachineInst->getWindowRootPos(window_, x, y);

  *x += getX();
  *y += getY();
}

void
CXWindow::
move(int x, int y)
{
  if (x != x_ || y != y_) {
    x_ = x;
    y_ = y;

    CXMachineInst->moveWindow(window_, x_, y_);
  }
}

void
CXWindow::
setSize(uint width, uint height)
{
  resize(width, height);
}

void
CXWindow::
getSize(uint *width, uint *height) const
{
  CXWindow *th = const_cast<CXWindow *>(this);

  if (th->graphics_)
    th->graphics_->getSize(&th->width_, &th->height_);
  else
    th->width_ = 1;

  if (width ) *width  = width_;
  if (height) *height = height_;
}

void
CXWindow::
getScreenSize(uint *width, uint *height) const
{
  if (width ) *width  = getCXScreen().getWidth ();
  if (height) *height = getCXScreen().getHeight();
}

void
CXWindow::
resize(uint width, uint height)
{
  if ((int) width != width_ || (int) height != height_) {
    width_  = width;
    height_ = height;

    CXMachineInst->resizeWindow(window_, width_, height_);
  }
}

void
CXWindow::
map()
{
  selectAllEvents();

  CXMachineInst->mapWindow(window_);

  move(x_, y_);

  resize(width_, height_);

  mapped_ = true;
}

void
CXWindow::
unmap()
{
  mapped_ = false;

  CXMachineInst->unmapWindow(window_);
}

bool
CXWindow::
isMapped()
{
  return mapped_;
}

Display *
CXWindow::
getDisplay() const
{
  return display_;
}

Window
CXWindow::
getXWindow() const
{
  return window_;
}

GC
CXWindow::
getXGC() const
{
  if (graphics_)
    return graphics_->getXGC();
  else
    return 0;
}

Visual *
CXWindow::
getXVisual() const
{
  return screen_.getVisual();
}

Colormap
CXWindow::
getXColormap() const
{
  return screen_.getColormap();
}

void
CXWindow::
selectAllEvents()
{
  selectButtonInput();
  selectKeyInput();
  selectExposures();
  selectEnterLeave();
  selectVisibility();
  selectPropertyNotify();

  CXMachineInst->addInput(window_, StructureNotifyMask);
}

void
CXWindow::
selectButtonInput()
{
  CXMachineInst->addInput(window_, ButtonPressMask | ButtonMotionMask | ButtonReleaseMask);
}

void
CXWindow::
selectPointerMotion()
{
  CXMachineInst->addInput(window_, PointerMotionMask);
}

void
CXWindow::
selectKeyInput()
{
  CXMachineInst->addInput(window_, KeyPressMask | KeyReleaseMask);
}

void
CXWindow::
selectExposures()
{
  CXMachineInst->addInput(window_, ExposureMask | StructureNotifyMask);
}

void
CXWindow::
selectEnterLeave()
{
  CXMachineInst->addInput(window_, EnterWindowMask | LeaveWindowMask);
}

void
CXWindow::
selectVisibility()
{
  CXMachineInst->addInput(window_, VisibilityChangeMask);
}

void
CXWindow::
selectPropertyNotify()
{
  CXMachineInst->addInput(window_, PropertyChangeMask);
}

void
CXWindow::
startDoubleBuffer(bool clear)
{
  if (graphics_)
    graphics_->startDoubleBuffer(clear);
}

void
CXWindow::
endDoubleBuffer()
{
  if (graphics_)
    graphics_->endDoubleBuffer();
}

void
CXWindow::
copyDoubleBuffer()
{
  if (graphics_)
    graphics_->copyDoubleBuffer();
}

void
CXWindow::
clear()
{
  if (graphics_)
    graphics_->clear(false);
}

void
CXWindow::
fill()
{
  if (graphics_)
    graphics_->fill();
}

void
CXWindow::
redraw()
{
  if (graphics_)
    graphics_->clear(true);
}

void
CXWindow::
drawLine(double x1, double y1, double x2, double y2)
{
  if (graphics_)
    graphics_->drawLine((int) x1, (int) y1, (int) x2, (int) y2);
}

void
CXWindow::
drawXorLine(double x1, double y1, double x2, double y2)
{
  createXorGraphics();

  if (xor_graphics_)
    xor_graphics_->drawLine((int) x1, (int) y1, (int) x2, (int) y2);
}

void
CXWindow::
drawRectangle(double x, double y, double width1, double height1)
{
  if (graphics_)
    graphics_->drawRectangle((int) x, (int) y, (int) (width1  - 1.0), (int) (height1 - 1.0));
}

void
CXWindow::
drawXorRectangle(double x, double y, double width1, double height1)
{
  createXorGraphics();

  if (xor_graphics_)
    xor_graphics_->drawRectangle((int) x, (int) y, (int) (width1  - 1.0), (int) (height1 - 1.0));
}

void
CXWindow::
fillRectangle(double x, double y, double width1, double height1)
{
  if (graphics_)
    graphics_->fillRectangle((int) x, (int) y, (int) width1, (int) height1);
}

void
CXWindow::
drawPolygon(double *x, double *y, uint num_xy)
{
  std::vector<int> xi; xi.resize(num_xy);
  std::vector<int> yi; yi.resize(num_xy);

  for (uint i = 0; i < num_xy; ++i) {
    xi[i] = (int) x[i];
    yi[i] = (int) y[i];
  }

  if (graphics_)
    graphics_->drawPolygon(&xi[0], &yi[0], num_xy);
}

void
CXWindow::
fillPolygon(double *x, double *y, uint num_xy)
{
  std::vector<int> xi; xi.resize(num_xy);
  std::vector<int> yi; yi.resize(num_xy);

  for (uint i = 0; i < num_xy; ++i) {
    xi[i] = (int) x[i];
    yi[i] = (int) y[i];
  }

  if (graphics_)
    graphics_->fillPolygon(&xi[0], &yi[0], num_xy);
}

void
CXWindow::
drawCircle(double x, double y, double r)
{
  if (graphics_)
    graphics_->drawCircle((int) x, (int) y, (int) r);
}

void
CXWindow::
fillCircle(double x, double y, double r)
{
  if (graphics_)
    graphics_->fillCircle((int) x, (int) y, (int) r);
}

void
CXWindow::
drawEllipse(double x, double y, double xr, double yr)
{
  if (graphics_)
    graphics_->drawEllipse((int) x, (int) y, (int) xr, (int) yr);
}

void
CXWindow::
fillEllipse(double x, double y, double xr, double yr)
{
  if (graphics_)
    graphics_->fillEllipse((int) x, (int) y, (int) xr, (int) yr);
}

void
CXWindow::
drawArc(double x, double y, double xr, double yr, double a1, double a2)
{
  if (graphics_)
    graphics_->drawArc((int) x, (int) y, (int) xr, (int) yr, a1, a2);
}

void
CXWindow::
fillArc(double x, double y, double xr, double yr, double a1, double a2)
{
  if (graphics_)
    graphics_->fillArc((int) x, (int) y, (int) xr, (int) yr, a1, a2);
}

void
CXWindow::
drawPoint(double x, double y)
{
  if (graphics_)
    graphics_->drawPoint((int) x, (int) y);
}

void
CXWindow::
drawImage(const CImagePtr &image, double x, double y)
{
  if (graphics_)
    graphics_->drawImage(image, (int) x, (int) y);
}

void
CXWindow::
drawAlphaImage(const CImagePtr &image, double x, double y)
{
  if (graphics_)
    graphics_->drawAlphaImage(image, (int) x, (int) y);
}

void
CXWindow::
drawSubImage(const CImagePtr &image, double src_x, double src_y,
             double dst_x, double dst_y, double width1, double height1)
{
  CXImage *ximage = image.cast<CXImage>();

  XImage *ximg = ximage->getXImage();

  uint width2  = image->getWidth();
  uint height2 = image->getHeight();

  if (src_x + width1 > width2)
    width1 = width2 - src_x;

  if (src_y + height1 > height2)
    height1 = height2 - src_y;

  if (graphics_)
    graphics_->drawSubImage(ximg, (int) src_x, (int) src_y, (int) dst_x, (int) dst_y,
                            (int) width1, (int) height1);
}

void
CXWindow::
drawText(double x, double y, const std::string &str)
{
  if (graphics_)
    graphics_->drawText((int) x, (int) y, str);
}

void
CXWindow::
clipRectangle(double x, double y, double width, double height)
{
  if (graphics_)
    graphics_->startClip((int) x, (int) y, (int) width, (int) height);
}

void
CXWindow::
clipReset()
{
  if (graphics_)
    graphics_->endClip();
}

void
CXWindow::
setForeground(const CRGB &rgb)
{
  if (graphics_)
    graphics_->setForeground(rgb);
}

void
CXWindow::
setForeground(const CRGBA &rgba)
{
  if (graphics_)
    graphics_->setForeground(rgba);
}

void
CXWindow::
setForeground(CXColor &color)
{
  if (graphics_)
    graphics_->setForeground(color);
}

void
CXWindow::
setLightForeground(CXColor &color)
{
  if (graphics_)
    graphics_->setForeground(color.getLightPixel());
}

void
CXWindow::
setDarkForeground(CXColor &color)
{
  if (graphics_)
    graphics_->setForeground(color.getDarkPixel());
}

void
CXWindow::
setBackground(const CRGB &rgb)
{
  setBackground(CRGBA(rgb));
}

void
CXWindow::
setBackground(const CRGBA &rgba)
{
  if (graphics_) {
    graphics_->setBackground(rgba);

    Pixel pixel;

    graphics_->getBackgroundPixel(pixel);

    CXMachineInst->setWindowBackgroundColor(window_, pixel);
  }
}

void
CXWindow::
setBackground(CXColor &color)
{
  if (graphics_) {
    graphics_->setBackground(color);

    Pixel pixel;

    graphics_->getBackgroundPixel(pixel);

    CXMachineInst->setWindowBackgroundColor(window_, pixel);
  }
}

void
CXWindow::
setLightBackground(CXColor &color)
{
  if (graphics_)
    graphics_->setBackground(color.getLightPixel());
}

void
CXWindow::
setDarkBackground(CXColor &color)
{
  if (graphics_)
    graphics_->setBackground(color.getDarkPixel());
}

void
CXWindow::
getForeground(CRGB &rgb) const
{
  if (graphics_)
    graphics_->getForeground(rgb);
  else
    rgb = CRGB(0,0,0);
}

void
CXWindow::
getForeground(CRGBA &rgba) const
{
  CRGB rgb;

  getForeground(rgb);

  rgba = CRGBA(rgb);
}

void
CXWindow::
getBackground(CRGB &rgb) const
{
  if (graphics_)
    graphics_->getBackground(rgb);
  else
    rgb = CRGB(1,1,1);
}

void
CXWindow::
getBackground(CRGBA &rgba) const
{
  CRGB rgb;

  getBackground(rgb);

  rgba = CRGBA(rgb);
}

void
CXWindow::
setLineWidth(double line_width)
{
  if (graphics_)
    graphics_->setLineWidth((int) line_width);
}

void
CXWindow::
setLineDash(int offset, char *dashes, uint num_dashes)
{
  if (graphics_)
    graphics_->setLineDash(offset, dashes, num_dashes);
}

void
CXWindow::
setLineDash(int offset, int *dashes, uint num_dashes)
{
  if (graphics_)
    graphics_->setLineDash(offset, dashes, num_dashes);
}

void
CXWindow::
setLineDash(const CILineDash &line_dash)
{
  if (graphics_)
    graphics_->setLineDash(line_dash);
}

void
CXWindow::
setLineType(CXLineType line_type)
{
  if (graphics_)
    graphics_->setLineType(line_type);
}

void
CXWindow::
setFillComplex(bool comp)
{
  if (graphics_)
    graphics_->setFillComplex(comp);
}

uint
CXWindow::
getStringWidth(const std::string &str) const
{
  if (graphics_)
    return graphics_->getStringWidth(str);
  else
    return 1;
}

void
CXWindow::
copyArea(const CXWindow &src_window)
{
  if (graphics_)
    graphics_->copyArea(*src_window.getCXGraphics(), 0, 0, 0, 0, getWidth(), getHeight());
}

CImagePtr
CXWindow::
getImage() const
{
  return getImage(0, 0, getWidth(), getHeight());
}

CImagePtr
CXWindow::
getImage(int x, int y, uint width, uint height) const
{
  if (! graphics_)
    return CImagePtr();

  CImagePtr image;

  graphics_->getImage(x, y, width, height, image);

  return image;
}

double
CXWindow::
flipY(double y)
{
  return height_ - y - 1.0;
}

void
CXWindow::
createXorGraphics() const
{
  if (! xor_graphics_) {
    CXWindow *th = const_cast<CXWindow *>(this);

    th->xor_graphics_ = std::make_unique<CXGraphics>(screen_, window_);

    th->xor_graphics_->setXor();

    CFontPtr font;

    getFont(font);

    th->xor_graphics_->setFont(font);
  }
}

bool
CXWindow::
raiseWait(uint timeout)
{
  raise();

  Window root = screen_.getRoot();

  for (uint i = 0; i < timeout; ++i) {
    CXMachineInst->flushEvents(true);

    Window *children;
    int     num_children;

    if (! CXMachineInst->getWindowChildren(root, &children, &num_children))
      return false;

    if (num_children > 0 && children[num_children - 1] == window_)
      return true;

    sleep(1);
  }

  return false;
}

void
CXWindow::
setWindowTitle(const std::string &title)
{
  CXWindow *top = getTopWindow();

  CXMachineInst->setWindowTitle(top->window_, title);
}

void
CXWindow::
setIconTitle(const std::string &title)
{
  CXWindow *top = getTopWindow();

  CXMachineInst->setIconTitle(top->window_, title);
}

void
CXWindow::
getWindowTitle(std::string &title) const
{
  CXWindow *th = const_cast<CXWindow *>(this);

  CXWindow *top = th->getTopWindow();

  CXMachineInst->getWindowTitle(top->window_, title);
}

void
CXWindow::
getIconTitle(std::string &title) const
{
  CXWindow *th = const_cast<CXWindow *>(this);

  CXWindow *top = th->getTopWindow();

  CXMachineInst->getIconTitle(top->window_, title);
}

bool
CXWindow::
setProperty(const std::string &name, const std::string &value)
{
  const CXAtom &atom = CXMachineInst->getAtom(name);

  CXMachineInst->setStringProperty(window_, atom, value);

  return true;
}

void
CXWindow::
iconize()
{
  CXMachineInst->unmapWindow(window_);
}

void
CXWindow::
deiconize()
{
  CXMachineInst->mapWindow(window_);
}

void
CXWindow::
lower()
{
  CXMachineInst->lowerWindow(window_);
}

void
CXWindow::
raise()
{
  CXMachineInst->raiseWindow(window_);
}

void
CXWindow::
minimize()
{
  CXMachineInst->minimizeWindow(window_);
}

void
CXWindow::
maximize()
{
  getSize(&last_width_, &last_height_);

  CXMachineInst->maximizeWindow(window_);
}

void
CXWindow::
demaximize()
{
  resize(getLastWidth(), getLastHeight());
}

bool
CXWindow::
setSelectText(const std::string &text)
{
  CXMachineInst->selectionSetText(window_, text);

  return true;
}

void
CXWindow::
expose()
{
  getPixelRenderer()->setContentsChanged();

  CXMachineInst->sendExposeEvent(window_);
}

void
CXWindow::
flushEvents()
{
  CXMachineInst->flushEvents();
}

void
CXWindow::
grabPointer()
{
  uint event_mask = ButtonPressMask | ButtonReleaseMask |
                    PointerMotionMask;

  CXMachineInst->grabPointer(window_, event_mask, None);
}

void
CXWindow::
ungrabPointer()
{
  CXMachineInst->ungrabPointer();
}

void
CXWindow::
translateFromRootCoords(int rx, int ry, int *wx, int *wy) const
{
  Window root = CXMachineInst->getRoot();

  CXMachineInst->translateCoords(root, window_, rx, ry, wx, wy, nullptr);
}

void
CXWindow::
translateToRootCoords(int wx, int wy, int *rx, int *ry) const
{
  Window root = CXMachineInst->getRoot();

  CXMachineInst->translateCoords(window_, root, wx, wy, rx, ry, nullptr);
}

void
CXWindow::
selectionSetText(const std::string &text)
{
  CXMachineInst->selectionSetText(window_, text);
}

void
CXWindow::
beep()
{
  CXMachineInst->beep();
}

void
CXWindow::
setShellProperties(const std::string &name)
{
  std::string str = COSUser::getHostName();

  const CXAtom &protocols     = CXMachineInst->getAtom("WM_PROTOCOLS");
  const CXAtom &delete_window = CXMachineInst->getAtom("WM_DELETE_WINDOW");
  const CXAtom &take_focus    = CXMachineInst->getAtom("WM_TAKE_FOCUS");

  CXMachineInst->setWMClientMachine(window_, str);

  CXAtom **atoms = new CXAtom * [2];

  atoms[0] = (CXAtom *) &delete_window;
  atoms[1] = (CXAtom *) &take_focus;

  CXMachineInst->setAtomArrayProperty(window_, protocols, (const CXAtom **) atoms, 2);

  CXMachineInst->setWMClassHint(window_, name, name);
}

void
CXWindow::
setMinSize(int width, int height)
{
  CXWindow *window = getTopWindow();

  if (window->window_ == 0)
    return;

  XSizeHints *hints;
  int         supplied;

  CXMachineInst->getWMNormalHints(window->window_, &hints, &supplied);

  hints->flags |= PMinSize;

  hints->min_width  = width;
  hints->min_height = height;

  CXMachineInst->setWMNormalHints(window->window_, hints);
}

void
CXWindow::
setResizeInc(int width, int height)
{
  CXWindow *window = getTopWindow();

  if (window->window_ == 0)
    return;

  XSizeHints *hints;
  int         supplied;

  CXMachineInst->getWMNormalHints(window->window_, &hints, &supplied);

  hints->flags |= PResizeInc;

  hints->width_inc  = width;
  hints->height_inc = height;

  CXMachineInst->setWMNormalHints(window->window_, hints);
}

void
CXWindow::
setBaseSize(int width, int height)
{
  CXWindow *window = getTopWindow();

  if (window->window_ == 0)
    return;

  XSizeHints *hints;
  int         supplied;

  CXMachineInst->getWMNormalHints(window->window_, &hints, &supplied);

  hints->flags |= PBaseSize;

  hints->base_width  = width;
  hints->base_height = height;

  CXMachineInst->setWMNormalHints(window->window_, hints);
}

void
CXWindow::
sendMessage(CXWindow *w, const char *msg)
{
  CXMachineInst->sendWindowMessage(window_, w->window_, msg);
}
