OBJ_FILES += $(ARCH_DIR)/boot.o \
	     $(ARCH_DIR)/descriptor_table.o \
	     $(ARCH_DIR)/8259_pic.o \
	     $(ARCH_DIR)/idt.o \
	     $(ARCH_DIR)/interrupts.o \
	     $(ARCH_DIR)/port_io.o \
             $(ARCH_DIR)/driver_irq.o \
             $(ARCH_DIR)/exceptions.o \
	     $(ARCH_DIR)/virtual_mm.o

KERNEL_BIN := $(ARCH_DIR)/kimage
LDSCRIPT := $(ARCH_DIR)/kernel.ld
LDSCRIPT_SRC := $(ARCH_DIR)/kernel.ld.S

ARCH_INC_DIR := $(ARCH_DIR)/include

CRTI_OBJ := $(ARCH_DIR)/crti.o
CRTN_OBJ := $(ARCH_DIR)/crtn.o
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

DEP_FILES += $(ARCH_DIR)/crti.d
DEP_FILES += $(ARCH_DIR)/crtn.d
