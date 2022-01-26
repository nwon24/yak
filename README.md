Yak
A hobby Unix-like kernel project.

# Goals
- [x] Basic ext2 implmentation (e.g., `open`, `read`, `write`, `close`, `link`, `unlink`)
- [x] Framebuffer console driver
- [ ] Basic process management syscalls (missing `exec`)
- [ ] Basic PS/2 keyboard driver (USB is very complicated)
- [ ] OS Specific Toolchain
- [ ] ext2 symlinks, mount/umount
- [ ] Port to x86_64
# Prerequisites
## Operating system
Any modern Linux operating system will do fine. The BSDs should also work.
## Toolchain
You can compile this with either the GNU toolchain or the LLVM toolchain.
See the [GCC](https://gnu.org/software/gcc) and [binutils](https://gnu.org/software/binutils) pages for more information on the GNU toolchain.
See the [LLVM](https://llvm.org) page for information on LLVM.
**WARNING: Currently, compiling with `clang` and linking with `lld` seems to break the kernel.
With the exact same code, a kernel compiled with `GCC` will run fine, while a kernel compiled with `clang` will hang
early with a page fault.
If you have the patience to hunt down why this is the case feel free.
Otherwise, it is recommended to build a `GCC` cross compiler and use that to build the kernel.
The rest of this section is for building a GCC cross compiler, as the LLVM toolchain is a cross compiler by default.**
See [here](https://wiki.osdev.org/GCC_Cross_Compiler) for how to build a cross compiler for the `i686-elf` target.
Since it is recommended to build a cross compiler that is the same version as the host compiler, you may want to upgrade
your system compiler to the latest version before building a cross compiler.
In that case see [here](https://wiki.osdev.org/Building_GCC) or [here](https://gcc.gnu.org/install) for more information.
## GRUB
The kernel uses the GRUB bootloader.
More specifically, it uses GRUB2, which you probably have. GRUB legacy is ancient.
If you are on Debian, you may need the `grub-pc-bin` package to get the necessary GRUB files for the `i386` platform.
If you are on Fedora, beware that the GRUB commands may be prefixed with `grub2-` rather than `grub-`.
If that is the case, remember to change the scripts.
## QEMU
QEMU is used for testing.
Make sure you install the QEMU package for your distribution that includes system emulation for `i386`.
## Other tools
Make sure you can get free loop devices with `sudo losetup -f`, as that is needed to build the disk image.
Make sure you have the `ext2` filesystem tools installed, such as `mke2fs` and `e2fsck`.
`sfdisk` is used to partition the disk used for testing. That comes from `util-linux`.
# Building
To build, run: `make`.
By default, it will use the GNU toolchain.
You can use the LLVM toolchain by 'make TOOLCHAIN=LLVM'.
# Running
To run in QEMU, run: `make run`.
This will build the disk image if necessary (located in `testing/disk.img`) and then run QEMU.
