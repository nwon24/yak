KERNEL_BIN := $(KERNEL_DIR)/kimage

GRUBCFG := $(TESTING_DIR)/grub.cfg
DISK := $(TESTING_DIR)/disk.img

$(SYSROOT_DIR):
	$(Q)$(TESTING_DIR)/build_sysroot.sh $@

$(SYSROOT_DIR)-install: $(GRUBCFG) $(KERNEL_BIN) $(SYSROOT_DIR)
	$(Q)cp $(GRUBCFG) $(SYSROOT_DIR)/boot/grub/
	$(Q)cp $(KERNEL_BIN) $(SYSROOT_DIR)/boot/
	$(E) " GEN	" $(SYSROOT_DIR)
$(GRUBCFG): $(KERNEL_BIN)
	$(Q)$(TESTING_DIR)/gen_grubcfg.sh $@ $(notdir $(KERNEL_BIN))
	$(E) " GEN	" $@
$(DISK):
	$(Q)$(TESTING_DIR)/grub_disk.sh $@ 40960
	$(E) " GEN	" $@
run: $(SYSROOT_DIR)-install $(DISK)
	$(E) " RUN	" $(DISK)
	$(Q)$(TESTING_DIR)/disk_run.sh  $(DISK) $(SYSROOT_DIR)
