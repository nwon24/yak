LIBC_CFLAGS := -std=c99 -Wall -Wextra -pedantic -O2 -D__LIBRARY__ -MP -MMD
LIBC_ASFLAGS := -MP -MMD

CRT0_OBJ := $(TARGET_ARCH)/crt0.o
CRTI_OBJ := $(TARGET_ARCH)/crti.o
CRTN_OBJ := $(TARGET_ARCH)/crtn.o
CRT_OBJ := $(CRT0_OBJ) $(CRTI_OBJ) $(CRTN_OBJ)

all: libc.a $(CRT_OBJ)

LIBC_OBJ :=

include $(TARGET_ARCH)/Makefile
include unistd/Makefile
include fcntl/Makefile
include errno/Makefile
include init/Makefile
include sys/Makefile
include string/Makefile

DEP_FILES := $(patsubst %.o, %.d, $(LIBC_OBJ)) $(patsubst %.o, %.d, $(CRT_OBJ))
-include $(DEP_FILES)

%.o: %.c
	$(CC) -c $< -o $@ $(LIBC_CFLAGS) -Iinclude -Iinclude/internal
%.o: %.S
	$(CC) -c $< -o $@ $(LIBC_ASFLAGS) -Iinclude -Iinclude/internal

libc.a: $(LIBC_OBJ) $(CRT_OBJ)
	$(AR) rcs $@ $(LIBC_OBJ)
clean:
	rm $(LIBC_OBJ) $(CRT_OBJ)

install-headers: $(SYSROOT_DIR)
	mkdir -p $(SYSROOT_DIR)/usr/include
	cp include/*.h $(SYSROOT_DIR)/usr/include/
	cp -r include/bits $(SYSROOT_DIR)/usr/include/
	cp -r include/sys/ $(SYSROOT_DIR)/usr/include/
install-libc: $(SYSROOT_DIR) libc.a
	mkdir -p $(SYSROOT_DIR)/usr/lib
	cp libc.a $(SYSROOT_DIR)/usr/lib
	cp $(CRT0_OBJ) $(CRTI_OBJ) $(CRTN_OBJ) $(SYSROOT_DIR)/usr/lib
