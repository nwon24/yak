TARGET_TRIPLET := i686-elf
TARGET_ARCH := i386

CC := $(TARGET_TRIPLET)-gcc
CPP := $(CC) -E

CFLAGS := -std=c99 -ffreestanding -O2 -Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes -MMD -MP -D_KERNEL_
CPPFLAGS := -ffreestanding -P
# Still C compiler flags as we assemble .S files using GCC
ASFLAGS := -ffreestanding -D_ASSEMBLY_ -MMD -MP
LDFLAGS := -nostdlib -lgcc -O2

ARCH_DIR := arch/$(TARGET_ARCH)
