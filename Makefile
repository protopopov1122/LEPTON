GNUEFI_INC=/usr/include/efi
GNUEFI_LIB=/usr/lib
OVMF_DIR=/usr/share/ovmf/x64
SOURCE_DIR=source
BIN_DIR=bin

CC=gcc
AS=as
LD=ld
CFLAGS=-I$(GNUEFI_INC) -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -ggdb
LDFLAGS=-shared -Bsymbolic -L$(GNUEFI_LIB) -T$(GNUEFI_LIB)/elf_x86_64_efi.lds $(GNUEFI_LIB)/crt0-efi-x86_64.o

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(BIN_DIR)/main.c.o: $(SOURCE_DIR)/main.c $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(SOURCE_DIR)/main.c -o $(BIN_DIR)/main.c.o

$(BIN_DIR)/main.s.o: $(SOURCE_DIR)/main.s $(BIN_DIR)
	$(AS) $(SOURCE_DIR)/main.s -o $(BIN_DIR)/main.s.o

$(BIN_DIR)/lepton.so: $(BIN_DIR)/main.c.o $(BIN_DIR)/main.s.o
	$(LD) $(LDFLAGS) $(BIN_DIR)/main.c.o $(BIN_DIR)/main.s.o -o $(BIN_DIR)/lepton.so -lgnuefi -lefi

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

$(BIN_DIR)/build:
	@mkdir -p $(BIN_DIR)/build

$(BIN_DIR)/build/lepton.efi: $(BIN_DIR)/lepton.efi $(BIN_DIR)/build
	@cp $(BIN_DIR)/lepton.efi $(BIN_DIR)/build/lepton.efi

$(BIN_DIR)/build/startup.nsh: $(SOURCE_DIR)/startup.nsh $(BIN_DIR)/build
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