.intel_syntax noprefix

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

# --------------- DATA ---------------

.section .data

INTERPRETER_STATE:
    .byte 0

# --------------- CODE ---------------

.section .text
.global lepton_bootstrap
.global lepton_interpret
.global lepton_colon
.global lepton_semicolon
.global lepton_left_bracket
.global lepton_right_bracket
.global lepton_literal
.global lepton_immediate
.global lepton_postpone
.global lepton_io_print
.global lepton_arith_add
.global lepton_arith_sub
.global lepton_arith_mul
.global lepton_arith_div

.extern leptonrt_define_word
.extern leptonrt_top_word
.extern leptonrt_read_input
.extern leptonrt_parse_input_word
.extern leptonrt_executable_area_current
.extern leptonrt_executable_area_append_call
.extern leptonrt_executable_area_append_push
.extern leptonrt_executable_area_patch_push
.extern leptonrt_executable_area_append_prologue
.extern leptonrt_executable_area_append_ret
.extern leptonrt_executable_area_append_jmp

.extern Print

lepton_interpret:
# Interpreter loop begins
# Read next word
    call leptonrt_read_input
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
    ret

lepton_interpret_word:
    call leptonrt_parse_input_word
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
    call leptonrt_parse_input_word
    cmp rax, 0
    je lepton_interp_compile_word
# Compile integer
    mov rdi, rdx
    call leptonrt_executable_area_append_push
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
    call leptonrt_executable_area_append_call
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

lepton_bootstrap:
    # Switch stacks
    # Return stack
    push rbp
    mov rax, rsp
    mov rsp, rdi
    mov rbp, rsp
    push rax
    # Data stack
    mov rbx, rsi

    call leptonrt_top_word
    cmp rax, 0
    je lepton_bootstrap_failure
    add rax, dictionary_entry_code
    call [rax]

    # Restore stack
    pop rax
    mov rsp, rax
    pop rbp
    xor rax, rax
    ret
lepton_bootstrap_failure:
    pop rax
    mov rsp, rax
    pop rbp
    xor rax, rax
    mov rax, 1
    ret

lepton_colon:
# Read word name
    call leptonrt_read_input
    cmp rax, 0
    je lepton_colon_fail
    mov r12, rax
# Retrieve code pointer
    call leptonrt_executable_area_current
# Create dictionary entry
    mov rdi, r12
    mov rsi, rax
    xor rdx, rdx
    xor rcx, rcx
    call leptonrt_define_word
# Add prologue
    call leptonrt_executable_area_append_prologue
# Switch to compilation mode
    mov rax, 1
    mov BYTE PTR INTERPRETER_STATE[rip], al
    ret
lepton_colon_fail:
    lea rdi, lepton_colon_fail_msg[rip]
    call Print
    ret
lepton_colon_fail_msg:
    .string16 "Fatal: failed to define word\n\000"

lepton_semicolon:
    call leptonrt_executable_area_append_ret
    xor rax, rax
    mov BYTE PTR INTERPRETER_STATE[rip], al
    ret

lepton_left_bracket:
    xor al, al
    mov BYTE PTR INTERPRETER_STATE[rip], al
    ret

lepton_right_bracket:
    mov al, 1
    mov BYTE PTR INTERPRETER_STATE[rip], al
    ret

lepton_literal:
    mov rdi, [rbx]
    add rbx, 8
    call leptonrt_executable_area_append_push
    ret

lepton_immediate:
    call leptonrt_top_word
    mov dl, 1
    mov [rax + dictionary_entry_immediate], dl
    ret

lepton_postpone:
    call leptonrt_read_input
    cmp rax, 0
    je lepton_postpone_fail
    mov rdi, rax
    call leptonrt_parse_input_word
    cmp rax, 0
    je lepton_postpone_compile_word
# Compile integer
    mov rdi, rdx
    call leptonrt_executable_area_append_push
    ret
# Compile word: execute it's compilation semantics
lepton_postpone_compile_word:
    cmp rdx, 0
    je lepton_postpone_fail
    mov rdi, [rdx + dictionary_entry_code]
    mov rsi, [rdx + dictionary_entry_data]
    call leptonrt_executable_area_append_call
    ret
lepton_postpone_fail:
    lea rdi, lepton_postpone_fail_msg[rip]
    call Print
    ret
lepton_postpone_fail_msg:
    .string16 "Fatal: failed to postpone word execution\n\000"

lepton_arith_add:
    mov rax, [rbx + 8]
    add rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    ret

lepton_arith_sub:
    mov rax, [rbx + 8]
    sub rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    ret

lepton_arith_mul:
    mov rax, [rbx + 8]
    imul rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    ret

lepton_arith_div:
    mov rax, [rbx + 8]
    cqo
    idiv rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    ret

.extern leptonrt_print_int
lepton_io_print:
    push rbp
    mov rbp, rsp
    mov rax, [rbx]
    add rbx, 8
    mov rdi, rax
    call leptonrt_print_int
    leave
    ret