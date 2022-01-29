CMDS := $(CMDS_DIR)/hello
CMDS_CFLAGS := -std=c99 -Wall -Wextra -O2

$(CMDS_DIR)/%: $(CMDS_DIR)/%.c
	%(CC) $< -o $@ $(CMDS_CFLAGS)
