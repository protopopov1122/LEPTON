#ifndef LEPTON_HELPERS_H_
#define LEPTON_HELPERS_H_

#include <efi.h>

struct lepton_input_parse_result {
    UINT64 is_integer;
    INT64 result;
};

struct lepton_input_parse_result lepton_helper_parse_input(const CHAR16 *);
void lepton_debug_wait();

#endif
