.PHONY: kernel

include make.config

KERNEL_DIR := sys
CMDS_DIR := cmd
TESTING_DIR := testing

CMDS := $(CMDS_DIR)/hello
export CMDS CMDS_DIR

all: kernel

$(CMDS):
	(cd $(CMDS_DIR); make)
kernel:
	(cd $(KERNEL_DIR); make)
clean:
	(cd $(KERNEL_DIR); make clean)

include $(TESTING_DIR)/Makefile
