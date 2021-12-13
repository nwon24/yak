TARGET_TRIPLET := i686-elf
TARGET_ARCH := i386

CC := $(TARGET_TRIPLET)-gcc
AR := $(TARGET_TRIPLET)-ar
CPP := $(CC) -E

CFLAGS :=  -std=c99 -ffreestanding -O2 -Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes -MMD -MP -D_KERNEL_
CPPFLAGS := -ffreestanding -P
# Still C compiler flags as we assemble .S files using GCC
ASFLAGS := -ffreestanding -D_ASSEMBLY_ -MMD -MP
LDFLAGS := -nostdlib -O2 -lgcc

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
