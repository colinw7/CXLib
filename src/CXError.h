struct CwmXErrorRoutine {
  int   code;
  char *routine;
};

struct CwmXErrorMessage {
  int   code;
  char *message;
};

static CwmXErrorRoutine
x_error_routines[] = {
  {   2, "XChangeWindowAttributes", },
  {   4, "XDestroyWindow"         , },
  {   8, "XMapWindow"             , },
  {  10, "XUnmapWindow"           , },
  {  12, "XConfigureWindow"       , },
  {  18, "XChangeProperty"        , },
  {  20, "XGetProperty"           , },
  {  66, "XDrawLine"              , },
  {  73, "XGetImage"              , },
  {  88, "XFreeColors"            , },
  { 113, "XKillClient"            , },
  {  -1, ""                       , },
};

static CwmXErrorMessage
x_error_messages[] = {
  { 0                , ""                                   , },
  { BadRequest       , "Bad Request Code"                   , },
  { BadValue         , "Integer Parameter Out of Range"     , },
  { BadWindow        , "Parameter not a Window"             , },
  { BadPixmap        , "Parameter not a Pixmap"             , },
  { BadAtom          , "Parameter not an Atom"              , },
  { BadCursor        , "Parameter not a Cursor"             , },
  { BadFont          , "Parameter not a Font"               , },
  { BadMatch         , "Parameter Mismatch"                 , },
  { BadDrawable      , "Parameter not a Pixmap or Window"   , },
  { BadAccess        , "Invalid Access"                     , },
  { BadAlloc         , "Insufficient Resources"             , },
  { BadColor         , "No such Colormap"                   , },
  { BadGC            , "Parameter not a GC"                 , },
  { BadIDChoice      , "Choice not in Range or Already Used", },
  { BadName          , "Font or Color Name doesn't Exist"   , },
  { BadLength        , "Request Length Incorrect"           , },
  { BadImplementation, "Server is Defective"                , },
  { -1               , ""                                   , },
};
