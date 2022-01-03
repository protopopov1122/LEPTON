.intel_syntax noprefix

# --------------- MACROS ---------------

.macro DEFINE_WORD identifier
.global lepton_word_\identifier
lepton_word_\identifier\():
    push rbp
    mov rbp, rsp
.endm

.macro END_WORD
    leave
    ret
.endm

# --------------- STRUCTURE DEFINITIONS ---------------

.struct 0
dictionary_entry_name:
    .struct dictionary_entry_name + 48
dictionary_entry_link:
    .struct dictionary_entry_link + 8
dictionary_entry_code:
    .struct dictionary_entry_code + 8
dictionary_entry_data:
    .struct dictionary_entry_data + 8
dictionary_entry_immediate:

# --------------- EXTERNALS ---------------

.section .code

.extern lepton_word_exit
.extern lepton_word_abort
.extern lepton_define_word
.extern lepton_top_word
.extern lepton_console_next_word
.extern lepton_helper_parse_input
.extern lepton_console_printf
.extern lepton_executable_area_current
.extern lepton_executable_area_append_call
.extern lepton_executable_area_append_push
.extern lepton_executable_area_patch_push
.extern lepton_executable_area_append_prologue
.extern lepton_executable_area_append_ret
.extern lepton_executable_area_append_jmp
