.include "source/header.inc.S"

# --------------- CODE ---------------

.section .text
.global lepton_entry

lepton_entry:
    # Switch stacks
    # Return stack
    push rbp
    mov rax, rsp
    mov rsp, rdi
    xor rbp, rbp
    push rbp
    mov rbp, rsp
    push rax
    # Data stack
    mov rbx, rsi

    call lepton_top_word
    cmp rax, 0
    je lepton_entry_failure
    mov r12, rax
    lea rdi, lepton_entry_msg[rip]
    lea rsi, [r12 + dictionary_entry_name]
    mov rdx, [r12 + dictionary_entry_data]
    mov rcx, [r12 + dictionary_entry_code]
    xor rax, rax
    call lepton_console_printf
    mov rdi, [r12 + dictionary_entry_data]
    mov rax, [r12 + dictionary_entry_code]
    call rax

    # Restore stack
    pop rax
    pop rbp
    mov rsp, rax
    pop rbp
    xor rax, rax
    ret
lepton_entry_failure:
    pop rax
    pop rbp
    mov rsp, rax
    pop rbp
    xor rax, rax
    mov rax, 1
    ret
lepton_entry_msg:
    .string16 "Starting %s(%ld) at %lx\n\000"
