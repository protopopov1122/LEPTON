#ifndef LEPTON_MEMORY_H_
#define LEPTON_MEMORY_H_

#include <efi.h>

struct LeptonMemory {
  EFI_PHYSICAL_ADDRESS code_stack;
  EFI_PHYSICAL_ADDRESS data_stack;
  EFI_PHYSICAL_ADDRESS dictionary;
  EFI_PHYSICAL_ADDRESS executable_area;
};

extern struct LeptonMemory LeptonMemory;

EFI_STATUS lepton_memory_initialize();
EFI_STATUS lepton_memory_free();

#endif