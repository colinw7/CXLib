#include <CXMachine.h>
#include <CXWindow.h>
#include <CXFont.h>
#include <CXPixmap.h>
#include <CXAtom.h>
#include <CXUtil.h>
#include <CXtTimer.h>
#include <CWindow.h>

#include <X11/XKBlib.h>
#include <X11/extensions/shape.h>

#include <COSSignal.h>
#include <COSTimer.h>
#include <COSTime.h>
#include <CEnv.h>
#include <CConfig.h>
#include <CTimer.h>
#include <CThrow.h>

static bool selection_notify_received;

CXMachine *
CXMachine::
getInstance()
{
  static CXMachine *instance_;

  if (! instance_)
    instance_ = new CXMachine();

  return instance_;
}

CXMachine::
CXMachine()
{
  static bool locked;

  if (locked)
    return;

  locked = true;

  event_adapter_ = std::make_unique<CXEventAdapter>();

  atomMgr_ = std::make_unique<CXAtomMgr>(*this);

  XSetErrorHandler(CXMachine::XErrorHandler);

  CWindowMgrInst->setFactory(new CXWindowFactory);

  locked = false;
}

CXMachine::
~CXMachine()
{
  for (int i = 0; i < num_screens_; ++i) {
    if (screens_.find(i) != screens_.end())
      delete screens_[i];

    if (displays_.find(i) != displays_.end())
      XCloseDisplay(displays_[i]);
  }
}

void
CXMachine::
init()
{
}

Display *
CXMachine::
openDisplay(const std::string &display_name)
{
  if (display_) {
    CTHROW("Display already open");
    return display_;
  }

  std::string display_name1 = display_name;

  if (display_name1 == "") {
    display_ = XOpenDisplay(0);

    if (display_)
      display_name1 = DisplayString(display_);
  }
  else
    display_ = XOpenDisplay(const_cast<char *>(display_name1.c_str()));

  if (! display_)
    return 0;

  CXUtil::decodeDisplayName(display_name1, hostname_, &display_num_, &screen_num_);

  displays_[screen_num_] = display_;

  num_screens_ = ScreenCount(display_);

  if (CEnvInst.exists("CX_LIB_DEBUG"))
    enterDebugMode();

  display_name_ = display_name;

  CXFont ::setPrototype();
  CXImage::setPrototype();

  return display_;
}

Display *
CXMachine::
openXtDisplay(const std::string &display_name, const std::string &app_name, int *argc, char **argv)
{
  if (display_) {
    CTHROW("Display already open");
    return display_;
  }

  app_context_ = XtCreateApplicationContext();

  std::string class_name = app_name;

  auto len = class_name.size();

  for (size_t i = 0; i < len; ++i) {
    if (islower(class_name[i])) {
      class_name[i] = char(toupper(class_name[i]));
      break;
    }
  }

  Display *display =
    XtOpenDisplay(app_context_, display_name.c_str(), app_name.c_str(), class_name.c_str(),
                  nullptr, 0, argc, argv);

  if (! display)
    return 0;

  XtDisplayInitialize(app_context_, display, app_name.c_str(),
                      class_name.c_str(), nullptr, 0, argc, argv);

  CXtTimer::setAppContext(app_context_);

  setDisplay(display);

  if (CEnvInst.exists("CX_LIB_DEBUG"))
    enterDebugMode();

  display_name_ = display_name;

  CXFont ::setPrototype();
  CXImage::setPrototype();

  return display;
}

void
CXMachine::
closeDisplay()
{
  if (! display_) {
    CTHROW("Display not open");
    return;
  }

  XCloseDisplay(display_);

  displays_[screen_num_] = 0;

  display_      = 0;
  display_name_ = "";
  hostname_     = "";
  display_num_  = 0;
  screen_num_   = 0;
  app_context_  = 0;
}

void
CXMachine::
setDisplay(Display *display)
{
  if (display) {
    display_ = display;

    std::string display_name = DisplayString(display_);

    CXUtil::decodeDisplayName(display_name, hostname_, &display_num_, &screen_num_);

    displays_[screen_num_] = display_;

    num_screens_ = ScreenCount(display_);
  }
  else {
    if (display_)
      displays_[screen_num_] = 0;

    display_      = 0;
    display_name_ = "";
    hostname_     = "";
    display_num_  = 0;
    screen_num_   = 0;
    app_context_  = 0;
  }
}

Display *
CXMachine::
getScreenDisplay(int screen_num) const
{
  CXMachine *th = const_cast<CXMachine *>(this);

  if (! display_) {
    th->openDisplay("");

    if (! display_) {
      std::cerr << "Failed to open display\n";
      exit(1);
    }
  }

  if (screen_num < 0 || screen_num >= num_screens_) {
    CTHROW("Invalid Screen Num");
    return 0;
  }

  if (screen_num_ == screen_num)
    return display_;

  if (displays_.find(screen_num) != displays_.end())
    return th->displays_[screen_num];

  char display_name1[256];

  sprintf(display_name1, "%s:%d.%d", th->hostname_.c_str(), th->display_num_, screen_num);

  Display *display = XOpenDisplay(display_name1);

  if (! display)
    return 0;

  th->displays_[screen_num] = display;

  return display;
}

CXScreen *
CXMachine::
getCXScreen(int screen_num) const
{
  Display *display = getScreenDisplay(screen_num);

  if (! display)
    return 0;

  CXMachine *th = const_cast<CXMachine *>(this);

  if (th->screens_.find(screen_num) == th->screens_.end())
    th->screens_[screen_num] = new CXScreen(screen_num);

  return th->screens_[screen_num];
}

Screen *
CXMachine::
getScreen(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getScreen();
}

Visual *
CXMachine::
getVisual(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getVisual();
}

Colormap
CXMachine::
getColormap(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getColormap();
}

int
CXMachine::
getWidth(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getWidth();
}

int
CXMachine::
getHeight(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getHeight();
}

int
CXMachine::
getDepth(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getDepth();
}

Pixel
CXMachine::
getBlackPixel(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getBlackPixel();
}

Pixel
CXMachine::
getWhitePixel(int screen_num) const
{
  CXScreen *cxscreen = getCXScreen(screen_num);

  return cxscreen->getWhitePixel();
}

Window
CXMachine::
getRoot(int screen_num) const
{
  if (screen_num < 0) {
    if (display_)
      return DefaultRootWindow(display_);
    else
      return 0;
  }
  else {
    CXScreen *cxscreen = getCXScreen(screen_num);

    if (cxscreen)
      return cxscreen->getRoot();
    else
      return 0;
  }
}

CXWindow *
CXMachine::
getCXRoot(int screen_num) const
{
  return new CXWindow(getRoot(screen_num));
}

Window
CXMachine::
getWindowRoot(Window xwin) const
{
  Window  root;
  Window  parent;
  Window *children;
  uint    num_children;

  if (! XQueryTree(display_, xwin, &root, &parent, &children, &num_children))
    return None;

  if (children)
    XFree(children);

  return root;
}

Window
CXMachine::
getWindowTop(Window xwin) const
{
  Window parent1 = xwin;
  Window parent2 = getWindowParent(parent1);

  while (parent2 != None) {
    parent1 = parent2;
    parent2 = getWindowParent(parent1);
  }

  return parent1;
}

Window
CXMachine::
getWindowParent(Window xwin) const
{
  Window  root;
  Window  parent;
  Window *children;
  uint    num_children;

  if (! XQueryTree(display_, xwin, &root, &parent, &children, &num_children))
    return None;

  if (children)
    XFree(children);

  if (parent == root)
    return None;

  return parent;
}

bool
CXMachine::
getWindowChildren(Window xwin, Window **children, int *num_children) const
{
  Window root;
  Window parent;
  uint   num_children1;

  if (! XQueryTree(display_, xwin, &root, &parent, children, &num_children1)) {
    *num_children = 0;
    return false;
  }

  *num_children = int(num_children1);

  return true;
}

void
CXMachine::
enterDebugMode() const
{
  synchronize();
}

void
CXMachine::
synchronize() const
{
  XSynchronize(display_, True);
}

void
CXMachine::
grabServer() const
{
  if (display_)
    XGrabServer(display_);
}

void
CXMachine::
ungrabServer() const
{
  if (display_)
    XUngrabServer(display_);
}

void
CXMachine::
setEventAdapter(CXEventAdapter *adapter)
{
  event_adapter_ = EventAdapterP(adapter);
}

void
CXMachine::
mainLoop(CXMachineEventProc proc)
{
  while (true) {
    while (eventPending()) {
      nextEvent();

      if (proc)
        (*proc)(this, &event_);
    }

    COSTimer::msleep(10);
  }
}

void
CXMachine::
mainLoop(uint twait, CXEventAdapter *adapter)
{
  while (true) {
    while (eventPending()) {
      nextEvent();

      processEvent();
    }

    CTimerMgrInst->tick();

    if      (adapter)
      adapter->idleEvent();
    else if (event_adapter_)
      event_adapter_->idleEvent();

    COSTimer::msleep(twait);
  }
}

void
CXMachine::
tickLoop(uint nframes, CXEventAdapter *adapter)
{
  long secs1, usecs1, secs2, usecs2, dsecs, dusecs;

  int usecs = 1000000/nframes;

  while (true) {
    COSTime::getHRTime(&secs1, &usecs1);

    while (eventPending()) {
      nextEvent();

      processEvent();
    }

    if      (adapter)
      adapter->tickEvent();
    else if (event_adapter_)
      event_adapter_->tickEvent();

    COSTime::getHRTime(&secs2, &usecs2);

    COSTime::diffHRTime(secs1, usecs1, secs2, usecs2, &dsecs, &dusecs);

    if (dsecs == 0 && dusecs < usecs)
      COSTimer::msleep(uint(usecs - dusecs));
  }
}

bool
CXMachine::
processEvent()
{
  CEventAdapter *event_adapter = 0;

  event_win_ = getEventWindow(&event_);

  CXWindow *window = lookupWindow(event_win_);

  if (window)
    event_adapter = window->getEventAdapter();

  if (! event_adapter)
    event_adapter = event_adapter_.get();

/*
  if (event_adapter) {
    if (! (event_.type & event_adapter->event_mask))
      return false;
  }
*/

  switch (event_.type) {
    case ButtonPress: {
      event_button_num_ = CMouseButton(event_.xbutton.button);

      event_pos_ = CIPoint2D(event_.xbutton.x, event_.xbutton.y);

      event_button_pressed_ = true;

      ++event_button_count_;

      uint dt = uint(event_last_time_ - event_button_time_);

      if (dt > 500) {
        event_button_time_  = event_last_time_;
        event_button_count_ = 1;
      }

      if (event_adapter) {
        CMouseEvent bevent(event_pos_, event_button_num_, uint(event_button_count_),
                           CEventModifier(event_modifier_));

        event_adapter->buttonPressEvent(bevent);
      }

      break;
    }
    case ButtonRelease: {
      event_button_num_ = CMouseButton(event_.xbutton.button);

      event_pos_ = CIPoint2D(event_.xbutton.x, event_.xbutton.y);

      event_button_pressed_ = false;

      if (event_adapter) {
        CMouseEvent bevent(event_pos_, event_button_num_, uint(event_button_count_),
                           CEventModifier(event_modifier_));

        event_adapter->buttonReleaseEvent(bevent);
      }

      break;
    }
    case MotionNotify: {
      event_pos_ = CIPoint2D(event_.xmotion.x, event_.xmotion.y);

      if (event_adapter) {
        CMouseEvent bevent(event_pos_, event_button_num_, uint(event_button_count_),
                           CEventModifier(event_modifier_));

        if (event_button_pressed_)
          event_adapter->buttonMotionEvent(bevent);
        else
          event_adapter->pointerMotionEvent(bevent);
      }

      break;
    }
    case KeyPress: {
      event_pos_ = CIPoint2D(event_.xkey.x, event_.xkey.y);

      event_keysym_ = getEventKeysym();

      if      (event_keysym_ == XK_Shift_L   || event_keysym_ == XK_Shift_R)
        setIsShift(true);
      else if (event_keysym_ == XK_Control_L || event_keysym_ == XK_Control_R)
        setIsCtrl (true);
      else if (event_keysym_ == XK_Alt_L     || event_keysym_ == XK_Alt_R ||
               event_keysym_ == XK_Meta_L    || event_keysym_ == XK_Meta_R)
        setIsAlt  (true);

      if (event_keysym_ == XK_Print ||
          (event_keysym_ == XK_p && getIsAlt())) {
        if (window) {
          std::cerr << "Print Window\n";

          CImagePtr image = window->getImage();

          image->write("snapshot.png", CFILE_TYPE_IMAGE_PNG);
        }
      }

      if (event_adapter) {
        std::string text;

        if (event_keysym_ != XK_Shift_L   && event_keysym_ != XK_Shift_R &&
            event_keysym_ != XK_Control_L && event_keysym_ != XK_Control_R &&
            event_keysym_ != XK_Alt_L     && event_keysym_ != XK_Alt_R)
          text = getEventString(&event_);

        if      (event_keysym_ == 0xff08) event_keysym_ = 0x08;
        else if (event_keysym_ == 0xff0d) event_keysym_ = 0x0d;
        else if (event_keysym_ == 0xff1b) event_keysym_ = 0x1b;

        CKeyEvent kevent(event_pos_, CKeyType(event_keysym_),
                         text, CEventModifier(event_modifier_));

        event_adapter->keyPressEvent(kevent);
      }

      break;
    }
    case KeyRelease: {
       event_keysym_ = getEventKeysym();

       if      (event_keysym_ == XK_Shift_L   || event_keysym_ == XK_Shift_R)
         setIsShift(false);
       else if (event_keysym_ == XK_Control_L || event_keysym_ == XK_Control_R)
         setIsCtrl (false);
       else if (event_keysym_ == XK_Alt_L     || event_keysym_ == XK_Alt_R)
         setIsAlt  (false);

      if (event_adapter) {
        std::string text;

        if (event_keysym_ != XK_Shift_L   && event_keysym_ != XK_Shift_R &&
            event_keysym_ != XK_Control_L && event_keysym_ != XK_Control_R &&
            event_keysym_ != XK_Alt_L     && event_keysym_ != XK_Alt_R)
          text = getEventString(&event_);

        if      (event_keysym_ == 0xff08) event_keysym_ = 0x08;
        else if (event_keysym_ == 0xff0d) event_keysym_ = 0x0d;
        else if (event_keysym_ == 0xff1b) event_keysym_ = 0x1b;

        CKeyEvent kevent(event_pos_, CKeyType(event_keysym_),
                         text, CEventModifier(event_modifier_));

        event_adapter->keyReleaseEvent(kevent);
      }

      break;
    }
    case EnterNotify: {
      XEvent event1;

      if (! checkWindowTypedEvent(event_win_, LeaveNotify, &event1)) {
        if (event_adapter)
          event_adapter->enterEvent();
      }

      break;
    }
    case LeaveNotify: {
      XEvent event1;

      if (! checkWindowTypedEvent(event_win_, EnterNotify, &event1)) {
        if (event_adapter)
          event_adapter->leaveEvent();
      }

      break;
    }
    case Expose: {
      if (event_.xexpose.count == 0) {
        if (event_adapter)
          event_adapter->exposeEvent();
      }

      break;
    }
    case ConfigureNotify: {
      if (event_adapter) {
        if (window) {
          uint w = uint(event_.xconfigure.width);
          uint h = uint(event_.xconfigure.height);

          if (window->getLastWidth() != w || window->getLastHeight() != h) {
            event_adapter->resizeEvent();

            window->setLastWidth (w);
            window->setLastHeight(h);
          }
        }
        else
          event_adapter->resizeEvent();
      }

      break;
    }
    case UnmapNotify: {
      if (event_adapter)
        event_adapter->visibilityEvent(false);

      break;
    }
    case VisibilityNotify: {
      if (event_adapter) {
        bool visible = (event_.xvisibility.state != VisibilityFullyObscured);

        event_adapter->visibilityEvent(visible);
      }

      break;
    }
    case SelectionRequest: {
      selectionRequestEvent(&event_.xselectionrequest);

      break;
    }
    case SelectionClear: {
      selectionClearEvent();

      break;
    }
    case SelectionNotify: {
      selectionNotifyEvent(&event_.xselection);

      break;
    }
    case PropertyNotify: {
//    const CXAtom &atom = getAtom(event_.xproperty.atom);

      break;
    }
    case ClientMessage: {
      const CXAtom &atom = getAtom(Atom(event_.xclient.message_type));

      if (isWMProtocolsAtom(atom)) {
        const CXAtom &atom1 = getAtom(Atom(event_.xclient.data.l[0]));

        if (isWMDeleteWindowAtom(atom1)) {
          if (event_adapter)
            event_adapter->closeEvent();
        }
      }
      else {
        const CXAtom &winMsgAtom = getAtom("WINDOW_MESSAGE");

        const CXAtom &clientAtom = getAtom(Atom(event_.xclient.data.l[0]));

        if (clientAtom == winMsgAtom) {
          if (event_adapter) {
            const CXAtom &msgAtom = getAtom(Atom(event_.xclient.data.l[1]));

            CXWindow *from = lookupWindow(Window(event_.xclient.data.l[2]));

            event_adapter->clientMessageEvent(from, msgAtom.getName().c_str());
          }
        }
      }

      break;
    }
    case DestroyNotify: {
      if (event_adapter)
        event_adapter->closeEvent();

      break;
    }
    default:
      return false;
  }

  return true;
}

KeySym
CXMachine::
getEventKeysym() const
{
  return keycodeToKeysym(event_.xkey.keycode, int(event_.xkey.state));
}

std::string
CXMachine::
getEventString() const
{
  return getEventString(&event_);
}

std::string
CXMachine::
getEventString(const XEvent *event) const
{
  if (event->type == KeyPress || event->type == KeyRelease)
    return keycodeToString(event->xkey.keycode, int(event->xkey.state));
  else
    return "";
}

char
CXMachine::
getEventChar() const
{
  return keycodeToChar(event_.xkey.keycode, int(event_.xkey.state));
}

char *
CXMachine::
getEventEscape() const
{
  static char buffer[2];

  buffer[0] = getEventChar();

  if (buffer[0] != '\0')
    return buffer;

  return keysymToString(event_keysym_);
}

CXWindow *
CXMachine::
getEventXWindow() const
{
  return lookupWindow(event_win_);
}

int
CXMachine::
getEventRootX() const
{
  Window root = getRoot();

  int root_x, root_y;

  translateCoords(event_win_, root, getEventX(), getEventY(), &root_x, &root_y, nullptr);

  return root_x;
}

int
CXMachine::
getEventRootY() const
{
  Window root = getRoot();

  int root_x, root_y;

  translateCoords(event_win_, root, getEventX(), getEventY(), &root_x, &root_y, nullptr);

  return root_y;
}

void
CXMachine::
getWindowRootPos(Window xwin, int *x, int *y) const
{
  Window root = getRoot();

  translateCoords(xwin, root, 0, 0, x, y, nullptr);
}

CEvent *
CXMachine::
convertEvent(XEvent *event)
{
  static CMouseEvent mevent;
  static CKeyEvent   kevent;

  switch (event_.type) {
    case ButtonPress:
    case ButtonRelease: {
      mevent = CMouseEvent(CIPoint2D(event->xbutton.x, event->xbutton.y),
                           convertEventButton(event->xbutton.button), 1,
                           convertEventState(event->xbutton.state));

      return &mevent;
    }
    case KeyPress:
    case KeyRelease: {
      kevent = CKeyEvent(CIPoint2D(event->xkey.x, event->xkey.y),
                         convertEventKeyCodeToType(event->xkey.keycode, event->xkey.state),
                         convertEventKeyCodeToString(event->xkey.keycode, event->xkey.state),
                         convertEventState(event->xkey.state));

      return &kevent;
    }
    default:
      return 0;
  }
}

CMouseButton
CXMachine::
convertEventButton(uint button)
{
  switch (button) {
    case 1 : return CBUTTON_LEFT;
    case 2 : return CBUTTON_MIDDLE;
    case 3 : return CBUTTON_RIGHT;
    default: return CBUTTON_NONE;
  }
}

CEventModifier
CXMachine::
convertEventState(uint state)
{
  uint modifiers = CMODIFIER_NONE;

  if (state & ShiftMask  ) modifiers |= CMODIFIER_SHIFT;
  if (state & ControlMask) modifiers |= CMODIFIER_CONTROL;
  if (state & Mod1Mask   ) modifiers |= CMODIFIER_ALT;
  if (state & Mod2Mask   ) modifiers |= CMODIFIER_META;

  return CEventModifier(modifiers);
}

CKeyType
CXMachine::
convertEventKeyCodeToType(uint keycode, uint state)
{
  KeySym keysym = keycodeToKeysym(keycode, int(state));

  return CKeyType(keysym);
}

std::string
CXMachine::
convertEventKeyCodeToString(uint keycode, uint state)
{
  std::string str = keycodeToString(keycode, int(state));

  return str;
}

bool
CXMachine::
eventPending() const
{
  return XPending(display_);
}

void
CXMachine::
nextEvent() const
{
  CXMachine *th = const_cast<CXMachine *>(this);

  nextEvent(&th->event_);

  th->event_last_time_ = getEventTime(&th->event_);
}

void
CXMachine::
nextEvent(XEvent *event) const
{
  if (app_context_ != 0)
    XtAppNextEvent(app_context_, event);
  else
    XNextEvent(display_, event);
}

bool
CXMachine::
flushEvents(bool sync) const
{
  XFlush(display_);

  if (sync)
    XSync(display_, False);

  return true;
}

bool
CXMachine::
flushWindowEvent(Window xwin, uint event_mask)
{
  flushEvents();

  if (checkWindowEvent(xwin, event_mask, &event_))
    return true;

  return false;
}

int
CXMachine::
flushWindowEvents(Window xwin, uint event_mask)
{
  flushEvents();

  int count = 0;

  while (checkWindowEvent(xwin, event_mask, &event_))
    ++count;

  return count;
}

bool
CXMachine::
compareEvents(XEvent *event1, XEvent *event2)
{
  if (event1->type != event2->type)
    return false;

  if      (event1->type == ButtonPress) {
    XButtonPressedEvent *button_event1 = reinterpret_cast<XButtonPressedEvent *>(event1);
    XButtonPressedEvent *button_event2 = reinterpret_cast<XButtonPressedEvent *>(event2);

    if (button_event1->button != button_event2->button ||
        button_event1->state  != button_event2->state)
      return false;
  }
  else if (event1->type == KeyPress) {
    XKeyPressedEvent *key_event1 = reinterpret_cast<XKeyPressedEvent *>(event1);
    XKeyPressedEvent *key_event2 = reinterpret_cast<XKeyPressedEvent *>(event2);

    if (key_event1->keycode != key_event2->keycode ||
        key_event1->state   != key_event2->state)
      return false;
  }
  else if (event1->type == KeyRelease) {
    XKeyReleasedEvent *key_event1 = reinterpret_cast<XKeyReleasedEvent *>(event1);
    XKeyReleasedEvent *key_event2 = reinterpret_cast<XKeyReleasedEvent *>(event2);

    if (key_event1->keycode != key_event2->keycode ||
        key_event1->state   != key_event2->state)
      return false;
  }
  else
    return false;

  return true;
}

bool
CXMachine::
waitForClientMessage(Window, XEvent *event)
{
  if (! event) {
    XEvent event1;

    while (! checkTypedEvent(ClientMessage, &event1))
      ;
  }
  else {
    while (! checkTypedEvent(ClientMessage, event))
      ;
  }

  return true;
}

bool
CXMachine::
checkWindowEvent(Window xwin, uint event_mask, XEvent *event) const
{
  return XCheckWindowEvent(display_, xwin, event_mask, event);
}

bool
CXMachine::
checkTypedEvent(int event_type, XEvent *event) const
{
  return XCheckTypedEvent(display_, event_type, event);
}

bool
CXMachine::
checkWindowTypedEvent(Window xwin, int event_type, XEvent *event) const
{
  return XCheckTypedWindowEvent(display_, xwin, event_type, event);
}

void
CXMachine::
maskEvent(uint mask, XEvent *event) const
{
  XMaskEvent(display_, mask, event);
}

void
CXMachine::
allowEvents(int mode) const
{
  XAllowEvents(display_, mode, CurrentTime);
}

Colormap
CXMachine::
getWindowColormap(Window xwin) const
{
  XWindowAttributes attr;

  if (! XGetWindowAttributes(display_, xwin, &attr))
    return 0;

  return attr.colormap;
}

uint
CXMachine::
getWindowEventMask(Window xwin) const
{
  XWindowAttributes attr;

  if (! XGetWindowAttributes(display_, xwin, &attr))
    return 0;

  return uint(attr.your_event_mask);
}

bool
CXMachine::
getWindowViewable(Window xwin) const
{
  XWindowAttributes attr;

  if (! XGetWindowAttributes(display_, xwin, &attr))
    return false;

  return (attr.map_state == IsViewable);
}

bool
CXMachine::
getWindowPosition(Window xwin, int *x, int *y) const
{
  int x1, y1;

  if (! getWindowGeometry(xwin, &x1, &y1))
    return false;

  Window parent = getWindowParent(xwin);

  while (parent != None) {
    int x2, y2;

    if (! getWindowGeometry(parent, &x2, &y2))
      return false;

    x1 += x2;
    y1 += y2;

    parent = getWindowParent(parent);
  }

  *x = x1;
  *y = y1;

  return true;
}

bool
CXMachine::
getWindowSize(Window xwin, int *w, int *h) const
{
  int x1, y1;

  if (! getWindowGeometry(xwin, &x1, &y1, w, h))
    return false;

  return true;
}

bool
CXMachine::
getWindowGeometry(Window xwin, int *x, int *y, int *width, int *height, int *border) const
{
  XWindowAttributes attr;

  if (XGetWindowAttributes(display_, xwin, &attr)) {
    if (x)
      *x = attr.x;

    if (y)
      *y = attr.y;

    if (width)
      *width = attr.width;

    if (height)
      *height = attr.height;

    if (border)
      *border = attr.border_width;

    return true;
  }
  else {
    if (x)
      *x = 0;

    if (y)
      *y = 0;

    if (width)
      *width = 1;

    if (height)
      *height = 1;

    if (border)
      *border = 0;

    return false;
  }
}

//----------------

CXWindow *
CXMachine::
lookupWindow(Window xwin) const
{
  for (int i = 0; i < num_screens_; ++i) {
    CXScreenMap::const_iterator p = screens_.find(i);

    if (p == screens_.end())
      continue;

    CXWindow *window = (*p).second->lookupWindow(xwin);

    if (window)
      return window;
  }

  return 0;
}

bool
CXMachine::
isValidWindow(Window xwin) const
{
  trapStart();

  int    x, y;
  Window root;
  uint   width, height, border, depth;

  Status status = XGetGeometry(display_, xwin, &root, &x, &y, &width, &height, &border, &depth);

  if (trapEnd() && status)
    return true;

  return false;
}

int
CXMachine::
getMulticlickTime() const
{
  if (app_context_ != 0)
    return XtGetMultiClickTime(display_);
  else
    return 500;
}

void
CXMachine::
setInputFocus(Window xwin) const
{
  if (xwin == None)
    xwin = PointerRoot;

  XSetInputFocus(display_, xwin, RevertToPointerRoot, CurrentTime);
}

bool
CXMachine::
translateCoords(Window src_w, Window dest_w, int src_x, int src_y,
                int *dest_x, int *dest_y, Window *child) const
{
  Window child1;
  int    dest_x1, dest_y1;

  if (! XTranslateCoordinates(display_, src_w, dest_w, src_x, src_y, &dest_x1, &dest_y1, &child1))
    return false;

  if (dest_x) *dest_x = dest_x1;
  if (dest_y) *dest_y = dest_y1;
  if (child ) *child  = child1;

  return true;
}

bool
CXMachine::
queryPointer(Window xwin, int *x, int *y)
{
  uint   mask;
  Window root, child;
  int    win_x, win_y;

  return XQueryPointer(display_, xwin, &root, &child, x, y, &win_x, &win_y, &mask);
}

void
CXMachine::
destroyWindow(Window xwin)
{
  XDestroyWindow(display_, xwin);
}

void
CXMachine::
mapWindow(Window xwin)
{
  XMapWindow(display_, xwin);
}

void
CXMachine::
mapWindowRaised(Window xwin)
{
  XMapRaised(display_, xwin);
}

void
CXMachine::
mapWindowChildren(Window xwin)
{
  XMapSubwindows(display_, xwin);
}

void
CXMachine::
unmapWindow(Window xwin)
{
  XUnmapWindow(display_, xwin);
}

void
CXMachine::
reparentWindow(Window xwin, Window parent_xwin, int x, int y)
{
  XReparentWindow(display_, xwin, parent_xwin, x, y);
}

void
CXMachine::
lowerWindow(Window xwin)
{
  XLowerWindow(display_, xwin);
}

void
CXMachine::
raiseWindow(Window xwin)
{
  XRaiseWindow(display_, xwin);
}

void
CXMachine::
minimizeWindow(Window xwin)
{
  resizeWindow(xwin, 10, 10);
}

void
CXMachine::
maximizeWindow(Window xwin)
{
  int border;

  MaximizeData max_data;

  getWindowGeometry(xwin, &max_data.x, &max_data.y, &max_data.width, &max_data.height, &border);

  max_data_map_[xwin] = max_data;

  int width  = getCXScreen(0)->getWidth();
  int height = getCXScreen(0)->getHeight();

  resizeWindow(xwin, width, height);
}

void
CXMachine::
unmaximizeWindow(Window xwin)
{
  MaximizeDataMap::iterator p = max_data_map_.find(xwin);

  if (p != max_data_map_.end()) {
    int border;

    MaximizeData max_data;

    getWindowGeometry(xwin, &max_data.x, &max_data.y, &max_data.width, &max_data.height, &border);

    MaximizeData max_data1 = (*p).second;

    moveResizeWindow(xwin, max_data1.x, max_data1.y, max_data1.width, max_data1.height);

    max_data_map_[xwin] = max_data;
  }
  else
    maximizeWindow(xwin);
}

void
CXMachine::
moveWindow(Window xwin, int x, int y)
{
  XMoveWindow(display_, xwin, x, y);
}

void
CXMachine::
resizeWindow(Window xwin, int width, int height)
{
  width  = std::max(width , 1);
  height = std::max(height, 1);

  XResizeWindow(display_, xwin, uint(width), uint(height));
}

void
CXMachine::
moveResizeWindow(Window xwin, int x, int y, int width, int height)
{
  XMoveResizeWindow(display_, xwin, x, y, uint(width), uint(height));
}

void
CXMachine::
clearWindow(Window xwin)
{
  XClearWindow(display_, xwin);
}

void
CXMachine::
configureWindow(Window xwin, uint mask, XWindowChanges *xwc)
{
  XConfigureWindow(display_, xwin, mask, xwc);
}

void
CXMachine::
restackWindows(Window *windows, int num_windows)
{
  XRestackWindows(display_, windows, num_windows);
}

void
CXMachine::
shapeCombineMask(Window xwin, Window mask, int x, int y, int op)
{
#ifdef SHAPE
  XShapeCombineMask(display_, xwin, ShapeBounding, x, y, mask, op);
#endif
}

bool
CXMachine::
grabPointer(Window xwin, uint event_mask, Cursor cursor) const
{
  Window root = getRoot();

  if (XGrabPointer(display_, xwin, True, event_mask, GrabModeAsync, GrabModeAsync, root,
                   cursor, CurrentTime) != GrabSuccess)
    return false;

  return true;
}

void
CXMachine::
ungrabPointer() const
{
  XUngrabPointer(display_, CurrentTime);
}

bool
CXMachine::
grabKeyboard(Window xwin)
{
  if (XGrabKeyboard(display_, xwin, True, GrabModeAsync,
                    GrabModeAsync, CurrentTime) != GrabSuccess)
    return false;

  return true;
}

void
CXMachine::
ungrabKeyboard()
{
  XUngrabKeyboard(display_, CurrentTime);
}

void
CXMachine::
grabKey(Window xwin, int keycode, int state)
{
  XGrabKey(display_, keycode, uint(state), xwin, True, GrabModeAsync, GrabModeAsync);
}

void
CXMachine::
ungrabKey(Window xwin, int keycode, int state)
{
  XUngrabKey(display_, keycode, uint(state), xwin);
}

void
CXMachine::
grabButton(Window xwin, int button, int modifiers, int event_mask, Cursor cursor)
{
  XGrabButton(display_, uint(button), uint(modifiers), xwin, True, uint(event_mask),
              GrabModeSync, GrabModeAsync, None, cursor);
}

void
CXMachine::
ungrabButton(Window xwin, int button, int modifiers)
{
  XUngrabButton(display_, uint(button), uint(modifiers), xwin);
}

Window
CXMachine::
createOverrideWindow(int x, int y, int width, int height, int border,
                     uint attr_mask, XSetWindowAttributes *attr)
{
  Window root = getRoot();

  attr_mask |= CWOverrideRedirect;

  XSetWindowAttributes attr1;

  if (attr)
    memcpy(&attr1, attr, sizeof(XSetWindowAttributes));

  attr1.override_redirect = True;

  Window xwin =
    XCreateWindow(display_, root, x, y, uint(width), uint(height), uint(border),
                  CopyFromParent, InputOutput, CopyFromParent, attr_mask, &attr1);

  return xwin;
}

Window
CXMachine::
createWindow(Window parent_xwin, int x, int y, int width, int height,
             int border, uint attr_mask, XSetWindowAttributes *attr)
{
  if (parent_xwin == None)
    parent_xwin = getRoot();

  Window xwin =
    XCreateWindow(display_, parent_xwin, x, y, uint(width), uint(height), uint(border),
                  CopyFromParent, InputOutput, CopyFromParent, attr_mask, attr);

  return xwin;
}

Window
CXMachine::
createWindow(int x, int y, int width, int height,
             int border, uint attr_mask, XSetWindowAttributes *attr)
{
  return createWindow(getRoot(), x, y, width, height, border, attr_mask, attr);
}

CXPixmap *
CXMachine::
createPixmap(Window xwin, uint width, uint height)
{
  return new CXPixmap(xwin, width, height);
}

CXPixmap *
CXMachine::
createPixmap(uint width, uint height)
{
  return new CXPixmap(width, height);
}

Pixmap
CXMachine::
createXPixmap(Window xwin, uint width, uint height, uint depth)
{
  assert(xwin != None);

  if (depth == 0)
    depth = uint(getDepth(0));

  Pixmap xpixmap = XCreatePixmap(display_, xwin, width, height, depth);

  return xpixmap;
}

Pixmap
CXMachine::
createXPixmap(uint width, uint height, uint depth)
{
  return createXPixmap(getRoot(), width, height, depth);
}

void
CXMachine::
freeXPixmap(Pixmap xpixmap)
{
  XFreePixmap(display_, xpixmap);
}

void
CXMachine::
setKeyAutoRepeatRate(uint timeout, uint interval)
{
  XkbSetAutoRepeatRate(display_, XkbUseCoreKbd, timeout, interval);
}

void
CXMachine::
getKeyAutoRepeatRate(uint *timeout, uint *interval)
{
  XkbGetAutoRepeatRate(display_, XkbUseCoreKbd, timeout, interval);
}

void
CXMachine::
installColormap(Colormap cmap)
{
  XInstallColormap(display_, cmap);
}

void
CXMachine::
changeWindowAtributes(Window xwin, uint attr_mask, XSetWindowAttributes *attr)
{
  XChangeWindowAttributes(display_, xwin, attr_mask, attr);
}

void
CXMachine::
killClient(Window xwin)
{
  XKillClient(display_, xwin);
}

void
CXMachine::
setWindowTitle(Window xwin, const std::string &title)
{
  const CXAtom &atom = getAtom(XA_WM_NAME);

  setStringProperty(xwin, atom, title);
}

void
CXMachine::
getWindowTitle(Window xwin, std::string &title)
{
  const CXAtom &atom1 = getAtom("_NET_WM_NAME");

  if (! getStringProperty(xwin, atom1, title)) {
    const CXAtom &atom2 = getAtom(XA_WM_NAME);

    if (! getStringProperty(xwin, atom2, title))
      title = "window";
  }
}

void
CXMachine::
setIconTitle(Window xwin, const std::string &title)
{
  const CXAtom &atom = getAtom(XA_WM_ICON_NAME);

  setStringProperty(xwin, atom, title);
}

void
CXMachine::
getIconTitle(Window xwin, std::string &title)
{
  const CXAtom &atom1 = getAtom("_NET_WM_ICON_NAME");

  if (! getStringProperty(xwin, atom1, title)) {
    const CXAtom &atom2 = getAtom(XA_WM_ICON_NAME);

    if (! getStringProperty(xwin, atom2, title))
      title = "icon";
  }
}

bool
CXMachine::
setIntegerProperty(Window xwin, const CXAtom &name, int value)
{
  CARD32 val = CARD32(value);

  XChangeProperty(display_, xwin, name.getXAtom(), XA_CARDINAL,
                  32, PropModeReplace, reinterpret_cast<uchar *>(&val), 1);

  return true;
}

bool
CXMachine::
setIntegerArrayProperty(Window xwin, const CXAtom &name, int *values, int num_values)
{
  std::vector<long> vals;

  auto num_values1 = size_t(num_values);

  vals.resize(uint(num_values1));

  for (size_t i = 0; i < num_values1; ++i)
    vals[i] = values[i];

  XChangeProperty(display_, xwin, name.getXAtom(), XA_CARDINAL,
                  32, PropModeReplace, reinterpret_cast<uchar *>(&vals[0]), num_values);

  return true;
}

bool
CXMachine::
setStringProperty(Window xwin, const CXAtom &name, const std::string &value)
{
  XChangeProperty(display_, xwin, name.getXAtom(), XA_STRING,
                  8, PropModeReplace, reinterpret_cast<uchar *>(const_cast<char *>(value.c_str())),
                  int(value.size()));

  return true;
}

bool
CXMachine::
setWindowProperty(const CXAtom &name, Window value)
{
  setWindowProperty(getRoot(), name, value);

  return true;
}

bool
CXMachine::
setWindowProperty(Window xwin, const CXAtom &name, Window value)
{
  CARD32 val = CARD32(value);

  XChangeProperty(display_, xwin, name.getXAtom(), XA_WINDOW,
                  32, PropModeReplace, reinterpret_cast<uchar *>(&val), 1);

  return true;
}

bool
CXMachine::
setWindowArrayProperty(Window xwin, const CXAtom &name, Window *xwins, int num_xwins)
{
  XChangeProperty(display_, xwin, name.getXAtom(), XA_WINDOW,
                  32, PropModeReplace, reinterpret_cast<uchar *>(xwins), num_xwins);

  return true;
}

bool
CXMachine::
setAtomProperty(Window xwin, const CXAtom &name, const CXAtom *atom)
{
  Atom xatom = atom->getXAtom();

  XChangeProperty(display_, xwin, name.getXAtom(), XA_ATOM,
                  32, PropModeReplace, reinterpret_cast<uchar *>(&xatom), 1);

  return true;
}

bool
CXMachine::
setAtomArrayProperty(Window xwin, const CXAtom &name, const CXAtom **atoms, int num_atoms)
{
  std::vector<Atom> atoms1;

  auto num_atoms1 = size_t(num_atoms);

  atoms1.resize(num_atoms1);

  for (size_t i = 0; i < num_atoms1; ++i)
    atoms1[i] = atoms[i]->getXAtom();

  XChangeProperty(display_, xwin, name.getXAtom(), XA_ATOM,
                  32, PropModeReplace, reinterpret_cast<uchar *>(&atoms1[0]), num_atoms);

  return true;
}

bool
CXMachine::
setStringListProperty(Window xwin, const CXAtom &name, char **strs, int num_strs)
{
  XTextProperty text_prop;

  if (XStringListToTextProperty(strs, num_strs, &text_prop)) {
    XSetTextProperty(display_, xwin, &text_prop, name.getXAtom());

    XFree(text_prop.value);
  }

  return true;
}

//------

bool
CXMachine::
getIntegerProperty(const CXAtom &name, int *value)
{
  return getIntegerProperty(getRoot(), name, value);
}

bool
CXMachine::
getIntegerProperty(Window xwin, const CXAtom &name, int *value)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 1, False, XA_CARDINAL,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (n == 0 || ! data)
    return false;

  CARD32 val = *(reinterpret_cast<CARD32 *>(data));

  *value = int(val);

  XFree(data);

  return true;
}

bool
CXMachine::
getStringProperty(const CXAtom &name, std::string &value)
{
  return getStringProperty(getRoot(), name, value);
}

bool
CXMachine::
getStringProperty(Window xwin, const CXAtom &name, std::string &value)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 1024, False, XA_STRING,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (n == 0 || ! data)
    return false;

  value = std::string(reinterpret_cast<char *>(data));

  XFree(data);

  return true;
}

bool
CXMachine::
getPixmapProperty(const CXAtom &name, Pixmap *value)
{
  return getPixmapProperty(getRoot(), name, value);
}

bool
CXMachine::
getPixmapProperty(Window xwin, const CXAtom &name, Pixmap *value)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 1, False, XA_PIXMAP,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (format != 32 || n != 1 || left != 0)
    return false;

  *value = *(reinterpret_cast<Pixmap *>(data));

  XFree(data);

  return true;
}

bool
CXMachine::
getWindowProperty(const CXAtom &name, Window *value) const
{
  return getWindowProperty(getRoot(), name, value);
}

bool
CXMachine::
getWindowProperty(Window xwin, const CXAtom &name, Window *value) const
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 1, False, XA_WINDOW,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (format != 32 || n != 1 || left != 0)
    return false;

  *value = *(reinterpret_cast<Window *>(data));

  XFree(data);

  return true;
}

bool
CXMachine::
getWindowArrayProperty(Window xwin, const CXAtom &name, std::vector<Window> &windows) const
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 65535, False, XA_WINDOW,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (format != 32 || left != 0)
    return false;

  for (ulong i = 0; i < n; ++i)
    windows.push_back((reinterpret_cast<Window *>(data))[i]);

  XFree(data);

  return true;
}

bool
CXMachine::
getAtomProperty(Window xwin, const CXAtom &name, CXAtom &atom)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 1, False, XA_ATOM,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (format != 32 || n != 1 || left != 0)
    return false;

  atom = getAtom(*(reinterpret_cast<Atom *>(data)));

  XFree(data);

  return true;
}

bool
CXMachine::
getAtomArrayProperty(Window xwin, const CXAtom &name, std::vector<CXAtom> &atoms)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, name.getXAtom(), 0, 65535, False, XA_ATOM,
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (format != 32 || left != 0)
    return false;

  for (ulong i = 0; i < n; ++i)
    atoms.push_back(getAtom((reinterpret_cast<Atom *>(data))[i]));

  XFree(data);

  return true;
}

//------

void
CXMachine::
deleteProperty(Window xwin, const CXAtom &name)
{
  XDeleteProperty(display_, xwin, name.getXAtom());
}

//------

bool
CXMachine::
isExtendedWM() const
{
  Window xwin;

  if (! getWindowProperty(getRoot(), getAtom("_NET_SUPPORTING_WM_CHECK"), &xwin))
    return false;

  return isValidWindow(xwin);
}

bool
CXMachine::
isWMWindow(Window xwin)
{
  XWindowAttributes attr;

  if (! XGetWindowAttributes(display_, xwin, &attr))
    return false;

  return (attr.map_state == IsViewable && ! attr.override_redirect);
}

void
CXMachine::
setWMStateNormal(Window xwin)
{
  uint data[2];

  data[0] = uint(NormalState);
  data[1] = None;

  XChangeProperty(display_, xwin,
                  getWMStateAtom().getXAtom(), getWMStateAtom().getXAtom(),
                  32, PropModeReplace, reinterpret_cast<uchar *>(data), 2);
}

void
CXMachine::
setWMStateIconic(Window xwin, Window icon_xwin)
{
  uint data[2];

  data[0] = uint(IconicState);
  data[1] = uint(icon_xwin);

  XChangeProperty(display_, xwin,
                  getWMStateAtom().getXAtom(), getWMStateAtom().getXAtom(),
                  32, PropModeReplace, reinterpret_cast<uchar *>(data), 2);
}

bool
CXMachine::
isWMStateNormal(Window xwin)
{
  int state = getWMState(xwin);

  return (state == NormalState);
}

bool
CXMachine::
isWMStateIconic(Window xwin)
{
  int state = getWMState(xwin);

  return (state == IconicState);
}

int
CXMachine::
getWMState(Window xwin)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return -1;

  if (XGetWindowProperty(display_, xwin, getWMStateAtom().getXAtom(),
                         0, 3, False, getWMStateAtom().getXAtom(),
                         &type, &format, &n, &left, &data) != Success)
    return -1;

  if (n == 0 || ! data)
    return -1;

  int state = (reinterpret_cast<int *>(data))[0];

  XFree(data);

  return state;
}

bool
CXMachine::
getWMName(Window xwin, std::string &name)
{
  name = "";

  XTextProperty text_prop;

  if (! XGetWMName(display_, xwin, &text_prop))
    return false;

  const char *cname = reinterpret_cast<const char *>(text_prop.value);

  if (cname && cname[0] != '\0')
    name = cname;

  return true;
}

bool
CXMachine::
getWMIconName(Window xwin, std::string &name)
{
  name = "";

  XTextProperty text_prop;

  if (! XGetWMIconName(display_, xwin, &text_prop))
    return false;

  const char *cname = reinterpret_cast<const char *>(text_prop.value);

  if (cname && cname[0] != '\0')
    name = cname;

  return true;
}

void
CXMachine::
getWMNormalHints(Window xwin, XSizeHints **size_hints, int *supplied)
{
  static XSizeHints size_hints1;

  long supplied1;

  if (! XGetWMNormalHints(display_, xwin, &size_hints1, &supplied1))
    size_hints1.flags = 0;

  *size_hints = &size_hints1;
  *supplied   = int(supplied1);
}

bool
CXMachine::
setWMNormalHints(Window xwin, XSizeHints *size_hints)
{
  XSetWMNormalHints(display_, xwin, size_hints);

  return true;
}

Window
CXMachine::
getWMIconWindowHint(Window xwin)
{
  XWMHints *wm_hints;

  getWMHints(xwin, &wm_hints);

  if (! wm_hints)
    return None;

  if (wm_hints->flags & IconWindowHint) {
    if (isValidWindow(wm_hints->icon_window))
      return wm_hints->icon_window;
  }

  return None;
}

void
CXMachine::
getWMHints(Window xwin, XWMHints **wm_hints)
{
  *wm_hints = XGetWMHints(display_, xwin);
}

void
CXMachine::
getWMTransientFor(Window xwin, Window *transient_xwin)
{
  if (! XGetTransientForHint(display_, xwin, transient_xwin))
    *transient_xwin = None;
}

bool
CXMachine::
setWMTransientFor(Window xwin, Window transient_xwin)
{
  return XSetTransientForHint(display_, xwin, transient_xwin);
}

void
CXMachine::
getWMClassHint(Window xwin, XClassHint **class_hint)
{
  static XClassHint class_hint1;

  if (! XGetClassHint(display_, xwin, &class_hint1)) {
    class_hint1.res_name  = 0;
    class_hint1.res_class = 0;
  }

  *class_hint = &class_hint1;
}

bool
CXMachine::
setWMClassHint(Window xwin, const std::string &res_name, const std::string &res_class)
{
  const char *strs[2] = { res_name.c_str(), res_class.c_str() };

  const CXAtom &wm_class = getAtom("WM_CLASS");

  setStringListProperty(xwin, wm_class, const_cast<char **>(strs), 2);

  return true;
}

void
CXMachine::
getWMClientMachine(Window xwin, std::string &client_machine)
{
  char *client_machine1 = nullptr;

  XTextProperty text_prop;

  if (XGetWMClientMachine(display_, xwin, &text_prop))
    client_machine1 = reinterpret_cast<char *>(text_prop.value);

  if (client_machine1 == 0)
    client_machine1 = const_cast<char *>("localhost");

  client_machine = client_machine1;
}

bool
CXMachine::
setWMClientMachine(Window xwin, const std::string &str)
{
  const CXAtom &client_machine = getAtom("WM_CLIENT_MACHINE");

  return setStringProperty(xwin, client_machine, str.c_str());
}

void
CXMachine::
getWMCommand(Window xwin, int *argc, char ***argv)
{
  if (! XGetCommand(display_, xwin, argv, argc))
    *argc = 0;
}

void
CXMachine::
getWMColormapWindows(Window xwin, Window **cmap_windows, int *num_cmap_windows)
{
  if (! XGetWMColormapWindows(display_, xwin, cmap_windows, num_cmap_windows))
    *num_cmap_windows = 0;
}

void
CXMachine::
getWMProtocols(Window xwin, const CXAtom ***protocols, int *num_protocols)
{
  Atom *protocols1;

  if (! XGetWMProtocols(display_, xwin, &protocols1, num_protocols))
    *num_protocols = 0;

  auto num_protocols1 = size_t(*num_protocols);

  *protocols = new const CXAtom * [num_protocols1];

  for (size_t i = 0; i < num_protocols1; ++i)
    (*protocols)[i] = &getAtom(protocols1[i]);
}

bool
CXMachine::
getWMMwmHints(Window xwin, MotifWmHints &mwm_hints)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  if (! display_)
    return false;

  if (XGetWindowProperty(display_, xwin, getMwmHintsAtom().getXAtom(),
                         0, 20, False, getMwmHintsAtom().getXAtom(),
                         &type, &format, &n, &left, &data) != Success)
    return false;

  if (! data || n != PROP_MOTIF_WM_HINTS_ELEMENTS)
    return false;

  long *ldata = reinterpret_cast<long *>(data);

  mwm_hints.flags       = int(ldata[0]);
  mwm_hints.functions   = int(ldata[1]);
  mwm_hints.decorations = int(ldata[2]);
  mwm_hints.input_mode  = int(ldata[3]);
  mwm_hints.status      = int(ldata[4]);

  return true;
}

const CXAtom &
CXMachine::
getWMStateAtom()
{
  return atomMgr_->getWMState();
}

const CXAtom &
CXMachine::
getMwmHintsAtom()
{
  return atomMgr_->getMwmHints();
}

const CXAtom &
CXMachine::
getWMProtocolsAtom()
{
  return atomMgr_->getWMProtocols();
}

const CXAtom &
CXMachine::
getWMDeleteWindowAtom()
{
  return atomMgr_->getWMDeleteWindow();
}

const CXAtom &
CXMachine::
getXSetRootIdAtom()
{
  return atomMgr_->getXSetRootId();
}

const CXAtom &
CXMachine::
getCwmDesktopAtom()
{
  return atomMgr_->getCwmDesktop();
}

bool
CXMachine::
isWMChangeStateAtom(const CXAtom &atom)
{
  return atomMgr_->isWMChangeState(atom);
}

bool
CXMachine::
isWMClassAtom(const CXAtom &atom)
{
  return atomMgr_->isWMClass(atom);
}

bool
CXMachine::
isWMClientMachineAtom(const CXAtom &atom)
{
  return atomMgr_->isWMClientMachine(atom);
}

bool
CXMachine::
isWMCommandAtom(const CXAtom &atom)
{
  return atomMgr_->isWMCommand(atom);
}

bool
CXMachine::
isWMDeleteWindowAtom(const CXAtom &atom)
{
  return atomMgr_->isWMDeleteWindow(atom);
}

bool
CXMachine::
isWMHintsAtom(const CXAtom &atom)
{
  return atomMgr_->isWMHints(atom);
}

bool
CXMachine::
isWMIconNameAtom(const CXAtom &atom)
{
  return atomMgr_->isWMIconName(atom);
}

bool
CXMachine::
isWMNameAtom(const CXAtom &atom)
{
  return atomMgr_->isWMName(atom);
}

bool
CXMachine::
isWMNormalHintsAtom(const CXAtom &atom)
{
  return atomMgr_->isWMNormalHints(atom);
}

bool
CXMachine::
isWMProtocolsAtom(const CXAtom &atom)
{
  return atomMgr_->isWMProtocols(atom);
}

bool
CXMachine::
isWMSaveYourselfAtom(const CXAtom &atom)
{
  return atomMgr_->isWMSaveYourself(atom);
}

bool
CXMachine::
isWMSizeHintsAtom(const CXAtom &atom)
{
  return atomMgr_->isWMSizeHints(atom);
}

bool
CXMachine::
isWMStateAtom(const CXAtom &atom)
{
  return atomMgr_->isWMState(atom);
}

bool
CXMachine::
isWMTakeFocusAtom(const CXAtom &atom)
{
  return atomMgr_->isWMTakeFocus(atom);
}

bool
CXMachine::
isWMTransientForAtom(const CXAtom &atom)
{
  return atomMgr_->isWMTransientFor(atom);
}

bool
CXMachine::
isWMZoomHintsAtom(const CXAtom &atom)
{
  return atomMgr_->isWMZoomHints(atom);
}

bool
CXMachine::
sendKeyPressedEvent(Window xwin, int x, int y, int keycode)
{
  XKeyPressedEvent event;

  event.type        = KeyPress;
  event.serial      = 0;
  event.send_event  = True;
  event.display     = display_;
  event.window      = xwin;
  event.root        = getRoot();
  event.subwindow   = 0;
  event.time        = getLastEventTime();
  event.x           = 0;
  event.y           = 0;
  event.x_root      = x;
  event.y_root      = y;
  event.state       = 0;
  event.keycode     = uint(keycode);
  event.same_screen = True;

  if (! XSendEvent(display_, xwin, False, KeyPressMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendKeyReleasedEvent(Window xwin, int x, int y, int keycode)
{
  XKeyPressedEvent event;

  event.type        = KeyRelease;
  event.serial      = 0;
  event.send_event  = True;
  event.display     = display_;
  event.window      = xwin;
  event.root        = getRoot();
  event.subwindow   = 0;
  event.time        = getLastEventTime();
  event.x           = 0;
  event.y           = 0;
  event.x_root      = x;
  event.y_root      = y;
  event.state       = 0;
  event.keycode     = uint(keycode);
  event.same_screen = True;

  if (! XSendEvent(display_, xwin, False, KeyReleaseMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendStringClientMessage(Window server_xwin, Window client_xwin, const std::string &str)
{
  const CXAtom &atom1 = getAtom("CLIENT_WINDOW");

  setWindowProperty(server_xwin, atom1, client_xwin);

  const CXAtom &atom2 = getAtom("CLIENT_MESSAGE");

  setStringProperty(server_xwin, atom2, str);

  XClientMessageEvent event;

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = server_xwin;
  event.message_type = XA_CARDINAL;
  event.format       = 32;
  event.data.l[0]    = long(atom2.getXAtom());

  if (! XSendEvent(display_, server_xwin, False, NoEventMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
readStringClientMessage(Window server_xwin, Window *client_xwin, std::string &str)
{
  const CXAtom &atom1 = getAtom("CLIENT_WINDOW");

  getWindowProperty(server_xwin, atom1, client_xwin);

  const CXAtom &atom2 = getAtom("CLIENT_MESSAGE");

  getStringProperty(server_xwin, atom2, str);

  return true;
}

bool
CXMachine::
sendStringServerMessage(Window client_xwin, Window server_xwin, const std::string &str)
{
  const CXAtom &atom1 = getAtom("SERVER_WINDOW");

  setWindowProperty(client_xwin, atom1, server_xwin);

  const CXAtom &atom2 = getAtom("SERVER_MESSAGE");

  setStringProperty(client_xwin, atom2, str);

  XClientMessageEvent event;

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = client_xwin;
  event.message_type = XA_CARDINAL;
  event.format       = 32;
  event.data.l[0]    = long(atom2.getXAtom());

  if (! XSendEvent(display_, client_xwin, False, NoEventMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
readStringServerMessage(Window client_xwin, Window *server_xwin, std::string &str)
{
  const CXAtom &atom1 = getAtom("SERVER_WINDOW");

  getWindowProperty(client_xwin, atom1, server_xwin);

  const CXAtom &atom2 = getAtom("SERVER_MESSAGE");

  getStringProperty(client_xwin, atom2, str);

  return true;
}

bool
CXMachine::
sendShowDesktop(bool show)
{
  Window root = getRoot();

  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = root;
  event.message_type = getAtom("_NET_SHOWING_DESKTOP").getXAtom();
  event.format       = 32;
  event.data.l[0]    = long(show ? 1 : 0);

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendClose(Window window)
{
  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_CLOSE_WINDOW").getXAtom();
  event.format       = 32;
  event.data.l[0]    = long(CurrentTime);
  event.data.l[1]    = 2;

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendActivate(Window window)
{
  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_ACTIVE_WINDOW").getXAtom();
  event.format       = 32;
  event.data.l[0]    = 2;
  event.data.l[1]    = long(CurrentTime);
  event.data.l[2]    = 0;

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendMoveWindowBy(Window window, int dx, int dy)
{
  int x, y;

  if (! CXMachineInst->getWindowPosition(window, &x, &y))
    return false;

  int w, h;

  if (! CXMachineInst->getWindowSize(window, &w, &h))
    return false;

  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_MOVERESIZE_WINDOW").getXAtom();
  event.format       = 32;
  event.data.l[0]    = long(StaticGravity | (0xF << 8) | (0x2 << 12));
  event.data.l[1]    = long(x + dx);
  event.data.l[2]    = long(y + dy);
  event.data.l[3]    = long(w);
  event.data.l[4]    = long(h);

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendDragWindowBy(Window window, int x, int y, int button, int action)
{
  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_WM_MOVERESIZE").getXAtom();
  event.format       = 32;
  event.data.l[0]    = long(x);
  event.data.l[1]    = long(y);
  event.data.l[2]    = long(action);
  event.data.l[3]    = long(button);
  event.data.l[4]    = 2;

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendRestackWindow(Window window, Window sibling, bool above)
{
  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_RESTACK_WINDOW").getXAtom();
  event.format       = 32;
  event.data.l[0]    = 2; // pager
  event.data.l[1]    = long(sibling);
  event.data.l[2]    = long(above ? Above : Below);

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendWmState(Window window, int action, const std::string &atom1, const std::string &atom2)
{
  XClientMessageEvent event;

  memset(&event, 0, sizeof(XClientMessageEvent));

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = window;
  event.message_type = getAtom("_NET_WM_STATE").getXAtom();
  event.format       = 32;
  event.data.l[0]    = long(action);
  event.data.l[1]    = long(getAtom(atom1).getXAtom());
  event.data.l[2]    = long(atom2 != "" ? getAtom(atom2).getXAtom() : 0);
  event.data.l[3]    = 2;

  Window root = getRoot();

  if (! XSendEvent(display_, root, False, (SubstructureNotifyMask|SubstructureRedirectMask),
                   reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendWindowMessage(Window from, Window to, const std::string &str)
{
  const CXAtom &atom = getAtom("WINDOW_MESSAGE");

  const CXAtom &msgAtom = getAtom(str);

  XClientMessageEvent event;

  event.type         = ClientMessage;
  event.serial       = 0;
  event.send_event   = True;
  event.display      = display_;
  event.window       = to;
  event.message_type = XA_CARDINAL;
  event.format       = 32;
  event.data.l[0]    = long(atom.getXAtom());
  event.data.l[1]    = long(msgAtom.getXAtom());
  event.data.l[2]    = long(from);

  if (! XSendEvent(display_, to, False, NoEventMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendEvent(Window xwin, int propagate, XEvent *event, uint event_mask)
{
  if (! XSendEvent(display_, xwin, propagate, event_mask, event))
    return true;

  return true;
}

bool
CXMachine::
sendExposeEvent(Window xwin)
{
  XExposeEvent event;

  event.type       = Expose;
  event.serial     = 0;
  event.send_event = True;
  event.display    = display_;
  event.window     = xwin;
  event.x          = 0;
  event.y          = 0;
  event.width      = 9999;
  event.height     = 9999;
  event.count      = 0;

  if (! XSendEvent(display_, xwin, False, ExposureMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
sendConfigureNotifyEvent(Window xwin, int x, int y, int width, int height,
                         int border, Window above_xwin)
{
  XConfigureEvent event;

  event.type              = ConfigureNotify;
  event.display           = display_;
  event.event             = xwin;
  event.window            = xwin;
  event.x                 = x;
  event.y                 = y;
  event.width             = width;
  event.height            = height;
  event.border_width      = border;
  event.above             = above_xwin;
  event.override_redirect = False;

  if (! XSendEvent(display_, xwin, False, StructureNotifyMask, reinterpret_cast<XEvent *>(&event)))
    return true;

  return false;
}

bool
CXMachine::
addInput(Window xwin, uint event_mask)
{
  uint win_event_mask = getWindowEventMask(xwin);

  if ((win_event_mask & event_mask) != event_mask) {
    selectInput(xwin, win_event_mask | event_mask);

    win_event_mask = getWindowEventMask(xwin);

    return ((win_event_mask & event_mask) == event_mask);
  }

  return true;
}

bool
CXMachine::
selectInput(Window xwin, uint event_mask)
{
  return XSelectInput(display_, xwin, event_mask);
}

void
CXMachine::
setWindowBackgroundColor(Window xwin, Pixel bg)
{
  XSetWindowBackground(display_, xwin, bg);
}

void
CXMachine::
setWindowBackgroundPixmap(Window xwin, Pixmap pixmap)
{
  XSetWindowBackgroundPixmap(display_, xwin, pixmap);
}

void
CXMachine::
setWindowBorderWidth(Window xwin, int width)
{
  XSetWindowBorderWidth(display_, xwin, uint(width));
}

void
CXMachine::
warpPointer(Window xwin, int x, int y)
{
  XWarpPointer(display_, None, xwin, 0, 0, 0, 0, x, y);
}

void
CXMachine::
addToSaveSet(Window xwin)
{
  XAddToSaveSet(display_, xwin);
}

void
CXMachine::
removeFromSaveSet(Window xwin)
{
  XRemoveFromSaveSet(display_, xwin);
}

GC
CXMachine::
createGC(const CXColor &fg, const CXColor &bg) const
{
  return createGC(fg.getPixel(), bg.getPixel());
}

GC
CXMachine::
createGC(Pixel fg, Pixel bg) const
{
  Window root = getRoot();

  return createGC(root, fg, bg);
}

GC
CXMachine::
createGC(Window xwin, const CXColor &fg, const CXColor &bg) const
{
  return createGC(xwin, fg.getPixel(), bg.getPixel());
}

GC
CXMachine::
createGC(Window xwin, Pixel fg, Pixel bg) const
{
  XGCValues gc_values;

  if (xwin == None)
    xwin = getRoot();

  gc_values.function   = GXcopy;
  gc_values.plane_mask = AllPlanes;
  gc_values.background = bg;
  gc_values.foreground = fg;

  GC gc = XCreateGC(display_, xwin,
                    GCFunction | GCPlaneMask | GCForeground | GCBackground,
                    &gc_values);

  return gc;
}

GC
CXMachine::
createXorGC() const
{
  Window root = getRoot();

  return createXorGC(root);
}

GC
CXMachine::
createXorGC(Window xwin) const
{
  return createXorGC(xwin, getBlackPixel(0), getWhitePixel(0));
}

GC
CXMachine::
createXorGC(Window xwin, const CXColor &fg, const CXColor &bg) const
{
  return createXorGC(xwin, fg.getPixel(), bg.getPixel());
}

GC
CXMachine::
createXorGC(Window xwin, Pixel fg, Pixel bg) const
{
  XGCValues gc_values;

  if (xwin == None)
    xwin = getRoot();

  gc_values.function       = GXxor;
  gc_values.plane_mask     = AllPlanes;
  gc_values.background     = bg;
  gc_values.foreground     = bg ^ fg;
  gc_values.subwindow_mode = IncludeInferiors;

  GC gc = XCreateGC(display_, xwin,
                    GCFunction | GCPlaneMask | GCForeground | GCSubwindowMode,
                    &gc_values);

  return gc;
}

void
CXMachine::
freeGC(GC gc)
{
  XFreeGC(display_, gc);
}

void
CXMachine::
setFillStipple(GC gc, Pixmap pixmap)
{
  XGCValues gc_values;

  gc_values.fill_style = FillStippled;
  gc_values.stipple    = pixmap;

  XChangeGC(display_, gc, GCFillStyle | GCStipple, &gc_values);
}

void
CXMachine::
setForeground(GC gc, const CXColor &fg)
{
  setForeground(gc, fg.getPixel());
}

void
CXMachine::
setForeground(GC gc, Pixel pixel)
{
  XSetForeground(display_, gc, pixel);
}

void
CXMachine::
getForeground(GC gc, Pixel *pixel)
{
  XGCValues values;

  XGetGCValues(display_, gc, GCForeground, &values);

  *pixel = values.foreground;
}

void
CXMachine::
setBackground(GC gc, const CXColor &bg)
{
  setBackground(gc, bg.getPixel());
}

void
CXMachine::
setBackground(GC gc, Pixel pixel)
{
  XSetBackground(display_, gc, pixel);
}

void
CXMachine::
getBackground(GC gc, Pixel *pixel)
{
  XGCValues values;

  XGetGCValues(display_, gc, GCBackground, &values);

  *pixel = values.background;
}

void
CXMachine::
startRectClip(GC gc, int x, int y, int width, int height)
{
  XPoint points[4];

  points[0].x = short(x);
  points[0].y = short(y);
  points[2].x = short(x + width  - 1);
  points[2].y = short(y + height - 1);

  points[1].x = short(points[2].x);
  points[1].y = short(points[0].y);
  points[3].x = short(points[0].x);
  points[3].y = short(points[2].y);

  Region region = XPolygonRegion(points, 4, EvenOddRule);

  XSetRegion(display_, gc, region);

  XDestroyRegion(region);
}

void
CXMachine::
startPixmapClip(GC gc, Pixmap xpixmap, int dx, int dy)
{
  XSetClipMask  (display_, gc, xpixmap);
  XSetClipOrigin(display_, gc, dx, dy);
}

void
CXMachine::
endPixmapClip(GC gc)
{
  XSetClipMask  (display_, gc, None);
  XSetClipOrigin(display_, gc, 0, 0);
}

void
CXMachine::
drawLine(Window xwin, GC gc, int x1, int y1, int x2, int y2)
{
  XDrawLine(display_, xwin, gc, x1, y1, x2, y2);
}

void
CXMachine::
drawRectangle(Window xwin, GC gc, int x, int y, int width, int height) const
{
  XDrawRectangle(display_, xwin, gc, x, y, uint(width), uint(height));
}

void
CXMachine::
fillRectangle(Window xwin, GC gc, int x, int y, int width, int height)
{
  XFillRectangle(display_, xwin, gc, x, y, uint(width), uint(height));
}

void
CXMachine::
drawPoint(Window xwin, GC gc, int x, int y)
{
  XDrawPoint(display_, xwin, gc, x, y);
}

void
CXMachine::
drawString(Window xwin, GC gc, int x, int y, const std::string &str)
{
  XDrawString(display_, xwin, gc, x, y, str.c_str(), int(str.size()));
}

void
CXMachine::
copyArea(Window src_xwin, Window dest_xwin, GC gc, int src_x, int src_y,
         int src_width, int src_height, int desy_x, int desy_y)
{
  XCopyArea(display_, src_xwin, dest_xwin, gc, src_x, src_y,
            uint(src_width), uint(src_height), desy_x, desy_y);
}

void
CXMachine::
copyPlanes(Window src_xwin, int src_depth, Window dest_xwin, int dest_depth,
           GC gc, int src_x, int src_y, int src_width, int src_height,
           int desy_x, int desy_y)
{
  for (int i = 0; i < src_depth && i < dest_depth; ++i)
    XCopyPlane(display_, src_xwin, dest_xwin, gc, src_x, src_y,
               uint(src_width), uint(src_height),
               desy_x, desy_y, uint(i + 1));
}

Pixmap
CXMachine::
createStipplePixmap()
{
  static Pixmap stipple_bitmap = None;
  static uchar  stipple_bits[] = { 0x02, 0x01 };

  if (stipple_bitmap == None) {
    Window root = getRoot();

    stipple_bitmap =
      XCreateBitmapFromData(display_, root, reinterpret_cast<char *>(stipple_bits), 2, 2);
  }

  return stipple_bitmap;
}

void
CXMachine::
drawImage(Window xwin, GC gc, const CImagePtr &image, int x, int y)
{
  CXImage *ximage = image.cast<CXImage>();

  if (! ximage)
    return;

  XImage *ximg = ximage->getXImage();

  if (! ximg)
    return;

  putImage(xwin, gc, ximg, 0, 0, x, y, image->getWidth(), image->getHeight());
}

void
CXMachine::
drawImage(Window xwin, GC gc, const CImagePtr &image, int src_x, int src_y,
          int dst_x, int dst_y, uint width, uint height)
{
  CXImage *ximage = image.cast<CXImage>();

  if (! ximage)
    return;

  XImage *ximg = ximage->getXImage();

  if (! ximg)
    return;

  putImage(xwin, gc, ximg, src_x, src_y, dst_x, dst_y, width, height);
}

void
CXMachine::
drawAlphaImage(Window xwin, GC gc, const CImagePtr &image, int x, int y)
{
  CXImage *ximage = image.cast<CXImage>();

  if (! ximage)
    return;

  XImage *ximg = ximage->getXImage();

  if (! ximg)
    return;

  XSetClipOrigin(display_, gc, x, y);

  XSetClipMask(display_, gc, ximage->getXMask());

  putImage(xwin, gc, ximg, 0, 0, x, y, image->getWidth(), image->getHeight());

  XSetClipOrigin(display_, gc, 0, 0);

  XSetClipMask(display_, gc, None);
}

void
CXMachine::
putImage(Window xwin, GC gc, XImage *ximage, int src_x, int src_y,
         int dst_x, int dst_y, uint width, uint height)
{
  XPutImage(display_, xwin, gc, ximage, src_x, src_y, dst_x, dst_y, width, height);
}

XFontStruct *
CXMachine::
loadFont(const char *name)
{
  return XLoadQueryFont(display_, name);
}

const CXAtom &
CXMachine::
getAtom(const std::string &name) const
{
  return atomMgr_->getCXAtom(name);
}

const CXAtom &
CXMachine::
getAtom(Atom atom) const
{
  return atomMgr_->getCXAtom(atom);
}

KeySym
CXMachine::
keycodeToKeysym(uint keycode) const
{
  //KeySym keysym = XKeycodeToKeysym(display_, keycode, 0);
  KeySym keysym = XkbKeycodeToKeysym(display_, KeyCode(keycode), 0, 0);

  return keysym;
}

KeySym
CXMachine::
keycodeToKeysym(uint keycode, int state) const
{
  KeySym keysym;

  if (app_context_ != 0) {
    Modifiers modifiers_return;

    XtTranslateKeycode(display_, KeyCode(keycode), uint(state), &modifiers_return, &keysym);

    if (keysym < 128) {
      if (state & ControlMask && ! (modifiers_return & ControlMask))
        keysym &= 0x9F;
    }
  }
  else {
    XKeyEvent event;
    char      buffer[64];

    memset(&event , 0, sizeof(event));
    memset(&keysym, 0, sizeof(keysym));

    event.display = display_;
    event.window  = getRoot();
    event.state   = uint(state);
    event.keycode = keycode;

    int len = XLookupString(&event, buffer, sizeof(buffer) - 1, &keysym, nullptr);

    buffer[len] = '\0';

//  if (event.state & ShiftMask)
//    keysym &= 0xCF;

    if (keysym < 128) {
      if (event.state & ControlMask)
        keysym &= 0x9F;
    }

//  keysym = XKeycodeToKeysym(display_, keycode, 0);
  }

  return keysym;
}

char *
CXMachine::
keycodeToString(uint keycode) const
{
  KeySym keysym = keycodeToKeysym(keycode);

  return keysymToString(keysym);
}

char *
CXMachine::
keycodeToString(uint keycode, int state) const
{
  KeySym keysym = keycodeToKeysym(keycode, state);

  return keysymToString(keysym);
}

char
CXMachine::
keycodeToChar(uint keycode) const
{
  KeySym keysym = keycodeToKeysym(keycode);

  if (getIsShift()) {
    KeySym lkeysym, ukeysym;

    XConvertCase(keysym, &lkeysym, &ukeysym);

    keysym = ukeysym;
  }

  return keysymToChar(keysym);
}

char
CXMachine::
keycodeToChar(uint keycode, int state) const
{
  KeySym keysym = keycodeToKeysym(keycode, state);

  return keysymToChar(keysym);
}

char
CXMachine::
keysymToChar(KeySym keysym) const
{
  char *str = keysymToString1(keysym, false);

  return str[0];
}

char *
CXMachine::
keysymToString(KeySym keysym) const
{
  return keysymToString1(keysym, true);
}

char *
CXMachine::
keysymToString1(KeySym keysym, bool multi) const
{
  static char str[32];

  bool match = true;

  str[1] = '\0';

  if      (keysym <= 255)
    str[0] = char(keysym);
  else if (keysym == XK_BackSpace)
    str[0] = '\b';
  else if (keysym == XK_Escape)
    str[0] = '\033';
  else if (keysym == XK_Return)
    str[0] = '\n';
  else if (keysym == XK_Tab)
    str[0] = '\t';
  else if (keysym == XK_ampersand)
    str[0] = '&';
  else if (keysym == XK_apostrophe)
    str[0] = '\'';
  else if (keysym == XK_asciicircum)
    str[0] = '^';
  else if (keysym == XK_asciitilde)
    str[0] = '~';
  else if (keysym == XK_asterisk)
    str[0] = '*';
  else if (keysym == XK_at)
    str[0] = '@';
  else if (keysym == XK_backslash)
    str[0] = '\\';
  else if (keysym == XK_bar)
    str[0] = '|';
  else if (keysym == XK_bracketleft)
    str[0] = '[';
  else if (keysym == XK_bracketright)
    str[0] = ']';
  else if (keysym == XK_braceleft)
    str[0] = '{';
  else if (keysym == XK_braceright)
    str[0] = '}';
  else if (keysym == XK_colon)
    str[0] = ':';
  else if (keysym == XK_comma)
    str[0] = ',';
  else if (keysym == XK_dollar)
    str[0] = '$';
  else if (keysym == XK_equal)
    str[0] = '=';
  else if (keysym == XK_exclam)
    str[0] = '!';
  else if (keysym == XK_grave)
    str[0] = '`';
  else if (keysym == XK_greater)
    str[0] = '>';
  else if (keysym == XK_less)
    str[0] = '<';
  else if (keysym == XK_minus)
    str[0] = '-';
  else if (keysym == XK_numbersign)
    str[0] = '#';
  else if (keysym == XK_parenleft)
    str[0] = '(';
  else if (keysym == XK_parenright)
    str[0] = ')';
  else if (keysym == XK_percent)
    str[0] = '%';
  else if (keysym == XK_period)
    str[0] = '.';
  else if (keysym == XK_plus)
    str[0] = '+';
  else if (keysym == XK_question)
    str[0] = '?';
  else if (keysym == XK_quotedbl)
    str[0] = '"';
  else if (keysym == XK_semicolon)
    str[0] = ';';
  else if (keysym == XK_slash)
    str[0] = '/';
  else if (keysym == XK_space)
    str[0] = ' ';
  else if (keysym == XK_underscore)
    str[0] = '_';
  else if (keysym == XK_Shift_L   || keysym == XK_Shift_R)
    str[0] = '\0';
  else if (keysym == XK_Control_L || keysym == XK_Control_R)
    str[0] = '\0';
  else if (keysym == XK_Alt_L     || keysym == XK_Alt_R)
    str[0] = '\0';
  else {
    str[0] = '\0';

    match = false;
  }

  if (match || ! multi)
    return str;

  const char *str1;

  if      (keysym == XK_Up       ) str1 = "\033[A";
  else if (keysym == XK_Down     ) str1 = "\033[B";
  else if (keysym == XK_Right    ) str1 = "\033[C";
  else if (keysym == XK_Left     ) str1 = "\033[D";
  else if (keysym == XK_F1       ) str1 = "\033OP";
  else if (keysym == XK_F2       ) str1 = "\033OQ";
  else if (keysym == XK_F3       ) str1 = "\033OR";
  else if (keysym == XK_F4       ) str1 = "\033OS";
  else if (keysym == XK_F5       ) str1 = "\033[15~";
  else if (keysym == XK_F6       ) str1 = "\033[17~";
  else if (keysym == XK_F7       ) str1 = "\033[18~";
  else if (keysym == XK_F8       ) str1 = "\033[19~";
  else if (keysym == XK_F9       ) str1 = "\033[20~";
  else if (keysym == XK_F10      ) str1 = "\033[21~";
  else if (keysym == XK_F11      ) str1 = "\033[22~";
  else if (keysym == XK_F12      ) str1 = "\033[24~";
  else if (keysym == XK_Insert   ) str1 = "\033[2~";
  else if (keysym == XK_Delete   ) str1 = "\b";
  else if (keysym == XK_Home     ) str1 = "\033OH";
  else if (keysym == XK_End      ) str1 = "\033OF";
  else if (keysym == XK_Page_Up  ) str1 = "\033[5~";
  else if (keysym == XK_Page_Down) str1 = "\033[6~";
  else                             str1 = XKeysymToString(keysym);

  return const_cast<char *>(str1);
}

uint
CXMachine::
keysymToKeycode(KeySym keysym) const
{
  uint keycode = XKeysymToKeycode(display_, keysym);

  return keycode;
}

Window
CXMachine::
getEventWindow(XEvent *event) const
{
  switch (event->type) {
    case MapRequest:
      return event->xmaprequest.window;
    case MapNotify:
      return event->xmap.window;
    case UnmapNotify:
      return event->xmap.window;
    case ConfigureRequest:
      return event->xconfigurerequest.window;
    case ConfigureNotify:
      return event->xconfigure.window;
    case ReparentNotify:
      return event->xreparent.window;
    case EnterNotify:
      return event->xcrossing.window;
    case LeaveNotify:
      return event->xcrossing.window;
    case ButtonPress:
      return event->xbutton.window;
    case ButtonRelease:
      return event->xbutton.window;
    case KeyPress:
      return event->xkey.window;
    case KeyRelease:
      return event->xkey.window;
    case MotionNotify:
      return event->xmotion.window;
    case Expose:
      return event->xexpose.window;
    case NoExpose:
      return event->xnoexpose.drawable;
    case FocusIn:
      return event->xfocus.window;
    case FocusOut:
      return event->xfocus.window;
    case PropertyNotify:
      return event->xproperty.window;
    case VisibilityNotify:
      return event->xvisibility.window;
    case DestroyNotify:
      return event->xdestroywindow.window;
    case ColormapNotify:
      return event->xcolormap.window;
    case MappingNotify:
      return event->xmapping.window;
    case ClientMessage:
      return event->xclient.window;
    default:
      return None;
  }
}

std::string
CXMachine::
getEventName(XEvent *event) const
{
  switch (event->type) {
    case MapRequest:
      return "MapRequest";
    case MapNotify:
      return "MapNotify";
    case UnmapNotify:
      return "UnmapNotify";
    case ConfigureRequest:
      return "ConfigureRequest";
    case ConfigureNotify:
      return "ConfigureNotify";
    case ReparentNotify:
      return "ReparentNotify";
    case EnterNotify:
      return "EnterNotify";
    case LeaveNotify:
      return "LeaveNotify";
    case ButtonPress:
      return "ButtonPress";
    case ButtonRelease:
      return "ButtonRelease";
    case KeyPress:
      return "KeyPress";
    case KeyRelease:
      return "KeyRelease";
    case MotionNotify:
      return "MotionNotify";
    case Expose:
      return "Expose";
    case NoExpose:
      return "NoExpose";
    case FocusIn:
      return "FocusIn";
    case FocusOut:
      return "FocusOut";
    case PropertyNotify:
      return "PropertyNotify";
    case VisibilityNotify:
      return "VisibilityNotify";
    case DestroyNotify:
      return "DestroyNotify";
    case ColormapNotify:
      return "ColormapNotify";
    case MappingNotify:
      return "MappingNotify";
    case ClientMessage:
      return "ClientMessage";
    default:
      return "????";
  }
}

Time
CXMachine::
getEventTime(XEvent *event) const
{
  switch (event->type) {
    case EnterNotify:
      return event->xcrossing.time;
    case LeaveNotify:
      return event->xcrossing.time;
    case ButtonPress:
      return event->xbutton.time;
    case ButtonRelease:
      return event->xbutton.time;
    case KeyPress:
      return event->xkey.time;
    case KeyRelease:
      return event->xkey.time;
    case MotionNotify:
      return event->xmotion.time;
    case PropertyNotify:
      return event->xproperty.time;
    default:
      return 0;
  }
}

void
CXMachine::
trapStart() const
{
  flushEvents();

  CXMachine *th = const_cast<CXMachine *>(this);

  th->trap_request_code_ = 0;
  th->trap_error_code_   = 0;

  th->trap_active_ = true;
}

bool
CXMachine::
trapEnd() const
{
  flushEvents();

  CXMachine *th = const_cast<CXMachine *>(this);

  th->trap_active_ = false;

  if (trap_request_code_ != 0 || trap_error_code_ != 0)
    return false;
  else
    return true;
}

int
CXMachine::
XErrorHandler(Display *, XErrorEvent *event)
{
  if (CXMachineInst->trap_active_)
    return False;

  const char *routine = "????";
  const char *message = "????";

  if (event) {
    CXMachineInst->trap_request_code_ = event->request_code;
    CXMachineInst->trap_error_code_   = event->error_code;

    routine = CXMachineInst->getXErrorRoutine(event->request_code);
    message = CXMachineInst->getXErrorMessage(event->error_code);
  }

  if (CXMachineInst->error_proc_) {
    std::string msg =
      CStrUtil::strprintf("X Error - %s (%d) : %s (%d)",
                          routine, event->request_code, message, event->error_code);

    (*CXMachineInst->error_proc_)(msg);
  }
  else
    fprintf(stderr, "X Error - %s (%d) : %s (%d)\n",
            routine, event->request_code, message, event->error_code);

  if (CXMachineInst->pedantic_)
    COSSignal::sendSignalToProcessGroup(SIGSEGV);

  return False;
}

void
CXMachine::
setXErrorProc(XErrorProc error_proc)
{
  error_proc_ = error_proc;
}

char *
CXMachine::
getXErrorRoutine(int code)
{
  static char buffer[256];

  std::string code_string = CStrUtil::toString(code);

  XGetErrorDatabaseText(display_, "XRequest", code_string.c_str(), "", buffer, 256);

  return buffer;
}

char *
CXMachine::
getXErrorMessage(int code)
{
  static char buffer[256];

  std::string code_string = CStrUtil::toString(code);

  XGetErrorDatabaseText(display_, "XProtoError", code_string.c_str(), "", buffer, 256);

  return buffer;
}

CXWindow *
CXMachine::
selectWindow() const
{
  CXCursor cursor(CURSOR_TYPE_QUESTION_ARROW);

  Window root = getRoot();

  if (! grabPointer(root, ButtonPressMask, cursor.getXCursor()))
    return 0;

  allowEvents(SyncPointer);

  XEvent event;

  XWindowEvent(display_, root, ButtonPressMask, &event);

  Window window = None;

  if (event.type == ButtonPress) {
    if (! translateCoords(root, root, event.xbutton.x_root, event.xbutton.y_root,
                          nullptr, nullptr, &window))
      window = None;
  }

  ungrabPointer();

  if (window == None)
    return 0;

  return new CXWindow(window);
}

bool
CXMachine::
selectRootRegion(int *xmin, int *ymin, int *xmax, int *ymax) const
{
  Window root = getRoot();

  GC gc = createXorGC(root);

  CXCursor cursor(CURSOR_TYPE_CROSS_HAIR);

  uint event_mask = ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;

  if (! grabPointer(root, event_mask, cursor.getXCursor()))
    return false;

  bool pressed = false;

  int x1 = 0, y1 = 0, x2 = 0, y2 = 0;

  while (TRUE) {
    nextEvent();

    if      (event_.type == ButtonPress) {
      x1 = event_.xbutton.x_root;
      y1 = event_.xbutton.y_root;
      x2 = x1;
      y2 = y1;

      drawRectangle(root, gc, std::min(x1, x2), std::min(y1, y2),
                    std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1);

      pressed = true;
    }
    else if (event_.type == MotionNotify) {
      if (pressed) {
        drawRectangle(root, gc, std::min(x1, x2), std::min(y1, y2),
                      std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1);

        x2 = event_.xmotion.x_root;
        y2 = event_.xmotion.y_root;

        drawRectangle(root, gc, std::min(x1, x2), std::min(y1, y2),
                      std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1);
      }
    }
    else {
      drawRectangle(root, gc, std::min(x1, x2), std::min(y1, y2),
                    std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1);

      pressed = false;

      break;
    }
  }

  ungrabPointer();

  if (x1 == x2 && y1 == y2)
    return false;

  if (xmin) *xmin = std::min(x1, x2);
  if (ymin) *ymin = std::min(y1, y2);
  if (xmax) *xmax = std::max(x1, x2);
  if (ymax) *ymax = std::max(y1, y2);

  return true;
}

void
CXMachine::
setCursor(Window xwin, Cursor cursor)
{
  XDefineCursor(display_, xwin, cursor);

  flushEvents();
}

void
CXMachine::
unsetCursor(Window xwin)
{
  XUndefineCursor(display_, xwin);

  flushEvents();
}

bool
CXMachine::
selectionSetText(Window xwin, const std::string &text)
{
  XSetSelectionOwner(display_, XA_PRIMARY, xwin, CurrentTime);

  if (XGetSelectionOwner(display_, XA_PRIMARY) != xwin)
    return false;

  selection_xwin_ = xwin;
  selection_text_ = text;
  selection_set_  = true;

  return true;
}

void
CXMachine::
selectionResetText(Window)
{
  XSetSelectionOwner(display_, XA_PRIMARY, None, CurrentTime);
}

void
CXMachine::
selectionClearEvent()
{
  CXEventAdapter *event_adapter = 0;

  if (selection_xwin_ != None) {
    CXWindow *window = lookupWindow(selection_xwin_);

    if (window)
      event_adapter = window->getXEventAdapter();
  }

  if (! event_adapter)
    event_adapter = event_adapter_.get();

  if (event_adapter)
    event_adapter->selectionClearEvent();

  selection_xwin_ = None;
  selection_text_ = "";
  selection_set_  = false;
}

void
CXMachine::
selectionRequestEvent(XSelectionRequestEvent *event)
{
  XSelectionEvent event1;

  if (selection_xwin_ == None || ! selection_set_)
    return;

  event1.type       = SelectionNotify;
  event1.serial     = 0;
  event1.send_event = True;
  event1.display    = event->display;
  event1.requestor  = event->requestor;
  event1.selection  = event->selection;
  event1.target     = event->target;
  event1.time       = event->time;

  if (event->target == XA_STRING) {
    XChangeProperty(event->display, event->requestor, event->property,
                    event->target, 8, PropModeReplace,
                    reinterpret_cast<uchar *>(const_cast<char *>(selection_text_.c_str())),
                    int(selection_text_.size()));

    event1.property = event->property;
  }
  else {
    event1.property = None;
  }

  XSendEvent(event->display, event->requestor, False, 0, reinterpret_cast<XEvent *>(&event1));
}

void
CXMachine::
selectionNotifyEvent(XSelectionEvent *event)
{
  ulong  n;
  Atom   type;
  ulong  left;
  uchar *data;
  int    format;

  selection_notify_received = true;

  selection_text_ = "";

  if (XGetWindowProperty(event->display, event->requestor, event->selection,
                         0, 256, False, event->target,
                         &type, &format, &n, &left, &data) != Success)
    return;

  if (! data)
    return;

  if (type != XA_STRING)
    return;

  selection_xwin_ = getEventWindow(reinterpret_cast<XEvent *>(event));
  selection_text_ = std::string(reinterpret_cast<char *>(data));
  selection_set_  = true;
}

std::string
CXMachine::
selectionGetText(Window xwin)
{
  XConvertSelection(display_, XA_PRIMARY, XA_STRING, XA_PRIMARY, xwin, CurrentTime);

  selection_notify_received = false;

  while (! selection_notify_received) {
    nextEvent();

    processEvent();
  }

  return selection_text_;
}

const CXColor &
CXMachine::
getCXColor(const CRGB &rgb)
{
  return getCXScreen(0)->getCXColor(rgb);
}

const CXColor &
CXMachine::
getCXColor(const CRGBA &rgba)
{
  return getCXScreen(0)->getCXColor(rgba);
}

const CXColor &
CXMachine::
getCXColor(Pixel pixel)
{
  return getCXScreen(0)->getCXColor(pixel);
}

void
CXMachine::
beep()
{
  XBell(display_, 0);
}

bool
CXMachine::
inchToPixel(double in, double *pixel)
{
  return mmToPixel(in*25.4, pixel);
}

bool
CXMachine::
mmToPixel(double mm, double *pixel)
{
  int width  = getCXScreen(0)->getWidth();
  int height = getCXScreen(0)->getHeight();

  double mwidth, mheight;

  if (! getMonitorWidthMM(&mwidth))
    return false;

  if (! getMonitorHeightMM(&mheight))
    return false;

  double xpixel = (width *mm)/mwidth ;
  double ypixel = (height*mm)/mheight;

  *pixel = (xpixel + ypixel)/2.0;

  return true;
}

bool
CXMachine::
getMonitorWidthMM(double *width)
{
  CConfig config("monitor");

  if (! config.getValue("width", width))
    return false;

  std::string units;

  if (! config.getValue("width_units", units))
    units = "mm";

  if (units == "in" || units == "inch" || units == "inches")
    *width *= 25.4;

  return true;
}

bool
CXMachine::
getMonitorHeightMM(double *height)
{
  CConfig config("monitor");

  if (! config.getValue("height", height))
    return false;

  std::string units;

  if (! config.getValue("height_units", units))
    units = "mm";

  if (units == "in" || units == "inch" || units == "inches")
    *height *= 25.4;

  return true;
}

//------------------

bool
CXEventAdapter::
idleEvent()
{
  COSTimer::msleep(10);

  return true;
}

//------------------

CXFontList::
CXFontList(const char *pattern, uint max_fonts)
{
  fonts_ = XListFonts(CXMachineInst->getDisplay(), pattern, int(max_fonts), &num_fonts_);
}

CXFontList::
~CXFontList()
{
  if (num_fonts_ > 0)
    XFreeFontNames(fonts_);
}
