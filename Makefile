BUILD_DIR ?= build
RUNNER_DIR ?= $(BUILD_DIR)/qemu-runner
EFI_PART_DIR ?= $(RUNNER_DIR)/efi-part

SRCS := src/breakout.c src/graphics.c src/time.c src/rand.c
OBJS := $(addprefix $(BUILD_DIR)/,$(SRCS:%.c=%.o))

# Allow overrides from environment variables or cli args with a preference for cli args
OVMF_ROOT ?= /usr/share/OVMF

QEMU_FLAGS ?=
CFLAGS := -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar \
		  -mno-red-zone -maccumulate-outgoing-args -std=c11
LDFLAGS :=  -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds
OBJ_SEGMENTS := -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela \
				-j .rel.* -j .rela.* -j .reloc

# Enable GDB support (will halt QEMU until GDB connects along with building debug symbols)
GDB ?= 0

# Enable a quick die method which will shutdown on die rather than entering a hcf loop.
QUICK_DIE ?= 0

ifeq ($(GDB),1)
	QEMU_FLAGS += -s -S
	CFLAGS += -DDEBUG_ENABLED=1 -g -ggdb
endif

ifeq ($(QUICK_DIE),1)
	CFLAGS += -DQUICK_DIE=1
endif

.PHONY: all
all: $(BUILD_DIR)/breakout.efi $(BUILD_DIR)/compile_commands.json

.PHONY: clean
clean: clean-runner
	rm -rfv $(BUILD_DIR)
	rm -vf *.{o,so,efi,debug}
	$(MAKE) -C gnu-efi clean

.PHONY: clean-runner
clean-runner:
	rm -rvf $(RUNNER_DIR)

.PHONY: run
run: $(EFI_PART_DIR)/BREAKOUT/breakout.efi $(BUILD_DIR)/breakout.debug
	@# Make a copy of OVMF_VARS to ensure that qemu can non-destructively write
	mkdir -p $(RUNNER_DIR)/FV
	cp -v $(OVMF_ROOT)/FV/OVMF_VARS.fd $(RUNNER_DIR)/FV/OVMF_VARS.fd
	chmod ug+w $(RUNNER_DIR)/FV/OVMF_VARS.fd

	@# Note: qemu documentation marks the `:rw` attribute on `-hdb` as experimental
	qemu-system-x86_64 \
	  -drive if=pflash,format=raw,readonly=on,file=$(OVMF_ROOT)/FV/OVMF_CODE.fd \
	  -drive if=pflash,format=raw,file=$(RUNNER_DIR)/FV/OVMF_VARS.fd \
	  -hdb fat:rw:$(EFI_PART_DIR) \
	  -m 2048 \
	  -enable-kvm \
	  -machine q35 \
	  -no-reboot \
	  -serial stdio \
	  -display gtk,zoom-to-fit=on $(QEMU_FLAGS)

$(BUILD_DIR)/breakout.efi: $(BUILD_DIR)/breakout.so
	mkdir -p $(BUILD_DIR)
	objcopy $(OBJ_SEGMENTS) --output-target efi-app-x86_64 --subsystem=10 $< $@

$(BUILD_DIR)/breakout.debug: $(BUILD_DIR)/breakout.so
	mkdir -p $(BUILD_DIR)
	objcopy --only-keep-debug $< $@

$(BUILD_DIR)/breakout.so: $(OBJS)
	mkdir -p $(BUILD_DIR)
	ld -shared -Bsymbolic $(LDFLAGS) gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o $@ \
		-lgnuefi -lefi

$(BUILD_DIR)/%.o: %.c gnu-efi
	mkdir -p $(BUILD_DIR)/$(<:$(lastword $(subst /, ,$<))=)
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: gnu-efi
gnu-efi:
	$(MAKE) -C gnu-efi

# Setup a basic EFI system partition with the UEFI application
$(EFI_PART_DIR)/BREAKOUT/breakout.efi: clean-runner $(BUILD_DIR)/breakout.efi
	mkdir -p $(EFI_PART_DIR)/EFI/BOOT
	cp $(BUILD_DIR)/breakout.efi $(EFI_PART_DIR)/EFI/BOOT/BOOTX64.EFI

	[ -n "$(UEFI_SHELL)" ] && cp $(UEFI_SHELL) $(EFI_PART_DIR)

.PHONY: $(BUILD_DIR)/compile_commands.json
$(BUILD_DIR)/compile_commands.json:
	mkdir -p $(BUILD_DIR)
	compiledb -d $(BUILD_DIR) -o $@ -n $(MAKE) $(BUILD_DIR)/breakout.efi -C .
