#ifndef CX_MACHINE_H
#define CX_MACHINE_H

#define CXMachineInst CXMachine::getInstance()

#include <std_Xt.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CEvent.h>
#include <CAutoPtr.h>
#include <CIPoint2D.h>
#include <MwmUtil.h>

class CXMachine;
class CXScreen;
class CXWindow;
class CXPixmap;
class CXAtomMgr;
class CXAtom;
class CXColor;

typedef void (*CXMachineEventProc)(CXMachine *machine, XEvent *event);

struct CXEventAdapter;

#define CXLIB_ALL_FONTS_PATTERN "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

enum { CXLIB_MAX_FONTS = 1000 };

class CXFontList {
 private:
  char **fonts_;
  int    num_fonts_;

 public:
  CXFontList(const char *pattern=CXLIB_ALL_FONTS_PATTERN, uint max_fonts=CXLIB_MAX_FONTS);
 ~CXFontList();

  uint getNumFonts() const { return num_fonts_; }

  const char *getFont(uint i) const { return fonts_[i]; }
};

class CXMachine {
 public:
  typedef void (*XErrorProc)(const std::string &str);

 private:
  struct MaximizeData {
    int x, y, width, height;
  };

  typedef std::map<int, Display  *>   DisplayMap;
  typedef std::map<int, CXScreen *>   CXScreenMap;
  typedef std::map<int, MaximizeData> MaximizeDataMap;

  Display      *display_;
  std::string   display_name_;
  std::string   hostname_;
  int           display_num_;
  int           screen_num_;
  int           num_screens_;
  XtAppContext  app_context_;

  CAutoPtr<CXEventAdapter> event_adapter_;
  XEvent                   event_;
  Time                     event_last_time_;
  Window                   event_win_;
  CIPoint2D                event_pos_;
  CMouseButton             event_button_num_;
  bool                     event_button_pressed_;
  Time                     event_button_time_;
  int                      event_button_count_;
  KeySym                   event_keysym_;
  uint                     event_modifier_;

  bool pedantic_;

  bool trap_active_;
  int  trap_request_code_;
  int  trap_error_code_;

  DisplayMap  displays_;
  CXScreenMap screens_;

  MaximizeDataMap max_data_map_;

  CAutoPtr<CXAtomMgr> atom_mgr_;

  XErrorProc  error_proc_;

  Window      selection_xwin_;
  bool        selection_set_;
  std::string selection_text_;

 public:
  static CXMachine *getInstance();

  CXMachine();
 ~CXMachine();

  Display      *getDisplay      () const { return display_        ; }
  std::string   getDisplayName  () const { return display_name_   ; }
  std::string   getHostName     () const { return hostname_       ; }
  int           getDisplayNum   () const { return display_num_    ; }
  int           getScreenNum    () const { return screen_num_     ; }
  int           getNumScreens   () const { return num_screens_    ; }
  XtAppContext  getAppContext   () const { return app_context_    ; }

  CXEventAdapter *getEventAdapter() const { return event_adapter_; }

  Time getLastEventTime() const { return event_last_time_; }

  bool getIsShift() const { return event_modifier_ & CMODIFIER_SHIFT  ; }
  bool getIsCtrl () const { return event_modifier_ & CMODIFIER_CONTROL; }
  bool getIsAlt  () const { return event_modifier_ & CMODIFIER_ALT    ; }

  void setIsShift(bool flag) {
    if (flag)
      event_modifier_ |=  CMODIFIER_SHIFT  ;
    else
      event_modifier_ &= ~CMODIFIER_SHIFT  ;
  }

  void setIsCtrl (bool flag) {
    if (flag)
      event_modifier_ |=  CMODIFIER_CONTROL;
    else
      event_modifier_ &= ~CMODIFIER_CONTROL;
  }

  void setIsAlt  (bool flag) {
    if (flag)
      event_modifier_ |=  CMODIFIER_ALT    ;
    else
      event_modifier_ &= ~CMODIFIER_ALT    ;
  }

  void setPedantic(bool flag) { pedantic_ = flag; }

  void init();

  Display *openDisplay  (const std::string &display_name);
  Display *openXtDisplay(const std::string &display_name, const std::string &appname,
                         int *argc, char **argv);
  void     closeDisplay ();
  void     setDisplay   (Display *display);

  Display *getScreenDisplay(int screen_num) const;

  CXScreen *getCXScreen  (int screen_num=0) const;
  Screen   *getScreen    (int screen_num=0) const;
  Visual   *getVisual    (int screen_num=0) const;
  Colormap  getColormap  (int screen_num=0) const;
  int       getWidth     (int screen_num=0) const;
  int       getHeight    (int screen_num=0) const;
  int       getDepth     (int screen_num=0) const;
  Pixel     getBlackPixel(int screen_num=0) const;
  Pixel     getWhitePixel(int screen_num=0) const;

  Window    getRoot  (int screen_num=0) const;
  CXWindow *getCXRoot(int screen_num=0) const;

  Window getWindowRoot    (Window xwin) const;
  Window getWindowTop     (Window xwin) const;
  Window getWindowParent  (Window xwin) const;
  bool   getWindowChildren(Window xwin, Window **children, int *num_children) const;

  void enterDebugMode() const;
  void synchronize() const;
  void grabServer() const;
  void ungrabServer() const;

  void setEventAdapter(CXEventAdapter *adapter);

  void mainLoop(CXMachineEventProc proc);
  void mainLoop(uint twait=10, CXEventAdapter *adapter=NULL);
  void tickLoop(uint nframes=30, CXEventAdapter *adapter=NULL);

  bool processEvent();

  XEvent *getEvent() { return &event_; }

  KeySym       getEventKeysym() const;
  std::string  getEventString() const;
  std::string  getEventString(const XEvent *event) const;
  char         getEventChar  () const;
  char        *getEventEscape() const;

  CXWindow *getEventXWindow() const;

  int getEventX() const { return event_pos_.x; }
  int getEventY() const { return event_pos_.y; }

  int getEventRootX() const;
  int getEventRootY() const;

  void getWindowRootPos(Window xwin, int *x, int *y) const;

  CEvent *convertEvent(XEvent *event);

  CMouseButton convertEventButton(uint button);

  CEventModifier convertEventState(uint state);

  CKeyType convertEventKeyCodeToType(uint keycode, uint state);

  std::string convertEventKeyCodeToString(uint keycode, uint state);

  int getEventButtonNum  () const { return int(event_button_num_); }
  int getEventButtonCount() const { return event_button_count_   ; }

  bool eventPending() const;
  void nextEvent() const;
  void nextEvent(XEvent *event) const;

  bool flushEvents(bool sync=true) const;
  bool flushWindowEvent(Window xwin, uint event_mask);
  int  flushWindowEvents(Window xwin, uint event_mask);

  bool compareEvents(XEvent *event1, XEvent *event2);

  bool waitForClientMessage(Window xwin, XEvent *event=NULL);

  bool checkWindowEvent(Window xwin, uint event_mask, XEvent *event) const;
  bool checkTypedEvent(int event_type, XEvent *event) const;
  bool checkWindowTypedEvent(Window xwin, int event_type, XEvent *event) const;

  void maskEvent(uint mask, XEvent *event) const;
  void allowEvents(int mode) const;

  Colormap getWindowColormap(Window xwin) const;

  uint getWindowEventMask(Window xwin) const;

  bool getWindowViewable(Window xwin) const;

  void getWindowGeometry(Window xwin, int *x, int *y, int *width, int *height, int *border) const;

  CXWindow *lookupWindow(Window xwin) const;

  bool isValidWindow(Window xwin) const;

  int getMulticlickTime() const;

  void setInputFocus(Window xwin) const;

  bool translateCoords(Window src_w, Window dest_w, int src_x, int src_y,
                       int *dest_x, int *dest_y, Window *child) const;

  bool queryPointer(Window xwin, int *x, int *y);

  void destroyWindow(Window xwin);

  void mapWindow(Window xwin);
  void mapWindowRaised(Window xwin);
  void mapWindowChildren(Window xwin);
  void unmapWindow(Window xwin);

  void reparentWindow(Window xwin, Window parent_xwin, int x, int y);

  void lowerWindow(Window xwin);
  void raiseWindow(Window xwin);

  void minimizeWindow(Window xwin);
  void maximizeWindow(Window xwin);
  void unmaximizeWindow(Window xwin);

  void moveWindow(Window xwin, int x, int y);
  void resizeWindow(Window xwin, int width, int height);
  void moveResizeWindow(Window xwin, int x, int y, int width, int height);

  void clearWindow(Window xwin);

  void configureWindow(Window xwin, uint mask, XWindowChanges *xwc);

  void restackWindows(Window *windows, int num_windows);

  void shapeCombineMask(Window xwin, Window mask, int x, int y, int op);

  bool grabPointer(Window xwin, uint event_mask, Cursor cursor) const;
  void ungrabPointer() const;

  bool grabKeyboard(Window xwin);
  void ungrabKeyboard();

  void grabKey(Window xwin, int keycode, int state);
  void ungrabKey(Window xwin, int keycode, int state);

  void grabButton(Window xwin, int button, int modifiers, int event_mask, Cursor cursor);
  void ungrabButton(Window xwin, int button, int modifiers);

  Window createOverrideWindow(int x, int y, int width, int height, int border = 0,
                              uint attr_mask = 0, XSetWindowAttributes *attr = NULL);

  Window createWindow(Window parent_xwin, int x, int y, int width, int height,
                      int border = 0, uint attr_mask = 0, XSetWindowAttributes *attr = NULL);
  Window createWindow(int x, int y, int width, int height,
                      int border = 0, uint attr_mask = 0, XSetWindowAttributes *attr = NULL);

  CXPixmap *createPixmap(Window xwin, uint width, uint height);
  CXPixmap *createPixmap(uint width, uint height);

  Pixmap createXPixmap(Window xwin, uint width, uint height, uint depth=0);
  Pixmap createXPixmap(uint width, uint height, uint depth=0);

  void freeXPixmap(Pixmap xpixmap);

  void setKeyAutoRepeatRate(uint timeout, uint interval);
  void getKeyAutoRepeatRate(uint *timeout, uint *interval);

  void installColormap(Colormap cmap);

  void changeWindowAtributes(Window xwin, uint attr_mask, XSetWindowAttributes *attr);

  void killClient(Window xwin);

  void setWindowTitle(Window xwin, const std::string &title);
  void getWindowTitle(Window xwin, std::string &title);
  void setIconTitle(Window xwin, const std::string &title);
  void getIconTitle(Window xwin, std::string &title);

  bool setIntegerProperty(Window xwin, const CXAtom &name, int value);
  bool setStringProperty(Window xwin, const CXAtom &name, const std::string &value);
  bool setWindowProperty(const CXAtom &name, Window value);
  bool setWindowProperty(Window xwin, const CXAtom &name, Window value);
  bool setWindowArrayProperty(Window xwin, const CXAtom &name, Window *xwins, int num_xwins);
  bool setAtomProperty(Window xwin, const CXAtom &name, const CXAtom *atom);
  bool setAtomArrayProperty(Window xwin, const CXAtom &name, const CXAtom **atoms, int num_atoms);
  bool setStringListProperty(Window xwin, const CXAtom &name, char **strs, int num_strs);

  bool getIntegerProperty(const CXAtom &name, int *value);
  bool getIntegerProperty(Window xwin, const CXAtom &name, int *value);
  bool getStringProperty(const CXAtom &name, std::string &value);
  bool getStringProperty(Window xwin, const CXAtom &name, std::string &value);
  bool getPixmapProperty(const CXAtom &name, Pixmap *value);
  bool getPixmapProperty(Window xwin, const CXAtom &name, Pixmap *value);
  bool getWindowProperty(const CXAtom &name, Window *value);
  bool getWindowProperty(Window xwin, const CXAtom &name, Window *value);

  void deleteProperty(Window xwin, const CXAtom &name);

  bool isWMWindow(Window xwin);

  void setWMStateNormal(Window xwin);
  void setWMStateIconic(Window xwin, Window icon_xwin);
  bool isWMStateNormal(Window xwin);
  bool isWMStateIconic(Window xwin);
  int  getWMState(Window xwin);

  void getWMName(Window xwin, std::string &name);
  void getWMIconName(Window xwin, std::string &name);

  void getWMNormalHints(Window xwin, XSizeHints **size_hints, int *supplied);
  bool setWMNormalHints(Window xwin, XSizeHints *size_hints);

  Window getWMIconWindowHint(Window xwin);
  void   getWMHints(Window xwin, XWMHints **wm_hints);

  void getWMTransientFor(Window xwin, Window *transient_xwin);
  bool setWMTransientFor(Window xwin, Window transient_xwin);

  void getWMClassHint(Window xwin, XClassHint **class_hint);
  bool setWMClassHint(Window xwin, const std::string &res_name, const std::string &res_class);

  void getWMClientMachine(Window xwin, std::string &client_machine);
  bool setWMClientMachine(Window xwin, const std::string &client_machine);

  void getWMCommand(Window xwin, int *argc, char ***argv);
  void getWMColormapWindows(Window xwin, Window **cmap_windows, int *num_cmap_windows);
  void getWMProtocols(Window xwin, const CXAtom ***protocols, int *num_protocols);
  bool getWMMwmHints(Window xwin, MotifWmHints **mwm_hints);

  const CXAtom &getWMStateAtom();
  const CXAtom &getMwmHintsAtom();
  const CXAtom &getWMProtocolsAtom();
  const CXAtom &getWMDeleteWindowAtom();
  const CXAtom &getXSetRootIdAtom();
  const CXAtom &getCwmDesktopAtom();

  bool isWMChangeStateAtom(const CXAtom &atom);
  bool isWMClassAtom(const CXAtom &atom);
  bool isWMClientMachineAtom(const CXAtom &atom);
  bool isWMCommandAtom(const CXAtom &atom);
  bool isWMDeleteWindowAtom(const CXAtom &atom);
  bool isWMHintsAtom(const CXAtom &atom);
  bool isWMIconNameAtom(const CXAtom &atom);
  bool isWMNameAtom(const CXAtom &atom);
  bool isWMNormalHintsAtom(const CXAtom &atom);
  bool isWMProtocolsAtom(const CXAtom &atom);
  bool isWMSaveYourselfAtom(const CXAtom &atom);
  bool isWMSizeHintsAtom(const CXAtom &atom);
  bool isWMStateAtom(const CXAtom &atom);
  bool isWMTakeFocusAtom(const CXAtom &atom);
  bool isWMTransientForAtom(const CXAtom &atom);
  bool isWMZoomHintsAtom(const CXAtom &atom);

  bool sendKeyPressedEvent(Window xwin, int x, int y, int keycode);
  bool sendKeyReleasedEvent(Window xwin, int x, int y, int keycode);

  bool sendStringClientMessage(Window server_xwin, Window client_xwin, const std::string &str);
  bool readStringClientMessage(Window server_xwin, Window *client_xwin, std::string &str);
  bool sendStringServerMessage(Window client_xwin, Window server_xwin, const std::string &str);
  bool readStringServerMessage(Window client_xwin, Window *server_xwin, std::string &str);

  bool sendWindowMessage(Window from, Window to, const std::string &str);

  bool sendEvent(Window xwin, int propagate, XEvent *event, uint event_mask);

  bool sendExposeEvent(Window xwin);

  bool sendConfigureNotifyEvent(Window xwin, int x, int y, int width, int height,
                                int border, Window above_xwin);

  bool addInput(Window xwin, uint event_mask);
  bool selectInput(Window xwin, uint event_mask);

  void setWindowBackgroundColor(Window xwin, Pixel bg);
  void setWindowBackgroundPixmap(Window xwin, Pixmap pixmap);
  void setWindowBorderWidth(Window xwin, int width);

  void warpPointer(Window xwin, int x, int y);

  void addToSaveSet(Window xwin);
  void removeFromSaveSet(Window xwin);

  GC createGC(const CXColor &fg, const CXColor &bg) const;
  GC createGC(Pixel fg, Pixel bg) const;

  GC createGC(Window xwin, const CXColor &fg, const CXColor &bg) const;
  GC createGC(Window xwin, Pixel fg, Pixel bg) const;

  GC createXorGC() const;
  GC createXorGC(Window xwin) const;
  GC createXorGC(Window xwin, const CXColor &fg, const CXColor &bg) const;
  GC createXorGC(Window xwin, Pixel fg, Pixel bg) const;

  void freeGC(GC gc);

  void setFillStipple(GC gc, Pixmap pixmap);

  void setForeground(GC gc, const CXColor &fg);
  void setForeground(GC gc, Pixel pixel);
  void getForeground(GC gc, Pixel *pixel);

  void setBackground(GC gc, const CXColor &bg);
  void setBackground(GC gc, Pixel pixel);
  void getBackground(GC gc, Pixel *pixel);

  void startRectClip(GC gc, int x, int y, int width, int height);
  void startPixmapClip(GC gc, Pixmap xpixmap, int dx, int dy);
  void endPixmapClip(GC gc);

  void drawLine(Window xwin, GC gc, int x1, int y1, int x2, int y2);

  void drawRectangle(Window xwin, GC gc, int x, int y, int width, int height) const;

  void fillRectangle(Window xwin, GC gc, int x, int y, int width, int height);

  void drawPoint(Window xwin, GC gc, int x, int y);

  void drawString(Window xwin, GC gc, int x, int y, const std::string &str);

  void copyArea(Window src_xwin, Window dest_xwin, GC gc,
                int src_x, int src_y, int src_width, int src_height,
                int desy_x, int desy_y);
  void copyPlanes(Window src_xwin, int src_depth, Window dest_xwin,
                  int dest_depth, GC gc, int src_x, int src_y,
                  int src_width, int src_height, int desy_x, int desy_y);

  Pixmap createStipplePixmap();

  void drawImage(Window xwin, GC gc, const CImagePtr &image, int x, int y);
  void drawImage(Window xwin, GC gc, const CImagePtr &image,
                 int src_x, int src_y, int dst_x, int dst_y,
                 uint width, uint height);

  void drawAlphaImage(Window xwin, GC gc, const CImagePtr &image, int x, int y);

  void putImage(Window xwin, GC gc, XImage *ximage, int src_x, int src_y,
                int dst_x, int dst_y, uint width, uint height);

  XFontStruct *loadFont(const char *name);

  const CXAtom &getAtom(const std::string &name) const;
  const CXAtom &getAtom(Atom atom) const;

  KeySym keycodeToKeysym(uint keycode) const;
  KeySym keycodeToKeysym(uint keycode, int state) const;

  char *keycodeToString(uint keycode) const;
  char *keycodeToString(uint keycode, int state) const;
  char  keycodeToChar(uint keycode) const;
  char  keycodeToChar(uint keycode, int state) const;

  char  keysymToChar  (KeySym keysym) const;
  char *keysymToString(KeySym keysym) const;

 private:
  char *keysymToString1(KeySym keysym, bool multi) const;

 public:
  uint keysymToKeycode(KeySym keysym) const;

  Window      getEventWindow(XEvent *event) const;
  std::string getEventName  (XEvent *event) const;
  Time        getEventTime  (XEvent *event) const;

  void trapStart() const;
  bool trapEnd  () const;

 private:
  static int XErrorHandler(Display *display, XErrorEvent *event);

 public:
  void setXErrorProc(XErrorProc error_proc);

 private:
  char *getXErrorRoutine(int code);
  char *getXErrorMessage(int code);

 public:
  CXWindow *selectWindow() const;

  bool selectRootRegion(int *xmin, int *ymin, int *xmax, int *ymax) const;

  void setCursor(Window xwin, Cursor cursor);
  void unsetCursor(Window xwin);

  bool        selectionSetText(Window xwin, const std::string &text);
  void        selectionResetText(Window xwin);
  void        selectionClearEvent();
  void        selectionRequestEvent(XSelectionRequestEvent *event);
  void        selectionNotifyEvent(XSelectionEvent *event);
  std::string selectionGetText(Window xwin);

  const CXColor &getCXColor(const CRGB &rgb);
  const CXColor &getCXColor(const CRGBA &rgba);
  const CXColor &getCXColor(Pixel pixel);

  void beep();

 public:
  bool inchToPixel(double in, double *pixel);
  bool mmToPixel(double mm, double *pixel);

 private:
  bool getMonitorWidthMM(double *width);
  bool getMonitorHeightMM(double *height);
};

#include <CXEventAdapter.h>

#endif
