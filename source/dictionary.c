#include "dictionary.h"
#include "console.h"
#include <efilib.h>

struct LeptonDictionary {
  void *base;
  struct lepton_dictionary_entry *current;
};

struct LeptonExecutableArea {
  void *base;
  UINT8 *current;
};

static struct LeptonDictionary LeptonDictionary = {0};
static struct LeptonExecutableArea LeptonExecutableArea = {0};

void lepton_dictionary_initialize(void *dictionary, void *executable_area) {
  LeptonDictionary.base = dictionary;
  LeptonExecutableArea.base = executable_area;
  lepton_console_printf(L"Initializing dictionary=%lx; executable area=%lx\n",
    dictionary, executable_area);
}

void lepton_dictionary_free() {
  // TODO Free dictionary
}

void lepton_define_word(const CHAR16 *name, const void *code, UINT64 data, BOOLEAN immediate) {
    struct lepton_dictionary_entry *entry = NULL;
    if (LeptonDictionary.current != NULL) {
        entry = LeptonDictionary.current + 1;
    } else {
        entry = LeptonDictionary.base;
    }

    StrnCpy(entry->name, name, LEPTON_DICTIONARY_WORD_MAXLEN);
    entry->code = code;
    entry->data = data;
    entry->immediate = immediate;
    entry->link = LeptonDictionary.current;
    LeptonDictionary.current = entry;
}

struct lepton_dictionary_entry *lepton_resolve_word(const CHAR16 *name) {
    struct lepton_dictionary_entry *entry = LeptonDictionary.current;
    while (entry != NULL) {
        if (StrCmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->link;
    }
    return NULL;
}

struct lepton_dictionary_entry *lepton_top_word() {
    return LeptonDictionary.current;
}

void *lepton_executable_area_current() {
    return LeptonExecutableArea.current;
}

void lepton_executable_area_append_call(UINT64 ptr, UINT64 data) {
    // mov rax, ptr
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0xb8;
    for (UINTN i = 0; i < 8; i++) {
        *(LeptonExecutableArea.current++) = (ptr >> (i << 3)) & 0xff;
    }
    // mov rdi, data
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0xbf;
    for (UINTN i = 0; i < 8; i++) {
        *(LeptonExecutableArea.current++) = (data >> (i << 3)) & 0xff;
    }
    // call rax
    *(LeptonExecutableArea.current++) = 0xff;
    *(LeptonExecutableArea.current++) = 0xd0;
}

void lepton_executable_area_append_push(UINT64 data) {
    // mov rdi, data
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0xbf;
    for (UINTN i = 0; i < 8; i++) {
        *(LeptonExecutableArea.current++) = (data >> (i << 3)) & 0xff;
    }
    // sub rbx, 8
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0x83;
    *(LeptonExecutableArea.current++) = 0xeb;
    *(LeptonExecutableArea.current++) = 0x08;
    // mov [rbx], rdi
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0x89;
    *(LeptonExecutableArea.current++) = 0x3b;
}

void lepton_executable_area_patch_push(UINT8 *ptr, UINT64 data) {
    ptr += 2;
    for (UINTN i = 0; i < 8; i++) {
        *(ptr++) = (data >> (i << 3)) & 0xff;
    }
}

void lepton_executable_area_append_prologue() {
    // push rbp
    *(LeptonExecutableArea.current++) = 0x55;
    // mov rbp, rsp
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0x89;
    *(LeptonExecutableArea.current++) = 0xe5;
}

void lepton_executable_area_append_ret() {
    *(LeptonExecutableArea.current++) = 0xc9;
    *(LeptonExecutableArea.current++) = 0xc3;
}

void lepton_executable_area_append_jmp(UINT64 addr) {
    // mov rax, addr
    *(LeptonExecutableArea.current++) = 0x48;
    *(LeptonExecutableArea.current++) = 0xb8;
    for (UINTN i = 0; i < 8; i++) {
        *(LeptonExecutableArea.current++) = (addr >> (i << 3)) & 0xff;
    }
    // jmp rax
    *(LeptonExecutableArea.current++) = 0xff;
    *(LeptonExecutableArea.current++) = 0xe0;
}
