.include "source/header.inc"

# --------------- CODE ---------------

.section .text

DEFINE_WORD arith_add
    mov rax, [rbx + 8]
    add rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    END_WORD

DEFINE_WORD arith_sub
    mov rax, [rbx + 8]
    sub rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    END_WORD

DEFINE_WORD arith_mul
    mov rax, [rbx + 8]
    imul rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    END_WORD

DEFINE_WORD arith_div
    mov rax, [rbx + 8]
    cqo
    idiv rax, [rbx]
    mov [rbx + 8], rax
    add rbx, 8
    END_WORD
