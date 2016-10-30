#ifndef CX_TIMER_H
#define CX_TIMER_H

#include <CTimer.h>

class CXTimer1;

class CXTimer {
 public:
  CXTimer(uint msecs, CTimerFlags flags);

  virtual ~CXTimer();

  virtual void timeOut() { }

  void restart();

 private:
  CXTimer1 *timer_ { nullptr };
};

#endif
