#include <CXFont.h>
#include <CXMachine.h>
#include <CXScreen.h>
#include <CXImage.h>
#include <CXrtFont.h>
#include <CFontMgr.h>

void
CXFont::
setPrototype()
{
  CFontPtr font(new CXFont("courier", CFONT_STYLE_NORMAL, 12));

  CFontMgrInst->setPrototype(font);
}

CXFont::
CXFont(const std::string &family, CFontStyle style, double size, double angle,
       double char_angle, int x_res, int y_res) :
 CFont(family, style, size, angle, char_angle, x_res, y_res),
 screen_(*CXMachineInst->getCXScreen(0)),
 font_family_(CFontFamily::lookup(family))
{
  init();
}

CXFont::
CXFont(CXScreen &screen, const std::string &family, CFontStyle style, double size, double angle,
       double char_angle, int x_res, int y_res) :
 CFont(family, style, size, angle, char_angle, x_res, y_res), screen_(screen),
 font_family_(CFontFamily::lookup(family))
{
  init();
}

CXFont::
CXFont(const std::string &full_name) :
 CFont(full_name), screen_(*CXMachineInst->getCXScreen(0)),
 font_family_(CFontFamily::lookup(getFamily()))
{
  init();
}

CXFont::
CXFont(CXScreen &screen, const std::string &full_name) :
 CFont(full_name), screen_(screen), font_family_(CFontFamily::lookup(getFamily()))
{
  init();
}

CXFont::
CXFont(XFontStruct *fs, uint angle) :
 CFont("", CFONT_STYLE_NORMAL, CFONT_DEF_SIZE, angle),
 screen_(*CXMachineInst->getCXScreen(0)), font_family_(CFontFamily::lookup(""))
{
  init(fs);
}

CXFont::
CXFont(CXScreen &screen, XFontStruct *fs, uint angle) :
 CFont("", CFONT_STYLE_NORMAL, CFONT_DEF_SIZE, angle),
 screen_(screen), font_family_(CFontFamily::lookup(""))
{
  init(fs);
}

CXFont::
CXFont(const CXFont &font) :
 CFont(font), screen_(font.screen_), font_family_(font.font_family_),
 font_width_(font.font_width_), font_ascent_(font.font_ascent_), font_descent_(font.font_descent_),
 proportional_(font.proportional_), font_aspect_(font.font_aspect_)
{
  xrt_font_ = new CXrtFont(*font.xrt_font_);
}

CXFont::
CXFont(const CFont &font) :
 CFont(font), screen_(*CXMachineInst->getCXScreen(0)),
 font_family_(CFontFamily::lookup(font.getFamily()))
{
  init();
}

CXFont::
~CXFont()
{
  delete xrt_font_;
}

CXFont &
CXFont::
operator=(const CXFont &font)
{
  screen_       = font.screen_;
  font_family_  = font.font_family_;
  font_width_   = font.font_width_;
  font_ascent_  = font.font_ascent_;
  font_descent_ = font.font_descent_;
  proportional_ = font.proportional_;
  font_aspect_  = font.font_aspect_;

  xrt_font_ = new CXrtFont(*font.xrt_font_);

  return *this;
}

CFontPtr
CXFont::
dup(const std::string &family, CFontStyle style, double size,
    double angle, double char_angle, int x_res, int y_res) const
{
  CXFont *font =
    new CXFont(family, style, size, angle, char_angle, x_res, y_res);

  return CFontPtr(font);
}

void
CXFont::
init()
{
  std::string x_font_name = getXFontName();

  Display *display = screen_.getDisplay();
  Window   root    = screen_.getRoot();

  xrt_font_ = new CXrtFont(display, root, x_font_name, getIAngle());

  int font_width, font_ascent, font_descent;

  xrt_font_->getExtents(&font_width, &font_ascent, &font_descent);

  font_width_   = uint(font_width);
  font_ascent_  = uint(font_ascent);
  font_descent_ = uint(font_descent);

  proportional_ = xrt_font_->isProportional();

  font_aspect_ = 1.2;
}

void
CXFont::
init(XFontStruct *fs)
{
  Display *display = screen_.getDisplay();
  Window   root    = screen_.getRoot();

  xrt_font_ = new CXrtFont(display, root, fs, getIAngle());

  int font_width, font_ascent, font_descent;

  xrt_font_->getExtents(&font_width, &font_ascent, &font_descent);

  font_width_   = uint(font_width);
  font_ascent_  = uint(font_ascent);
  font_descent_ = uint(font_descent);

  setSize(font_ascent_ + font_descent_);

  proportional_ = xrt_font_->isProportional();

  font_aspect_ = 1.2;
}

uint
CXFont::
getIStringWidth(const std::string &str) const
{
  int width;

  xrt_font_->textExtents(str, &width, nullptr, nullptr);

  return uint(width);
}

CImagePtr
CXFont::
getStringImage(const std::string &str)
{
  uint w = getIStringWidth(str);
  uint h = getICharHeight();

  std::string fileName = "app://getStringImage";

  CImageFileSrc src(fileName);

  image_ = CImageMgrInst->createImage(src);

  image_->setDataSize(int(w), int(h));

  image_->setRGBAData(CRGBA(0, 0, 0, 0));

  CXImage *ximage = image_.cast<CXImage>();

  Pixmap pixmap = ximage->getXPixmap();

  GC gc = CXMachineInst->createGC(pixmap, 0, 0);

  CXMachineInst->setForeground(gc, 0);

  xrt_font_->draw(pixmap, gc, 0, 0, str);

  CXMachineInst->freeGC(gc);

  return image_;
}

//-------------------

#define ALL_FONTS "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"

void
CXFont::
loadFontDatabase()
{
  CXFontList font_list;

  uint num_fonts = font_list.getNumFonts();

  std::string family;
  CFontStyle  style;
  uint        size, x_res, y_res;

  for (uint i = 0; i < num_fonts; i++) {
    const char *font_name = font_list.getFont(i);

    if (! decodeXFontName(font_name, family, style, size, x_res, y_res))
      continue;

    CFontFamily &font_family = CFontFamily::lookup(family);

    CFontDef &font = font_family.lookupFontDef(style, size);

    if (x_res > font.x_res || y_res > font.y_res) {
      font.x_res = x_res;
      font.y_res = y_res;
    }
  }
}
