#ifndef LEPTON_DICTIONARY_H_
#define LEPTON_DICTIONARY_H_

#include <efi.h>
#include "core.h"

struct lepton_dictionary_entry {
  CHAR16 name[LEPTON_DICTIONARY_WORD_MAXLEN + 1];
  struct lepton_dictionary_entry *link;
  const void *code;
  UINT64 data;
  BOOLEAN immediate;
};

void lepton_dictionary_initialize(void *, void *);
void lepton_dictionary_free();
void lepton_define_word(const CHAR16 *, const void *, UINT64, BOOLEAN);
struct lepton_dictionary_entry *lepton_resolve_word(const CHAR16 *);
struct lepton_dictionary_entry *lepton_top_word();
void *lepton_executable_area_current();
void lepton_executable_area_append_call(UINT64, UINT64);
void lepton_executable_area_append_push(UINT64);
void lepton_executable_area_patch_push(UINT8 *, UINT64);
void lepton_executable_area_append_prologue();
void lepton_executable_area_append_ret();
void lepton_executable_area_append_jmp(UINT64);

#endif
