#include <CXLibI.h>

class CXTimer1 : public CTimer {
 private:
  CXTimer *timer_;

 public:
  CXTimer1(CXTimer *timer, uint msecs, CTimerFlags flags);
 ~CXTimer1();

 private:
  static list<CXTimer1 *> timers_;

  void timeOut();
};

list<CXTimer1 *> CXTimer1::timers_;

CXTimer::
CXTimer(uint msecs, CTimerFlags flags)
{
  timer_ = new CXTimer1(this, msecs, flags);
}

CXTimer::
~CXTimer()
{
  delete timer_;
}

void
CXTimer::
restart()
{
  timer_->restart();
}

CXTimer1::
CXTimer1(CXTimer *timer, uint msecs, CTimerFlags flags) :
 CTimer(msecs, flags), timer_(timer)
{
  timers_.push_back(this);
}

CXTimer1::
~CXTimer1()
{
  timer_ = NULL;

  timers_.remove(this);
}

void
CXTimer1::
timeOut()
{
  if (timer_)
    timer_->timeOut();
}
