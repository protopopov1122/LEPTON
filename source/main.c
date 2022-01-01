#include <efi.h>
#include <efilib.h>

static EFI_HANDLE EFI_ImageHandle;
static EFI_SYSTEM_TABLE *EFI_SystemTable;
 
#define CODE_STACK_PAGES 8
#define DATA_STACK_PAGES 8
#define DICTIONARY_PAGES 8
#define EXECUTABLE_AREA_PAGES 64

#define MAX_DICT_ENTRY_NAME 23
struct dictionary_entry {
  CHAR16 name[MAX_DICT_ENTRY_NAME + 1];
  struct dictionary_entry *link;
  const void *code;
  UINT64 data;
  BOOLEAN immediate;
};

struct {
  void *base;
  struct dictionary_entry *current;
} Dictionary = {0};

struct {
  void *head;
  UINT8 *current;
} ExecutableArea = {0};

void leptonrt_define_word(const CHAR16 *name, const void *code, UINT64 data, BOOLEAN immediate) {
    struct dictionary_entry *entry = NULL;
    if (Dictionary.current != NULL) {
        entry = Dictionary.current + 1;
    } else {
        entry = Dictionary.base;
    }

    StrnCpy(entry->name, name, MAX_DICT_ENTRY_NAME);
    entry->code = code;
    entry->data = data;
    entry->immediate = immediate;
    entry->link = Dictionary.current;
    Dictionary.current = entry;
}

struct dictionary_entry *leptonrt_resolve_word(const CHAR16 *name) {
    struct dictionary_entry *entry = Dictionary.current;
    while (entry != NULL) {
        if (StrCmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->link;
    }
    return NULL;
}

struct dictionary_entry *leptonrt_top_word() {
    return Dictionary.current;
}

static BOOLEAN isspace(CHAR16 chr) {
  return chr == L' ' || chr == L'\n' || chr == L'\r' || chr == L'\t';
}

static CHAR16 getchar() {
  EFI_STATUS status;
  EFI_INPUT_KEY key;

  WaitForSingleEvent(EFI_SystemTable->ConIn->WaitForKey, 0);
  status = uefi_call_wrapper(EFI_SystemTable->ConIn->ReadKeyStroke, 2, EFI_SystemTable->ConIn, &key);
  if (EFI_ERROR(status)) {
    return L'\0';
  }

  CHAR16 echo[] = {key.UnicodeChar, L'\0'};
  uefi_call_wrapper(EFI_SystemTable->ConOut->OutputString, 2, EFI_SystemTable->ConOut, echo);
  return key.UnicodeChar;
}

static void readline(CHAR16 *content, UINTN maxlen) {
  CHAR16 chr;
  UINTN index = 0;

  Print(L"> ");
  do {
    chr = getchar();
    if (chr == L'\b') {
      if (index > 0) {
        uefi_call_wrapper(EFI_SystemTable->ConOut->OutputString, 2, EFI_SystemTable->ConOut, L" \b");
        index--;
      }
    } else {
      content[index++] = chr;
    }
  } while (chr != L'\r' && chr != L'\n' && chr != '\0' && index + 1 < maxlen);
  content[index] = L'\0';
  Print(L"\n");
}

const CHAR16 *leptonrt_read_input() {
  static CHAR16 line[1024] = {L'\0'};
  static UINTN line_idx = 0;
  if (line[line_idx] == L'\0') {
    readline(line, (sizeof(line) - 1) / sizeof(line[0]));
    line_idx = 0;
  }

  static CHAR16 word[MAX_DICT_ENTRY_NAME + 1] = {0};
  CHAR16 chr;

  do {
    chr = line[line_idx++];
  } while (isspace(chr));
  if (chr == L'\0') {
      return NULL;
  }

  unsigned int i = 0;
  for (; !isspace(chr) && chr != L'\0' && i < MAX_DICT_ENTRY_NAME; i++) {
      word[i] = chr;
      chr = line[line_idx++];
  }
  word[i] = L'\0';
  return word;
}

struct input_parse_result {
    UINT64 is_integer;
    INT64 result;
} leptonrt_parse_input_word(const CHAR16 *input) {
    struct dictionary_entry *dict_entry = leptonrt_resolve_word(input);
    if (dict_entry != NULL) {
        return (struct input_parse_result){FALSE, (INT64) dict_entry};
    }

    INT64 integer = 0;
    for (unsigned int i = 0; input[i] != '\0'; i++) {
        if (!(input[i] >= L'0' && input[i] <= L'9')) {
            return (struct input_parse_result){FALSE, 0};
        }
        integer *= 10;
        integer += input[i] - '0';
    }
    return (struct input_parse_result){TRUE, integer};
}

void *leptonrt_executable_area_current() {
    return ExecutableArea.current;
}

void leptonrt_executable_area_append_call(UINT64 ptr, UINT64 data) {
    // mov rax, ptr
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0xb8;
    for (UINTN i = 0; i < 8; i++) {
        *(ExecutableArea.current++) = (ptr >> (i << 3)) & 0xff;
    }
    // mov rdi, data
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0xbf;
    for (UINTN i = 0; i < 8; i++) {
        *(ExecutableArea.current++) = (data >> (i << 3)) & 0xff;
    }
    // call rax
    *(ExecutableArea.current++) = 0xff;
    *(ExecutableArea.current++) = 0xd0;
}

void leptonrt_executable_area_append_push(UINT64 data) {
    // mov rdi, data
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0xbf;
    for (UINTN i = 0; i < 8; i++) {
        *(ExecutableArea.current++) = (data >> (i << 3)) & 0xff;
    }
    // sub rbx, 8
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0x83;
    *(ExecutableArea.current++) = 0xeb;
    *(ExecutableArea.current++) = 0x08;
    // mov [rbx], rdi
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0x89;
    *(ExecutableArea.current++) = 0x3b;
}

void leptonrt_executable_area_patch_push(UINT8 *ptr, UINT64 data) {
    ptr += 2;
    for (UINTN i = 0; i < 8; i++) {
        *(ptr++) = (data >> (i << 3)) & 0xff;
    }
}

void leptonrt_executable_area_append_ret() {
    *(ExecutableArea.current++) = 0xc3;
}

void leptonrt_executable_area_append_jmp(UINT64 addr) {
    // mov rax, addr
    *(ExecutableArea.current++) = 0x48;
    *(ExecutableArea.current++) = 0xb8;
    for (UINTN i = 0; i < 8; i++) {
        *(ExecutableArea.current++) = (addr >> (i << 3)) & 0xff;
    }
    // jmp rax
    *(ExecutableArea.current++) = 0xff;
    *(ExecutableArea.current++) = 0xe0;
}

extern int lepton_bootstrap(void *, void *);
extern void lepton_interpret();

extern void lepton_colon();
extern void lepton_semicolon();
extern void lepton_left_bracket();
extern void lepton_right_bracket();
extern void lepton_literal();
extern void lepton_immediate();
extern void lepton_postpone();

extern void lepton_io_print();
extern void lepton_arith_add();
extern void lepton_arith_sub();
extern void lepton_arith_mul();
extern void lepton_arith_div();

static void *fntoptr(void (*fn)()) {
    void **pptr = (void **) &fn;
    return *pptr;
}

static void dbg_wait() {
  int wait = 1;
  while (wait) {
    __asm__ __volatile__ ("pause");
  }
}

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);
  EFI_ImageHandle = ImageHandle;
  EFI_SystemTable = SystemTable;

  EFI_STATUS status;
  EFI_LOADED_IMAGE *loaded_image = NULL;
  EFI_PHYSICAL_ADDRESS code_stack;
  EFI_PHYSICAL_ADDRESS data_stack;
  EFI_PHYSICAL_ADDRESS dictionary;
  EFI_PHYSICAL_ADDRESS executable_area;

  status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &LoadedImageProtocol, (void **)&loaded_image);
                            
  if (EFI_ERROR(status)) {
      Print(L"Fatal: HandleProtocol failure: %r\n", status);
  }
  Print(L"Image base: %lx\n", loaded_image->ImageBase);

  Print(L"Begin initial memory allocation\n");
  status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, CODE_STACK_PAGES, &code_stack);
  if (status != EFI_SUCCESS) {
    Print(L"Fatal: Failed to allocate code stack: %r\n", status);
    return status;
  }
  Print(L"Allocated code stack of %d pages at 0x%llx\n", CODE_STACK_PAGES, code_stack);

  status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, DATA_STACK_PAGES, &data_stack);
  if (EFI_ERROR(status)) {
    Print(L"Fatal: Failed to allocate data stack: %r\n", status);
    return status;
  }
  Print(L"Allocated data stack of %d pages at 0x%llx\n", DATA_STACK_PAGES, data_stack);

  status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesData, DICTIONARY_PAGES, &dictionary);
  if (EFI_ERROR(status)) {
    Print(L"Fatal: Failed to allocate dictionary: %r\n", status);
    return status;
  }
  Print(L"Allocated dictionary of %d pages at 0x%llx\n", DICTIONARY_PAGES, dictionary);

  status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAnyPages, EfiRuntimeServicesCode, EXECUTABLE_AREA_PAGES, &executable_area);
  if (EFI_ERROR(status)) {
    Print(L"Fatal: Failed to allocate executable area: %r\n", status);
    return status;
  }
  Print(L"Allocated executable area of %d pages at 0x%llx\n", EXECUTABLE_AREA_PAGES, executable_area);

  Print(L"Done initial memory allocation\n");

  Dictionary.base = (void *) dictionary;
  ExecutableArea.head = (void *) executable_area;

  Print(L"Initializing core dictionary\n");
  leptonrt_define_word(L":", fntoptr(lepton_colon), 0, FALSE);
  leptonrt_define_word(L";", fntoptr(lepton_semicolon), 0, TRUE);
  leptonrt_define_word(L"[", fntoptr(lepton_left_bracket), 0, TRUE);
  leptonrt_define_word(L"]", fntoptr(lepton_right_bracket), 0, FALSE);
  leptonrt_define_word(L"LITERAL", fntoptr(lepton_literal), 0, TRUE);
  leptonrt_define_word(L"IMMEDIATE", fntoptr(lepton_immediate), 0, FALSE);
  leptonrt_define_word(L"POSTPONE", fntoptr(lepton_postpone), 0, TRUE);

  leptonrt_define_word(L".", fntoptr(lepton_io_print), 0, FALSE);

  leptonrt_define_word(L"+", fntoptr(lepton_arith_add), 0, FALSE);
  leptonrt_define_word(L"-", fntoptr(lepton_arith_sub), 0, FALSE);
  leptonrt_define_word(L"*", fntoptr(lepton_arith_mul), 0, FALSE);
  leptonrt_define_word(L"/", fntoptr(lepton_arith_div), 0, FALSE);

  leptonrt_define_word(L"INTERPRET", fntoptr(lepton_interpret), 0, FALSE);

  Print(L"Starting interpreter\n");
  if (lepton_bootstrap((void *) (code_stack + CODE_STACK_PAGES * EFI_PAGE_SIZE), (void *) (data_stack + DATA_STACK_PAGES + EFI_PAGE_SIZE))) {
    Print(L"Fatal: interpreter failed\n");
  }
  Print(L"Exiting\n");
  return EFI_SUCCESS;
}