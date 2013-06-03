#include <CStrUtil.h>
#include <CFontMgr.h>
#include <CXLib.h>

using std::string;
using std::vector;
using std::map;
using std::list;
using std::cout;
using std::cerr;
using std::endl;

#define CTHROW(m) \
{ std::cerr << m << std::endl; assert(false); }
