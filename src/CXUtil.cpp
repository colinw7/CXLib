#include <CXUtil.h>
#include <CStrUtil.h>
#include <cstring>

void
CXUtil::
decodeVisualMask(uint full_mask, int *shift, uint *mask)
{
  *shift = 0;

  while (! (full_mask & 0x0001)) {
    full_mask >>= 1;

    (*shift)++;
  }

  *mask = full_mask;
}

void
CXUtil::
decodeDisplayName(const std::string &display_name, std::string &hostname,
                  int *display_num, int *screen_num)
{
  char hostname1[256];

  /* Check for "<name>:<display>.<screen>" */

  int num = sscanf(const_cast<char *>(display_name.c_str()), "%[a-zA-Z0-9_]:%d.%d",
                   hostname1, display_num, screen_num);

  if (num == 3) {
    hostname = hostname1;
    return;
  }

  /*------*/

  /* Check for "<name>:<display>" */

  num = sscanf(const_cast<char *>(display_name.c_str()), "%[a-zA-Z0-9_]:%d",
               hostname1, display_num);

  if (num == 2) {
    *screen_num = 0;

    hostname = hostname1;

    return;
  }

  /*------*/

  /* Check for ":<display>.<screen>" */

  num = sscanf(const_cast<char *>(display_name.c_str()), ":%d.%d",
               display_num, screen_num);

  if (num == 2) {
    strcpy(hostname1, "");

    hostname = hostname1;

    return;
  }

  /*------*/

  /* Check for ":<display> */

  num = sscanf(const_cast<char *>(display_name.c_str()), ":%d", display_num);

  if (num == 1) {
    strcpy(hostname1, "");

    *screen_num = 0;

    hostname = hostname1;

    return;
  }

  /*------*/

  /* Invalid display name format so fail */

  hostname = "";

  *screen_num  = 0;
  *display_num = 0;
}

std::string
CXUtil::
encodeXFontName(const std::string &name, CFontStyle style, int size)
{
  std::string style_str;
  std::string slant_str;

  if (style & CFONT_STYLE_BOLD)
    style_str = "bold";
  else
    style_str = "medium";

  if (style & CFONT_STYLE_ITALIC) {
    if (CStrUtil::casecmp(name, "helvetica") == 0)
      slant_str = "o";
    else
      slant_str = "i";
  }
  else
    slant_str = "r";

  std::string size_str = CStrUtil::toString(size);

  std::string font_name = "-*-" + name + "-" + style_str + "-" + slant_str +
                          "-normal-*-" + size_str + "-*-*-*-*-*-*-*";

  return font_name;
}
