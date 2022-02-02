#ifndef LEPTON_MEMORY_H_
#define LEPTON_MEMORY_H_

#include <efi.h>

struct LeptonMemory {
  EFI_PHYSICAL_ADDRESS code_stack;
  EFI_PHYSICAL_ADDRESS data_stack;
  EFI_PHYSICAL_ADDRESS dictionary;
  EFI_PHYSICAL_ADDRESS executable_area;
  EFI_PHYSICAL_ADDRESS bump_area;

  struct {
    char *current;
  } bump;
};

extern struct LeptonMemory LeptonMemory;

EFI_STATUS lepton_memory_initialize();
EFI_STATUS lepton_memory_free();

void *lepton_bump_area_mark();
void *lepton_bump_area_alloc(UINT64);
void lepton_bump_area_set(void *);

#endif