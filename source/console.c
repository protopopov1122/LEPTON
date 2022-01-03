#include "console.h"
#include "util.h"
#include "core.h"
#include <efilib.h>
#include <stdarg.h>

BOOLEAN lepton_console_echo_input = TRUE;

CHAR16 lepton_console_getchar() {
  EFI_STATUS status;
  EFI_INPUT_KEY key;

  WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
  status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);
  if (EFI_ERROR(status)) {
    return L'\0';
  }

  if (lepton_console_echo_input) {
    lepton_console_putchar(key.UnicodeChar);
  }
  return key.UnicodeChar;
}

void lepton_console_putchar(CHAR16 chr) {
  CHAR16 str[] = {chr, L'\0'};
  uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, str);
}

void lepton_console_putstring(const CHAR16 *str) {
  if (str != NULL) {
    uefi_call_wrapper(ST->ConOut->OutputString, 2, ST->ConOut, str);
  }
}

void lepton_console_printf(const CHAR16 *fmt, ...) {
  va_list list;
  va_start(list, fmt);
  VPrint(fmt, list);
  va_end(list);
}

UINTN lepton_console_readline(CHAR16 *content, UINTN max_length, const CHAR16 *prompt) {
  CHAR16 chr;
  UINTN index = 0;

  if (prompt != NULL) {
    lepton_console_putstring(prompt);
  }

  do {
    chr = lepton_console_getchar();
    if (chr == L'\b') {
      if (index > 0) {
        lepton_console_putstring(L" \b");
        index--;
      }
    } else {
      content[index++] = chr;
    }
  } while (!lepton_util_isnewline(chr) && chr != '\0' && index + 1 < max_length);
  content[index] = L'\0';
  Print(L"\r\n");
  return index;
}
