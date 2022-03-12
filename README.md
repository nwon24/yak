# Yak
A hobby Unix-like kernel project.

**HUGE WARNING: This is purely experimental. Not to be tested on real hardware. Will probably screw a lot of things up.**

# Goals
- [x] Basic ext2 implmentation (e.g., `open`, `read`, `write`, `close`, `link`, `unlink`, `rename`)
- [x] Framebuffer console driver
- [x] Higher half kernel
- [x] `exec` system call (with `ELF` files - no dynamic linking)
- [x] Basic PS/2 keyboard driver (USB is very complicated)
- [x] OS Specific Toolchain
- [ ] Write a basic C library
- [x] ext2 symlinks
- [ ] mount/umount system calls
- [ ] Abstract installation of IRQ handlers for easier porting
- [ ] Port to x86_64
# Prerequisites
## Operating system
Any modern Linux operating system will do fine. The BSDs should also work, but make sure GNU make is being used.
## Toolchain
The operating system now has its own toolchain, which is basically a hosted `i686-elf` toolchain.
The tarballs for GCC and Binutils are in the [toolchain](./toolchain) folder.
Follow instructions [here](https://wiki.osdev.org/OS_Specific_Toolchain) for how to build the toolchain.
Before building, remember to run `make headers-install`.

**IMPORTANT: Do not install the toolchain into your system directories.**
## GRUB
The kernel uses the GRUB bootloader.
More specifically, it uses GRUB2, which you probably have. GRUB legacy is ancient.
If you are on Debian, you may need the `grub-pc-bin` package to get the necessary GRUB files for the i386 platform.
If you are on Fedora, beware that the GRUB commands may be prefixed with `grub2-` rather than `grub-`.
If that is the case, remember to change the scripts.
## QEMU
QEMU is used for testing.
Make sure you install the QEMU package for your distribution that includes system emulation for i386.
## Other tools
Make sure you can get free loop devices with `sudo losetup -f`, as that is needed to build the disk image.
If you get weird errors when running `sudo losetup -f` and have just done a kernel update, reboot your machine.
Make sure you have the `ext2` filesystem tools installed, such as `mke2fs` and `e2fsck`.
`sfdisk` is used to partition the disk used for testing. That comes from `util-linux`.
# Building
To build, run: `make`.
By default, it will use the GNU toolchain.
You can use the LLVM toolchain by `make TOOLCHAIN=LLVM`.
# Running
To run in QEMU, run: `make run`.
This will build the disk image if necessary (located in `testing/disk.img`) and then run QEMU.
