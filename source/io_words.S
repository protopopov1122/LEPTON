.include "source/header.inc.S"

# --------------- CODE ---------------

.section .text

DEFINE_WORD io_print
    POPDATA rsi
    lea rdi, lepton_io_print_msg[rip]
    xor rax, rax
    call lepton_console_printf
    END_WORD
lepton_io_print_msg:
    .string16 "%ld\n\000"
