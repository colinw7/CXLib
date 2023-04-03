#ifndef CX_EVENT_ADAPTER_H
#define CX_EVENT_ADAPTER_H

#include <CXMachine.h>
#include <CEvent.h>

class CXEventAdapter : public CEventAdapter {
 public:
  CXEventAdapter(uint event_mask=0) :
   event_mask_(event_mask) {
  }

  virtual ~CXEventAdapter() { }

  bool buttonPressEvent  (const CMouseEvent &) override { return false; }
  bool buttonMotionEvent (const CMouseEvent &) override { return false; }
  bool buttonReleaseEvent(const CMouseEvent &) override { return false; }

  bool keyPressEvent  (const CKeyEvent &) override { return false; }
  bool keyReleaseEvent(const CKeyEvent &) override { return false; }

  bool pointerMotionEvent(const CMouseEvent &) override { return false; }

  bool exposeEvent() override { return false; }
  bool resizeEvent() override { return false; }

  bool enterEvent() override { return false; }
  bool leaveEvent() override { return false; }

  bool visibilityEvent(bool) override { return false; }

  bool idleEvent() override;
  virtual bool tickEvent() { return false; }

  virtual bool selectionClearEvent() { return false; }

  bool closeEvent() override { return false; }

  bool clientMessageEvent(void *, const char *) override { return false; }

  //------

  void updateKeyPressState();
  void updateKeyReleaseState();

  CXWindow *getXWindow() const { return CXMachineInst->getEventXWindow(); }

  int getEventX() const { return CXMachineInst->getEventX(); }
  int getEventY() const { return CXMachineInst->getEventY(); }

  int getEventRootX() const { return CXMachineInst->getEventRootX(); }
  int getEventRootY() const { return CXMachineInst->getEventRootY(); }

  int getEventButtonNum() const { return CXMachineInst->getEventButtonNum(); }

 private:
  uint event_mask_ { 0 };
};

#endif
