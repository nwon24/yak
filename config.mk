TARGET_TRIPLET := i686-elf
TARGET_ARCH := i386

CC := $(TARGET_TRIPLET)-gcc
LD := $(CC)
AR := $(TARGET_TRIPLET)-ar
CPP := $(CC) -E

CFLAGS :=  -std=c99 -ffreestanding -O2 -Wall -Wextra -Werror -Winit-self -Wundef -pedantic -Wshadow -Wstrict-prototypes -MMD -MP -DKERNEL
CPPFLAGS := -ffreestanding -P
# Still C compiler flags as we assemble .S files using GCC
ASFLAGS := -ffreestanding -MMD -MP
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
