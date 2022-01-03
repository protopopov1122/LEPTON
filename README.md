## LEPTON

This repo contains an on-going experiment in implementing Forth-like environment written as UEFI application for x86_64 architecture. Currently, it is
capable of producing a runnable EFI executable file that provides interactive environment with a few essential Forth words. Debugging
is possible via QEMU debugging interface and a few GDB commands:
```
target remote :1234
add-symbol-file bin/lepton.so [IMAGE_OFFSET+TEXT_OFFSET] -s .data [IMAGE_OFFSET+DATA_OFFSET]
set architecture i386:x86-64:intel
```

Author: Jevgēnijs Protopopovs