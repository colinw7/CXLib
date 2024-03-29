#ifndef CXWINDOW_H
#define CXWINDOW_H

#include <CXScreen.h>
#include <CXGraphics.h>
#include <CXCursor.h>
#include <CXImage.h>
#include <CXColor.h>
#include <CWindow.h>
#include <CEvent.h>
#include <accessor.h>
#include <memory>

class CXEventAdapter;
class CXLibPixelRenderer;

class CXWindowFactory : public CWindowFactory {
 public:
  CWindow *createWindow(int x, int y, uint width, uint height) override;

  CWindow *createWindow(CWindow *parent, int x, int y, uint width, uint height) override;
};

class CXWindow : public CWindow {
 public:
  explicit CXWindow(CWindowType type=CWINDOW_TYPE_NORMAL);

  CXWindow(CXScreen &screen, Window parent_window, int x, int y, uint width, uint height);
  CXWindow(CXWindow *window, int x, int y, uint width, uint height);

  CXWindow(CXScreen &screen, int x, int y, uint width, uint height,
           CWindowType type=CWINDOW_TYPE_NORMAL);
  CXWindow(int x, int y, uint width, uint height, CWindowType type=CWINDOW_TYPE_NORMAL);
  CXWindow(uint width, uint height, CWindowType type=CWINDOW_TYPE_NORMAL);

  CXWindow(CXScreen &screen, Window window);
  CXWindow(CXScreen &screen, Window window, uint width, uint height);
  CXWindow(Window window);
  CXWindow(Window window, uint width, uint height);

  virtual ~CXWindow();

  void destroy() override;

  virtual CXLibPixelRenderer *getPixelRenderer() const;

  CXScreen &getCXScreen() const;

  CXWindow *getParentWindow() const { return parent_window_; }

  Window getParentXWindow() const { return parent_xwindow_; }

  CXWindow *getTopWindow();

  CXGraphics *getCXGraphics() const;
  CXGraphics *getXorCXGraphics() const;

  virtual void setFont(const std::string &name, CFontStyle style, int size);
  virtual void setFont(const std::string &full_name);
  virtual void setFont(CFontPtr font);

  void getFont(CFontPtr &font) const;

  CXCursor &getCXCursor() const;

  virtual void setCursor(CCursorType type);
  virtual void setCursor(CXCursor &cursor);
  virtual void unsetCursor();

  int getX() const { return x_; }
  int getY() const { return y_; }

  uint getWidth () const;
  uint getHeight() const;

  ACCESSOR(LastWidth , uint, last_width )
  ACCESSOR(LastHeight, uint, last_height)

  void setEventAdapter(CEventAdapter *adapter) override;
  CEventAdapter *getEventAdapter() const { return event_adapter_.get(); }
  CXEventAdapter *getXEventAdapter() const;

  virtual void move(int x, int y) override;
  virtual void resize(uint width, uint height) override;

  virtual void setPosition(int x, int y);

  void getPosition(int *x, int *y) const override;
  void getRootPosition(int *x, int *y) const;

  virtual void setSize(uint width, uint height);

  void getSize(uint *width, uint *height) const override;

  void getScreenSize(uint *width, uint *height) const override;

  void map() override;
  void unmap() override;

  bool isMapped() override;

  Display *getDisplay() const;

  Window getXWindow() const;

  GC getXGC() const;

  Visual *getXVisual() const;

  Colormap getXColormap() const;

  void selectAllEvents();
  void selectKeyInput();
  void selectButtonInput();
  void selectPointerMotion();
  void selectExposures();
  void selectEnterLeave();
  void selectVisibility();
  void selectPropertyNotify();

  void clear();
  void fill();

  void startDoubleBuffer(bool clear=true);
  void endDoubleBuffer();
  void copyDoubleBuffer();

  void redraw() override;

  // renderer stuff
  void drawLine(double x1, double y1, double x2, double y2);
  void drawXorLine(double x1, double y1, double x2, double y2);

  void drawRectangle(double x, double y, double width1, double height1);
  void fillRectangle(double x, double y, double width1, double height1);
  void drawXorRectangle(double x, double y, double width1, double height1);

  void drawPolygon(double *x, double *y, uint num_xy);
  void fillPolygon(double *x, double *y, uint num_xy);

  void drawCircle(double x, double y, double r);
  void fillCircle(double x, double y, double r);

  void drawEllipse(double x, double y, double xr, double yr);
  void fillEllipse(double x, double y, double xr, double yr);

  void drawArc(double x, double y, double xr, double yr, double a1, double a2);
  void fillArc(double x, double y, double xr, double yr, double a1, double a2);

  void drawPoint(double x, double y);

  void drawImage(const CImagePtr &image, double x, double y);

  void drawAlphaImage(const CImagePtr &image, double x, double y);

  void drawSubImage(const CImagePtr &image, double src_x, double src_y,
                    double dst_x, double dst_y, double width1, double height1);

  void drawText(double x, double y, const std::string &str);

  void clipRectangle(double x, double y, double width, double height);
  void clipReset();

  void setForeground(const CRGB &rgb);
  void setForeground(const CRGBA &rgba);
  void setForeground(CXColor &color);

  void setLightForeground(CXColor &color);
  void setDarkForeground(CXColor &color);

  void setBackground(const CRGB &rgb);
  void setBackground(const CRGBA &rgba);
  void setBackground(CXColor &color);

  void setLightBackground(CXColor &color);
  void setDarkBackground(CXColor &color);

  void getForeground(CRGB &rgb) const;
  void getForeground(CRGBA &rgb) const;

  void getBackground(CRGB &rgb) const;
  void getBackground(CRGBA &rgb) const;

  void setLineWidth(double line_width);
  void setLineType(CXLineType line_type);

  void setLineDash(int offset=0, char *dashes=nullptr, uint num_dashes=0);
  void setLineDash(int offset, int *dashes, uint num_dashes);
  void setLineDash(const CILineDash &line_dash);

  void setFillComplex(bool comp);

  uint getStringWidth(const std::string &str) const;

  void copyArea(const CXWindow &src_window);

  CImagePtr getImage() const;
  CImagePtr getImage(int x, int y, uint width, uint height) const;

  bool raiseWait(uint timeout);

  void setWindowTitle(const std::string &title) override;
  void getWindowTitle(std::string &title) const override;

  void setIconTitle(const std::string &title) override;
  void getIconTitle(std::string &title) const override;

  bool setProperty(const std::string &name, const std::string &value) override;

  void iconize() override;
  void deiconize() override;
  void lower() override;
  void raise() override;
  virtual void minimize();
  void maximize() override;
  void demaximize() override;

  bool setSelectText(const std::string &str) override;

  void expose() override;

  void flushEvents();

  virtual void grabPointer();
  virtual void ungrabPointer();

  void translateFromRootCoords(int rx, int ry, int *wx, int *wy) const;
  void translateToRootCoords(int wx, int wy, int *rx, int *ry) const;

  void selectionSetText(const std::string &text);

  void beep() override;

  void setShellProperties(const std::string &name);

  virtual void setMinSize(int width, int height);
  virtual void setResizeInc(int width, int height);
  virtual void setBaseSize(int width, int height);

  virtual void sendMessage(CXWindow *w, const char *msg);

  bool exposeEvent() override { return false; }
  bool resizeEvent() override { return false; }

  bool buttonPressEvent  (const CMouseEvent &) override { return false; }
  bool buttonMotionEvent (const CMouseEvent &) override { return false; }
  bool buttonReleaseEvent(const CMouseEvent &) override { return false; }

  bool keyPressEvent  (const CKeyEvent &) override { return false; }
  bool keyReleaseEvent(const CKeyEvent &) override { return false; }

  bool pointerMotionEvent(const CMouseEvent &) override { return false; }

  bool enterEvent() override { return false; }
  bool leaveEvent() override { return false; }

  virtual bool selectionClearEvent() { return false; }

  bool closeEvent() override { return false; }

  virtual bool idleEvent() { return false; }

  virtual bool clientMessageEvent(CXWindow *, const char *) { return false; }

 protected:
  void init(CWindowType type);
  void init(Window window);
  void init(Window window, uint width, uint height);

  void init1(CWindowType type);

  void term();

  void create();

  double flipY(double y);

  void createXorGraphics() const;

 protected:
  using GraphicsP     = std::unique_ptr<CXGraphics>;
  using CursorP       = std::unique_ptr<CXCursor>;
  using EventAdapterP = std::unique_ptr<CEventAdapter>;

  CXScreen&           screen_;
  CXWindow*           parent_window_  { nullptr };
  Window              parent_xwindow_ { 0 };
  int                 x_              { 0 };
  int                 y_              { 0 };
  int                 width_          { 100 };
  int                 height_         { 100 };
  uint                last_width_     { 0 };
  uint                last_height_    { 0 };
  CWindowType         type_;
  bool                mapped_         { false };
  Window              window_         { 0 };
  Display*            display_        { nullptr };
  GraphicsP           graphics_;
  GraphicsP           xor_graphics_;
  CursorP             cursor_;
  CXLibPixelRenderer* renderer_       { nullptr };
  bool                renderer_alloc_ { false };
  EventAdapterP       event_adapter_;
};

//------

#include <CXEventAdapter.h>

class CXWindowEventAdapter : public CXEventAdapter {
 public:
  CXWindowEventAdapter(CXWindow *window) :
   CXEventAdapter(0), window_(window) {
  }

  virtual ~CXWindowEventAdapter() { }

  CXWindow *getWindow() const { return window_; }

  bool buttonPressEvent(const CMouseEvent &bevent) override {
    if (! window_->buttonPressEvent(bevent))
      CXMachineInst->getEventAdapter()->buttonPressEvent(bevent);

    return true;
  }

  bool buttonMotionEvent(const CMouseEvent &bevent) override {
    if (! window_->buttonMotionEvent(bevent))
      CXMachineInst->getEventAdapter()->buttonMotionEvent(bevent);

    return true;
  }

  bool buttonReleaseEvent(const CMouseEvent &bevent) override {
    if (! window_->buttonReleaseEvent(bevent))
      CXMachineInst->getEventAdapter()->buttonReleaseEvent(bevent);

    return true;
  }

  bool keyPressEvent(const CKeyEvent &kevent) override {
    if (! window_->keyPressEvent(kevent))
      CXMachineInst->getEventAdapter()->keyPressEvent(kevent);

    return true;
  }

  bool keyReleaseEvent(const CKeyEvent &kevent) override {
    if (! window_->keyReleaseEvent(kevent))
      CXMachineInst->getEventAdapter()->keyReleaseEvent(kevent);

    return true;
  }

  bool pointerMotionEvent(const CMouseEvent &bevent) override {
    if (! window_->pointerMotionEvent(bevent))
      CXMachineInst->getEventAdapter()->pointerMotionEvent(bevent);

    return true;
  }

  bool exposeEvent() override {
    if (! window_->exposeEvent())
      CXMachineInst->getEventAdapter()->exposeEvent();

    return true;
  }

  bool resizeEvent() override {
    if (! window_->resizeEvent())
      CXMachineInst->getEventAdapter()->resizeEvent();

    return true;
  }

  bool enterEvent() override {
    if (! window_->enterEvent())
      CXMachineInst->getEventAdapter()->enterEvent();

    return true;
  }

  bool leaveEvent() override {
    if (! window_->leaveEvent())
      CXMachineInst->getEventAdapter()->leaveEvent();

    return true;
  }

  bool visibilityEvent(bool) override {
    return false;
  }

  bool selectionClearEvent() override {
    if (! window_->selectionClearEvent())
      CXMachineInst->getEventAdapter()->selectionClearEvent();

    return true;
  }

  bool closeEvent() override {
    if (! window_->closeEvent())
      CXMachineInst->getEventAdapter()->closeEvent();

    return true;
  }

  bool idleEvent() override {
    if (! window_->idleEvent())
      CXMachineInst->getEventAdapter()->idleEvent();

    return true;
  }

  bool clientMessageEvent(void *from, const char *msg) override {
    if (! window_->clientMessageEvent(reinterpret_cast<CXWindow *>(from), msg))
      return false;

    return true;
  }

 protected:
  CXWindow *window_ { nullptr };
};

#endif
