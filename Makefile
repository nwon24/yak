# Makefile

include config.mk

OBJ_FILES :=
DEP_FILES :=

LIB_DIR := lib
INC_DIR := include
KERNEL_DIR := kernel
DRIVERS_DIR := drivers
TESTING_DIR := testing

include $(ARCH_DIR)/Makefile
include $(KERNEL_DIR)/Makefile
include $(DRIVERS_DIR)/Makefile
include $(LIB_DIR)/Makefile

DEP_FILES += ${OBJ_FILES:.o=.d} ${LIB_OBJ:.o=.d}

OBJ_LINK_LIST := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ_FILES) $(CRTEND_OBJ) $(CRTN_OBJ)

all: $(KERNEL_BIN)
-include $(DEP_FILES)
$(KERNEL_BIN): $(OBJ_LINK_LIST) $(LDSCRIPT) $(LIBK)
	$(CC) $(OBJ_LINK_LIST) -o $@ -T$(LDSCRIPT) $(LDFLAGS) -L$(LIB_DIR) -lk
$(LIBK): $(LIB_OBJ)
	$(AR) rcs $@ $^
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(INC_DIR) -I$(ARCH_INC_DIR)

%.o: %.S
	$(CC) -c $< -o $@ -I$(INC_DIR) -I$(ARCH_INC_DIR) $(ASFLAGS)
$(LDSCRIPT): $(LDSCRIPT_SRC)
	$(CPP) $< -o $@ -I$(INC_DIR) -I$(ARCH_INC_DIR) $(CPPFLAGS)
clean:
	rm -rf $(CRTI_OBJ) $(CRTN_OBJ) $(OBJ_FILES) $(KERNEL_BIN) $(DEP_FILES) $(LDSCRIPT) $(LIBK) $(LIB_OBJ) $(SYSROOT) $(GRUBCFG)

include $(TESTING_DIR)/Makefile

distclean:
	make clean
	rm -rf $(SYSROOT_DIR) $(DISK) qemu.log
