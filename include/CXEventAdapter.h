#ifndef CX_EVENT_ADAPTER_H
#define CX_EVENT_ADAPTER_H

#include <CXMachine.h>
#include <CEvent.h>

class CXEventAdapter : public CEventAdapter {
 private:
  uint event_mask_;

 public:
  CXEventAdapter(uint event_mask=0) :
   event_mask_(event_mask) {
  }

  virtual ~CXEventAdapter() { }

  virtual bool buttonPressEvent  (const CMouseEvent &) { return false; }
  virtual bool buttonMotionEvent (const CMouseEvent &) { return false; }
  virtual bool buttonReleaseEvent(const CMouseEvent &) { return false; }

  virtual bool keyPressEvent  (const CKeyEvent &) { return false; }
  virtual bool keyReleaseEvent(const CKeyEvent &) { return false; }

  virtual bool pointerMotionEvent(const CMouseEvent &) { return false; }

  virtual bool exposeEvent() { return false; }
  virtual bool resizeEvent() { return false; }

  virtual bool enterEvent() { return false; }
  virtual bool leaveEvent() { return false; }

  virtual bool visibilityEvent(bool) { return false; }

  virtual bool idleEvent();
  virtual bool tickEvent() { return false; }

  virtual bool selectionClearEvent() { return false; }

  virtual bool closeEvent() { return false; }

  virtual bool clientMessageEvent(void *, const char *) { return false; }

  //------

  void updateKeyPressState();
  void updateKeyReleaseState();

  CXWindow *getXWindow() const { return CXMachineInst->getEventXWindow(); }

  int getEventX() const { return CXMachineInst->getEventX(); }
  int getEventY() const { return CXMachineInst->getEventY(); }

  int getEventRootX() const { return CXMachineInst->getEventRootX(); }
  int getEventRootY() const { return CXMachineInst->getEventRootY(); }

  int getEventButtonNum() const { return CXMachineInst->getEventButtonNum(); }
};

#endif
