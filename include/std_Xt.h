#ifndef std_Xt_H
#define std_Xt_H

#ifdef __STDC__
#  define NeedFunctionPrototypes 1
#endif

#ifdef __cplusplus
extern "C" {
#define String XString
#endif

/* The X11 public header files. */

#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

#if defined(__cplusplus) || defined(c_plusplus)
#  define GetVisualClass(v) ((v)->c_class)
#else
#  define GetVisualClass(v) ((v)->class)
#endif

#define IsVisualClass(v,c) (GetVisualClass(v) == (c))

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#ifdef MWM_UTIL
#include <MwmUtil.h>
#endif

#ifdef __cplusplus
#undef String

#ifdef index
# undef index
#endif
}
#endif

#endif
