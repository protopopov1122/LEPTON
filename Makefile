GNUEFI_INC=/usr/include/efi
GNUEFI_LIB=/usr/lib
OVMF_DIR=/usr/share/ovmf/x64
SOURCE_DIR=source
BIN_DIR=bin

PLATFORM_SRC=$(SOURCE_DIR)/platform

CC=gcc
AS=as
LD=ld
CFLAGS=-I$(GNUEFI_INC) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -ggdb
LDFLAGS=-shared -Bsymbolic -L$(GNUEFI_LIB) -T$(GNUEFI_LIB)/elf_x86_64_efi.lds $(GNUEFI_LIB)/crt0-efi-x86_64.o

$(PLATFORM_SRC)/platform/dictionary.h: $(PLATFORM_SRC)/platform/core.h
$(PLATFORM_SRC)/platform/util.c: $(PLATFORM_SRC)/platform/util.h
$(PLATFORM_SRC)/platform/console.c: $(PLATFORM_SRC)/platform/console.h $(PLATFORM_SRC)/platform/util.h $(PLATFORM_SRC)/platform/core.h
$(PLATFORM_SRC)/platform/memory.c: $(PLATFORM_SRC)/platform/console.h $(PLATFORM_SRC)/platform/memory.h $(PLATFORM_SRC)/platform/core.h
$(PLATFORM_SRC)/platform/dictionary.c: $(PLATFORM_SRC)/platform/dictionary.h $(PLATFORM_SRC)/platform/console.h
$(PLATFORM_SRC)/platform/helpers.c: $(PLATFORM_SRC)/platform/helpers.h $(PLATFORM_SRC)/platform/dictionary.h $(PLATFORM_SRC)/platform/console.h
$(PLATFORM_SRC)/platform/core.c: $(PLATFORM_SRC)/platform/core.h $(PLATFORM_SRC)/platform/console.h
$(PLATFORM_SRC)/platform/input.c: $(PLATFORM_SRC)/platform/input.h $(PLATFORM_SRC)/platform/console.h $(PLATFORM_SRC)/platform/core.h $(PLATFORM_SRC)/platform/util.h
$(PLATFORM_SRC)/platform/main.c: $(PLATFORM_SRC)/platform/console.h $(PLATFORM_SRC)/platform/util.h $(PLATFORM_SRC)/platform/core.h $(PLATFORM_SRC)/platform/memory.h $(PLATFORM_SRC)/platform/dictionary.h

$(SOURCE_DIR)/core/entry.S: $(SOURCE_DIR)/core/header.inc.S
$(SOURCE_DIR)/core/io_words.S: $(SOURCE_DIR)/core/header.inc.S
$(SOURCE_DIR)/core/arithmetic_words.S: $(SOURCE_DIR)/core/header.inc.S
$(SOURCE_DIR)/core/core_words.S: $(SOURCE_DIR)/core/header.inc.S

$(BIN_DIR)/%.c.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(shell dirname "$@")
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.S.o: $(SOURCE_DIR)/%.S
	@mkdir -p $(shell dirname "$@")
	$(AS) $< -o $@

$(BIN_DIR)/lepton.so: \
	$(BIN_DIR)/platform/main.c.o \
	$(BIN_DIR)/platform/console.c.o \
	$(BIN_DIR)/platform/util.c.o \
	$(BIN_DIR)/platform/memory.c.o \
	$(BIN_DIR)/platform/dictionary.c.o \
	$(BIN_DIR)/platform/helpers.c.o \
	$(BIN_DIR)/platform/core.c.o \
	$(BIN_DIR)/platform/input.c.o \
	$(BIN_DIR)/core/io_words.S.o \
	$(BIN_DIR)/core/arithmetic_words.S.o \
	$(BIN_DIR)/core/core_words.S.o \
	$(BIN_DIR)/core/entry.S.o
	$(LD) $(LDFLAGS) $^ -o $@ -lgnuefi -lefi

$(BIN_DIR)/lepton.efi: $(BIN_DIR)/lepton.so
	objcopy \
		-j .text \
		-j .sdata \
		-j .data \
		-j .dynamic \
		-j .dynsym \
		-j .rel \
		-j .rela \
		-j .rel.* \
		-j .rela.* \
		-j .reloc \
		--target efi-app-x86_64 --subsystem=10 \
		$(BIN_DIR)/lepton.so $(BIN_DIR)/lepton.efi

$(BIN_DIR)/OVMF_CODE.fd:
	@cp $(OVMF_DIR)/OVMF_CODE.fd $(BIN_DIR)/OVMF_CODE.fd

$(BIN_DIR)/OVMF_VARS.fd:
	@cp $(OVMF_DIR)/OVMF_VARS.fd $(BIN_DIR)/OVMF_VARS.fd

$(BIN_DIR)/build/lepton.efi: $(BIN_DIR)/lepton.efi
	@mkdir -p $(shell dirname "$@")
	@cp $(BIN_DIR)/lepton.efi $(BIN_DIR)/build/lepton.efi

$(BIN_DIR)/build/startup.nsh: $(SOURCE_DIR)/startup.nsh
	@mkdir -p $(shell dirname "$@")
	@cp $(SOURCE_DIR)/startup.nsh $(BIN_DIR)/build/startup.nsh

run: $(BIN_DIR)/build/lepton.efi $(BIN_DIR)/build/startup.nsh $(BIN_DIR)/OVMF_VARS.fd $(BIN_DIR)/OVMF_CODE.fd
	qemu-system-x86_64 \
		-nodefaults \
		-vga std \
		-machine q35,accel=kvm:tcg \
		-m 128M \
		-drive if=pflash,format=raw,readonly=on,file=$(BIN_DIR)/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=$(BIN_DIR)/OVMF_VARS.fd \
		-drive format=raw,file=fat:rw:$(BIN_DIR)/build \
		-serial stdio \
		-monitor vc:1024x768

debug: $(BIN_DIR)/build/lepton.efi $(BIN_DIR)/build/startup.nsh $(BIN_DIR)/OVMF_VARS.fd $(BIN_DIR)/OVMF_CODE.fd
	qemu-system-x86_64 \
		-s -S \
		-nodefaults \
		-vga std \
		-machine q35,accel=kvm:tcg \
		-m 128M \
		-drive if=pflash,format=raw,readonly=on,file=$(BIN_DIR)/OVMF_CODE.fd \
		-drive if=pflash,format=raw,file=$(BIN_DIR)/OVMF_VARS.fd \
		-drive format=raw,file=fat:rw:$(BIN_DIR)/build \
		-serial stdio \
		-monitor vc:1024x768

clean:
	@rm -rf $(BIN_DIR)

.PHONY: run clean