RUNNER_DIR ?= build/qemu-runner
EFI_PART_DIR ?= $(RUNNER_DIR)/efi-part

# Allow overrides from environment variables or cli args with a preference for cli args
OVMF_ROOT ?= /usr/share/OVMF

.PHONY: all
all: breakout.efi compile_commands.json

.PHONY: clean
clean: clean-runner
	rm -vf *.{o,so,efi}
	$(MAKE) -C gnu-efi clean

.PHONY: clean-runner
clean-runner:
	rm -rvf $(RUNNER_DIR)

.PHONY: run
run: $(EFI_PART_DIR)/BREAKOUT/breakout.efi
	@# Make a copy of OVMF_VARS to ensure that qemu can non-destructively write
	mkdir -p $(RUNNER_DIR)/FV
	cp -v $(OVMF_ROOT)/FV/OVMF_VARS.fd $(RUNNER_DIR)/FV/OVMF_VARS.fd

	@# Note: qemu documentation marks the `:rw` attribute on `-hdb` as experimental
	sudo -ES qemu-system-x86_64 \
	  -drive if=pflash,format=raw,readonly=on,file=$(OVMF_ROOT)/FV/OVMF_CODE.fd \
	  -drive if=pflash,format=raw,file=$(RUNNER_DIR)/FV/OVMF_VARS.fd \
	  -hdb fat:rw:$(EFI_PART_DIR) \
	  -m 2048 \
	  -enable-kvm \
	  -machine q35 \
	  -serial stdio \
	  -display none

breakout.efi: breakout.so
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela \
		-j .rel.* -j .rela.* -j .reloc --output-target efi-app-x86_64 --subsystem=10 breakout.so \
		breakout.efi

breakout.so: breakout.o gnu-efi
	ld -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi \
		-Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o breakout.o \
		-o breakout.so -lgnuefi -lefi

breakout.o: breakout.c
	$(CC) -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar \
		-mno-red-zone -maccumulate-outgoing-args \
		-c breakout.c -o breakout.o

.PHONY: gnu-efi
gnu-efi:
	$(MAKE) -C gnu-efi

# Setup a basic EFI system partition with the UEFI application
$(EFI_PART_DIR)/BREAKOUT/breakout.efi: clean-runner breakout.efi
	mkdir -p $(EFI_PART_DIR)/EFI/BOOT
	cp breakout.efi $(EFI_PART_DIR)/EFI/BOOT/BOOTX64.EFI

	[ -n "$(UEFI_SHELL)" ] && cp $(UEFI_SHELL) $(EFI_PART_DIR)

.PHONY: compile_commands.json
compile_commands.json:
	compiledb -n $(MAKE) breakout.efi -C .
