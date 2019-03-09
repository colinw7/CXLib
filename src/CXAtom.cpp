#include <CXAtom.h>
#include <CXMachine.h>
#include <CThrow.h>

CXAtomMgr::
CXAtomMgr() :
 machine_(*CXMachineInst)
{
}

CXAtomMgr::
CXAtomMgr(CXMachine &machine) :
 machine_(machine)
{
}

bool
CXAtomMgr::
isWMClass(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_CLASS);
}

bool
CXAtomMgr::
isWMClientMachine(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_CLIENT_MACHINE);
}

bool
CXAtomMgr::
isWMCommand(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_COMMAND);
}

bool
CXAtomMgr::
isWMHints(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_HINTS);
}

bool
CXAtomMgr::
isWMIconName(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_ICON_NAME);
}

bool
CXAtomMgr::
isWMName(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_NAME);
}

bool
CXAtomMgr::
isWMNormalHints(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_NORMAL_HINTS);
}

bool
CXAtomMgr::
isWMSizeHints(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_SIZE_HINTS);
}

bool
CXAtomMgr::
isWMTransientFor(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_TRANSIENT_FOR);
}

bool
CXAtomMgr::
isWMZoomHints(const CXAtom &atom)
{
  return atom.isXAtom(XA_WM_ZOOM_HINTS);
}

bool
CXAtomMgr::
isWMProtocols(const CXAtom &atom)
{
  return atom == getWMProtocols();
}

bool
CXAtomMgr::
isWMTakeFocus(const CXAtom &atom)
{
  return atom == getWMTakeFocus();
}

bool
CXAtomMgr::
isWMSaveYourself(const CXAtom &atom)
{
  return atom == getWMSaveYourself();
}

bool
CXAtomMgr::
isWMDeleteWindow(const CXAtom &atom)
{
  return atom == getWMDeleteWindow();
}

bool
CXAtomMgr::
isWMState(const CXAtom &atom)
{
  return atom == getWMState();
}

bool
CXAtomMgr::
isWMChangeState(const CXAtom &atom)
{
  return atom == getWMChangeState();
}

bool
CXAtomMgr::
isMwmHints(const CXAtom &atom)
{
  return atom == getMwmHints();
}

bool
CXAtomMgr::
isCwmDesktop(const CXAtom &atom)
{
  return atom == getCwmDesktop();
}

const CXAtom &
CXAtomMgr::
getWMProtocols()
{
  if (! XA_WM_PROTOCOLS)
    XA_WM_PROTOCOLS = &getCXAtom("WM_PROTOCOLS");

  return *XA_WM_PROTOCOLS;
}

const CXAtom &
CXAtomMgr::
getWMTakeFocus()
{
  if (! XA_WM_TAKE_FOCUS)
    XA_WM_TAKE_FOCUS = &getCXAtom("WM_TAKE_FOCUS");

  return *XA_WM_TAKE_FOCUS;
}

const CXAtom &
CXAtomMgr::
getWMSaveYourself()
{
  if (! XA_WM_SAVE_YOURSELF)
    XA_WM_SAVE_YOURSELF = &getCXAtom("WM_SAVE_YOURSELF");

  return *XA_WM_SAVE_YOURSELF;
}

const CXAtom &
CXAtomMgr::
getWMDeleteWindow()
{
  if (! XA_WM_DELETE_WINDOW)
    XA_WM_DELETE_WINDOW = &getCXAtom("WM_DELETE_WINDOW");

  return *XA_WM_DELETE_WINDOW;
}

const CXAtom &
CXAtomMgr::
getWMState()
{
  if (! XA_WM_STATE)
    XA_WM_STATE = &getCXAtom("WM_STATE");

  return *XA_WM_STATE;
}

const CXAtom &
CXAtomMgr::
getWMChangeState()
{
  if (! XA_WM_CHANGE_STATE)
    XA_WM_CHANGE_STATE = &getCXAtom("WM_CHANGE_STATE");

  return *XA_WM_CHANGE_STATE;
}

const CXAtom &
CXAtomMgr::
getMwmHints()
{
  if (! XA_MWM_HINTS)
    XA_MWM_HINTS = &getCXAtom("_MOTIF_WM_HINTS");

  return *XA_MWM_HINTS;
}

const CXAtom &
CXAtomMgr::
getXSetRootId()
{
  if (! XA_XSETROOT_ID)
    XA_XSETROOT_ID = &getCXAtom("_XSETROOT_ID");

  return *XA_XSETROOT_ID;
}

const CXAtom &
CXAtomMgr::
getCwmDesktop()
{
  if (! XA_CWM_DESKTOP)
    XA_CWM_DESKTOP = &getCXAtom("CWM_DESKTOP");

  return *XA_CWM_DESKTOP;
}

const CXAtom &
CXAtomMgr::
getCXAtom(const std::string &name)
{
  CXAtomMap::const_iterator patom1 = atom_map_.find(name);
  CXAtomMap::const_iterator patom2 = atom_map_.end();

  if (patom1 != patom2)
    return *((*patom1).second);

  Display *display = machine_.getDisplay();

  if (! display) {
    static CXAtom atom("", 0);
    CTHROW("No display");
    return atom;
  }

  Atom xatom = XInternAtom(display, name.c_str(), False);

  CXAtom *atom = new CXAtom(name, xatom);

  atom_map_[name] = atom;

  return *atom;
}

const CXAtom &
CXAtomMgr::
getCXAtom(Atom atom)
{
  Display *display = machine_.getDisplay();

  char *name = XGetAtomName(display, atom);

  return getCXAtom(name);
}

//---

CXAtom::
CXAtom(const std::string &name, Atom xatom) :
 name_(name), xatom_(xatom)
{
}
