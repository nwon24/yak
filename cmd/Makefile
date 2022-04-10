CMDS_CFLAGS := -std=c99 -Wall -Wextra -O2

CMDS := init \
        hello

all: $(CMDS)

$(CMDS): $(SYSROOT_DIR)/usr/lib/libc.a

%: %.c
	$(CC) $< -o  $@ $(CMDS_CFLAGS)
clean:
	rm $(CMDS)
install-cmd: $(CMDS)
	mkdir -p $(SYSROOT_DIR)/usr/bin
	cp $^ $(SYSROOT_DIR)/usr/bin/
