#include <CXLibI.h>

CXNamedEvent::
CXNamedEvent(const std::string &name) :
 name_(name)
{
  parse();
}

CXNamedEvent::
~CXNamedEvent()
{
  delete event_;
}

bool
CXNamedEvent::
parse()
{
  bool flag = false;

  //----------

  // Parse String

  std::string modifiers = "";
  std::string object    = "";
  std::string data      = "";

  uint state = 0;

  std::string::size_type pos1 = name_. find("<");
  std::string::size_type pos2 = name_.rfind(">");

  if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 >= pos2)
    goto done;

  modifiers = name_.substr(0, pos1);

  object = name_.substr(pos1 + 1, pos2 - pos1 - 1);

  data = name_.substr(pos2 + 1);

  if (modifiers != "") {
    std::vector<std::string> words;

    CStrUtil::addFields(modifiers, words, "+");

    uint j = 0;

    while (j < words.size()) {
      if      (CStrUtil::casecmp(words[j], "Shift") == 0)
        state |= ShiftMask;
      else if (CStrUtil::casecmp(words[j], "Ctrl" ) == 0)
        state |= ControlMask;
      else if (CStrUtil::casecmp(words[j], "Alt"  ) == 0)
        state |= Mod1Mask;
      else
        break;

      j++;
    }

    if (j < words.size())
      goto done;
  }

  //----------

  // Create Event

  if     (CStrUtil::casecmp(object, "Button1") == 0) {
    XButtonPressedEvent *event1 = new XButtonPressedEvent;

    event1->type   = ButtonPress;
    event1->button = 1;
    event1->state  = state;

    event_ = (XEvent *) event1;
  }
  else if (CStrUtil::casecmp(object, "Button2") == 0) {
    XButtonPressedEvent *event1 = new XButtonPressedEvent;

    event1->type   = ButtonPress;
    event1->button = 2;
    event1->state  = state;

    event_ = (XEvent *) event1;
  }
  else if (CStrUtil::casecmp(object, "Button3") == 0) {
    XButtonPressedEvent *event1 = new XButtonPressedEvent;

    event1->type   = ButtonPress;
    event1->button = 3;
    event1->state  = state;

    event_ = (XEvent *) event1;
  }
  else if (CStrUtil::casecmp(object, "Key"     ) == 0 ||
           CStrUtil::casecmp(object, "KeyPress") == 0) {
    KeySym keysym = XStringToKeysym(data.c_str());

    if (keysym == NoSymbol) {
      std::cerr << "Unknowm KeySym Name " << data << "\n";
      goto done;
    }

    uint keycode = CXMachineInst->keysymToKeycode(keysym);

    XKeyPressedEvent *event1 = new XKeyPressedEvent;

    event1->type    = KeyPress;
    event1->keycode = keycode;
    event1->state   = state;

    event_ = (XEvent *) event1;
  }
  else if (CStrUtil::casecmp(object, "KeyRelease") == 0) {
    KeySym keysym = XStringToKeysym(data.c_str());

    if (keysym == NoSymbol) {
      std::cerr << "Unknowm KeySym Name " << data << "\n";
      goto done;
    }

    uint keycode = CXMachineInst->keysymToKeycode(keysym);

    XKeyReleasedEvent *event1 = new XKeyReleasedEvent;

    event1->type    = KeyRelease;
    event1->keycode = keycode;
    event1->state   = state;

    event_ = (XEvent *) event1;
  }
  else
    goto done;

  //----------

  // Create User String

  if (CStrUtil::casecmp(object, "Key"       ) == 0 ||
      CStrUtil::casecmp(object, "KeyPress"  ) == 0 ||
      CStrUtil::casecmp(object, "KeyRelease") == 0) {
    if (state & ControlMask)
      text_ += "Ctrl";

    if (state & Mod1Mask) {
      if (text_ != "")
        text_ += " Alt";
      else
        text_ += "Alt";
    }

    if (state & ShiftMask) {
      if (text_ != "")
        text_ += " Shift";
      else
        text_ += "Shift";
    }

    if (text_ != "")
      text_ += "+";

    text_ += data;
  }

  //----------

  flag = true;

 done:
  if (! event_)
    std::cerr << "Invalid Event Name '" << name_ << "'\n";

  return flag;
}

bool
CXNamedEvent::
matchEvent(CXNamedEvent *event)
{
  if (! event->event_)
    return false;

  return matchEvent(event->event_);
}

bool
CXNamedEvent::
matchEvent(XEvent *event)
{
  if (! event_)
    return false;

  return CXMachineInst->compareEvents(event_, event);
}

XKeyPressedEvent *
CXNamedEvent::
getKeyPressedEvent()
{
  if (! event_)
    return nullptr;

  if (event_->type != KeyPress)
    return nullptr;

  return &event_->xkey;
}
