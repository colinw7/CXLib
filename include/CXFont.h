#ifndef CX_FONT_H
#define CX_FONT_H

#include <std_Xt.h>
#include <CFont.h>

class CXScreen;
class CXrtFont;
class CXFont;

class CXFont : public CFont {
 protected:
  friend class CXMachine;

  CXFont(const std::string &family, CFontStyle style, double size,
         double angle=0, double char_angle=0, int x_res=100, int y_res=100);

  CXFont(CXScreen &screen, const std::string &family, CFontStyle style,
         double size, double angle=0, double char_angle=0,
         int x_res=100, int y_res=100);

  explicit CXFont(const std::string &full_name);

  CXFont(CXScreen &screen, const std::string &full_name);

  CXFont(XFontStruct *fs, uint angle=0);

  CXFont(CXScreen &screen, XFontStruct *fs, uint angle=0);

  virtual ~CXFont();

  CFontPtr dup(const std::string &family, CFontStyle style, double size,
               double angle=0, double char_angle=0,
               int x_res=100, int y_res=100) const;

 private:
  CXFont(const CFont &font);
  CXFont(const CXFont &font);

  const CXFont &operator=(const CFont &font);
  const CXFont &operator=(const CXFont &font);

 public:
  double getCharWidth   () const { return getICharWidth(); }
  uint   getICharWidth  () const { return font_width_; }
  double getCharAscent  () const { return getICharAscent(); }
  uint   getICharAscent () const { return font_ascent_; }
  double getCharDescent () const { return getICharDescent(); }
  uint   getICharDescent() const { return font_descent_; }
  double getCharHeight  () const { return getICharHeight(); }
  uint   getICharHeight () const { return font_ascent_ + font_descent_; }

  double getStringWidth (const std::string &str) const {
    return getIStringWidth(str);
  }
  uint   getIStringWidth(const std::string &str) const;

  bool isProportional() const { return proportional_; }

  double getCharAspect() const { return font_aspect_; }

  CXrtFont *getXrtFont() const { return xrt_font_; }

  CImagePtr getStringImage(const std::string &str);

  static void setPrototype();

  static void loadFontDatabase();

 private:
  void init();
  void init(XFontStruct *fs);

 private:
  friend class CXFontMgr;

  CXScreen    &screen_;
  CFontFamily &font_family_;
  CXrtFont    *xrt_font_ { nullptr };
  uint         font_width_ { 0 };
  uint         font_ascent_ { 0 };
  uint         font_descent_ { 0 };
  bool         proportional_ { false };
  double       font_aspect_ { 1.0 };
  CImagePtr    image_;
};

#endif
