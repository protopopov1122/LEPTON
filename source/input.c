#include "input.h"
#include "console.h"
#include "core.h"
#include "util.h"

#define LEPTON_CONSOLE_LINE_MAXLEN 1023
static CHAR16 ConsoleLine[LEPTON_CONSOLE_LINE_MAXLEN + 1] = {L'\0'};

static const CHAR16 *InputBuffer = ConsoleLine;
static UINTN InputBufferOffset = 0;
static UINTN InputBufferLength = 0;

void lepton_set_input_buffer(const CHAR16 *buf, UINTN length) {
  if (buf != NULL) {
    InputBuffer = buf;
    InputBufferLength = length;
    InputBufferOffset = 0;
  }
}

const CHAR16 *lepton_input_next_word() {
  if (InputBufferOffset >= InputBufferLength || InputBuffer[InputBufferOffset] == L'\0') {
    lepton_set_input_buffer(ConsoleLine, lepton_console_readline(ConsoleLine, LEPTON_CONSOLE_LINE_MAXLEN, L"> "));
  }

  static CHAR16 WORD[LEPTON_DICTIONARY_WORD_MAXLEN + 1] = {L'\0'};
  CHAR16 chr;

  do {
    chr = InputBuffer[InputBufferOffset++];
  } while (lepton_util_isspace(chr) && InputBufferOffset < InputBufferLength);
  if (chr == L'\0' || lepton_util_isspace(chr)) {
      return NULL;
  }

  UINTN i = 0;
  for (; !lepton_util_isspace(chr) && chr != L'\0' && i < LEPTON_DICTIONARY_WORD_MAXLEN && InputBufferOffset < InputBufferLength; i++) {
      WORD[i] = chr;
      chr = InputBuffer[InputBufferOffset++];
  }
  WORD[i] = L'\0';
  return WORD;
}