#include "memory.h"
#include "console.h"
#include "core.h"
#include <efilib.h>

struct LeptonMemory LeptonMemory;

EFI_STATUS lepton_memory_initialize() {
  EFI_STATUS status;

  lepton_console_printf(L"Begin initial memory allocation\n");
  status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, LEPTON_CODE_STACK_PAGES, &LeptonMemory.code_stack);
  if (status != EFI_SUCCESS) {
    lepton_console_printf(L"Fatal: Failed to allocate code stack: %r\n", status);
    return status;
  }
  lepton_console_printf(L"Allocated code stack of %d pages at 0x%llx\n", LEPTON_CODE_STACK_PAGES, LeptonMemory.code_stack);

  status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, LEPTON_DATA_STACK_PAGES, &LeptonMemory.data_stack);
  if (EFI_ERROR(status)) {
    lepton_console_printf(L"Fatal: Failed to allocate data stack: %r\n", status);
    return status;
  }
  lepton_console_printf(L"Allocated data stack of %d pages at 0x%llx\n", LEPTON_DATA_STACK_PAGES, LeptonMemory.data_stack);

  status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, LEPTON_DICTIONARY_PAGES, &LeptonMemory.dictionary);
  if (EFI_ERROR(status)) {
    lepton_console_printf(L"Fatal: Failed to allocate dictionary: %r\n", status);
    return status;
  }
  lepton_console_printf(L"Allocated dictionary of %d pages at 0x%llx\n", LEPTON_DICTIONARY_PAGES, LeptonMemory.dictionary);

  status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesCode, LEPTON_EXECUTABLE_AREA_PAGES, &LeptonMemory.executable_area);
  if (EFI_ERROR(status)) {
    lepton_console_printf(L"Fatal: Failed to allocate executable area: %r\n", status);
    return status;
  }
  lepton_console_printf(L"Allocated executable area of %d pages at 0x%llx\n", LEPTON_EXECUTABLE_AREA_PAGES, LeptonMemory.executable_area);

  status = uefi_call_wrapper(ST->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesCode, LEPTON_BUMP_AREA_PAGES, &LeptonMemory.bump_area);
  if (EFI_ERROR(status)) {
    lepton_console_printf(L"Fatal: Failed to allocate bump area: %r\n", status);
    return status;
  }
  lepton_console_printf(L"Allocated bump area of %d pages at 0x%llx\n", LEPTON_BUMP_AREA_PAGES, LeptonMemory.bump_area);
  lepton_console_printf(L"Done initial memory allocation\n");

  LeptonMemory.bump.current = (void *) LeptonMemory.bump_area;
}

EFI_STATUS lepton_memory_free() {
  // TODO Free allocated memory pages
}

void *lepton_bump_area_mark() {
  return LeptonMemory.bump.current;
}

void *lepton_bump_area_alloc(UINT64 sz) {
  void *ptr = LeptonMemory.bump.current;
  LeptonMemory.bump.current += sz;
  return ptr;
}

void lepton_bump_area_set(void *ptr) {
  LeptonMemory.bump.current = ptr;
}