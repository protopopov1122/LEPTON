#include "helpers.h"
#include "dictionary.h"
#include "console.h"

struct lepton_input_parse_result lepton_helper_parse_input(const CHAR16 *input) {
  struct lepton_dictionary_entry *dict_entry = lepton_resolve_word(input);
  if (dict_entry != NULL) {
    return (struct lepton_input_parse_result){FALSE, (INT64) dict_entry};
  }

  INT64 integer = 0;
  for (UINTN i = 0; input[i] != L'\0'; i++) {
    if (!(input[i] >= L'0' && input[i] <= L'9')) {
      return (struct lepton_input_parse_result){FALSE, 0};
    }
    integer *= 10;
    integer += input[i] - L'0';
  }
  return (struct lepton_input_parse_result){TRUE, integer};
}

void lepton_debug_wait() {
  int wait = 1;
  while (wait) {
    __asm__ __volatile__ ("pause");
  }
}
