.include "source/core/header.inc.S"

# --------------- CODE ---------------

.section .text

DEFINE_WORD arith_add
    PEEKDATA rax, 1
    PEEKDATA rcx, 0
    add rax, rcx
    REPLACEDATA rax, 1
    END_WORD

DEFINE_WORD arith_sub
    PEEKDATA rax, 1
    PEEKDATA rcx, 0
    sub rax, rcx
    REPLACEDATA rax, 1
    END_WORD

DEFINE_WORD arith_mul
    PEEKDATA rax, 1
    PEEKDATA rcx, 0
    imul rax, rcx
    REPLACEDATA rax, 1
    END_WORD

DEFINE_WORD arith_div
    PEEKDATA rax, 1
    cqo
    PEEKDATA rcx, 0
    idiv rax, rcx
    REPLACEDATA rax, 1
    END_WORD
