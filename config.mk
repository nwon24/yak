HOSTED_TARGET_TRIPLET := i686-hobbix
TARGET_TRIPLET := i686-elf
TARGET_ARCH := i386

TOOLCHAIN ?= GNU

ifeq ($(TOOLCHAIN), GNU)
CC := $(HOSTED_TARGET_TRIPLET)-gcc
LD := $(CC)
AR := $(HOSTED_TARGET_TRIPLET)-ar
else ifeq ($(TOOLCHAIN), LLVM)
CC := clang -target $(TARGET_TRIPLET)
LD := ld.lld
AR := llvm-ar
else
$(error No toolchain specified. Use 'make TOOLCHAIN={GNU, LLVM}')
endif
CPP := $(CC) -E

CFLAGS :=  -std=c99 -ffreestanding -O2 -Wall -Wextra -Winit-self -Wundef -pedantic -Wshadow -Wstrict-prototypes -MMD -MP -DKERNEL
ifeq ($(CC), clang -target $(TARGET_TRIPLET))
CFLAGS += -mno-sse
endif
CPPFLAGS := -ffreestanding -P
# Still C compiler flags as we assemble .S files using GCC
ASFLAGS := -ffreestanding -MMD -MP
LDFLAGS := -nostdlib -O2 -ffreestanding
ifeq ($(CC), $(HOSTED_TARGET_TRIPLET)-gcc)
LDFLAGS += -lgcc -Xlinker -Map=kernel.map
else
LDFLAGS += -Map kernel.map
endif

ARCH_DIR := arch/$(TARGET_ARCH)

# Makefile build options
V = 0
ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif
export E Q
