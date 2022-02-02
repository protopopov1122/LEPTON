#include "util.h"

BOOLEAN lepton_util_isnewline(CHAR16 chr) {
  switch (chr) {
    case L'\r':
    case L'\n':
      return TRUE;
    
    default:
      return FALSE;
  }
}

BOOLEAN lepton_util_isspace(CHAR16 chr) {
  switch (chr) {
    case L' ':
    case L'\t':
    case L'\v':
    case L'\r':
    case L'\n':
      return TRUE;
    
    default:
      return FALSE;
  }
}
