#include "core.h"
#include "console.h"
#include <efilib.h>

void lepton_core_init(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  EFI_STATUS status;
  EFI_LOADED_IMAGE *loaded_image = NULL;

  status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &LoadedImageProtocol, (void **)&loaded_image);
                            
  if (EFI_ERROR(status)) {
      lepton_console_printf(L"Fatal: HandleProtocol failure: %r\n", status);
  }
  lepton_console_printf(L"Image base: %lx\n", loaded_image->ImageBase);

  status = uefi_call_wrapper(SystemTable->BootServices->SetWatchdogTimer, 4, 0, 0, 0, NULL);
  if (EFI_ERROR(status)) {
      lepton_console_printf(L"Fatal: failed to disable watchdog timer: %r\n", status);
  }
  lepton_console_printf(L"Disabled UEFI watchdog timer\n");
}

void lepton_core_free() {}
