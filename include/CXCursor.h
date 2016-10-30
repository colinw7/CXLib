#ifndef CX_CURSOR_H
#define CX_CURSOR_H

class CXScreen;

#include <CRGBA.h>
#include <CCursor.h>

class CXCursor {
 public:
  CXCursor(CXScreen &screen, CCursorType type);
  CXCursor(CCursorType type);
  CXCursor(const CXCursor &cursor);
 ~CXCursor();

  void recolor(const CRGBA &bg, const CRGBA &fg);

  CCursorType getType() const;

  Cursor getXCursor() const;

 private:
  void init();

  uint lookupCursorShape(CCursorType type);

 private:
  CXScreen    &screen_;
  CCursorType  type_;
  Cursor       xcursor_;
};

#endif
