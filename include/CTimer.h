#ifndef CTIMER_H
#define CTIMER_H

#include <CHRTime.h>

#include <sys/time.h>

#include <vector>
#include <set>
#include <list>

typedef unsigned long ulong;

enum CTimerFlags {
  CTIMER_FLAGS_NONE      = 0,
  CTIMER_FLAGS_REPEAT    = (1<<0),
  CTIMER_FLAGS_NO_DELETE = (1<<1),
  CTIMER_FLAGS_NO_ACTIVE = (1<<2)
};

#define CTimerMgrInst CTimerMgr::getInstance()

class CTimer;

class CTimerMgr {
 public:
  static CTimerMgr *getInstance() {
    static CTimerMgr *instance;

    if (! instance)
      instance = new CTimerMgr;

    return instance;
  }

  void setExternalUpdate(bool flag) { external_update_ = flag; }

  void addTimer(CTimer *timer);

  void removeTimer(CTimer *timer);

  void tick();

 private:
  CTimerMgr();

  static void signalHandler(int sig);

  void updateTimers();

  void updateTimer(ulong msecs);
  void getTimer(ulong *msecs);
  void setTimer(ulong msecs);

 private:
  typedef std::list<CTimer *> TimerList;
  typedef std::set <CTimer *> TimerSet;

  bool             external_update_;
  bool             signal_handler_;
  TimerList        timers_;
  TimerSet         rtimers_;
  bool             itimer_active_;
  ulong            itimer_msecs_;
  bool             timer_updating_;
  struct itimerval itimer_;
  CHRTime          last_t_;
};

class CTimer {
 private:
  ulong       msecs_init_;
  ulong       msecs_left_;
  CTimerFlags flags_;

 protected:
  bool active_;

 public:
  CTimer(ulong msecs, CTimerFlags flags=CTIMER_FLAGS_NONE);

  virtual ~CTimer();

  ulong getMSecsInit() const { return msecs_init_; }
  ulong getMSecsLeft() const { return msecs_left_; }

  void setMSecsLeft(ulong msecs_left) { msecs_left_ = msecs_left; }

  void stop();

  void restart();

  void restart(ulong msecs);

  bool isActive() const { return active_; }

  void setActive(bool active) { active_ = active; }

  bool isRepeat() const { return (flags_ & CTIMER_FLAGS_REPEAT); }

  bool isDelete() const { return ! (flags_ & CTIMER_FLAGS_NO_DELETE); }

  virtual void timeOut() { }
};

#endif
