# Makefile

include make.config

KERNEL_TAR := kernel_$$(date +%y%m%d%H%M%S).tar

OBJ_FILES :=
DEP_FILES :=

ARCH_DIR := arch/$(TARGET_ARCH)
LIB_DIR := lib
INC_DIR := include
KERNEL_DIR := kernel
MM_DIR := mm
FS_DIR := fs
DRIVERS_DIR := drivers

KERNEL_BIN := kimage

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

install-kernel-headers: $(SYSROOT_DIR)
	$(Q)mkdir -p $(SYSROOT_DIR)/usr/include/hobbix
	$(Q)cp -r $(INC_DIR)/* $(SYSROOT_DIR)/usr/include/hobbix/ 
	$(Q)cp -r $(ARCH_INC_DIR)/* $(SYSROOT_DIR)/usr/include/hobbix/ 
	$(E) " INSTALLED KERNEL HEADERS "
clean:
	$(Q)rm -rf $(CRTI_OBJ) $(CRTN_OBJ) $(OBJ_FILES) $(KERNEL_BIN) $(DEP_FILES) $(LDSCRIPT) $(LIBK) $(LIB_OBJ) $(SYSROOT_DIR) $(GRUBCFG)
	$(E) " CLEAN"

distclean:
	$(Q)make clean
	$(Q)sudo rm -rf $(SYSROOT_DIR) $(DISK) qemu.log kernel_*.tar
	$(E) " DISTCLEAN"
dist: distclean
	$(Q)./dist.sh $(KERNEL_TAR)
	$(E) " ./dist.sh	$@"
