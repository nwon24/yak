CMDS_CFLAGS := -std=c99 -Wall -Wextra -O2

TARGETS :=$(patsubst $(CMDS_DIR)/%, %, $(CMDS))

all: $(TARGETS)

%: %.c
	$(CC) $< -o  $@ $(CMDS_CFLAGS)
