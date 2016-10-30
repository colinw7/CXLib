#ifndef CX_ATOM_H
#define CX_ATOM_H

#include <map>

class CXMachine;
class CXAtom;

class CXAtomMgr {
 public:
  CXAtomMgr();
  CXAtomMgr(CXMachine &machine);

  bool isWMClass(const CXAtom &atom);
  bool isWMClientMachine(const CXAtom &atom);
  bool isWMCommand(const CXAtom &atom);
  bool isWMHints(const CXAtom &atom);
  bool isWMIconName(const CXAtom &atom);
  bool isWMName(const CXAtom &atom);
  bool isWMNormalHints(const CXAtom &atom);
  bool isWMSizeHints(const CXAtom &atom);
  bool isWMTransientFor(const CXAtom &atom);
  bool isWMZoomHints(const CXAtom &atom);
  bool isWMProtocols(const CXAtom &atom);
  bool isWMTakeFocus(const CXAtom &atom);
  bool isWMSaveYourself(const CXAtom &atom);
  bool isWMDeleteWindow(const CXAtom &atom);
  bool isWMState(const CXAtom &atom);
  bool isWMChangeState(const CXAtom &atom);
  bool isMwmHints(const CXAtom &atom);
  bool isCwmDesktop(const CXAtom &atom);

  const CXAtom &getWMProtocols();
  const CXAtom &getWMTakeFocus();
  const CXAtom &getWMSaveYourself();
  const CXAtom &getWMDeleteWindow();
  const CXAtom &getWMState();
  const CXAtom &getWMChangeState();
  const CXAtom &getMwmHints();
  const CXAtom &getXSetRootId();
  const CXAtom &getCwmDesktop();

  const CXAtom &getCXAtom(const std::string &name);
  const CXAtom &getCXAtom(Atom atom);

 private:
  const CXAtom *XA_WM_PROTOCOLS { 0 };
  const CXAtom *XA_WM_TAKE_FOCUS { 0 };
  const CXAtom *XA_WM_SAVE_YOURSELF { 0 };
  const CXAtom *XA_WM_DELETE_WINDOW { 0 };
  const CXAtom *XA_WM_STATE { 0 };
  const CXAtom *XA_WM_CHANGE_STATE { 0 };
  const CXAtom *XA_MWM_HINTS { 0 };
  const CXAtom *XA_XSETROOT_ID { 0 };
  const CXAtom *XA_CWM_DESKTOP { 0 };

  typedef std::map<std::string, CXAtom *> CXAtomMap;

  CXMachine &machine_;
  CXAtomMap  atom_map_;
};

//------

class CXAtom {
 public:
  CXAtom(const std::string &name="", Atom xatom=0);

  std::string getName () const { return name_ ; }

  Atom getXAtom() const { return xatom_; }

  bool isXAtom(Atom xatom) const { return xatom == xatom_; }

  bool operator==(const CXAtom &atom) const { return xatom_ == atom.xatom_; }

 private:
  std::string name_;
  Atom        xatom_ { 0 };
};

#endif
