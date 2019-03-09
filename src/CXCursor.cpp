#include <CXCursor.h>
#include <CXMachine.h>
#include <CXScreen.h>

CXCursor::
CXCursor(CXScreen &screen, CCursorType type) :
 screen_(screen), type_(type)
{
  init();
}

CXCursor::
CXCursor(CCursorType type) :
 screen_(*CXMachineInst->getCXScreen(0)), type_(type)
{
  init();
}

CXCursor::
CXCursor(const CXCursor &cursor) :
 screen_(cursor.screen_), type_(cursor.type_)
{
  init();
}

CXCursor::
~CXCursor()
{
  XFreeCursor(screen_.getDisplay(), xcursor_);
}

void
CXCursor::
init()
{
  uint shape = lookupCursorShape(type_);

  xcursor_ = XCreateFontCursor(screen_.getDisplay(), shape);
}

void
CXCursor::
recolor(const CRGBA &bg, const CRGBA &fg)
{
  Display *display = screen_.getDisplay();

  XColor fg_xcolor;
  XColor bg_xcolor;

  fg_xcolor.red   = (int) (fg.getRed  ()*65535);
  fg_xcolor.green = (int) (fg.getGreen()*65535);
  fg_xcolor.blue  = (int) (fg.getBlue ()*65535);

  bg_xcolor.red   = (int) (bg.getRed  ()*65535);
  bg_xcolor.green = (int) (bg.getGreen()*65535);
  bg_xcolor.blue  = (int) (bg.getBlue ()*65535);

  XRecolorCursor(display, xcursor_, &fg_xcolor, &bg_xcolor);
}

CCursorType
CXCursor::
getType() const
{
  return type_;
}

Cursor
CXCursor::
getXCursor() const
{
  return xcursor_;
}

uint
CXCursor::
lookupCursorShape(CCursorType type)
{
  switch (type) {
    case CURSOR_TYPE_ARROW:
      return XC_arrow;
    case CURSOR_TYPE_TOP_LEFT_ARROW:
      return XC_top_left_arrow;
    case CURSOR_TYPE_QUESTION_ARROW:
      return XC_question_arrow;
    case CURSOR_TYPE_TOP_SIDE:
      return XC_top_side;
    case CURSOR_TYPE_BOTTOM_SIDE:
      return XC_bottom_side;
    case CURSOR_TYPE_LEFT_SIDE:
      return XC_left_side;
    case CURSOR_TYPE_RIGHT_SIDE:
      return XC_right_side;
    case CURSOR_TYPE_TOP_LEFT_CORNER:
      return XC_top_left_corner;
    case CURSOR_TYPE_TOP_RIGHT_CORNER:
      return XC_top_right_corner;
    case CURSOR_TYPE_BOTTOM_LEFT_CORNER:
      return XC_bottom_left_corner;
    case CURSOR_TYPE_BOTTOM_RIGHT_CORNER:
      return XC_bottom_right_corner;
    case CURSOR_TYPE_SB_LEFT_ARROW:
      return XC_sb_left_arrow;
    case CURSOR_TYPE_SB_H_DOUBLE_ARROW:
      return XC_sb_h_double_arrow;
    case CURSOR_TYPE_SB_V_DOUBLE_ARROW:
      return XC_sb_v_double_arrow;
    case CURSOR_TYPE_TOP_LEFT_ANGLE:
      return XC_ul_angle;
    case CURSOR_TYPE_FLEUR:
      return XC_fleur;
    case CURSOR_TYPE_WATCH:
      return XC_watch;
    case CURSOR_TYPE_CROSS_HAIR:
      return XC_crosshair;
    case CURSOR_TYPE_SKULL:
      return XC_pirate;
    case CURSOR_TYPE_CYCLE:
      return XC_box_spiral;
    case CURSOR_TYPE_SPRAY:
      return XC_spraycan;
    case CURSOR_TYPE_TEXT:
      return XC_xterm;
    default:
      return XC_top_left_arrow;
  }
}
