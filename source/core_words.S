.include "source/header.inc.S"

# --------------- DATA ---------------

.section .data

INTERPRETER_STATE:
    .byte 0

# --------------- CODE ---------------

.section .text

DEFINE_WORD interpret
lepton_interpret:
# Interpreter loop begins
# Read next word
    call lepton_console_next_word
    cmp rax, 0
    je lepton_interp_exit
    mov rdi, rax
# Check current interpreter state
    mov al, BYTE PTR INTERPRETER_STATE[rip]
    cmp al, 0
    je lepton_interpret_word
    jmp lepton_compile_word
# Interpreter loop ends
lepton_interp_exit:
    END_WORD

lepton_interpret_word:
    call lepton_helper_parse_input
    cmp rax, 0
    je lepton_interpret_execute_word
# Interpret integer: push it
    sub rbx, 8
    mov [rbx], rdx
    jmp lepton_interpret
# Interpret word: execute it
lepton_interpret_execute_word:
    cmp rdx, 0
    je lepton_interpret_execute_word_fail
    mov rax, [rdx + dictionary_entry_code]
    mov rdi, [rdx + dictionary_entry_data]
    call rax
    jmp lepton_interpret
lepton_interpret_execute_word_fail:
    lea rdi, lepton_interpret_execute_word_fail_msg[rip]
    call Print
    jmp lepton_interpret
lepton_interpret_execute_word_fail_msg:
    .string16 "Fatal: failed to execute word\n\000"

lepton_compile_word:
    call lepton_helper_parse_input
    cmp rax, 0
    je lepton_interp_compile_word
# Compile integer
    mov rdi, rdx
    call lepton_executable_area_append_push
    jmp lepton_interpret
# Compile word: execute it's compilation semantics
lepton_interp_compile_word:
    cmp rdx, 0
    je lepton_interp_compile_word_fail
    mov rdi, [rdx + dictionary_entry_code]
    mov rsi, [rdx + dictionary_entry_data]
    mov al, [rdx + dictionary_entry_immediate]
    cmp al, 0
    jne lepton_interp_compile_immediate
    call lepton_executable_area_append_call
    jmp lepton_interpret
lepton_interp_compile_immediate:
    call rdi
    jmp lepton_interpret
lepton_interp_compile_word_fail:
    lea rdi, lepton_interp_compile_word_fail_msg[rip]
    call Print
    jmp lepton_interpret
lepton_interp_compile_word_fail_msg:
    .string16 "Fatal: failed to compile word\n\000"

DEFINE_WORD colon
# Read word name
    call lepton_console_next_word
    cmp rax, 0
    je lepton_colon_fail
    mov r12, rax
# Retrieve code pointer
    call lepton_executable_area_current
# Create dictionary entry
    mov rdi, r12
    mov rsi, rax
    xor rdx, rdx
    xor rcx, rcx
    call lepton_define_word
# Add prologue
    call lepton_executable_area_append_prologue
# Switch to compilation mode
    mov rax, 1
    mov BYTE PTR INTERPRETER_STATE[rip], al
    END_WORD
lepton_colon_fail:
    lea rdi, lepton_colon_fail_msg[rip]
    call lepton_console_printf
    END_WORD
lepton_colon_fail_msg:
    .string16 "Fatal: failed to define word\n\000"

DEFINE_WORD semicolon
    call lepton_executable_area_append_ret
    xor rax, rax
    mov BYTE PTR INTERPRETER_STATE[rip], al
    END_WORD

DEFINE_WORD left_bracket
    xor al, al
    mov BYTE PTR INTERPRETER_STATE[rip], al
    END_WORD

DEFINE_WORD right_bracket
    mov al, 1
    mov BYTE PTR INTERPRETER_STATE[rip], al
    END_WORD

DEFINE_WORD literal
    mov rdi, [rbx]
    add rbx, 8
    call lepton_executable_area_append_push
    END_WORD

DEFINE_WORD immediate
    call lepton_top_word
    mov dl, 1
    mov [rax + dictionary_entry_immediate], dl
    END_WORD

DEFINE_WORD postpone
    call lepton_console_next_word
    cmp rax, 0
    je lepton_postpone_fail
    mov rdi, rax
    call lepton_helper_parse_input
    cmp rax, 0
    je lepton_postpone_compile_word
# Compile integer
    mov rdi, rdx
    call lepton_executable_area_append_push
    END_WORD
# Compile word: execute it's compilation semantics
lepton_postpone_compile_word:
    cmp rdx, 0
    je lepton_postpone_fail
    mov rdi, [rdx + dictionary_entry_code]
    mov rsi, [rdx + dictionary_entry_data]
    call lepton_executable_area_append_call
    END_WORD
lepton_postpone_fail:
    lea rdi, lepton_postpone_fail_msg[rip]
    call lepton_console_printf
    END_WORD
lepton_postpone_fail_msg:
    .string16 "Fatal: failed to postpone word execution\n\000"
