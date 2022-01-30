.PHONY: kernel libc

include make.config

SRC_DIR := $(shell pwd)
KERNEL_DIR := sys
LIBC_DIR := libc
CMDS_DIR := cmd
TESTING_DIR := testing
SYSROOT_DIR := $(SRC_DIR)/$(TESTING_DIR)/sysroot
export SYSROOT_DIR

CMDS := $(CMDS_DIR)/hello
export CMDS CMDS_DIR

all: kernel libc

$(CMDS):
	(cd $(CMDS_DIR); make)
libc: install-kernel-headers
	(cd $(LIBC_DIR); make)
kernel:
	(cd $(KERNEL_DIR); make)
install-kernel-headers:
	(cd $(KERNEL_DIR); make install-kernel-headers)
install-libc-headers:
	(cd $(LIBC_DIR); make install-headers)
install-libc:
	(cd $(LIBC_DIR); make install-libc)
install-headers: install-kernel-headers install-libc-headers
kernel-clean:
	(cd $(KERNEL_DIR); make clean)
libc-clean:
	(cd $(LIBC_DIR); make clean)
clean: kernel-clean libc-clean

include $(TESTING_DIR)/Makefile
