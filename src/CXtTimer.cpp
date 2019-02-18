#include <CXLibI.h>

class CXtTimer1 {
 private:
  CXtTimer     *timer_;
  uint          msecs_;
  CTimerFlags   flags_;
  XtIntervalId  timer_id_;

 public:
  CXtTimer1(CXtTimer *timer, uint msecs, CTimerFlags flags);
 ~CXtTimer1();

  static void setAppContext(XtAppContext app_context);

 private:
  static XtAppContext           app_context_;
  static std::list<CXtTimer1 *> timers_;

  static void calltimeOut(void *data, XtIntervalId *id);
};

XtAppContext           CXtTimer1::app_context_ = 0;
std::list<CXtTimer1 *> CXtTimer1::timers_;

CXtTimer::
CXtTimer(uint msecs, CTimerFlags flags)
{
  timer_ = new CXtTimer1(this, msecs, flags);
}

CXtTimer::
~CXtTimer()
{
  delete timer_;
}

void
CXtTimer::
setAppContext(XtAppContext app_context)
{
  CXtTimer1::setAppContext(app_context);
}

CXtTimer1::
CXtTimer1(CXtTimer *timer, uint msecs, CTimerFlags flags) :
 timer_(timer), msecs_(msecs), flags_(flags)
{
  timer_id_ =
    XtAppAddTimeOut(app_context_, msecs_,
                    (XtTimerCallbackProc) &CXtTimer1::calltimeOut,
                    (char *) this);

  timers_.push_back(this);
}

CXtTimer1::
~CXtTimer1()
{
  if (timer_id_ != 0)
    XtRemoveTimeOut(timer_id_);

  timers_.remove(this);
}

void
CXtTimer1::
calltimeOut(void *data, XtIntervalId *)
{
  CXtTimer1 *timer = (CXtTimer1 *) data;

  timer->timer_->timeOut();

  if (timer->flags_ & CTIMER_FLAGS_REPEAT)
    timer->timer_id_ =
      XtAppAddTimeOut(app_context_, timer->msecs_,
                      (XtTimerCallbackProc) &CXtTimer1::calltimeOut,
                      (char *) timer);
  else
    timer->timer_id_ = 0;
}

void
CXtTimer1::
setAppContext(XtAppContext app_context)
{
  app_context_ = app_context;
}
