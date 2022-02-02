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
DECLARE_WORD(create);
DECLARE_WORD(does);
DECLARE_WORD(tick);
DECLARE_WORD(comma);
DECLARE_WORD(branch);
DECLARE_WORD(branch0);
DECLARE_WORD(here);
DECLARE_WORD(make_forward_reference);
DECLARE_WORD(resolve_forward_reference);
DECLARE_WORD(bump);
DECLARE_WORD(bump_mark);
DECLARE_WORD(bump_set);

DECLARE_WORD(io_print);

DECLARE_WORD(arith_add);
DECLARE_WORD(arith_sub);
DECLARE_WORD(arith_mul);
DECLARE_WORD(arith_div);

#undef DECLARE_WORD

static EFI_HANDLE EFI_ImageHandle;

static void lepton_word_abort() {
  lepton_console_printf(L"Aborting LEPTON\n");
  uefi_call_wrapper(ST->BootServices->Exit, 4, EFI_ImageHandle, EFI_ABORTED, 0, NULL);
}

static void lepton_word_exit() {
  lepton_dictionary_free();
  lepton_memory_free();
  lepton_console_printf(L"Exiting LEPTON\n");
  lepton_core_free();
  uefi_call_wrapper(ST->BootServices->Exit, 4, EFI_ImageHandle, EFI_SUCCESS, 0, NULL);
}

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
  INIT_WORD(L"CREATE", create, 0, FALSE);
  INIT_WORD(L"DOES>", does, 0, FALSE);
  INIT_WORD(L"'", tick, 0, TRUE);
  INIT_WORD(L",", comma, 0, FALSE);
  INIT_WORD(L"BRANCH", branch, 0, FALSE);
  INIT_WORD(L"BRANCH0", branch0, 0, FALSE);
  INIT_WORD(L"HERE", here, 0, FALSE);
  INIT_WORD(L"MAKE-FORWARD-REF", make_forward_reference, 0, FALSE);
  INIT_WORD(L"RESOLVE-FORWARD-REF", resolve_forward_reference, 0, FALSE);
  INIT_WORD(L"BUMP", bump, 0, FALSE);
  INIT_WORD(L"BUMP-MARK", bump_mark, 0, FALSE);
  INIT_WORD(L"BUMP-SET", bump_set, 0, FALSE);

  INIT_WORD(L".", io_print, 0, FALSE);

  INIT_WORD(L"+", arith_add, 0, FALSE);
  INIT_WORD(L"-", arith_sub, 0, FALSE);
  INIT_WORD(L"*", arith_mul, 0, FALSE);
  INIT_WORD(L"/", arith_div, 0, FALSE);

  INIT_WORD(L"EXIT", exit, 0, FALSE);
  INIT_WORD(L"ABORT", abort, 0, FALSE);
  INIT_WORD(L"INTERPRET", interpret, 0, FALSE);
#undef INIT_WORD
}

extern int lepton_entry(void *, void *);

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  EFI_ImageHandle = ImageHandle;
  lepton_core_init(ImageHandle, SystemTable);
  lepton_console_printf(L"Starting LEPTON\n");
  lepton_memory_initialize();
  lepton_dictionary_initialize((void *) LeptonMemory.dictionary, (void *) LeptonMemory.executable_area);
  initialize_core_dictionary();

  lepton_console_printf(L"Entering LEPTON\n");
  if (lepton_entry((void *) (LeptonMemory.code_stack + LEPTON_CODE_STACK_PAGES * EFI_PAGE_SIZE), (void *) (LeptonMemory.data_stack + LEPTON_DATA_STACK_PAGES + EFI_PAGE_SIZE))) {
    lepton_console_printf(L"Fatal: entry failed\n");
  }

  lepton_word_exit();
  return EFI_SUCCESS;
}