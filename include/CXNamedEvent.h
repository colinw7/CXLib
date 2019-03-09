#ifndef CX_NAMED_EVENT_H
#define CX_NAMED_EVENT_H

#include <std_Xt.h>
#include <string>

class CXNamedEvent {
 public:
  CXNamedEvent(const std::string &name);
 ~CXNamedEvent();

  const std::string &getName () const { return name_ ; }
  XEvent            *getEvent() const { return event_; }
  const std::string &getText () const { return text_ ; }

  bool matchEvent(CXNamedEvent *event);
  bool matchEvent(XEvent *event);

  XKeyPressedEvent *getKeyPressedEvent();

 private:
  bool parse();

 private:
  std::string  name_;
  XEvent      *event_ { nullptr };
  std::string  text_;
};

#endif
