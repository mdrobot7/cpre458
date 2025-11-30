TARGET_ELF := cpre458.elf # Application only
BUILD_DIR := ./build
SRC_DIR := ./src

# CMSIS flags
CPU := __ATSAMD21E15L__
CMSIS_PATH := ./lib/CMSIS/samd21c/include
CMSIS_CORE_PATH := ./lib/CMSIS/Core/Include

# Picolibc flags
PICOLIBC_SPECS_PATH := ./lib/picolibc/lib/gcc/arm-none-eabi/12.2.1/picolibc.specs
PICOLIBC_PREFIX := ./lib/picolibc/arm-none-eabi

LD_SCRIPT := ./scripts/samd21e15l_flash.ld

CC := arm-none-eabi-gcc
COMMON_FLAGS := -specs=$(PICOLIBC_SPECS_PATH) --picolibc-prefix=$(PICOLIBC_PREFIX) \
	-O3 -g -flto -march=armv6-m -mtune=cortex-m0plus -mthumb -mfloat-abi=soft \
	-D$(CPU) -nostartfiles -ffreestanding -fstack-usage
CFLAGS := -Wall -Wextra -Wno-address-of-packed-member -Wno-discarded-qualifiers \
	-fdata-sections -ffunction-sections
CPPFLAGS := -MMD -MP -I$(CMSIS_PATH) -I$(CMSIS_CORE_PATH)
LDFLAGS := --gc-sections

SOURCES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) # Shell "find" sucks on Windows, so we're doing this
OBJS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d) # Generate sub-makefiles for each C source

# Turn LDFLAGS into -Wl,[flag],[flag]... to pass to GCC
space := $() $()
comma := ,
LDFLAGS := -Wl,$(subst $(space),$(comma),$(LDFLAGS))

all: $(BUILD_DIR)/$(TARGET_ELF) stack-analyze compiledb

# Link C sources into final executable
$(BUILD_DIR)/$(TARGET_ELF): $(OBJS)
	$(CC) $(COMMON_FLAGS) $(LDFLAGS) -T$(LD_SCRIPT) $(OBJS) -o $@

# Build startup script (needs -fno-lto)
$(filter $(BUILD_DIR)/$(SRC_DIR)/startup_samd21.c.o,$(OBJS)): $(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(CPPFLAGS) $(CFLAGS) -fno-lto -c $< -o $@

# Build C sources
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build ASM sources
$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(CPPFLAGS) $(DFU_CPPFLAGS) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

# Run stack analyzer
stack-analyze: $(BUILD_DIR)/$(TARGET_ELF)
	python ./scripts/stack_analyze.py $(BUILD_DIR)/$(TARGET_ELF) $(BUILD_DIR)/$(subst .elf,.stack,$(TARGET_ELF))

# Generate ./build/compile_commands.json using compiledb
compiledb:
	mkdir -p $(BUILD_DIR)
	compiledb -n -o $(BUILD_DIR)/compile_commands.json make

# Find the installed version of GDB (arm-none-eabi or multiarch)
find-gdb:
ifneq ($(shell gdb-multiarch --version),)
	@echo "Found gdb-multiarch"
	$(eval GDB := gdb-multiarch)
else ifneq ($(shell arm-none-eabi-gdb --version),)
	@echo "Found arm-none-eabi-gdb"
	$(eval GDB := arm-none-eabi-gdb)
else
	@echo
	@echo "ERROR: Please install GDB!"
	@exit 1
endif

# Find an attached BMP
find-debugger:
ifeq ($(shell python ./scripts/detect_bmp.py),None)
	@echo "ERROR: No debugger found. Is it plugged in?"
	@exit 1
endif
	$(eval DEBUGGER := $(shell python ./scripts/detect_bmp.py))
	@echo Found debugger at $(DEBUGGER)

# Build and flash the firmware over SWD
flash: $(BUILD_DIR)/$(TARGET_ELF) $(BUILD_DIR)/$(TARGET_DFU_ELF) stack-analyze find-gdb find-debugger
	$(GDB) -ex "tar ext $(DEBUGGER)" -ex "mon s" -ex "att 1" -ex "load" -ex "compare-sections" -ex "starti" --batch $(BUILD_DIR)/$(TARGET_ELF)

wipe: find-gdb find-debugger
	$(GDB) -ex "tar ext $(DEBUGGER)" -ex "mon s" -ex "att 1" -ex "mon erase_mass" -ex "mon reset" -ex "quit" --batch-silent

# Configure launch.json for VSCode debugging
configure-debug: find-gdb find-debugger
	$(eval SVD_NAME := $(subst __,,$(CPU)).svd)
	python ./scripts/configure_debug.py $(BUILD_DIR)/$(TARGET_ELF) $(DEBUGGER) $(CMSIS_PATH)/../svd/$(SVD_NAME) $(GDB)

setup:
	mkdir -p ./.vscode
	cp ./scripts/vscode/* ./.vscode

clean:
	rm -r $(BUILD_DIR)

.PHONY: all stack-analyze compiledb find-gdb find-debugger flash flash-dfu configure-debug clean
-include $(DEPS)