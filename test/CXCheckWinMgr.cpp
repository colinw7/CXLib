#include <std_c++.h>
#include <CXLib/CXLib.h>

int
main(int, char **)
{
  if (CXMachineInst->getCXScreen(0)->selectWMInput())
    return 0;

  return 1;
}
