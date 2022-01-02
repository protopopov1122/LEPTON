#ifndef LEPTON_CORE_H_
#define LEPTON_CORE_H_

#include <efi.h>

#define LEPTON_CODE_STACK_PAGES 8
#define LEPTON_DATA_STACK_PAGES 8
#define LEPTON_DICTIONARY_PAGES 8
#define LEPTON_EXECUTABLE_AREA_PAGES 64
#define LEPTON_DICTIONARY_WORD_MAXLEN 23

void lepton_core_init(EFI_HANDLE, EFI_SYSTEM_TABLE *);
void lepton_core_free();

#endif
