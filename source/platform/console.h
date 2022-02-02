#ifndef LEPTON_CONSOLE_H_
#define LEPTON_CONSOLE_H_

#include <efi.h>

extern BOOLEAN lepton_console_echo_input;

CHAR16 lepton_console_getchar();
void lepton_console_putchar(CHAR16);
void lepton_console_putstring(const CHAR16 *);
void lepton_console_printf(const CHAR16 *, ...);
UINTN lepton_console_readline(CHAR16 *, UINTN, const CHAR16 *);
const CHAR16 *lepton_input_next_word();

#endif
