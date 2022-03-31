#include <CXLib.h>
#include <COSTimer.h>

int
main(int, char **)
{
  auto *xm = CXMachineInst;

  auto root = xm->getRoot();

  std::vector<Window> windows;

#if 0
  Window *children;
  int     num_children;

  if (! xm->getWindowChildren(root, &children, &num_children))
    exit(1);

  for (size_t i = 0; i < size_t(num_children); ++i)
    windows.push_back(children[i]);
#else
  const auto &clientListAtom = xm->getAtom("_NET_CLIENT_LIST_STACKING");

  xm->getWindowArrayProperty(root, clientListAtom, windows);
#endif

  //auto atom = xm->getAtom(XA_WM_COMMAND);
  auto atom = xm->getAtom(XA_WM_CLASS);

  std::vector<Window> rwins;

//std::string matchName = "thunderbird";
//std::string matchName = "firefox";
  std::string matchName = "Navigator";

  auto raiseWindow = [&](Window win) {
    //xm->raiseWindow(win);
    //xm->flushEvents(/*sync*/true);

    xm->sendActivate(win);
    xm->flushEvents(/*sync*/true);

    //xm->sendActivate(win);
    //xm->flushEvents(/*sync*/true);
  };

  for (const auto &window : windows) {
    std::string value;

    if (! xm->getStringProperty(window, atom, value))
      continue;

    std::cerr << window << " (" << value << ")\n";

    if (value == matchName)
      rwins.push_back(window);
  }

  for (const auto &rwin : rwins) {
    std::cerr << "Raise: " << rwin << "\n";

    raiseWindow(rwin);
  }

  return 0;
}
