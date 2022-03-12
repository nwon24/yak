.PHONY: kernel libc cmds

include make.config

SRC_DIR := $(shell pwd)
KERNEL_DIR := sys
LIBC_DIR := libc
CMDS_DIR := cmd
TESTING_DIR := testing
SYSROOT_DIR := $(SRC_DIR)/$(TESTING_DIR)/sysroot
export SYSROOT_DIR

all: $(SYSROOT_DIR) install-headers kernel install-libc install-cmds

cmds:
	(cd $(CMDS_DIR); make)
libc: install-kernel-headers
	(cd $(LIBC_DIR); make)
kernel:
	(cd $(KERNEL_DIR); make)
install-headers: install-kernel-headers install-libc-headers
install-kernel-headers:
	(cd $(KERNEL_DIR); make install-kernel-headers)
install-libc-headers:
	(cd $(LIBC_DIR); make install-headers)
install-libc: libc
	(cd $(LIBC_DIR); make install-libc)
install-cmds: cmds
	(cd $(CMDS_DIR); make install-cmd)
install-headers: install-kernel-headers install-libc-headers
kernel-clean:
	(cd $(KERNEL_DIR); make clean)
libc-clean:
	(cd $(LIBC_DIR); make clean)
cmds-clean:
	(cd $(CMDS_DIR); make clean)
clean: kernel-clean libc-clean cmds-clean

include $(TESTING_DIR)/Makefile
