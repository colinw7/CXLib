#ifndef CX_UTIL_H
#define CX_UTIL_H

#include <CFontStyle.h>

class CXUtil {
 public:
  static void decodeVisualMask(uint full_mask, int *shift, uint *mask);
  static void decodeDisplayName(const std::string &display_name,
                                std::string &hostname, int *display_num,
                                int *screen_num);

  static std::string encodeXFontName(const std::string &name, CFontStyle style, int size);
};

#endif
