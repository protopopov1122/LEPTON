#include <efi.h>
#include <efilib.h>
#include "console.h"
#include "util.h"
#include "core.h"
#include "memory.h"
#include "dictionary.h"

#define DECLARE_WORD(_identifier) \
  extern void lepton_word_##_identifier()

DECLARE_WORD(interpret);

DECLARE_WORD(colon);
DECLARE_WORD(semicolon);
DECLARE_WORD(left_bracket);
DECLARE_WORD(right_bracket);
DECLARE_WORD(literal);
DECLARE_WORD(immediate);
DECLARE_WORD(postpone);

DECLARE_WORD(io_print);

DECLARE_WORD(arith_add);
DECLARE_WORD(arith_sub);
DECLARE_WORD(arith_mul);
DECLARE_WORD(arith_div);

#undef DECLARE_WORD

static void initialize_core_dictionary() {
  lepton_console_printf(L"Initializing core dictionary\n");
#define INIT_WORD(_literal, _identifier, _data, _immediate) \
  do { \
    void (*fn)() = &lepton_word_##_identifier; \
    void **fnptr = (void **) &fn; \
    lepton_define_word((_literal), *fnptr, (_data), (_immediate)); \
  } while (0)

  INIT_WORD(L":", colon, 0, FALSE);
  INIT_WORD(L";", semicolon, 0, TRUE);
  INIT_WORD(L"[", left_bracket, 0, TRUE);
  INIT_WORD(L"]", right_bracket, 0, FALSE);
  INIT_WORD(L"LITERAL", literal, 0, TRUE);
  INIT_WORD(L"IMMEDIATE", immediate, 0, FALSE);
  INIT_WORD(L"POSTPONE", postpone, 0, TRUE);

  INIT_WORD(L".", io_print, 0, FALSE);

  INIT_WORD(L"+", arith_add, 0, FALSE);
  INIT_WORD(L"-", arith_sub, 0, FALSE);
  INIT_WORD(L"*", arith_mul, 0, FALSE);
  INIT_WORD(L"/", arith_div, 0, FALSE);

  INIT_WORD(L"INTERPRET", interpret, 0, FALSE);
#undef INIT_WORD
}

extern int lepton_entry(void *, void *);

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  lepton_core_init(ImageHandle, SystemTable);
  lepton_console_printf(L"Starting LEPTON\n");
  lepton_memory_initialize();
  lepton_dictionary_initialize((void *) LeptonMemory.dictionary, (void *) LeptonMemory.executable_area);
  initialize_core_dictionary();

  lepton_console_printf(L"Entering LEPTON\n");
  if (lepton_entry((void *) (LeptonMemory.code_stack + LEPTON_CODE_STACK_PAGES * EFI_PAGE_SIZE), (void *) (LeptonMemory.data_stack + LEPTON_DATA_STACK_PAGES + EFI_PAGE_SIZE))) {
    lepton_console_printf(L"Fatal: entry failed\n");
  }

  lepton_dictionary_free();
  lepton_memory_free();
  lepton_console_printf(L"Exiting LEPTON\n");
  lepton_core_free();
  return EFI_SUCCESS;
}