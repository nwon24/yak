# Makefile

include config.mk

KERNEL_TAR := kernel_$$(date +%y%m%d%H%M%S).tar

OBJ_FILES :=
DEP_FILES :=

LIB_DIR := lib
INC_DIR := include
KERNEL_DIR := kernel
MM_DIR := mm
FS_DIR := fs
DRIVERS_DIR := drivers
TESTING_DIR := testing

include $(ARCH_DIR)/Makefile
include $(KERNEL_DIR)/Makefile
include $(DRIVERS_DIR)/Makefile
include $(LIB_DIR)/Makefile
include $(MM_DIR)/Makefile
include $(FS_DIR)/Makefile

DEP_FILES += ${OBJ_FILES:.o=.d} ${LIB_OBJ:.o=.d}

OBJ_LINK_LIST := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ_FILES) $(CRTEND_OBJ) $(CRTN_OBJ)

all: $(KERNEL_BIN)
-include $(DEP_FILES)
$(KERNEL_BIN): $(OBJ_LINK_LIST) $(LDSCRIPT) $(LIBK) $(LIBFONT)
	$(Q)$(LD) $(OBJ_LINK_LIST) -o $@ -T$(LDSCRIPT) $(LDFLAGS) -L$(LIB_DIR) -L$(LIBFONT_DIR) -lk -lfont
	$(E) " LD	" $@
$(LIBK): $(LIB_OBJ)
	$(Q)$(AR) rcs $@ $^
	$(E) " AR	" $@
$(LIBFONT): $(LIBFONT_OBJ)
	$(Q)$(AR) rcs $@ $^
	$(E) " AR	" $@
.c.o:
	$(Q)$(CC) -c $< -o $@ $(CFLAGS) -I$(INC_DIR) -I$(ARCH_INC_DIR)
	$(E) " CC	" $@

.S.o:
	$(Q)$(CC) -c $< -o $@ -I$(INC_DIR) -I$(ARCH_INC_DIR) $(ASFLAGS)
	$(E) " AS	" $@
$(LDSCRIPT): $(LDSCRIPT_SRC)
	$(Q)$(CPP) $< -o $@ -I$(INC_DIR) -I$(ARCH_INC_DIR) $(CPPFLAGS)
	$(E) " CPP	" $@
clean:
	$(Q)rm -rf $(CRTI_OBJ) $(CRTN_OBJ) $(OBJ_FILES) $(KERNEL_BIN) $(DEP_FILES) $(LDSCRIPT) $(LIBK) $(LIB_OBJ) $(SYSROOT) $(GRUBCFG)
	$(E) " CLEAN"

include $(TESTING_DIR)/Makefile

distclean:
	$(Q)make clean
	$(Q)rm -rf $(SYSROOT_DIR) $(DISK) qemu.log kernel_*.tar
	$(E) " DISTCLEAN"
dist: distclean
	$(Q)./dist.sh $(KERNEL_TAR)
	$(E) " ./dist.sh	$@"
