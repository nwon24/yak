SYSROOT_DIR := $(TESTING_DIR)/sysroot

GRUBCFG := $(TESTING_DIR)/grub.cfg
DISK := $(TESTING_DIR)/disk.img

$(SYSROOT_DIR): $(GRUBCFG) $(KERNEL_BIN)
	mkdir -p $@/boot/grub
	cp $(GRUBCFG) $@/boot/grub/
	cp $(KERNEL_BIN) $@/boot/
$(GRUBCFG):
	$(TESTING_DIR)/gen_grubcfg.sh $@ $(notdir $(KERNEL_BIN))
$(DISK):
	$(TESTING_DIR)/grub_disk.sh $@ 40960
run: $(SYSROOT_DIR) $(DISK)
	$(TESTING_DIR)/disk_run.sh  $(DISK) $(SYSROOT_DIR)
