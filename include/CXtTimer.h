#ifndef CXT_TIMER_H
#define CXT_TIMER_H

#include <CTimer.h>

class CXtTimer1;

class CXtTimer {
 private:
  CXtTimer1 *timer_;

 public:
  CXtTimer(uint msecs, CTimerFlags flags);

  virtual ~CXtTimer();

  virtual void timeOut() = 0;

  static void setAppContext(XtAppContext app_context);
};

#endif